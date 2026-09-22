#include "economy/economic_bank_publication.h"
#include "economy/economic_command_admission.h"
#include "economy/currency_transaction.h"
#include "player/player_save_pipeline.h"
#include "core/prototypes.h"
#include "core/files.h"
#include "core/utils.h"
#include "world/db.h"
#include <map>
#include <new>
#include <string>
#include <strings.h>
namespace
{
struct obligation
{
	critical_command command;
	currency_command_payload payload;
	player_revision_t save_revision = 0;
	bool published = false;
};
std::map<std::string, obligation> pending;
std::string cursor;
std::string key(const critical_operation_id &id)
{
	return { reinterpret_cast<const char *>(id.bytes.data()), id.bytes.size() };
}
bool valid(const critical_command &command)
{
	return command.type == critical_command_type::account_bank &&
	       economic_command_admission_supported(command);
}
bool same(const critical_command &left, const critical_command &right)
{
	auto expected = left;
	if (!expected.accepted_at_usec)
		expected.accepted_at_usec = right.accepted_at_usec;
	return critical_command_equal(expected, right);
}
bool advance(obligation &entry)
{
	critical_completion completed{};
	if (!critical_command_coordinator_get_completed(entry.command.operation_id, &completed) ||
	    !critical_operation_id_equal(completed.operation_id, entry.command.operation_id))
		return false;
	if (completed.outcome == critical_apply_outcome::terminal_failure)
		return critical_command_coordinator_acknowledge_publication(
			entry.command.operation_id);
	if ((completed.outcome != critical_apply_outcome::applied &&
	     completed.outcome != critical_apply_outcome::already_applied) ||
	    completed.error_code)
		return false;
	currency_command_result result;
	if (!currency_command_decode_result(completed.result_payload.data(), completed.result_size,
					    &result))
		return false;
	P_char character = find_player_by_pid(entry.payload.pid);
	if (!character || IS_NPC(character) || !character->only.pc ||
	    GET_PID(character) != static_cast<int>(entry.payload.pid) ||
	    GET_RACEWAR(character) != entry.payload.racewar)
		return false;
	const char *account = get_account_name_safe(character);
	if (!account || strcasecmp(account, entry.payload.account_name.data()))
		return false;
	if (entry.published && character->only.pc->wallet_revision < result.wallet_revision)
	{
		entry.published = false;
		entry.save_revision = 0;
	}
	if (!entry.published)
	{
		if (!currency_transaction_publish_balances(
			    character, account, entry.payload.racewar, result.wallet, result.bank,
			    result.wallet_revision, result.bank_revision))
			return false;
		entry.published = true;
	}
	const int room = character->in_room == NOWHERE ? NOWHERE : world[character->in_room].number;
	if (!entry.save_revision)
	{
		const auto queued = player_save_pipeline_request(character, PLAYER_COMPONENT_STATUS,
								 RENT_CRASH, room);
		if (queued != player_save_pipeline_result::queued &&
		    queued != player_save_pipeline_result::coalesced)
			return false;
		player_revision_snapshot revision{};
		if (!player_revision_snapshot_copy(entry.payload.pid, &revision) ||
		    revision.overflowed)
			return false;
		entry.save_revision = revision.current_revision;
		return false; // A queued or journaled snapshot is not a native save acknowledgement.
	}
	player_revision_snapshot revision{};
	if (!player_revision_snapshot_copy(entry.payload.pid, &revision))
	{
		// Disconnection can evict revision state; publish/capture afresh on re-entry.
		entry.published = false;
		entry.save_revision = 0;
		return false;
	}
	if (revision.current_revision < entry.save_revision)
	{
		// Revision state may be reset and rehydrated entirely between pulses.
		// Its old unsaved generation cannot complete; capture the current wallet again.
		entry.save_revision = 0;
		return false;
	}
	if (revision.acknowledged_revision >= entry.save_revision &&
	    !(revision.unacknowledged_components & PLAYER_COMPONENT_STATUS))
		return critical_command_coordinator_acknowledge_publication(
			entry.command.operation_id);
	// Retain the same save generation when capture/worker backpressure clears.
	(void)player_save_pipeline_checkpoint_dirty(character, RENT_CRASH, room);
	return false;
}
}
bool economic_bank_publication_restore(const critical_command &command, void *)
{
	if (command.schema_version != CRITICAL_COMMAND_ACCOUNTING_SCHEMA_VERSION ||
	    command.type != critical_command_type::account_bank)
		return true;
	try
	{
		if (!valid(command))
			return false;
		const auto identity = key(command.operation_id);
		const auto found = pending.find(identity);
		if (found != pending.end())
			return same(found->second.command, command);
		if (pending.size() >= CURRENCY_PENDING_MAX)
			return false;
		currency_command_payload payload;
		if (!currency_command_decode_payload(command, &payload))
			return false;
		pending.emplace(identity, obligation{ command, payload });
		return true;
	}
	catch (const std::bad_alloc &)
	{
		return false;
	}
}
critical_submit_result economic_bank_publication_submit(const critical_command &command)
{
	try
	{
		if (!valid(command))
			return critical_submit_result::invalid;
		const auto identity = key(command.operation_id);
		const bool existed = pending.find(identity) != pending.end();
		if (!economic_bank_publication_restore(command, nullptr))
			return critical_submit_result::unavailable;
		critical_submit_result submitted;
		try
		{
			submitted = critical_command_coordinator_submit_for_publication(command);
		}
		catch (const std::bad_alloc &)
		{
			if (!existed)
				pending.erase(identity);
			return critical_submit_result::unavailable;
		}
		if (!existed && !critical_submit_result_keeps_operation(submitted))
			pending.erase(identity);
		return submitted;
	}
	catch (const std::bad_alloc &)
	{
		return critical_submit_result::unavailable;
	}
}
void economic_bank_publication_pulse()
{
	try
	{
		// Fair bounded work even when the first records belong to offline players.
		const size_t budget = std::min<size_t>(32, pending.size());
		for (size_t i = 0; i < budget && !pending.empty(); ++i)
		{
			auto at = pending.upper_bound(cursor);
			if (at == pending.end())
				at = pending.begin();
			cursor = at->first;
			if (advance(at->second))
				pending.erase(at);
		}
	}
	catch (const std::bad_alloc &)
	{ /* Keep every obligation and fence for retry. */
	}
}
size_t economic_bank_publication_pending()
{
	return pending.size();
}
void economic_bank_publication_reset()
{
	pending.clear();
	cursor.clear();
}
