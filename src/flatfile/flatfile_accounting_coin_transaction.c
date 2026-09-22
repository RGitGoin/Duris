#include "flatfile/flatfile_accounting_coin_transaction.h"
#include "flatfile/flatfile_accounting_authority.h"
#include "flatfile/flatfile_identity_repository.h"
#include "flatfile/flatfile_player_domain_repository.h"
#include "flatfile/flatfile_item_repository.h"
#include "flatfile/flatfile_store.h"
#include "economy/economic_coin_adapter.h"
#include <algorithm>
#include <cerrno>
#include <climits>
#include <new>
namespace
{
struct failure
{
	unsigned int code;
};
void need(bool value, unsigned int code = EILSEQ)
{
	if (!value)
		throw failure{ code };
}
void checked(unsigned int code)
{
	if (code)
		throw failure{ code };
}
void checked(economic_accounting_error value)
{
	need(value == economic_accounting_error::ok,
	     value == economic_accounting_error::capacity ? ENOMEM : EILSEQ);
}
void checked(flatfile_accounting_status value, bool read_only = false)
{
	if (value == flatfile_accounting_status::ok)
		return;
	if (value == flatfile_accounting_status::conflict ||
	    value == flatfile_accounting_status::already_exists)
		throw failure{ EEXIST };
	if (value == flatfile_accounting_status::capacity)
		throw failure{ static_cast<unsigned int>(read_only ? ENOMEM : ENOSPC) };
	if (value == flatfile_accounting_status::io_error)
		throw failure{ EIO };
	throw failure{ EILSEQ };
}
void checked(flatfile_player_domain_result value)
{
	if (value == flatfile_player_domain_result::ok)
		return;
	if (value == flatfile_player_domain_result::not_found)
		throw failure{ ENOENT };
	if (value == flatfile_player_domain_result::conflict)
		throw failure{ EACCES };
	if (value == flatfile_player_domain_result::io_error)
		throw failure{ EIO };
	throw failure{ EILSEQ };
}
std::string canonical(const std::string &name)
{
	need(!name.empty() && name.size() <= CURRENCY_ACCOUNT_NAME_MAX_BYTES, EINVAL);
	std::string result;
	for (auto c : name)
	{
		if (c >= 'A' && c <= 'Z')
			c = static_cast<char>(c + ('a' - 'A'));
		need((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-',
		     EINVAL);
		result += c;
	}
	return result;
}
uint64_t number(std::span<const uint8_t> input, size_t offset)
{
	uint64_t value = 0;
	for (size_t byte = 0; byte < 8; ++byte)
		value |= uint64_t(input[offset + byte]) << (8 * byte);
	return value;
}
bool equal(const currency_command_result &a, const currency_command_result &b)
{
	return a.wallet.amount == b.wallet.amount && a.bank.amount == b.bank.amount &&
	       a.wallet_revision == b.wallet_revision && a.bank_revision == b.bank_revision;
}
currency_command_result balances(const flatfile_player_domain_record &record)
{
	currency_command_result value;
	for (size_t part = 0; part < 4; ++part)
	{
		need(record.domains.wallet[part] <= INT_MAX &&
		     record.domains.bank[part] <= INT_MAX);
		value.wallet.amount[part] = record.domains.wallet[part];
		value.bank.amount[part] = record.domains.bank[part];
	}
	value.wallet_revision = record.domains.wallet_revision;
	value.bank_revision = record.domains.bank_revision;
	return value;
}

struct identity
{
	economic_frozen_intent intent;
	coin_transfer_payload payload;
	std::array<currency_command_payload, 2> currency;
	std::array<economic_account_key, 2> wallets, banks;
};
identity decode(const critical_command &command)
{
	identity value;
	need(coin_transfer_command_decode_payload(command, &value.payload), EINVAL);
	checked(economic_intent_decode(command.accounting_intent, &value.intent));
	const auto &facts = value.intent.admission.facts;
	need(facts.size() == ECONOMIC_COIN_WALLET_FACT_BYTES, EINVAL);
	const auto &meta = value.intent.admission.metadata;
	const coin_transfer_endpoint *ends[] = { &value.payload.source,
						 &value.payload.destination };
	for (size_t i = 0; i < 2; ++i)
	{
		need(currency_command_decode_payload(ends[i]->change, &value.currency[i]), EINVAL);
		need(value.currency[i].pid <= INT32_MAX && value.currency[i].racewar <= INT8_MAX,
		     EINVAL);
		value.wallets[i] = { meta.lineage, economic_account_kind::wallet,
				     number(facts, 24 * i), 0 };
		value.banks[i] = { meta.lineage, economic_account_kind::bank,
				   number(facts, 24 * i + 8), number(facts, 24 * i + 16) };
	}
	auto admission = command;
	admission.schema_version = CRITICAL_COMMAND_SCHEMA_VERSION;
	admission.accounting_intent.clear();
	std::vector<uint8_t> expected;
	checked(economic_coin_wallets_intent(admission, meta.epoch, value.wallets, value.banks,
					     &expected));
	need(expected == command.accounting_intent, EACCES);
	return value;
}
unsigned int decision_code(economic_accounting_error error)
{
	switch (error)
	{
	case economic_accounting_error::stale_revision:
		return ESTALE;
	case economic_accounting_error::negative_holding:
		return ENOSPC;
	case economic_accounting_error::overflow:
		return ERANGE;
	default:
		return 0;
	}
}
void retained_identity(const std::string &root, const flatfile_authority_lock &lock,
		       const critical_command &command, const identity &value)
{
	flatfile_economic_control control;
	checked(flatfile_economic_control_read(root, lock, &control, nullptr));
	const size_t bucket = command.operation_id.bytes[0];
	need(control.lineage.bytes == value.wallets[0].lineage.bytes &&
	     (control.evidence_initialized[bucket / 8] & (1U << (bucket % 8))));
	flatfile_economic_epoch epoch;
	checked(flatfile_economic_epoch_read(root, lock, value.wallets[0].lineage,
					     value.intent.admission.metadata.epoch, &epoch,
					     nullptr));
	for (size_t i = 0; i < 2; ++i)
	{
		flatfile_economic_mapping wallet, bank;
		checked(flatfile_economic_mapping_read(root, lock, value.wallets[i], &wallet,
						       nullptr));
		checked(flatfile_economic_mapping_read(root, lock, value.banks[i], &bank, nullptr));
		need(wallet.locator.kind == 1 &&
		     wallet.locator.native_id == value.currency[i].pid && bank.locator.kind == 2 &&
		     bank.locator.native_id == value.banks[i].authority_id);
	}
}
economic_coin_wallet_authority authority_for(const identity &value,
					     const coin_transfer_result &before)
{
	economic_coin_wallet_authority authority;
	const coin_transfer_endpoint *ends[] = { &value.payload.source,
						 &value.payload.destination };
	for (size_t i = 0; i < 2; ++i)
		authority[i] = { value.intent.admission.metadata.epoch,
				 value.wallets[i],
				 value.banks[i],
				 ends[i]->change.keys[0],
				 ends[i]->change.keys[1],
				 before.wallets[i] };
	return authority;
}
void verify(const std::string &root, const flatfile_authority_lock &lock,
	    const flatfile_accounting_record &record)
{
	const auto value = decode(record.command);
	retained_identity(root, lock, record.command, value);
	coin_transfer_result result;
	need(coin_transfer_command_decode_result(value.payload, record.result.data(),
						 record.result.size(), &result));
	std::array<uint8_t, COIN_TRANSFER_RESULT_BYTES> encoded;
	need(coin_transfer_command_encode_result(value.payload, result, &encoded));
	need(record.result.size() == encoded.size() &&
	     std::equal(encoded.begin(), encoded.end(), record.result.begin()));
	auto before = result;
	economic_accounting_plan plan;
	if (!record.result_code)
	{
		checked(economic_plan_decode(record.plan, &plan));
		for (size_t i = 0; i < 2; ++i)
		{
			bool wallet = false, bank = false;
			for (const auto &effect : plan.accounts)
			{
				if (economic_account_key_equal(effect.key, value.wallets[i]))
				{
					need(!wallet);
					wallet = true;
					before.wallets[i].wallet.amount = effect.before;
					before.wallets[i].wallet_revision = effect.before_revision;
				}
				if (economic_account_key_equal(effect.key, value.banks[i]))
				{
					need(!bank);
					bank = true;
					before.wallets[i].bank.amount = effect.before;
					before.wallets[i].bank_revision = effect.before_revision;
				}
			}
			need(wallet && bank);
		}
	}
	std::optional<economic_prepared_coin_wallets> prepared;
	critical_failure_stage stage;
	const auto decision = economic_coin_wallets_prepare(
		record.command, value.intent, authority_for(value, before),
		currency_revision_policy::flatfile_legacy, &prepared, &stage);
	need(decision != economic_accounting_error::capacity, ENOMEM);
	need(stage == record.failure_stage);
	if (record.result_code)
		need(record.plan.empty() && !record.durable_revision && !prepared &&
		     decision_code(decision) == record.result_code);
	else
	{
		checked(decision);
		checked(prepared->agrees_with(plan));
		uint64_t revision = 0;
		for (size_t i = 0; i < 2; ++i)
		{
			need(equal(prepared->mutations()[i].after(), result.wallets[i]));
			revision = std::max({ revision, result.wallets[i].wallet_revision,
					      result.wallets[i].bank_revision });
		}
		need(record.durable_revision == revision);
	}
}
critical_apply_result completion(const flatfile_accounting_record &record, bool replay)
{
	need(record.result.size() <= CRITICAL_COMPLETION_RESULT_MAX_BYTES);
	critical_apply_result result{ record.result_code ?
					      critical_apply_outcome::terminal_failure :
				      replay ? critical_apply_outcome::already_applied :
					       critical_apply_outcome::applied,
				      record.durable_revision, record.result_code };
	result.failure_stage = record.failure_stage;
	result.result_size = record.result.size();
	std::copy(record.result.begin(), record.result.end(), result.result_payload.begin());
	return result;
}
}
critical_apply_result flatfile_accounting_coin_transaction::apply(const std::string &root,
								  const critical_command &command)
{
	bool publishing = false;
	try
	{
		need(!root.empty() &&
			     command.schema_version == CRITICAL_COMMAND_ACCOUNTING_SCHEMA_VERSION &&
			     command.type == critical_command_type::coin_transfer &&
			     critical_command_envelope_valid(command),
		     EINVAL);
		flatfile_identity_lock identity_lock;
		flatfile_authority_lock lock;
		need(identity_lock.acquire(root, nullptr) && lock.acquire(root, nullptr), EIO);
		checked(flatfile_player_domain_recover_locked(root, lock, nullptr));
		flatfile_accounting_record retained;
		const auto lookup =
			flatfile_accounting_lookup(root, lock, command, &retained, nullptr);
		if (lookup == flatfile_accounting_status::ok)
		{
			verify(root, lock, retained);
			return completion(retained, true);
		}
		if (lookup != flatfile_accounting_status::not_found)
			checked(lookup, true);
		const auto value = decode(command);
		retained_identity(root, lock, command, value);
		const std::array<critical_operation_id, 3> ids{
			command.operation_id, value.payload.source.change.operation_id,
			value.payload.destination.change.operation_id
		};
		bool legacy = false;
		const auto catalog = flatfile_item_repository_operation_ids_present_locked(
			root, lock, ids, &legacy, nullptr);
		need(catalog == flatfile_item_repository_result::ok,
		     catalog == flatfile_item_repository_result::not_found ? ENOENT :
		     catalog == flatfile_item_repository_result::io_error  ? EIO :
									     EILSEQ);
		need(!legacy, EEXIST);
		const coin_transfer_endpoint *ends[] = { &value.payload.source,
							 &value.payload.destination };
		for (size_t i = 0; i < 2; ++i)
		{
			flatfile_accounting_record collision;
			const auto child = flatfile_accounting_lookup(root, lock, ends[i]->change,
								      &collision, nullptr);
			if (child == flatfile_accounting_status::ok)
				need(false, EEXIST);
			if (child != flatfile_accounting_status::not_found)
				checked(child, true);
			for (const auto &id : ids)
			{
				std::optional<flatfile_legacy_domain_receipt> receipt;
				checked(flatfile_player_domain_legacy_receipt_locked(
					root, lock, value.currency[i].pid, id, &receipt, nullptr));
				need(!receipt, EEXIST);
			}
		}
		std::array<std::string, 2> accounts;
		std::vector<flatfile_economic_mapping_request> requests;
		coin_transfer_result before;
		for (size_t i = 0; i < 2; ++i)
		{
			flatfile_identity_record native_identity;
			const auto status = flatfile_identity_lookup_pid_locked(
				root, identity_lock, lock, value.currency[i].pid, &native_identity,
				nullptr);
			need(status == flatfile_identity_result::ok,
			     status == flatfile_identity_result::io_error  ? EIO :
			     status == flatfile_identity_result::not_found ? ENOENT :
									     EILSEQ);
			accounts[i] = canonical(value.currency[i].account_name.data());
			need(native_identity.active &&
				     canonical(native_identity.account) == accounts[i] &&
				     native_identity.racewar == value.currency[i].racewar,
			     EACCES);
			requests.push_back({ value.wallets[i], { 1, value.currency[i].pid, {} } });
			if (!i || !economic_account_key_equal(value.banks[0], value.banks[1]))
				requests.push_back(
					{ value.banks[i],
					  { 2, value.banks[i].authority_id, accounts[i] } });
		}
		flatfile_economic_authority_snapshot snapshot;
		checked(economic_flatfile_lock_authority(root, lock, value.wallets[0].lineage,
							 value.intent.admission.metadata.epoch,
							 requests, &snapshot, nullptr));
		for (size_t i = 0; i < 2; ++i)
		{
			flatfile_player_domain_record native;
			checked(flatfile_player_domain_load_locked(
				root, lock, value.currency[i].pid, accounts[i],
				value.currency[i].racewar, &native, nullptr));
			before.wallets[i] = balances(native);
		}
		std::optional<economic_prepared_coin_wallets> prepared;
		flatfile_accounting_record record;
		record.command = command;
		const auto decision = economic_coin_wallets_prepare(
			command, value.intent, authority_for(value, before),
			currency_revision_policy::flatfile_legacy, &prepared,
			&record.failure_stage);
		coin_transfer_result result = before;
		std::vector<flatfile_authority_operation> operations;
		if (decision == economic_accounting_error::ok)
		{
			std::vector<flatfile_authority_after_image> images;
			unsigned int native_code = 0;
			checked(flatfile_player_domain_prepare_coin_wallets(
				root, lock, value.payload, &result, &images, &native_code,
				nullptr));
			checked(native_code);
			need(images.size() ==
			     (economic_account_key_equal(value.banks[0], value.banks[1]) ? 3 : 4));
			for (size_t i = 0; i < 2; ++i)
			{
				need(equal(result.wallets[i], prepared->mutations()[i].after()));
				record.durable_revision =
					std::max({ record.durable_revision,
						   result.wallets[i].wallet_revision,
						   result.wallets[i].bank_revision });
			}
			checked(economic_plan_encode(prepared->plan(), &record.plan));
			for (auto &image : images)
				operations.push_back({ flatfile_authority_store::domains,
						       flatfile_authority_operation_kind::write,
						       std::move(image.filename),
						       std::move(image.bytes) });
		}
		else
		{
			need(decision != economic_accounting_error::capacity, ENOMEM);
			record.result_code = decision_code(decision);
			need(record.result_code != 0);
		}
		std::array<uint8_t, COIN_TRANSFER_RESULT_BYTES> encoded;
		need(coin_transfer_command_encode_result(value.payload, result, &encoded));
		record.result.assign(encoded.begin(), encoded.end());
		const auto domain_images = operations.size();
		checked(flatfile_accounting_storage::stage(root, lock, record, &operations,
							   nullptr));
		publishing = true;
		need(flatfile_accounting_storage::commit(root, lock, operations, nullptr) ==
			     flatfile_authority_transaction_result::ok,
		     EIO);
		checked(flatfile_accounting_lookup(root, lock, command, &retained, nullptr), true);
		verify(root, lock, retained);
		need(retained.plan == record.plan && retained.result == record.result &&
		     retained.result_code == record.result_code &&
		     retained.failure_stage == record.failure_stage &&
		     retained.durable_revision == record.durable_revision);
		for (size_t i = 0; i < domain_images; ++i)
		{
			std::vector<uint8_t> bytes;
			need(flatfile_read(root + "/domains", operations[i].filename, 64 * 1024,
					   &bytes, nullptr) == flatfile_read_result::ok,
			     EIO);
			need(bytes == operations[i].bytes);
		}
		for (size_t i = 0; i < 2; ++i)
		{
			flatfile_player_domain_record native;
			checked(flatfile_player_domain_load_locked(
				root, lock, value.currency[i].pid, accounts[i],
				value.currency[i].racewar, &native, nullptr));
			auto expected = result.wallets[i];
			if (!record.result_code && !i &&
			    economic_account_key_equal(value.banks[0], value.banks[1]))
			{
				expected.bank = result.wallets[1].bank;
				expected.bank_revision = result.wallets[1].bank_revision;
			}
			need(equal(balances(native), expected));
		}
		return completion(retained, false);
	}
	catch (const failure &e)
	{
		return { publishing	  ? critical_apply_outcome::ambiguous_commit :
			 e.code == EEXIST ? critical_apply_outcome::terminal_failure :
					    critical_apply_outcome::retryable_failure,
			 0, e.code };
	}
	catch (const std::bad_alloc &)
	{
		return { publishing ? critical_apply_outcome::ambiguous_commit :
				      critical_apply_outcome::retryable_failure,
			 0, ENOMEM };
	}
}
