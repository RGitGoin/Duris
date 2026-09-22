#include "flatfile_accounting_wallet_fixture.h"
void refreeze(critical_command &cmd, const critical_operation_id &epoch = id(90005))
{
	cmd.schema_version = 1;
	cmd.accounting_intent.clear();
	assert(economic_bank_transfer_intent(cmd, epoch, wallet(), bank(),
					     &cmd.accounting_intent) ==
	       economic_accounting_error::ok);
	cmd.schema_version = 2;
}
flatfile_accounting_record retained(const std::string &root, const critical_command &cmd)
{
	flatfile_authority_lock lock;
	assert(lock.acquire(root, nullptr));
	flatfile_accounting_record value;
	assert(flatfile_accounting_lookup(root, lock, cmd, &value, nullptr) ==
	       flatfile_accounting_status::ok);
	return value;
}
void same(const critical_apply_result &a, const critical_apply_result &b)
{
	assert(a.error_code == b.error_code && a.durable_revision == b.durable_revision &&
	       a.failure_stage == b.failure_stage && a.result_size == b.result_size &&
	       a.result_payload == b.result_payload);
}
void basic(const std::string &root)
{
	auto legacy = command(root, 1, 1, false);
	assert(flatfile_player_domain_apply(root, legacy).outcome == outcome::applied);
	auto clash = legacy;
	refreeze(clash);
	assert(flatfile_accounting_bank_transaction::apply(root, clash).error_code == EEXIST);
	assert(flatfile_identity_set_blocked(root, 1, true, nullptr) ==
	       flatfile_identity_result::ok);
	const auto before = state(root);
	const auto transfer = command(root, 2);
	const auto applied = flatfile_accounting_bank_transaction::apply(root, transfer);
	assert(applied.outcome == outcome::applied && !applied.error_code);
	auto replay = flatfile_accounting_bank_transaction::apply(root, transfer);
	assert(replay.outcome == outcome::already_applied);
	same(applied, replay);
	auto changed = transfer;
	++changed.accepted_at_usec;
	assert(flatfile_accounting_bank_transaction::apply(root, changed).error_code == EEXIST);
	const auto after = state(root);
	assert(after.domains.wallet[0] == before.domains.wallet[0] + 1 &&
	       after.domains.bank[0] + 1 == before.domains.bank[0]);
	assert(after.domains.epics == before.domains.epics &&
	       after.domains.frags == before.domains.frags &&
	       after.domains.base_stats == before.domains.base_stats &&
	       after.recent_pvp_deaths == before.recent_pvp_deaths);
	// Regenerate the same canonical plan with SQL revision policy.
	const auto record = retained(root, transfer);
	economic_frozen_intent intent;
	assert(economic_intent_decode(transfer.accounting_intent, &intent) ==
	       economic_accounting_error::ok);
	economic_currency_authority authority = { id(90005),	    wallet(),	      bank(),
						  transfer.keys[0], transfer.keys[1], {} };
	for (size_t i = 0; i < 4; ++i)
	{
		authority.state.wallet.amount[i] = before.domains.wallet[i];
		authority.state.bank.amount[i] = before.domains.bank[i];
	}
	authority.state.wallet_revision = before.domains.wallet_revision;
	authority.state.bank_revision = before.domains.bank_revision;
	std::optional<economic_prepared_currency> prepared;
	assert(economic_bank_transfer_prepare(transfer, intent, authority,
					      currency_revision_policy::sql_legacy,
					      &prepared) == economic_accounting_error::ok);
	bytes canonical;
	assert(economic_plan_encode(prepared->plan(), &canonical) ==
		       economic_accounting_error::ok &&
	       canonical == record.plan);
	auto rejected = command(root, 3, 10000);
	const auto rejection = flatfile_accounting_bank_transaction::apply(root, rejected);
	assert(rejection.outcome == outcome::terminal_failure && rejection.error_code == ENOSPC);
	same(rejection, flatfile_accounting_bank_transaction::apply(root, rejected));
	assert(retained(root, rejected).plan.empty());
	auto stale = command(root, 4);
	stale.expected_revisions[0].revision = 0;
	refreeze(stale);
	assert(flatfile_accounting_bank_transaction::apply(root, stale).error_code == ESTALE);
	// New accounting evidence must not append to the capped legacy receipt list.
	for (uint64_t n = 5; n < 519; ++n)
	{
		auto next = command(root, n, n % 2 ? 1 : -1);
		assert(flatfile_accounting_bank_transaction::apply(root, next).outcome ==
		       outcome::applied);
	}
	same(applied, flatfile_accounting_bank_transaction::apply(root, transfer));
	{
		flatfile_authority_lock lock;
		assert(lock.acquire(root, nullptr));
		std::optional<flatfile_legacy_domain_receipt> old;
		assert(flatfile_player_domain_legacy_receipt_locked(
			       root, lock, 1, legacy.operation_id, &old, nullptr) ==
			       flatfile_player_domain_result::ok &&
		       old);
		assert(flatfile_player_domain_legacy_receipt_locked(
			       root, lock, 1, transfer.operation_id, &old, nullptr) ==
			       flatfile_player_domain_result::ok &&
		       !old);
		ops changes;
		initialize_bucket(root, lock, bucket(2, 1, 0, "renamed"));
		flatfile_economic_mapping mapping;
		assert(flatfile_economic_mapping_read(root, lock, bank(), &mapping, nullptr) == 0);
		assert(access_type::rename(root, lock, control(root, lock).revision, bank(),
					   mapping.revision, "renamed", id(91000), &changes,
					   nullptr) == 0);
		commit(root, lock, changes);
		flatfile_economic_epoch later;
		later.epoch = id(91003);
		later.predecessor = id(90005);
		later.ordinal = 2;
		later.creating_operation = id(91004);
		later.transition_kind = 1;
		later.transition_digest[0] = 42;
		assert(access_type::append_epoch(root, lock, control(root, lock).revision, later,
						 &changes, nullptr) == 0);
		commit(root, lock, changes);
		assert(access_type::select_epoch(root, lock, control(root, lock).revision, true,
						 id(91005), &changes, nullptr) == 0);
		commit(root, lock, changes);
		assert(access_type::select_epoch(root, lock, control(root, lock).revision, false,
						 id(91001), &changes, nullptr) == 0);
		commit(root, lock, changes);
		for (const auto &key : { wallet(), bank() })
		{
			assert(flatfile_economic_mapping_read(root, lock, key, &mapping, nullptr) ==
			       0);
			assert(access_type::retire(root, lock, control(root, lock).revision, key,
						   mapping.revision, id(91002), &changes,
						   nullptr) == 0);
			commit(root, lock, changes);
		}
	}
	fs::remove(fs::path(root) / "domains/player-1.domain");
	fs::remove(fs::path(root) / "domains/bank-account-one-1.domain");
	same(applied, flatfile_accounting_bank_transaction::apply(root, transfer));
	same(rejection, flatfile_accounting_bank_transaction::apply(root, rejected));
	std::cout
		<< "bank root: success, exact replay beyond 512 later IDs, legacy preservation, rejection, SQL plan parity, rename/retirement/paused epoch passed\n";
}

