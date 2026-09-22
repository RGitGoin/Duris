#include "flatfile_accounting_wallet_fixture.h"
#include "flatfile/flatfile_accounting_coin_transaction.h"
#include "flatfile/flatfile_item_repository.h"
#include "economy/economic_coin_adapter.h"
#include <set>
using coin_owner = flatfile_accounting_coin_transaction;
struct coin_fixture
{
	std::string root, second_account;
	economic_account_key second_wallet, second_bank;
};
coin_fixture setup_coin(const fs::path &path, bool shared, bool overflow = false)
{
	setup(path);
	coin_fixture f{ path.string(), shared ? "account-one" : "account-two", {}, bank() };
	int32_t pid;
	assert(flatfile_identity_allocate_pid(f.root, &pid, nullptr) ==
		       flatfile_identity_result::ok &&
	       pid == 2);
	assert(flatfile_identity_claim(f.root, pid, "Second", f.second_account, nullptr) ==
	       flatfile_identity_result::ok);
	std::vector<flatfile_identity_record> identities;
	assert(flatfile_identity_list_account(f.root, f.second_account, &identities, nullptr) ==
	       flatfile_identity_result::ok);
	for (auto &entry : identities)
		entry.racewar = 1;
	assert(flatfile_identity_sync_account(f.root, f.second_account, identities, nullptr) ==
	       flatfile_identity_result::ok);
	auto player = state(f.root);
	player.pid = 2;
	player.domains.wallet_revision = 0;
	player.domains.bank_revision = 0;
	player.domains.epic_revision = 0;
	player.domains.frag_revision = 0;
	player.account_name = f.second_account;
	player.domains.wallet[0] = overflow ? INT32_MAX : 100;
	assert(flatfile_player_domain_establish(f.root, player, nullptr) ==
	       flatfile_player_domain_result::ok);
	assert(flatfile_item_repository_establish_owner(f.root, { item_owner_type::player, 1, 0 },
							{}, nullptr) ==
	       flatfile_item_baseline_result::applied);
	flatfile_authority_lock lock;
	assert(lock.acquire(f.root, nullptr));
	initialize_bucket(f.root, lock, bucket(1, 0, 2, {}));
	ops changes;
	flatfile_economic_mapping mapping;
	assert(access_type::create(f.root, lock, control(f.root, lock).revision,
				   economic_account_kind::wallet, 0, { 1, 2, {} }, id(91001),
				   &mapping, &changes, nullptr) == 0);
	f.second_wallet = mapping.account;
	commit(f.root, lock, changes);
	if (!shared)
	{
		initialize_bucket(f.root, lock, bucket(2, 1, 0, f.second_account));
		assert(access_type::create(f.root, lock, control(f.root, lock).revision,
					   economic_account_kind::bank, 1,
					   { 2, 0, f.second_account }, id(91002), &mapping,
					   &changes, nullptr) == 0);
		f.second_bank = mapping.account;
		commit(f.root, lock, changes);
	}
	return f;
}
flatfile_player_domain_record second_state(const coin_fixture &f)
{
	flatfile_player_domain_record value;
	assert(flatfile_player_domain_load(f.root, 2, f.second_account, 1, &value, nullptr) ==
	       flatfile_player_domain_result::ok);
	return value;
}
critical_command coin_command(const coin_fixture &f, uint64_t sequence, unsigned int mode = 0)
{
	const std::array<flatfile_player_domain_record, 2> states{ state(f.root), second_state(f) };
	coin_transfer_payload payload;
	coin_transfer_endpoint *ends[]{ &payload.source, &payload.destination };
	for (size_t i = 0; i < 2; ++i)
	{
		auto &end = *ends[i];
		currency_command_payload native{};
		native.pid = i + 1;
		native.racewar = 1;
		native.reason = currency_reason_type::coin_transfer;
		strcpy(native.account_name.data(), states[i].account_name.c_str());
		for (size_t part = 0; part < 4; ++part)
			end.before[part] = states[i].domains.wallet[part];
		if (mode == 2 && !i)
			end.before[0] = 200;
		if (mode == 3 && i)
			end.before[0] = 100;
		const int delta = mode == 2 ? 200 : 10;
		end.after = end.before;
		end.after[0] += i ? delta : -delta;
		native.wallet_delta.amount[0] = i ? delta : -delta;
		assert(currency_command_build(
			&end.change, id(sequence + i + 1000), native,
			states[i].domains.wallet_revision + (mode == 1 && !i ? 1 : 0),
			states[i].domains.bank_revision, critical_source_site::command,
			critical_deadline_class::interactive));
	}
	critical_command command;
	assert(coin_transfer_command_build(&command, id(sequence), payload,
					   critical_source_site::command,
					   critical_deadline_class::interactive));
	command.accepted_at_usec = 12;
	assert(economic_coin_wallets_intent(
		       command, id(90005), { wallet(), f.second_wallet }, { bank(), f.second_bank },
		       &command.accounting_intent) == economic_accounting_error::ok);
	command.schema_version = 2;
	assert(coin_transfer_command_decode_payload(command, &payload));
	const std::set<size_t> buckets{ command.operation_id.bytes[0],
					payload.source.change.operation_id.bytes[0],
					payload.destination.change.operation_id.bytes[0] };
	flatfile_authority_lock lock;
	assert(lock.acquire(f.root, nullptr));
	for (auto number : buckets)
	{
		const auto current = control(f.root, lock);
		if (current.evidence_initialized[number / 8] & (1U << (number % 8)))
			continue;
		ops changes;
		assert(access_type::initialize_evidence(f.root, lock, current.revision, number,
							id(92000 + number), &changes,
							nullptr) == 0);
		commit(f.root, lock, changes);
	}
	return command;
}
void same_coin(const critical_apply_result &a, const critical_apply_result &b)
{
	assert(a.error_code == b.error_code && a.failure_stage == b.failure_stage &&
	       a.durable_revision == b.durable_revision && a.result_size == b.result_size &&
	       a.result_payload == b.result_payload);
}
void parity(const coin_fixture &f, const critical_command &command)
{
	coin_transfer_payload payload;
	economic_frozen_intent intent;
	assert(coin_transfer_command_decode_payload(command, &payload));
	assert(economic_intent_decode(command.accounting_intent, &intent) ==
	       economic_accounting_error::ok);
	const std::array<flatfile_player_domain_record, 2> states{ state(f.root), second_state(f) };
	const std::array<economic_account_key, 2> wallets{ wallet(), f.second_wallet },
		banks{ bank(), f.second_bank };
	const coin_transfer_endpoint *ends[]{ &payload.source, &payload.destination };
	economic_coin_wallet_authority authority;
	for (size_t i = 0; i < 2; ++i)
	{
		currency_command_result before;
		for (size_t part = 0; part < 4; ++part)
		{
			before.wallet.amount[part] = states[i].domains.wallet[part];
			before.bank.amount[part] = states[i].domains.bank[part];
		}
		before.wallet_revision = states[i].domains.wallet_revision;
		before.bank_revision = states[i].domains.bank_revision;
		authority[i] = { intent.admission.metadata.epoch,
				 wallets[i],
				 banks[i],
				 ends[i]->change.keys[0],
				 ends[i]->change.keys[1],
				 before };
	}
	std::optional<economic_prepared_coin_wallets> sql, flat;
	critical_failure_stage sql_stage, flat_stage;
	const auto sql_code = economic_coin_wallets_prepare(
		command, intent, authority, currency_revision_policy::sql_legacy, &sql, &sql_stage);
	const auto flat_code = economic_coin_wallets_prepare(
		command, intent, authority, currency_revision_policy::flatfile_legacy, &flat,
		&flat_stage);
	assert(sql_code == flat_code && sql_stage == flat_stage && bool(sql) == bool(flat));
	if (sql)
	{
		bytes a, b;
		assert(economic_plan_encode(sql->plan(), &a) == economic_accounting_error::ok);
		assert(economic_plan_encode(flat->plan(), &b) == economic_accounting_error::ok);
		assert(a == b);
	}
}
void legacy_fences(const fs::path &base)
{
	for (unsigned int slot = 0; slot < 3; ++slot)
	{
		const auto f = setup_coin(base / std::to_string(slot), true);
		const auto cmd = coin_command(f, 40);
		coin_transfer_payload payload;
		assert(coin_transfer_command_decode_payload(cmd, &payload));
		const std::array<critical_operation_id, 3> ids{
			cmd.operation_id, payload.source.change.operation_id,
			payload.destination.change.operation_id
		};
		auto old = command(f.root, 50, 1, false);
		old.operation_id = ids[slot];
		assert(flatfile_player_domain_apply(f.root, old).outcome == outcome::applied);
		const auto before = state(f.root);
		const auto refused = coin_owner::apply(f.root, cmd);
		assert(refused.outcome == outcome::terminal_failure &&
		       refused.error_code == EEXIST);
		assert(state(f.root).domains.wallet == before.domains.wallet);
		assert(second_state(f).domains.wallet[0] == 100);
	}
	const auto f = setup_coin(base / "missing-catalog", true);
	const auto cmd = coin_command(f, 60);
	assert(fs::remove(fs::path(f.root) / "domains" / "item_ownership"));
	const auto refused = coin_owner::apply(f.root, cmd);
	assert(refused.outcome == outcome::retryable_failure && refused.error_code == ENOENT);
	assert(state(f.root).domains.wallet[0] == 100 && second_state(f).domains.wallet[0] == 100);
}
void forged_coin(const coin_fixture &seed, const critical_command &cmd, const fs::path &base)
{
	auto source = seed;
	source.root = (base / "valid").string();
	fs::create_directories(base);
	fs::copy(seed.root, source.root, fs::copy_options::recursive);
	assert(coin_owner::apply(source.root, cmd).outcome == outcome::applied);
	const auto rejection_command = coin_command(source, 3, 1);
	const auto rejection = coin_owner::apply(source.root, rejection_command);
	assert(rejection.outcome == outcome::terminal_failure && rejection.error_code == ESTALE);
	flatfile_accounting_record original, rejected;
	{
		flatfile_authority_lock lock;
		assert(lock.acquire(source.root, nullptr));
		assert(flatfile_accounting_lookup(source.root, lock, rejection_command, &rejected,
						  nullptr) == flatfile_accounting_status::ok);
		assert(flatfile_accounting_lookup(source.root, lock, cmd, &original, nullptr) ==
		       flatfile_accounting_status::ok);
	}
	for (unsigned int variant = 0; variant < 3; ++variant)
	{
		auto f = seed;
		f.root = (base / std::to_string(variant)).string();
		fs::copy(seed.root, f.root, fs::copy_options::recursive);
		auto record = original;
		if (variant == 0)
		{
			record = rejected;
			record.failure_stage = critical_failure_stage::none;
		}
		else if (variant == 1)
			++record.durable_revision;
		else
		{
			coin_transfer_payload payload;
			coin_transfer_result result;
			assert(coin_transfer_command_decode_payload(cmd, &payload));
			assert(coin_transfer_command_decode_result(payload, record.result.data(),
								   record.result.size(), &result));
			++result.wallets[0].wallet.amount[0];
			std::array<uint8_t, COIN_TRANSFER_RESULT_BYTES> encoded;
			assert(coin_transfer_command_encode_result(payload, result, &encoded));
			record.result.assign(encoded.begin(), encoded.end());
		}
		{
			flatfile_authority_lock lock;
			assert(lock.acquire(f.root, nullptr));
			ops changes;
			assert(access_type::stage(f.root, lock, record, &changes, nullptr) ==
			       flatfile_accounting_status::ok);
			commit(f.root, lock, changes);
		}
		const auto refused = coin_owner::apply(f.root, record.command);
		assert(refused.outcome == outcome::retryable_failure &&
		       refused.error_code == EILSEQ);
		assert(state(f.root).domains.wallet[0] == 100 &&
		       second_state(f).domains.wallet[0] == 100);
	}
}
int main(int argc, char **argv)
{
	assert(argc == 2);
	const fs::path base(argv[1]);
	for (bool shared : { false, true })
		for (unsigned int mode = 0; mode < 4; ++mode)
		{
			const auto f = setup_coin(
				base / (std::to_string(shared) + "-" + std::to_string(mode)),
				shared, mode == 3);
			const auto cmd = coin_command(f, 1, mode);
			const auto before = state(f.root), second_before = second_state(f);
			parity(f, cmd);
			const auto result = coin_owner::apply(f.root, cmd);
			const unsigned int expected = mode == 1 ? ESTALE :
						      mode == 2 ? ENOSPC :
						      mode == 3 ? ERANGE :
								  0;
			if (result.error_code != expected)
				std::cerr << "coin apply mode " << mode << " error "
					  << result.error_code << " outcome " << int(result.outcome)
					  << "\n";
			assert(result.error_code == expected &&
			       result.outcome ==
				       (mode ? outcome::terminal_failure : outcome::applied));
			assert(result.result_size == COIN_TRANSFER_RESULT_BYTES);
			assert(result.failure_stage ==
			       (mode == 1 ? critical_failure_stage::coin_source_wallet_revision :
					    critical_failure_stage::none));
			const auto replay = coin_owner::apply(f.root, cmd);
			assert(replay.outcome ==
			       (mode ? outcome::terminal_failure : outcome::already_applied));
			same_coin(result, replay);
			const auto after = state(f.root), second_after = second_state(f);
			assert(after.domains.wallet[0] ==
			       before.domains.wallet[0] - (mode ? 0 : 10));
			assert(second_after.domains.wallet[0] ==
			       second_before.domains.wallet[0] + (mode ? 0 : 10));
			assert(after.domains.epics == before.domains.epics &&
			       after.domains.base_stats == before.domains.base_stats &&
			       after.recent_pvp_deaths == before.recent_pvp_deaths);
			assert(after.domains.bank_revision ==
			       before.domains.bank_revision + (mode   ? 0 :
							       shared ? 2 :
									1));
			{
				flatfile_authority_lock lock;
				assert(lock.acquire(f.root, nullptr));
				ops changes;
				assert(access_type::select_epoch(
					       f.root, lock, control(f.root, lock).revision, false,
					       id(93000), &changes, nullptr) == 0);
				commit(f.root, lock, changes);
			}
			same_coin(result, coin_owner::apply(f.root, cmd));
		}
	legacy_fences(base / "legacy");
	const auto seed = setup_coin(base / "seed", true);
	const auto cmd = coin_command(seed, 2);
	forged_coin(seed, cmd, base / "forged");
	coin_transfer_payload payload;
	assert(coin_transfer_command_decode_payload(cmd, &payload));
	const std::set<size_t> buckets{ cmd.operation_id.bytes[0],
					payload.source.change.operation_id.bytes[0],
					payload.destination.change.operation_id.bytes[0] };
	const size_t images = 3 + 2 * buckets.size();
	for (size_t boundary = 0; boundary <= images; ++boundary)
	{
		auto f = seed;
		f.root = (base / ("crash-" + std::to_string(boundary))).string();
		fs::copy(seed.root, f.root, fs::copy_options::recursive);
		const auto child = fork();
		assert(child >= 0);
		if (!child)
		{
			if (!boundary)
				setenv("DURIS_FLATFILE_TEST_INTERRUPT_AFTER_AUTHORITY_JOURNAL", "1",
				       1);
			else
				setenv("DURIS_FLATFILE_TEST_INTERRUPT_AFTER_AUTHORITY_OPERATION",
				       std::to_string(boundary).c_str(), 1);
			const auto result = coin_owner::apply(f.root, cmd);
			assert(result.outcome == outcome::ambiguous_commit);
			_exit(77);
		}
		int status;
		assert(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
		       WEXITSTATUS(status) == 77);
		const auto recovered = coin_owner::apply(f.root, cmd);
		assert(recovered.outcome == outcome::already_applied);
		same_coin(recovered, coin_owner::apply(f.root, cmd));
		assert(state(f.root).domains.wallet[0] == 90 &&
		       second_state(f).domains.wallet[0] == 110);
		assert(state(f.root).domains.bank_revision ==
		       state(seed.root).domains.bank_revision + 2);
	}
	std::cout
		<< "flatfile coin owner: shared/separate bank, rejection, retained epoch replay and every publication boundary passed\n";
}
