#include "economy/economic_coin_adapter.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <new>
#include <utility>

namespace
{
using error = economic_accounting_error;
error facts_for(const critical_command &command, const critical_operation_id &epoch,
		const std::array<economic_account_key, 2> &wallets,
		const std::array<economic_account_key, 2> &banks, economic_admission_facts *facts)
{
	coin_transfer_payload payload;
	if (!coin_transfer_command_decode_payload(command, &payload))
		return error::corrupt_evidence;
	const coin_transfer_endpoint *ends[] = { &payload.source, &payload.destination };
	if (economic_account_key_equal(wallets[0], wallets[1]))
		return error::invalid_identity;
	for (size_t i = 0; i < 2; ++i)
	{
		currency_command_payload currency;
		if (ends[i]->change.type != critical_command_type::account_bank ||
		    !currency_command_decode_payload(ends[i]->change, &currency))
			return error::unauthorized;
		if (!economic_account_key_valid(wallets[i]) ||
		    !economic_account_key_valid(banks[i]) ||
		    wallets[i].kind != economic_account_kind::wallet || wallets[i].context_id ||
		    banks[i].kind != economic_account_kind::bank ||
		    banks[i].context_id != currency.racewar ||
		    wallets[i].lineage.bytes != wallets[0].lineage.bytes ||
		    banks[i].lineage.bytes != wallets[0].lineage.bytes)
			return error::invalid_identity;
		const uint64_t values[] = { wallets[i].authority_id, banks[i].authority_id,
					    banks[i].context_id };
		for (auto value : values)
			for (size_t byte = 0; byte < 8; ++byte)
				facts->facts.push_back(static_cast<uint8_t>(value >> (byte * 8)));
	}
	// A shared native bank must be exactly the same retained lifetime mapping,
	// and distinct native banks must never alias one accounting account.
	if (critical_entity_key_equal(payload.source.change.keys[1],
				      payload.destination.change.keys[1]) !=
	    economic_account_key_equal(banks[0], banks[1]))
		return error::invalid_identity;
	facts->metadata.lineage = wallets[0].lineage;
	facts->metadata.epoch = epoch;
	facts->metadata.actor_kind = economic_actor_kind::domain;
	facts->metadata.actor_id = wallets[0].authority_id;
	facts->metadata.writer_id = ECONOMIC_WRITER_COIN_WALLETS;
	facts->metadata.reason = economic_reason::wallet_transfer;
	return error::ok;
}
error domain_error(unsigned int value)
{
	switch (value)
	{
	case 0:
		return error::ok;
	case ESTALE:
		return error::stale_revision;
	case ERANGE:
		return error::overflow;
	case ENOSPC:
		return error::negative_holding;
	case EILSEQ:
		return error::corrupt_evidence;
	default:
		return error::invalid_identity;
	}
}
}

economic_prepared_coin_wallets::economic_prepared_coin_wallets(
	std::array<currency_prepared_mutation, 2> mutations, economic_accounting_plan plan,
	std::vector<uint8_t> encoded)
	: mutations_(std::move(mutations))
	, plan_(std::move(plan))
	, encoded_(std::move(encoded))
{
}

economic_accounting_error
economic_prepared_coin_wallets::agrees_with(const economic_accounting_plan &candidate) const
{
	std::vector<uint8_t> encoded;
	const auto result = economic_plan_encode(candidate, &encoded);
	if (result != error::ok)
		return result;
	return encoded == encoded_ ? error::ok : error::payload_conflict;
}

economic_accounting_error
economic_coin_wallets_intent(const critical_command &command, const critical_operation_id &epoch,
			     const std::array<economic_account_key, 2> &wallets,
			     const std::array<economic_account_key, 2> &banks,
			     std::vector<uint8_t> *encoded)
{
	try
	{
		economic_admission_facts facts;
		const auto result = facts_for(command, epoch, wallets, banks, &facts);
		if (result != error::ok)
			return result;
		return economic_intent_freeze(command, facts, encoded);
	}
	catch (const std::bad_alloc &)
	{
		return error::capacity;
	}
}

