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
bool available = false, ack_allowed = false, fail_capture = true;
unsigned saves = 0, retries = 0, acknowledgements = 0, publications = 0;
player_revision_t queued_revision = 0;
player_component_mask_t queued_components = 0;
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
player_save_pipeline_result
player_save_pipeline_request(P_char actor, player_component_mask_t components, int, int)
{
	++saves;
	if (fail_capture)
		return player_save_pipeline_result::unavailable;
	assert(components == PLAYER_COMPONENT_STATUS && GET_COPPER(actor) == 12);
	assert(player_revision_mark(GET_PID(actor), components, nullptr));
	assert(player_revision_queue(GET_PID(actor), &queued_revision, &queued_components));
	assert(player_revision_begin_inflight(GET_PID(actor), queued_revision, queued_components));
	return player_save_pipeline_result::queued;
}
player_save_pipeline_result player_save_pipeline_checkpoint_dirty(P_char, int, int)
{
	++retries;
	return player_save_pipeline_result::unchanged;
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
	payload.racewar = 1;
	payload.reason = currency_reason_type::atm_withdraw;
	strcpy(payload.account_name.data(), "fixture");
	payload.wallet_delta.amount[0] = 7;
	payload.bank_delta.amount[0] = -7;
	critical_command command;
	assert(currency_command_build(&command, id(3), payload, 4, 9, critical_source_site::command,
				      critical_deadline_class::interactive));
	assert(economic_bank_transfer_intent(
		       command, id(2), { id(1), economic_account_kind::wallet, 1001, 0 },
		       { id(1), economic_account_kind::bank, 2001, 1 },
		       &command.accounting_intent) == economic_accounting_error::ok);
	command.schema_version = 2;
	command.accepted_at_usec = 123;
	return command;
}
int main()
{
	auto command = bank();
	pc_only_data player{};
	player.pid = 7;
	player.wallet_revision = 4;
	char_data actor{};
	actor.only.pc = &player;
	actor.player.racewar = 1;
	actor.in_room = NOWHERE;
	GET_COPPER(&actor) = 5;
	assert(player_revision_hydrate(7, 10));
	assert(economic_bank_publication_submit(command) ==
	       critical_submit_result::awaiting_durability);
	economic_bank_publication_pulse();
	assert(!saves && economic_bank_publication_pending() == 1);
	// Rebuild only from immutable command bytes after a simulated process restart.
	economic_bank_publication_reset();
	assert(economic_bank_publication_restore(command, nullptr));
	assert(economic_bank_publication_restore(command, nullptr));
	auto conflict = command;
	conflict.accepted_at_usec++;
	assert(!economic_bank_publication_restore(conflict, nullptr));
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
	available = true;
	economic_bank_publication_pulse();
	assert(!saves && !acknowledgements); // Offline obligations stay retained.
	online = &actor;
	economic_bank_publication_pulse();
	assert(saves == 1 && !acknowledgements && economic_bank_publication_pending() == 1);
	fail_capture = false;
	economic_bank_publication_pulse();
	assert(saves == 2 && publications == 1 && !acknowledgements && GET_COPPER(&actor) == 12);
	// An unrelated component acknowledgement must not release this status save.
	assert(player_revision_acknowledge_durable(7, queued_revision, PLAYER_COMPONENT_LANGUAGES));
	economic_bank_publication_pulse();
	assert(!acknowledgements && retries == 1);
	assert(player_revision_acknowledge(7, queued_revision, queued_components));
	economic_bank_publication_pulse();
	assert(acknowledgements == 1 && economic_bank_publication_pending() == 1);
	ack_allowed = true;
	economic_bank_publication_pulse();
	assert(acknowledgements == 2 && !economic_bank_publication_pending() && saves == 2 &&
	       publications == 1);
	// Simulate revision-state replacement between pulses without exposing a missing PID.
	// The wallet domain is already current, but the status save was never acknowledged.
	assert(economic_bank_publication_restore(command, nullptr));
	economic_bank_publication_pulse();
	assert(saves == 3 && queued_revision == 12);
	player_revision_forget(7);
	assert(player_revision_hydrate(7, 10));
	economic_bank_publication_pulse();
	assert(acknowledgements == 2 && economic_bank_publication_pending() == 1);
	economic_bank_publication_pulse();
	assert(saves == 4 && queued_revision == 11 && acknowledgements == 2 &&
	       economic_bank_publication_pending() == 1 && publications == 2);
	assert(player_revision_acknowledge(7, queued_revision, queued_components));
	economic_bank_publication_pulse();
	assert(acknowledgements == 3 && !economic_bank_publication_pending());
	// A retained business rejection has no publication/save effect to repeat.
	assert(economic_bank_publication_restore(command, nullptr));
	receipt.outcome = critical_apply_outcome::terminal_failure;
	receipt.error_code = ESTALE;
	economic_bank_publication_pulse();
	assert(!economic_bank_publication_pending() && saves == 4 && publications == 2);
	assert(economic_bank_publication_restore(command, nullptr));
	receipt.outcome = critical_apply_outcome::ambiguous_commit;
	economic_bank_publication_pulse();
	assert(economic_bank_publication_pending() == 1 && saves == 4);
	economic_bank_publication_reset();
}