void clone(const fs::path &seed, const fs::path &target)
{
	fs::copy(seed, target, fs::copy_options::recursive);
}
void crashes(const fs::path &seed, const fs::path &base)
{
	for (size_t boundary = 0; boundary <= 4; ++boundary)
	{
		const auto path = base / ("crash-" + std::to_string(boundary));
		clone(seed, path);
		const auto root = path.string();
		const auto cmd = command(root, 2);
		const auto player_path = path / "domains/player-1.domain";
		const auto bank_path = path / "domains/bank-account-one-1.domain";
		const auto index_path = path / "economic-evidence/bucket-07.eai";
		const auto original_player = read(player_path), original_bank = read(bank_path),
			   original_index = read(index_path);
		const auto child = fork();
		assert(child >= 0);
		if (!child)
		{
			if (boundary == 0)
				setenv("DURIS_FLATFILE_TEST_INTERRUPT_AFTER_AUTHORITY_JOURNAL", "1",
				       1);
			else
				setenv("DURIS_FLATFILE_TEST_INTERRUPT_AFTER_AUTHORITY_OPERATION",
				       std::to_string(boundary).c_str(), 1);
			const auto result = flatfile_accounting_bank_transaction::apply(root, cmd);
			_exit(result.outcome == outcome::ambiguous_commit ? 77 : 78);
		}
		int status;
		assert(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
		       WEXITSTATUS(status) == 77);
		const auto journal = path / "domains/.critical-authority-transaction";
		assert(fs::exists(journal));
		if (boundary == 0)
		{
			// Check raw files before any recovery-capable read: only the journal
			// may have been published at this boundary.
			assert(!read(journal).empty() && read(player_path) == original_player &&
			       read(bank_path) == original_bank &&
			       read(index_path) == original_index &&
			       !fs::exists(path / "economic-evidence/bucket-07-0.eas"));
		}
		const auto replay = flatfile_accounting_bank_transaction::apply(root, cmd);
		assert(replay.outcome == outcome::already_applied);
		const auto after = state(root);
		assert(after.domains.wallet[0] == 101 && after.domains.bank[0] == 99);
		assert(!fs::exists(journal));
		const auto receipt = retained(root, cmd);
		bytes expected, actual;
		assert(critical_command_encode(cmd, &expected) ==
		       critical_command_codec_result::ok);
		assert(critical_command_encode(receipt.command, &actual) ==
		       critical_command_codec_result::ok);
		assert(actual == expected && replay.failure_stage == critical_failure_stage::none);
		same(replay, flatfile_accounting_bank_transaction::apply(root, cmd));
	}
	std::cout
		<< "bank root: journal-only and all four domain/evidence publication interruptions recover exactly once\n";
}
void allocations(const fs::path &seed, const fs::path &base)
{
	size_t failures = 0, ambiguous = 0;
	bool complete = false;
	for (size_t nth = 1; nth < 4096; ++nth)
	{
		const auto path = base / ("allocation-" + std::to_string(nth));
		clone(seed, path);
		const auto root = path.string();
		const auto cmd = command(root, 2);
		allocation_seen = 0;
		allocation_target = nth;
		const auto result = flatfile_accounting_bank_transaction::apply(root, cmd);
		allocation_target = 0;
		if (allocation_seen < nth)
		{
			assert(result.outcome == outcome::applied);
			complete = true;
			fs::remove_all(path);
			break;
		}
		assert(result.outcome == outcome::retryable_failure ||
		       result.outcome == outcome::ambiguous_commit);
		++failures;
		if (result.outcome == outcome::ambiguous_commit)
			++ambiguous;
		const auto retry = flatfile_accounting_bank_transaction::apply(root, cmd);
		assert(retry.outcome == outcome::applied ||
		       retry.outcome == outcome::already_applied);
		const auto after = state(root);
		assert(after.domains.wallet[0] == 101 && after.domains.bank[0] == 99);
		same(retry, flatfile_accounting_bank_transaction::apply(root, cmd));
		fs::remove_all(path);
	}
	assert(complete && failures && ambiguous);
	std::cout << "bank root: " << failures << " allocation failures recover exactly once ("
		  << ambiguous << " during/after commit)\n";
}
void forged(const fs::path &seed, const fs::path &base)
{
	const auto source = base / "valid";
	clone(seed, source);
	const auto valid_command = command(source.string(), 2);
	assert(flatfile_accounting_bank_transaction::apply(source.string(), valid_command).outcome ==
	       outcome::applied);
	const auto original = retained(source.string(), valid_command);
	const auto rejected_command = command(source.string(), 3, 10000);
	const auto rejected_result =
		flatfile_accounting_bank_transaction::apply(source.string(), rejected_command);
	assert(rejected_result.outcome == outcome::terminal_failure &&
	       rejected_result.error_code == ENOSPC);
	const auto legitimate_rejection = retained(source.string(), rejected_command);
	assert(legitimate_rejection.failure_stage == critical_failure_stage::none);
	same(rejected_result,
	     flatfile_accounting_bank_transaction::apply(source.string(), rejected_command));
	for (int variant = 0; variant < 5; ++variant)
	{
		const auto path = base / ("forged-" + std::to_string(variant));
		clone(seed, path);
		const auto root = path.string();
		auto record = original;
		if (variant == 4)
		{
			// All receipt fields except the stage come from a verified bank
			// rejection. A valid enum from another domain is still not a bank stage.
			record = legitimate_rejection;
			record.failure_stage = critical_failure_stage::coin_source_wallet_revision;
			assert(critical_failure_stage_valid(record.failure_stage));
		}
		else if (variant == 0)
		{
			currency_command_result result;
			assert(currency_command_decode_result(record.result.data(),
							      record.result.size(), &result));
			++result.wallet.amount[0];
			std::array<uint8_t, CURRENCY_RESULT_PAYLOAD_BYTES> encoded;
			assert(currency_command_encode_result(result, &encoded));
			record.result.assign(encoded.begin(), encoded.end());
		}
		else if (variant == 1)
		{
			economic_accounting_plan plan;
			assert(economic_plan_decode(record.plan, &plan) ==
			       economic_accounting_error::ok);
			plan.postings.push_back({ 2, 0, 0, { 1, 0, 0, 0 }, 1 });
			plan.postings.push_back({ 3, 0, 0, { -1, 0, 0, 0 }, -1 });
			assert(economic_plan_normalize(&plan) == economic_accounting_error::ok);
			assert(economic_plan_encode(plan, &record.plan) ==
			       economic_accounting_error::ok);
		}
		else
		{
			record.plan.clear();
			record.result_code = variant == 2 ? EACCES : ESTALE;
			if (variant == 3)
				refreeze(record.command, id(99999));
		}
		{
			flatfile_authority_lock lock;
			assert(lock.acquire(root, nullptr));
			ops changes;
			assert(access_type::stage(root, lock, record, &changes, nullptr) ==
			       flatfile_accounting_status::ok);
			commit(root, lock, changes);
		}
		const auto result =
			flatfile_accounting_bank_transaction::apply(root, record.command);
		assert(result.outcome == outcome::retryable_failure && result.error_code);
		assert(state(root).domains.wallet[0] == 100);
	}
	std::cout
		<< "bank root: forged results, extra cancelling legs, arbitrary rejection, nonexistent epoch and forged valid failure stage refused\n";
}
void legacy_capacity(const fs::path &seed, const fs::path &path)
{
	clone(seed, path);
	const auto root = path.string();
	critical_command first;
	for (uint64_t n = 0; n < 512; ++n)
	{
		critical_command cmd;
		epic_command_payload payload = { 1, 1, epic_reason_type::quest_award, 0, 1 };
		assert(epic_command_build(&cmd, id(10000 + n), payload, UINT64_MAX,
					  critical_source_site::command,
					  critical_deadline_class::interactive));
		cmd.accepted_at_usec = 1;
		assert(critical_command_normalize(&cmd));
		if (!n)
			first = cmd;
		assert(flatfile_player_domain_apply(root, cmd).outcome == outcome::applied);
	}
	const auto old = flatfile_player_domain_apply(root, first);
	assert(old.outcome == outcome::already_applied);
	auto full = first;
	full.operation_id = id(11000);
	assert(flatfile_player_domain_apply(root, full).error_code == ENOSPC);
	const auto cmd = command(root, 2);
	assert(flatfile_accounting_bank_transaction::apply(root, cmd).outcome == outcome::applied);
	same(old, flatfile_player_domain_apply(root, first));
	assert(state(root).domains.epics == 519);
	std::cout
		<< "bank root: 512 legacy receipts survive typed transfer without consuming a legacy slot\n";
}
void contention(const fs::path &seed, const fs::path &path)
{
	clone(seed, path);
	const auto root = path.string();
	int32_t pid;
	assert(flatfile_identity_allocate_pid(root, &pid, nullptr) ==
		       flatfile_identity_result::ok &&
	       pid == 2);
	assert(flatfile_identity_claim(root, pid, "Second", "account-one", nullptr) ==
	       flatfile_identity_result::ok);
	std::vector<flatfile_identity_record> identities;
	assert(flatfile_identity_list_account(root, "account-one", &identities, nullptr) ==
	       flatfile_identity_result::ok);
	for (auto &entry : identities)
		entry.racewar = 1;
	assert(flatfile_identity_sync_account(root, "account-one", identities, nullptr) ==
	       flatfile_identity_result::ok);
	auto player = state(root);
	player.pid = 2;
	const auto bank_revision = player.domains.bank_revision;
	player.domains.bank_revision = 0;
	assert(flatfile_player_domain_establish(root, player, nullptr) ==
	       flatfile_player_domain_result::ok);
	player.domains.bank_revision = bank_revision;
	economic_account_key second;
	{
		flatfile_authority_lock lock;
		assert(lock.acquire(root, nullptr));
		initialize_bucket(root, lock, bucket(1, 0, 2, {}));
		ops changes;
		flatfile_economic_mapping mapping;
		assert(access_type::create(root, lock, control(root, lock).revision,
					   economic_account_kind::wallet, 0, { 1, 2, {} },
					   id(90020), &mapping, &changes, nullptr) == 0);
		second = mapping.account;
		commit(root, lock, changes);
	}
	critical_command commands[2] = { command(root, 2), command(root, 3) };
	currency_command_payload payload;
	assert(currency_command_decode_payload(commands[1], &payload));
	payload.pid = 2;
	assert(currency_command_build(&commands[1], id(3), payload, player.domains.wallet_revision,
				      player.domains.bank_revision, critical_source_site::command,
				      critical_deadline_class::interactive));
	commands[1].accepted_at_usec = 12;
	assert(critical_command_normalize(&commands[1]));
	assert(economic_bank_transfer_intent(commands[1], id(90005), second, bank(),
					     &commands[1].accounting_intent) ==
	       economic_accounting_error::ok);
	commands[1].schema_version = 2;
	pid_t children[2];
	for (size_t i = 0; i < 2; ++i)
	{
		children[i] = fork();
		assert(children[i] >= 0);
		if (!children[i])
		{
			const auto result =
				flatfile_accounting_bank_transaction::apply(root, commands[i]);
			_exit(result.outcome == outcome::applied ? 10 :
			      result.outcome == outcome::terminal_failure &&
					      result.error_code == ESTALE ?
								   11 :
								   12);
		}
	}
	size_t committed = 0, rejected = 0;
	for (auto child : children)
	{
		int status;
		assert(waitpid(child, &status, 0) == child && WIFEXITED(status));
		committed += WEXITSTATUS(status) == 10;
		rejected += WEXITSTATUS(status) == 11;
	}
	assert(committed == 1 && rejected == 1);
	flatfile_player_domain_record two;
	assert(flatfile_player_domain_load(root, 2, "account-one", 1, &two, nullptr) ==
	       flatfile_player_domain_result::ok);
	assert(state(root).domains.wallet[0] + two.domains.wallet[0] == 201 &&
	       two.domains.bank[0] == 99);
	for (const auto &cmd : commands)
	{
		const auto replay = flatfile_accounting_bank_transaction::apply(root, cmd);
		assert(replay.outcome == outcome::already_applied ||
		       (replay.outcome == outcome::terminal_failure &&
			replay.error_code == ESTALE));
	}
	std::cout
		<< "bank root: concurrent players sharing a bank serialize with one exact stale rejection\n";
}
int main(int argc, char **argv)
{
	assert(argc == 2);
	alarm(600);
	const fs::path base = argv[1];
	fs::create_directories(base);
	fs::permissions(base, fs::perms::owner_all);
	const auto seed = base / "seed";
	setup(seed);
	crashes(seed, base);
	legacy_capacity(seed, base / "legacy-full");
	contention(seed, base / "contended");
	forged(seed, base);
	allocations(seed, base);
	const auto journey = base / "basic";
	clone(seed, journey);
	basic(journey.string());
	std::cout << "flatfile typed bank root journeys passed\n";
}
