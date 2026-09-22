#include "flatfile_accounting_wallet_fixture.h"
#include "flatfile/flatfile_accounting_dispatch.h"
#include "economy/economic_command_admission.h"
#include "item/item_ownership_runtime.h"
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
// Unused item/pet surfaces are sentinels: status capture must not call them.
P_index obj_index = nullptr, mob_index = nullptr;
int top_of_objt = -1, top_of_mobt = -1, top_of_world = -1;
Skill skills[1];
bool has_innate(P_char, int)
{
	return false;
} // The synthetic actor has no innate abilities.
P_char get_linked_char(P_char, ush_int)
{
	assert(false);
	return nullptr;
}
bool summoned_pet_capture(P_char, std::string *)
{
	assert(false);
	return false;
}
bool item_ownership_runtime_lookup(uint64_t, item_ownership_runtime_entry *)
{
	assert(false);
	return false;
}
bool item_ownership_runtime_snapshot_owner(const item_owner_identity &, size_t,
					   std::vector<item_ownership_runtime_entry> *)
{
	assert(false);
	return false;
}
P_room world = nullptr;
P_char online = nullptr;
unsigned publications = 0;
[[noreturn]] int panic_corruption_int(const char *, const char *, ...)
{
	abort();
}
const char *get_account_name_safe(P_char)
{
	return "account-one";
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
	out.pid = 1;
	out.revision = revision;
	out.components = components;
	out.save_intent = 3;
	out.room_vnum = NOWHERE;
	out.encoded_size_bound = 8192;
	out.status_integers = { { player_status_field::level, 50, 0, false },
				{ player_status_field::racewar, 1, 0, false },
				{ player_status_field::copper, 100, 0, false },
				{ player_status_field::silver, 20, 0, false },
				{ player_status_field::gold, 3, 0, false },
				{ player_status_field::platinum, 1, 0, false },
				{ player_status_field::epics, 7, 0, false },
				{ player_status_field::frags, 9, 0, false },
				{ player_status_field::old_frags, 0, 0, false } };
	for (int i = 0; i < 10; ++i)
		out.status_integers.push_back(
			{ static_cast<player_status_field>(
				  static_cast<unsigned>(player_status_field::base_strength) + i),
			  20 + i, 0, false });
	out.status_strings.push_back({ player_status_string_field::name, "Player" });
	out.recipes_are_external = true;
	return out;
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

extern "C" critical_apply_result unexpected_legacy(const critical_command &, void *) asm(
	"__wrap__Z51flatfile_critical_command_repository_apply_selectedRK16critical_commandPv");
extern "C" critical_apply_result unexpected_legacy(const critical_command &, void *)
{
	assert(false && "accounting operation must not enter legacy dispatch");
	return { critical_apply_outcome::terminal_failure, 0, EINVAL };
}
critical_command restored_command;
bool saw_restored_command = false;
bool observe_restore(const critical_command &cmd, void *context)
{
	assert(!saw_restored_command || critical_command_equal(restored_command, cmd));
	restored_command = cmd;
	saw_restored_command = true;
	return economic_bank_publication_restore(cmd, context);
}
void initialize_coordinator(const std::string &journal)
{
	assert(critical_command_coordinator_init(
		journal.c_str(), flatfile_accounting_apply_selected, native_root.data(), 1,
		observe_restore, nullptr, economic_command_admission_supported));
}
critical_completion await_completion()
{
	critical_completion completion{};
	wait_for([&] { return critical_command_coordinator_pulse(&completion, 1) == 1; });
	return completion;
}
void retained(const critical_command &cmd)
{
	assert(economic_bank_publication_pending() == 1);
	assert(critical_command_journal_health_copy().checkpoints == 0);
	assert(critical_command_journal_health_copy().records == 1);
	for (const auto &key : cmd.keys)
		assert(critical_command_coordinator_is_fenced(key, nullptr));
}
int main(int argc, char **argv)
{
	assert(argc == 3);
	const std::string mode = argv[2];
	const std::string base = argv[1];
	native_root = base + "/authority";
	const bool recovering = mode == "recover-commit" || mode == "recover-save" ||
				mode == "recover-save-ack";
	std::string error;
	if (!recovering)
	{
		setup(native_root, false);
		fs::create_directories(native_root + "/players");
		fs::permissions(native_root + "/players", fs::perms::owner_all);
		const auto baseline = flatfile_player_snapshot_apply(
			native_root, snapshot(10, PLAYER_CHECKPOINT_COMPONENT_ALL), &error);
		if (baseline.outcome != player_save_apply_outcome::applied)
			std::cerr << error << '\n';
		assert(baseline.outcome == player_save_apply_outcome::applied);
	}
	pc_only_data player{};
	player.pid = 1;
	player.wallet_revision = 1;
	char_data actor{};
	char name[] = "Player";
	actor.player.name = name;
	player.epics = 7;
	player.frags = 9;
	actor.base_stats.Str = 20;
	actor.base_stats.Dex = 21;
	actor.base_stats.Agi = 22;
	actor.base_stats.Con = 23;
	actor.base_stats.Pow = 24;
	actor.base_stats.Int = 25;
	actor.base_stats.Wis = 26;
	actor.base_stats.Cha = 27;
	actor.base_stats.Kar = 28;
	actor.base_stats.Luk = 29;
	actor.only.pc = &player;
	actor.player.racewar = 1;
	actor.in_room = NOWHERE;
	GET_COPPER(&actor) = 100;
	GET_SILVER(&actor) = 20;
	GET_GOLD(&actor) = 3;
	GET_PLATINUM(&actor) = 1;
	online = character_list = &actor;
	const std::string journal = base + "/critical";
	const std::string saves_journal = base + "/saves";
	critical_command cmd;
	critical_completion completed{};
	if (recovering)
	{
		// A new process gets its command only from the durable coordinator journal.
		assert(player_save_pipeline_init(saves_journal.c_str()));
		wait_for([] { return player_save_pipeline_health_copy().replay_complete; });
		player_snapshot durable{};
		assert(flatfile_player_snapshot_load(native_root, 1, &durable, &error) ==
		       flatfile_player_load_result::ok);
		assert(durable.revision == (mode == "recover-commit" ? 10U : 11U));
		assert(player_revision_hydrate(1, durable.revision));
		initialize_coordinator(journal);
		completed = await_completion();
		assert(completed.outcome == critical_apply_outcome::already_applied);
		assert(saw_restored_command);
		cmd = restored_command;
		assert(critical_operation_id_equal(cmd.operation_id, id(100)));
	}
	else
	{
		assert(player_revision_hydrate(1, 10));
		cmd = command(native_root, 100);
		initialize_coordinator(journal);
		assert(economic_bank_publication_submit(cmd) ==
		       critical_submit_result::awaiting_durability);
		completed = await_completion();
		assert(completed.outcome == critical_apply_outcome::applied);
	}
	const auto committed = state(native_root);
	assert(committed.domains.wallet[0] == 101 && committed.domains.bank[0] == 99);
	assert(GET_COPPER(&actor) == 100);
	retained(cmd);
	if (mode == "crash-commit")
		std::_Exit(73); // No destructors or pipeline shutdown.
	if (mode == "committed-replay")
	{
		critical_command_coordinator_shutdown();
		economic_bank_publication_reset();
		initialize_coordinator(journal);
		const auto replay = await_completion();
		assert(replay.outcome == critical_apply_outcome::already_applied);
		assert(replay.result_payload == completed.result_payload &&
		       replay.result_size == completed.result_size);
		retained(cmd);
	}
	if (!recovering)
		assert(player_save_pipeline_init(saves_journal.c_str()));
	economic_bank_publication_pulse();
	assert(GET_COPPER(&actor) == 101);
	wait_for([] { return player_save_journal_health_copy().appended == 1; });
	retained(cmd);
	if (mode == "crash-save")
		std::_Exit(74); // Journal durable, native worker not submitted.
	if (mode == "crash-save-ack")
	{
		wait_for(
			[]
			{
				player_save_pipeline_pulse();
				player_revision_snapshot revision{};
				return player_revision_snapshot_copy(1, &revision) &&
				       revision.acknowledged_revision == 11 &&
				       !(revision.unacknowledged_components &
					 PLAYER_COMPONENT_STATUS);
			});
		retained(cmd);
		std::_Exit(
			75); // Native save complete; owner has not acknowledged the original command.
	}
	player_snapshot loaded{};
	if (mode == "save-replay")
	{
		player_save_pipeline_shutdown();
		critical_command_coordinator_shutdown();
		economic_bank_publication_reset();
		player_revision_reset_for_tests();
		assert(player_save_pipeline_init(saves_journal.c_str()));
		wait_for([] { return player_save_pipeline_health_copy().replay_complete; });
		assert(flatfile_player_snapshot_load(native_root, 1, &loaded, &error) ==
		       flatfile_player_load_result::ok);
		assert(loaded.revision == 11);
		assert(player_revision_hydrate(1, loaded.revision));
		initialize_coordinator(journal);
		assert(await_completion().outcome == critical_apply_outcome::already_applied);
		retained(cmd);
		economic_bank_publication_pulse();
		retained(cmd);
	}
	{
		flatfile_player_snapshot_lock held;
		assert(held.acquire(native_root, 1, &error));
		wait_for(
			[&]
			{
				player_save_pipeline_pulse();
				economic_bank_publication_pulse();
				retained(cmd);
				return player_save_worker_health_copy().inflight_pids == 1;
			});
	}
	wait_for(
		[]
		{
			player_save_pipeline_pulse();
			economic_bank_publication_pulse();
			return economic_bank_publication_pending() == 0;
		});
	assert(critical_command_journal_health_copy().checkpoints == 1);
	assert(critical_command_journal_health_copy().records == 0);
	for (const auto &key : cmd.keys)
		assert(!critical_command_coordinator_is_fenced(key, nullptr));
	assert(flatfile_player_snapshot_load(native_root, 1, &loaded, &error) ==
	       flatfile_player_load_result::ok);
	assert(loaded.revision ==
	       ((mode == "save-replay" || mode == "recover-save" || mode == "recover-save-ack") ?
			12U :
			11U));
	bool copper = false;
	for (const auto &row : loaded.status_integers)
		if (row.field == player_status_field::copper)
		{
			assert(row.signed_value == 101);
			copper = true;
		}
	assert(copper);
	const auto final = state(native_root);
	assert(final.domains.wallet == committed.domains.wallet &&
	       final.domains.bank == committed.domains.bank);
	assert(final.domains.wallet_revision == committed.domains.wallet_revision &&
	       final.domains.bank_revision == committed.domains.bank_revision);
	player_save_pipeline_shutdown();
	critical_command_coordinator_shutdown();
	economic_bank_publication_reset();
}
