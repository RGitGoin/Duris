#include "economy/economic_bank_publication.h"
#include "economy/economic_currency_adapter.h"
#include "economy/currency_transaction.h"
#include "player/player_save_pipeline.h"
#include "sql/sql_player.h"
#include "core/utils.h"
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cerrno>
P_room world = nullptr;
P_char online = nullptr;
critical_completion receipt{};
bool available = false, ack_allowed = false;
unsigned saves = 0, acknowledgements = 0, publications = 0;
[[noreturn]] int panic_corruption_int(const char *, const char *, ...)
{
	abort();
}
const char *get_account_name_safe(P_char)
{
	return "fixture";
}
P_char find_player_by_pid(int pid)
{
	return online && GET_PID(online) == pid ? online : nullptr;
}
void gmcp_char_vitals(P_char)
{
	++publications;
}
void publish_account_bank_balances_revision(const char *, int, const AccountBankBalances *,
					    uint64_t)
{
}
critical_submit_result critical_command_coordinator_submit_for_publication(critical_command)
{
	return critical_submit_result::awaiting_durability;
}
bool critical_command_coordinator_get_completed(const critical_operation_id &id,
						critical_completion *out)
{
	if (!available || !critical_operation_id_equal(id, receipt.operation_id))
		return false;
	*out = receipt;
	return true;
}
bool critical_command_coordinator_acknowledge_publication(const critical_operation_id &id)
{
	assert(critical_operation_id_equal(id, receipt.operation_id));
	++acknowledgements;
	return ack_allowed;
}
critical_operation_id id(uint8_t value)
{
	critical_operation_id result{};
	result.bytes[0] = value;
	return result;
}
critical_command bank()
{
	currency_command_payload payload{};
	payload.pid = 7;
	payload.racewar = 0;
	payload.reason = currency_reason_type::atm_withdraw;
	strcpy(payload.account_name.data(), "fixture");
	payload.wallet_delta.amount[0] = 7;
	payload.bank_delta.amount[0] = -7;
	critical_command command;
	assert(currency_command_build(&command, id(3), payload, 4, 9, critical_source_site::command,
				      critical_deadline_class::interactive));
	assert(economic_bank_transfer_intent(
		       command, id(2), { id(1), economic_account_kind::wallet, 1001, 0 },
		       { id(1), economic_account_kind::bank, 2001, 0 },
		       &command.accounting_intent) == economic_accounting_error::ok);
	command.schema_version = 2;
	command.accepted_at_usec = 123;
	return command;
}