economic_accounting_error
economic_coin_wallets_prepare(const critical_command &command, const economic_frozen_intent &intent,
			      const economic_coin_wallet_authority &authority,
			      currency_revision_policy policy,
			      std::optional<economic_prepared_coin_wallets> *prepared)
{
	if (!prepared)
		return error::invalid_identity;
	try
	{
		auto result = economic_intent_verify_binding(command, intent);
		if (result != error::ok)
			return result;
		auto admission = command;
		admission.schema_version = CRITICAL_COMMAND_SCHEMA_VERSION;
		admission.accounting_intent.clear();
		std::vector<uint8_t> expected, supplied;
		result = economic_coin_wallets_intent(
			admission, authority[0].epoch,
			{ authority[0].wallet_account, authority[1].wallet_account },
			{ authority[0].bank_account, authority[1].bank_account }, &expected);
		if (result != error::ok)
			return result;
		result = economic_intent_encode(intent, &supplied);
		if (result != error::ok)
			return result;
		if (expected != supplied || authority[0].epoch.bytes != authority[1].epoch.bytes)
			return error::unauthorized;
		coin_transfer_payload payload;
		if (!coin_transfer_command_decode_payload(command, &payload))
			return error::corrupt_evidence;
		const coin_transfer_endpoint *ends[] = { &payload.source, &payload.destination };
		// Corrupt authority is never a durable stale/insufficient-funds decision,
		// including when the other endpoint would reject before it is visited.
		for (size_t i = 0; i < 2; ++i)
		{
			const auto &child = ends[i]->change;
			if (!critical_entity_key_equal(authority[i].player_fence, child.keys[0]) ||
			    !critical_entity_key_equal(authority[i].bank_fence, child.keys[1]))
				return error::invalid_identity;
			for (const auto &amounts :
			     { authority[i].state.wallet.amount, authority[i].state.bank.amount })
				for (auto amount : amounts)
					if (amount < 0 || amount > INT_MAX)
						return error::corrupt_evidence;
		}
		const bool shared_bank = economic_account_key_equal(authority[0].bank_account,
								    authority[1].bank_account);
		if (shared_bank &&
		    (authority[0].state.bank.amount != authority[1].state.bank.amount ||
		     authority[0].state.bank_revision != authority[1].state.bank_revision))
			return error::corrupt_evidence;
		std::array<std::optional<currency_prepared_mutation>, 2> mutations;
		coin_transfer_result after;
		economic_accounting_plan plan;
		result = economic_intent_plan_metadata(command, intent, &plan.metadata);
		if (result != error::ok)
			return result;
		for (size_t i = 0; i < 2; ++i)
		{
			auto child = ends[i]->change;
			auto state = authority[i].state;
			if (i)
			{
				if (!coin_transfer_command_destination_after_source(payload, after,
										    &child))
					return error::stale_revision;
				if (shared_bank)
				{
					state.bank = after.wallets[0].bank;
					state.bank_revision = after.wallets[0].bank_revision;
				}
			}
			currency_command_payload currency;
			if (!currency_command_decode_payload(child, &currency))
				return error::corrupt_evidence;
			result = domain_error(currency_prepare_mutation(
				currency, state, child.expected_revisions[0].revision,
				child.expected_revisions[1].revision, policy, &mutations[i]));
			if (result != error::ok)
				return result;
			after.wallets[i] = mutations[i]->after();
			if (!std::equal(ends[i]->after.begin(), ends[i]->after.end(),
					after.wallets[i].wallet.amount.begin()))
				return error::stale_revision;
			const auto &next = after.wallets[i];
			const auto wallet_index = static_cast<uint16_t>(plan.accounts.size());
			plan.accounts.push_back({ authority[i].wallet_account, state.wallet.amount,
						  next.wallet.amount, state.wallet_revision,
						  next.wallet_revision });
			if (i && shared_bank)
			{
				plan.accounts[1].after = next.bank.amount;
				plan.accounts[1].after_revision = next.bank_revision;
			}
			else
				plan.accounts.push_back({ authority[i].bank_account,
							  state.bank.amount, next.bank.amount,
							  state.bank_revision,
							  next.bank_revision });
			int64_t copper = 0;
			result = economic_coin_value(currency.wallet_delta.amount, &copper);
			if (result != error::ok)
				return result;
			plan.children.push_back(
				{ child.operation_id, COIN_OPERATION_DOMAIN, i, 0, 1 });
			plan.postings.push_back({ static_cast<uint32_t>(i), wallet_index,
						  static_cast<uint16_t>(i + 1),
						  currency.wallet_delta.amount, copper });
		}
		result = economic_plan_normalize(&plan);
		if (result != error::ok)
			return result;
		std::vector<uint8_t> encoded;
		result = economic_plan_encode(plan, &encoded);
		if (result != error::ok)
			return result;
		*prepared = economic_prepared_coin_wallets({ *mutations[0], *mutations[1] },
							   std::move(plan), std::move(encoded));
		return error::ok;
	}
	catch (const std::bad_alloc &)
	{
		return error::capacity;
	}
}
