#pragma once
// Shared synthetic wallet/lifetime setup for standalone native owner harnesses.
#include "flatfile/flatfile_accounting_bank_transaction.h"
#include "flatfile/flatfile_accounting_authority.h"
#include "flatfile/flatfile_identity_repository.h"
#include "flatfile/flatfile_player_domain_repository.h"
#include "flatfile/currency_flatfile_mutation_writer.h"
#include "flatfile/flatfile_store.h"
#include "economy/economic_currency_adapter.h"
#include "world/epic_command.h"
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <new>
#include <openssl/sha.h>
#include <sys/wait.h>
#include <unistd.h>

class flatfile_accounting_test_access
{
    public:
	static constexpr auto bootstrap = &flatfile_accounting_authority_storage::bootstrap;
	static constexpr auto initialize_native =
		&flatfile_accounting_authority_storage::initialize_native_bucket;
	static constexpr auto initialize_evidence =
		&flatfile_accounting_authority_storage::initialize_evidence_bucket;
	static constexpr auto create = &flatfile_accounting_authority_storage::create_mapping;
	static constexpr auto rename = &flatfile_accounting_authority_storage::rename_bank;
	static constexpr auto retire = &flatfile_accounting_authority_storage::retire_mapping;
	static constexpr auto append_epoch = &flatfile_accounting_authority_storage::append_epoch;
	static constexpr auto select_epoch = &flatfile_accounting_authority_storage::select_epoch;
	static constexpr auto stage = &flatfile_accounting_storage::stage;
	static constexpr auto commit = &flatfile_accounting_storage::commit;
	static constexpr auto native_stage = &currency_flatfile_mutation_writer::stage;
};
using access_type = flatfile_accounting_test_access;
using ops = std::vector<flatfile_authority_operation>;
using bytes = std::vector<uint8_t>;
using outcome = critical_apply_outcome;
namespace fs = std::filesystem;
size_t allocation_target = 0, allocation_seen = 0;
extern "C" void *__real__Znwm(size_t);
extern "C" void *__real__Znam(size_t);
extern "C" void *__wrap__Znwm(size_t count)
{
	if (allocation_target && ++allocation_seen == allocation_target)
		throw std::bad_alloc();
	return __real__Znwm(count);
}
extern "C" void *__wrap__Znam(size_t count)
{
	if (allocation_target && ++allocation_seen == allocation_target)
		throw std::bad_alloc();
	return __real__Znam(count);
}
critical_operation_id id(uint64_t n)
{
	critical_operation_id value = {};
	value.bytes[0] = 7;
	for (size_t i = 0; i < 8; ++i)
		value.bytes[i + 1] = static_cast<uint8_t>(n >> (i * 8));
	return value;
}
bytes read(const fs::path &path)
{
	bytes value;
	assert(flatfile_read(path.parent_path().string(), path.filename().string(), 8 * 1024 * 1024,
			     &value, nullptr) == flatfile_read_result::ok);
	return value;
}
void write(const fs::path &path, const bytes &value)
{
	assert(flatfile_atomic_write(path.parent_path().string(), path.filename().string(), value,
				     nullptr));
}
size_t bucket(uint16_t kind, uint64_t context, uint64_t pid, const std::string &name)
{
	bytes key;
	const auto add = [&](uint64_t n, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
			key.push_back(static_cast<uint8_t>(n >> (8 * i)));
	};
	add(kind, 2);
	add(context, 8);
	add(kind, 2);
	if (kind == 1)
		add(pid, 8);
	else
		key.insert(key.end(), name.begin(), name.end());
	std::array<uint8_t, 32> digest;
	SHA256(key.data(), key.size(), digest.data());
	return digest[0];
}
flatfile_economic_control control(const std::string &root, const flatfile_authority_lock &lock)
{
	flatfile_economic_control value;
	assert(flatfile_economic_control_read(root, lock, &value, nullptr) == 0);
	return value;
}
void commit(const std::string &root, const flatfile_authority_lock &lock, ops &changes)
{
	assert(access_type::commit(root, lock, changes, nullptr) ==
	       flatfile_authority_transaction_result::ok);
	changes.clear();
}
void initialize_bucket(const std::string &root, const flatfile_authority_lock &lock, size_t index)
{
	ops changes;
	const auto result = access_type::initialize_native(root, lock, control(root, lock).revision,
							   index, id(90000), &changes, nullptr);
	assert(result == 0 || result == EALREADY);
	if (!result)
		commit(root, lock, changes);
}
void setup(const fs::path &path)
{
	const auto root = path.string();
	for (const auto &dir : { path, path / "domains", path / "economic-evidence",
				 path / "identities", path / "identities/names" })
	{
		fs::create_directories(dir);
		fs::permissions(dir, fs::perms::owner_all);
	}
	int32_t pid;
	assert(flatfile_identity_allocate_pid(root, &pid, nullptr) ==
		       flatfile_identity_result::ok &&
	       pid == 1);
	assert(flatfile_identity_claim(root, 1, "Player", "ACCOUNT-ONE", nullptr) ==
	       flatfile_identity_result::ok);
	flatfile_identity_record identity;
	assert(flatfile_identity_lookup_pid(root, 1, &identity, nullptr) ==
	       flatfile_identity_result::ok);
	identity.racewar = 1;
	assert(flatfile_identity_sync_account(root, "account-one", { identity }, nullptr) ==
	       flatfile_identity_result::ok);
	flatfile_player_domain_record player;
	player.pid = 1;
	player.account_name = "account-one";
	player.racewar = 1;
	player.domains.wallet = { 100, 20, 3, 1 };
	player.domains.bank = { 100, 20, 3, 1 };
	player.domains.epics = 7;
	player.domains.frags = 9;
	player.domains.base_stats = { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
	player.domains.base_stat_revision = 1;
	player.recent_pvp_deaths = { 50, 40 };
	player.completed_epic_zones = { 3, 5 };
	assert(flatfile_player_domain_establish(root, player, nullptr) ==
	       flatfile_player_domain_result::ok);
	flatfile_authority_lock lock;
	assert(lock.acquire(root, nullptr));
	ops changes;
	assert(access_type::bootstrap(root, lock, id(90001), id(90002), &changes, nullptr) == 0);
	commit(root, lock, changes);
	initialize_bucket(root, lock, bucket(1, 0, 1, {}));
	initialize_bucket(root, lock, bucket(2, 1, 0, "account-one"));
	flatfile_economic_mapping mapping;
	assert(access_type::create(root, lock, control(root, lock).revision,
				   economic_account_kind::wallet, 0, { 1, 1, {} }, id(90003),
				   &mapping, &changes, nullptr) == 0);
	assert(mapping.account.authority_id == 1);
	commit(root, lock, changes);
	assert(access_type::create(root, lock, control(root, lock).revision,
				   economic_account_kind::bank, 1, { 2, 0, "account-one" },
				   id(90004), &mapping, &changes, nullptr) == 0);
	assert(mapping.account.authority_id == 2);
	commit(root, lock, changes);
	flatfile_economic_epoch epoch;
	epoch.epoch = id(90005);
	epoch.ordinal = 1;
	epoch.creating_operation = id(90006);
	epoch.transition_kind = 1;
	epoch.transition_digest[0] = 42;
	assert(access_type::append_epoch(root, lock, control(root, lock).revision, epoch, &changes,
					 nullptr) == 0);
	commit(root, lock, changes);
	assert(access_type::select_epoch(root, lock, control(root, lock).revision, true, id(90007),
					 &changes, nullptr) == 0);
	commit(root, lock, changes);
	assert(access_type::initialize_evidence(root, lock, control(root, lock).revision, 7,
						id(90008), &changes, nullptr) == 0);
	commit(root, lock, changes);
}
flatfile_player_domain_record state(const std::string &root)
{
	flatfile_player_domain_record value;
	assert(flatfile_player_domain_load(root, 1, "account-one", 1, &value, nullptr) ==
	       flatfile_player_domain_result::ok);
	return value;
}
economic_account_key wallet()
{
	return { id(90001), economic_account_kind::wallet, 1, 0 };
}
economic_account_key bank()
{
	return { id(90001), economic_account_kind::bank, 2, 1 };
}
critical_command command(const std::string &root, uint64_t n, int64_t amount = 1,
			 bool accounted = true)
{
	const auto before = state(root);
	currency_command_payload payload = {};
	payload.pid = 1;
	payload.racewar = 1;
	payload.reason = amount > 0 ? currency_reason_type::atm_withdraw :
				      currency_reason_type::atm_deposit;
	strcpy(payload.account_name.data(), "ACCOUNT-ONE");
	payload.wallet_delta.amount[0] = amount;
	payload.bank_delta.amount[0] = -amount;
	critical_command result;
	assert(currency_command_build(&result, id(n), payload, before.domains.wallet_revision,
				      before.domains.bank_revision, critical_source_site::command,
				      critical_deadline_class::interactive));
	result.accepted_at_usec = 12;
	assert(critical_command_normalize(&result));
	if (accounted)
	{
		assert(economic_bank_transfer_intent(result, id(90005), wallet(), bank(),
						     &result.accounting_intent) ==
		       economic_accounting_error::ok);
		result.schema_version = 2;
	}
	return result;
}
