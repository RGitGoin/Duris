#include "economy/economic_coin_adapter.h"
#include "economy/economic_command_admission.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <type_traits>

using error = economic_accounting_error;
critical_operation_id id(uint8_t byte)
{
	critical_operation_id result{};
	result.bytes[0] = byte;
	return result;
}
struct fixture
{
	critical_command command;
	economic_frozen_intent intent;
	economic_coin_wallet_authority authority;
};
fixture make_fixture(bool shared, bool convert = false)
{
	fixture f;
	coin_transfer_payload payload;
	coin_transfer_endpoint *ends[] = { &payload.source, &payload.destination };
	for (size_t i = 0; i < 2; ++i)
	{
		auto &end = *ends[i];
		end.before = i ? std::array<int32_t, 4>{} : std::array<int32_t, 4>{ 100, 0, 0, 0 };
		end.after = i ? (convert ? std::array<int32_t, 4>{ 0, 1, 0, 0 } :
					   std::array<int32_t, 4>{ 10, 0, 0, 0 }) :
				std::array<int32_t, 4>{ 90, 0, 0, 0 };
		currency_command_payload currency{};
		currency.pid = 7 + i;
		currency.racewar = 1;
		currency.reason = currency_reason_type::coin_transfer;
		std::strcpy(currency.account_name.data(), i && !shared ? "other" : "fixture");
		for (size_t part = 0; part < 4; ++part)
			currency.wallet_delta.amount[part] =
				int64_t(end.after[part]) - end.before[part];
		assert(currency_command_build(&end.change, id(10 + i), currency, 4 + i, 9,
					      critical_source_site::command,
					      critical_deadline_class::interactive));
		auto &a = f.authority[i];
		a.epoch = id(2);
		a.wallet_account = { id(1), economic_account_kind::wallet, 100 + i, 0 };
		a.bank_account = { id(1), economic_account_kind::bank, 200 + (shared ? 0 : i), 1 };
		a.player_fence = end.change.keys[0];
		a.bank_fence = end.change.keys[1];
		for (size_t part = 0; part < 4; ++part)
			a.state.wallet.amount[part] = end.before[part];
		a.state.bank.amount = { 7, 0, 0, 0 };
		a.state.wallet_revision = 4 + i;
		a.state.bank_revision = 9;
	}
	assert(coin_transfer_command_build(&f.command, id(3), payload,
					   critical_source_site::command,
					   critical_deadline_class::interactive));
	assert(economic_coin_wallets_intent(
		       f.command, id(2),
		       { f.authority[0].wallet_account, f.authority[1].wallet_account },
		       { f.authority[0].bank_account, f.authority[1].bank_account },
		       &f.command.accounting_intent) == error::ok);
	f.command.schema_version = CRITICAL_COMMAND_ACCOUNTING_SCHEMA_VERSION;
	assert(economic_intent_decode(f.command.accounting_intent, &f.intent) == error::ok);
	return f;
}
int main()
{
	static_assert(!std::is_default_constructible_v<economic_prepared_coin_wallets>);
	static_assert(!std::is_aggregate_v<economic_prepared_coin_wallets>);
	for (bool shared : { false, true })
		for (bool convert : { false, true })
		{
			auto f = make_fixture(shared, convert);
			// This pure adapter cannot open runtime admission before the SQL and
			// flat-file domain owners implement their atomic evidence append.
			assert(!economic_command_admission_supported(f.command));
			std::optional<economic_prepared_coin_wallets> prepared, flat;
			assert(economic_coin_wallets_prepare(f.command, f.intent, f.authority,
							     currency_revision_policy::sql_legacy,
							     &prepared) == error::ok);
			assert(economic_coin_wallets_prepare(
				       f.command, f.intent, f.authority,
				       currency_revision_policy::flatfile_legacy,
				       &flat) == error::ok);
			assert(prepared->agrees_with(flat->plan()) == error::ok);
			const auto plan = prepared->plan();
			assert(plan.accounts.size() == (shared ? 3 : 4));
			assert(plan.children.size() == 2 && plan.postings.size() == 2);
			assert(plan.item_events.empty() && plan.items_before.empty() &&
			       plan.items_after.empty());
			assert(prepared->mutations()[0].after().wallet.amount[0] == 90);
			assert(prepared->mutations()[1].before().bank_revision ==
			       (shared ? 10 : 9));
			assert(prepared->mutations()[1].after().bank_revision ==
			       (shared ? 11 : 10));
			for (const auto &effect : plan.accounts)
				if (effect.key.kind == economic_account_kind::bank)
				{
					assert(effect.before == effect.after);
					assert(effect.before_revision == 9 &&
					       effect.after_revision == (shared ? 11 : 10));
				}
			for (const auto &child : plan.children)
			{
				critical_operation_id derived;
				assert(critical_operation_id_derive(f.command.operation_id,
								    child.domain,
								    child.discriminator, &derived));
				assert(derived.bytes == child.operation_id.bytes);
			}
			auto altered = plan;
			altered.postings.push_back({ 2, 0, 0, { 1, 0, 0, 0 }, 1 });
			altered.postings.push_back({ 3, 0, 0, { -1, 0, 0, 0 }, -1 });
			assert(economic_plan_validate_structure(altered) == error::ok);
			assert(prepared->agrees_with(altered) == error::payload_conflict);
			auto reject = [&](const fixture &bad)
			{
				assert(economic_coin_wallets_prepare(
					       bad.command, bad.intent, bad.authority,
					       currency_revision_policy::sql_legacy,
					       &prepared) != error::ok);
				assert(prepared->agrees_with(plan) == error::ok);
			};
			auto bad = f;
			++bad.authority[0].state.wallet.amount[0];
			reject(bad);
			bad = f;
			++bad.authority[1].state.wallet_revision;
			reject(bad);
			bad = f;
			++bad.authority[1].state.bank_revision;
			reject(bad);
			bad = f;
			bad.authority[1].epoch = id(8);
			reject(bad);
			bad = f;
			++bad.authority[0].player_fence.id;
			reject(bad);
			bad = f;
			++bad.authority[1].bank_fence.id;
			reject(bad);
			bad = f;
			++bad.authority[1].wallet_account.authority_id;
			reject(bad);
			bad = f;
			bad.authority[1].wallet_account = bad.authority[0].wallet_account;
			reject(bad);
			bad = f;
			bad.authority[1].bank_account.authority_id = shared ? 999 : 200;
			reject(bad);
			bad = f;
			bad.intent.admission.metadata.reason = economic_reason::npc_reward;
			reject(bad);
			bad = f;
			bad.intent.admission.metadata.writer_id = ECONOMIC_WRITER_BANK_DEPOSIT;
			reject(bad);
			bad = f;
			++bad.intent.admission.facts[0];
			reject(bad);
			if (shared)
			{
				bad = f;
				++bad.authority[1].state.bank.amount[0];
				reject(bad);
			}
		}
	for (int mode = 0; mode < 5; ++mode)
	{
		auto f = make_fixture(true);
		std::optional<economic_prepared_coin_wallets> prepared;
		auto expected = error::corrupt_evidence;
		if (mode == 0)
		{
			f.authority[0].state.wallet.amount[0] = 5;
			expected = error::negative_holding;
		}
		if (mode == 1)
		{
			f.authority[1].state.wallet.amount[0] = INT32_MAX;
			expected = error::overflow;
		}
		if (mode == 2)
		{
			f.authority[0].state.wallet.amount[0] = 101;
			expected = error::stale_revision;
		}
		if (mode == 3)
		{
			++f.authority[0].state.wallet_revision;
			f.authority[1].state.wallet.amount[0] = -1;
		}
		if (mode == 4)
			++f.authority[1].state.bank_revision;
		assert(economic_coin_wallets_prepare(f.command, f.intent, f.authority,
						     currency_revision_policy::sql_legacy,
						     &prepared) == expected);
		assert(!prepared);
	}
	std::cout
		<< "coin wallet adapter: typed effects, child legs, shared bank, conversion, rejection and backend arithmetic parity passed\n";
}