#include "flatfile/flatfile_player_repository.h"
#include "flatfile/flatfile_identity_repository.h"
#include "player/player_snapshot_capture.h"
#include "player/player_save_journal.h"
#include <filesystem>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
P_char character_list = nullptr;
std::string native_root;
const char *persistence_mode_flatfile_root()
{
	return native_root.c_str();
}
void logit(const char *, const char *, ...) {}
void persistence_alert(int, const char *, const char *, const char *, const char *, const char *,
		       const char *, ...)
{
}
player_snapshot snapshot(player_revision_t revision, player_component_mask_t components)
{
	player_snapshot out{};
	out.schema_version = PLAYER_SNAPSHOT_SCHEMA_VERSION;
	out.pid = 7;
	out.revision = revision;
	out.components = components;
	out.save_intent = 3;
	out.room_vnum = NOWHERE;
	out.encoded_size_bound = 8192;
	out.status_integers = { { player_status_field::level, 50, 0, false },
				{ player_status_field::racewar, 0, 0, false },
				{ player_status_field::copper, 12, 0, false },
				{ player_status_field::silver, 0, 0, false },
				{ player_status_field::gold, 0, 0, false },
				{ player_status_field::platinum, 0, 0, false },
				{ player_status_field::epics, 0, 0, false },
				{ player_status_field::frags, 0, 0, false },
				{ player_status_field::old_frags, 0, 0, false } };
	for (int i = 0; i < 10; ++i)
		out.status_integers.push_back(
			{ static_cast<player_status_field>(
				  static_cast<unsigned>(player_status_field::base_strength) + i),
			  50, 0, false });
	out.status_strings.push_back({ player_status_string_field::name, "Player" });
	out.recipes_are_external = true;
	return out;
}
player_snapshot_capture_result player_snapshot_capture(P_char actor, player_revision_t revision,
						       player_component_mask_t components, int, int,
						       player_snapshot *out)
{
	++saves;
	assert(GET_COPPER(actor) == 12);
	*out = snapshot(revision, components);
	return player_snapshot_capture_result::ok;
}
template <class Predicate> void wait_for(Predicate predicate)
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
	while (!predicate())
	{
		assert(std::chrono::steady_clock::now() < deadline);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
int main(int argc, char **argv)
{
	assert(argc == 3);
	const bool replay = std::string(argv[2]) == "replay";
	native_root = std::string(argv[1]) + "/state";
	for (const auto &name : { "", "/players", "/identities", "/identities/names", "/domains" })
	{
		auto path = native_root + name;
		std::filesystem::create_directories(path);
		std::filesystem::permissions(path, std::filesystem::perms::owner_all);
	}
	std::string error;
	int32_t pid = 0;
	for (int i = 1; i <= 7; ++i)
		assert(flatfile_identity_allocate_pid(native_root, &pid, &error) ==
		       flatfile_identity_result::ok);
	assert(flatfile_identity_claim(native_root, 7, "Player", "fixture", &error) ==
	       flatfile_identity_result::ok);
	auto baseline = flatfile_player_snapshot_apply(
		native_root, snapshot(10, PLAYER_CHECKPOINT_COMPONENT_ALL), &error);
	if (baseline.outcome != player_save_apply_outcome::applied)
		std::cerr << error << '\n';
	assert(baseline.outcome == player_save_apply_outcome::applied);
	pc_only_data player{};
	player.pid = 7;
	player.wallet_revision = 4;
	char_data actor{};
	actor.only.pc = &player;
	actor.player.racewar = 0;
	actor.in_room = NOWHERE;
	GET_COPPER(&actor) = 5;
	online = character_list = &actor;
	assert(player_revision_hydrate(7, 10));
	auto command = bank();
	currency_command_result result{};
	result.wallet.amount[0] = 12;
	result.wallet_revision = 5;
	result.bank_revision = 10;
	std::array<uint8_t, CURRENCY_RESULT_PAYLOAD_BYTES> encoded;
	assert(currency_command_encode_result(result, &encoded));
	receipt.operation_id = command.operation_id;
	receipt.outcome = critical_apply_outcome::already_applied;
	receipt.result_size = encoded.size();
	std::copy(encoded.begin(), encoded.end(), receipt.result_payload.begin());
	available = ack_allowed = true;
	const std::string journal = std::string(argv[1]) + "/journal";
	assert(player_save_pipeline_init(journal.c_str()));
	assert(economic_bank_publication_restore(command, nullptr));
	economic_bank_publication_pulse();
	assert(saves == 1 && !acknowledgements);
	// Let the native journal append finish without submitting it to the save worker.
	wait_for([] { return player_save_journal_health_copy().appended == 1; });
	economic_bank_publication_pulse();
	assert(!acknowledgements && economic_bank_publication_pending() == 1);
	player_snapshot loaded{};
	assert(flatfile_player_snapshot_load(native_root, 7, &loaded, &error) ==
	       flatfile_player_load_result::ok);
	assert(loaded.revision == 10);
	if (replay)
	{
		// Tear down runtime owners while the native save journal still owns revision 11.
		player_save_pipeline_shutdown();
		economic_bank_publication_reset();
		player_revision_reset_for_tests();
		assert(player_save_pipeline_init(journal.c_str()));
		wait_for([] { return player_save_pipeline_health_copy().replay_complete; });
		assert(flatfile_player_snapshot_load(native_root, 7, &loaded, &error) ==
		       flatfile_player_load_result::ok);
		assert(loaded.revision == 11 && !acknowledgements);
		assert(player_save_journal_health_copy().replayed == 1);
		assert(player_revision_hydrate(7, loaded.revision));
		assert(economic_bank_publication_restore(command, nullptr));
		economic_bank_publication_pulse();
		assert(saves == 2 && !acknowledgements);
	}
	if (std::string(argv[2]) == "failure")
	{
		const std::string path = native_root + "/players/7.snapshot";
		std::ifstream input(path, std::ios::binary);
		const std::string bytes((std::istreambuf_iterator<char>(input)), {});
		assert(!bytes.empty());
		input.close();
		{
			std::ofstream corrupt(path, std::ios::binary | std::ios::trunc);
			corrupt << "invalid";
		}
		wait_for(
			[]
			{
				player_save_pipeline_pulse();
				return player_save_worker_health_copy().terminal_failures == 1;
			});
		economic_bank_publication_pulse();
		assert(!acknowledgements && economic_bank_publication_pending() == 1);
		assert(player_save_journal_health_copy().records > 0);
		{
			flatfile_player_snapshot_lock held;
			assert(held.acquire(native_root, 7, &error));
			std::ofstream repaired(path, std::ios::binary | std::ios::trunc);
			repaired.write(bytes.data(), bytes.size());
		}
	}
	{
		// Hold the repository's real lock while the real worker attempts the save.
		flatfile_player_snapshot_lock held;
		assert(held.acquire(native_root, 7, &error));
		wait_for(
			[]
			{
				player_save_pipeline_pulse();
				economic_bank_publication_pulse();
				return player_save_worker_health_copy().inflight_pids == 1;
			});
		for (int i = 0; i < 32; ++i)
		{
			player_save_pipeline_pulse();
			economic_bank_publication_pulse();
			assert(!acknowledgements && economic_bank_publication_pending() == 1);
		}
	}
	wait_for(
		[]
		{
			player_save_pipeline_pulse();
			economic_bank_publication_pulse();
			return economic_bank_publication_pending() == 0;
		});
	assert(acknowledgements == 1 && saves >= (replay ? 2U : 1U));
	assert(flatfile_player_snapshot_load(native_root, 7, &loaded, &error) ==
	       flatfile_player_load_result::ok);
	assert(loaded.revision == (replay ? 12U : 11U));
	bool copper = false;
	for (const auto &row : loaded.status_integers)
		if (row.field == player_status_field::copper)
		{
			assert(row.signed_value == 12);
			copper = true;
		}
	assert(copper);
	player_save_pipeline_shutdown();
	economic_bank_publication_reset();
}
