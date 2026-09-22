#include "persistence/economic_sql_bank_transaction.h"
#include "persistence/economic_accounting_repository.h"
#include "economy/currency_sql_mutation_writer.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <charconv>
#include <cstring>
#include <limits>
#include <new>
#include <optional>
#include <openssl/sha.h>
#include <string>
#include <strings.h>
#include <type_traits>
#include <utility>
#include <vector>

#include "persistence/economic_sql_wallet_internal.h"

using namespace economic_sql_wallet_detail;

bool economic_sql_bank_command_supported(const critical_command &command)
{
	try
	{
		(void)decode(command);
		return true;
	}
	catch (const failure &)
	{
		return false;
	}
	catch (const std::bad_alloc &)
	{
		return false;
	}
}

#ifdef __NO_MYSQL__
struct economic_sql_bank_transaction::implementation
{
	currency_command_result result = {};
};
economic_sql_bank_transaction::economic_sql_bank_transaction(std::unique_ptr<implementation> state)
	: state_(std::move(state))
{
}
economic_sql_bank_transaction::~economic_sql_bank_transaction() = default;
unsigned int
economic_sql_bank_transaction::prepare(MYSQL *, const critical_command &,
				       std::unique_ptr<economic_sql_bank_transaction> *)
{
	return ENOTSUP;
}
unsigned int economic_sql_bank_transaction::apply()
{
	return ENOTSUP;
}
unsigned int economic_sql_bank_transaction::finalize()
{
	return ENOTSUP;
}
unsigned int economic_sql_bank_transaction::verify_root_completion()
{
	return ENOTSUP;
}
const currency_command_result &economic_sql_bank_transaction::result() const
{
	return state_->result;
}
unsigned int economic_sql_bank_transaction::result_code() const
{
	return ENOTSUP;
}
unsigned int economic_sql_bank_verify_retained(MYSQL *, const critical_command &, unsigned int,
					       std::span<const uint8_t>)
{
	return ENOTSUP;
}
#else

struct economic_sql_bank_transaction::implementation
{
	MYSQL *connection = nullptr;
	unsigned long session = 0;
	critical_command command;
	bank_identity identity;
	economic_sql_authority_snapshot authority;
	std::array<economic_sql_mapping_request, 2> requests;
	uint64_t bank_id = 0;
	std::string checkpoint;
	currency_command_result before = {}, result = {};
	unsigned int code = 0;
	std::optional<economic_prepared_currency> prepared;
	enum class phase
	{
		prepared,
		applied,
		finalized,
		verified,
		failed
	} phase = phase::prepared;
};
economic_sql_bank_transaction::economic_sql_bank_transaction(std::unique_ptr<implementation> state)
	: state_(std::move(state))
{
}
economic_sql_bank_transaction::~economic_sql_bank_transaction() = default;
const currency_command_result &economic_sql_bank_transaction::result() const
{
	return state_->result;
}
unsigned int economic_sql_bank_transaction::result_code() const
{
	return state_->code;
}

unsigned int
economic_sql_bank_transaction::prepare(MYSQL *connection, const critical_command &command,
				       std::unique_ptr<economic_sql_bank_transaction> *transaction)
{
	try
	{
		require(connection && transaction, EINVAL);
		auto state = std::make_unique<implementation>();
		state->connection = connection;
		state->session = mysql_thread_id(connection);
		active(connection, state->session);
		state->identity = decode(command);
		state->command = command;
		inbox(connection, command, true);
		const auto &identity = state->identity;
		state->bank_id = mapping_hint(connection, identity.bank.authority_id);
		require(state->bank_id && state->bank_id <= UINT32_MAX, EILSEQ);
		state->requests = { economic_sql_mapping_request{ identity.wallet, PLAYER_LOCATOR,
								  identity.payload.pid },
				    economic_sql_mapping_request{ identity.bank, BANK_LOCATOR,
								  state->bank_id } };
		const auto error =
			economic_sql_lock_authority(connection, identity.wallet.lineage,
						    identity.intent.admission.metadata.epoch,
						    state->requests, &state->authority);
		require(!error, error);
		state->before = balances(connection, identity, state->bank_id);
		state->result = state->before;
		economic_currency_authority authority = { identity.intent.admission.metadata.epoch,
							  identity.wallet,
							  identity.bank,
							  command.keys[0],
							  command.keys[1],
							  state->before };
		const auto preparation = economic_bank_transfer_prepare(
			command, identity.intent, authority, currency_revision_policy::sql_legacy,
			&state->prepared);
		if (preparation != economic_accounting_error::ok)
		{
			require(preparation != economic_accounting_error::capacity, ENOMEM);
			std::optional<currency_prepared_mutation> domain;
			state->code = currency_prepare_mutation(
				identity.payload, state->before,
				command.expected_revisions[0].revision,
				command.expected_revisions[1].revision,
				currency_revision_policy::sql_legacy, &domain);
			// Invalid native holdings and malformed preparation are unresolved
			// authority failures, never durable business rejections.
			require(business_error(state->code), state->code ? state->code : EINVAL);
		}
		else
			state->result = state->prepared->mutation().after();
		active(connection, state->session);
		critical_operation_id checkpoint_id = {};
		require(critical_operation_id_generate(&checkpoint_id), ENOMEM);
		char checkpoint_hex[CRITICAL_COMMAND_ID_HEX_SIZE] = {};
		require(critical_operation_id_to_hex(checkpoint_id, checkpoint_hex,
						     sizeof(checkpoint_hex)));
		state->checkpoint = std::string("economic_bank_") + checkpoint_hex;
		execute(connection, "SAVEPOINT " + state->checkpoint);
		*transaction = std::unique_ptr<economic_sql_bank_transaction>(
			new economic_sql_bank_transaction(std::move(state)));
		return 0;
	}
	catch (const failure &error)
	{
		return error.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}

unsigned int economic_sql_bank_transaction::apply()
{
	auto &state = *state_;
	if (state.phase != implementation::phase::prepared)
		return EPERM;
	state.phase = implementation::phase::failed;
	try
	{
		active(state.connection, state.session);
		// A transaction-local marker also detects rollback/restart on the same
		// connection, and invalidates this receipt after an outer savepoint rollback.
		execute(state.connection, "RELEASE SAVEPOINT " + state.checkpoint);
		execute(state.connection, "SAVEPOINT " + state.checkpoint);
		inbox(state.connection, state.command, true);
		require(equal(balances(state.connection, state.identity, state.bank_id),
			      state.before),
			ESTALE);
		const auto where = "operation_id=" + id(state.command.operation_id);
		count(state.connection, "currency_ledger", where, 0);
		count(state.connection, "economic_accounting_operation", where, 0);
		if (state.prepared)
		{
			const auto write_error = currency_sql_mutation_writer::write(
				state.connection, state.command, state.prepared->mutation(),
				static_cast<uint32_t>(state.bank_id));
			require(!write_error, write_error);
		}
		active(state.connection, state.session);
		state.phase = implementation::phase::applied;
		return 0;
	}
	catch (const failure &error)
	{
		return error.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}

unsigned int economic_sql_bank_transaction::finalize()
{
	auto &state = *state_;
	if (state.phase != implementation::phase::applied)
		return EPERM;
	state.phase = implementation::phase::failed;
	try
	{
		active(state.connection, state.session);
		// A transaction-local marker also detects rollback/restart on the same
		// connection, and invalidates this receipt after an outer savepoint rollback.
		execute(state.connection, "RELEASE SAVEPOINT " + state.checkpoint);
		execute(state.connection, "SAVEPOINT " + state.checkpoint);
		inbox(state.connection, state.command, true);
		auto verify_authority = [&]
		{
			economic_sql_authority_snapshot authority;
			const auto error = economic_sql_lock_authority(state.connection,
								       state.authority.lineage,
								       state.authority.epoch,
								       state.requests, &authority);
			require(!error, error);
			require(authority.lineage_revision == state.authority.lineage_revision &&
					authority.mappings.size() ==
						state.authority.mappings.size(),
				ESTALE);
			for (size_t index = 0; index < authority.mappings.size(); ++index)
				require(authority.mappings[index].revision ==
						state.authority.mappings[index].revision,
					ESTALE);
		};
		verify_authority();
		require(equal(balances(state.connection, state.identity, state.bank_id),
			      state.result));
		count(state.connection, "currency_ledger",
		      "operation_id=" + id(state.command.operation_id), state.prepared ? 1 : 0);
		if (state.prepared)
			count(state.connection, "currency_ledger",
			      predicate(ledger(state.command, state.identity, state.bank_id,
					       state.result)),
			      1);
		evidence(state.connection, state.command, state.identity, state.code,
			 state.prepared ? &state.prepared->plan() : nullptr, true);
		evidence(state.connection, state.command, state.identity, state.code,
			 state.prepared ? &state.prepared->plan() : nullptr, false);
		inbox(state.connection, state.command, true);
		verify_authority();
		count(state.connection, "currency_ledger",
		      "operation_id=" + id(state.command.operation_id), state.prepared ? 1 : 0);
		require(equal(balances(state.connection, state.identity, state.bank_id),
			      state.result));
		if (state.prepared)
			count(state.connection, "currency_ledger",
			      predicate(ledger(state.command, state.identity, state.bank_id,
					       state.result)),
			      1);
		active(state.connection, state.session);
		state.phase = implementation::phase::finalized;
		return 0;
	}
	catch (const failure &error)
	{
		return error.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}

unsigned int economic_sql_bank_transaction::verify_root_completion()
{
	auto &state = *state_;
	if (state.phase != implementation::phase::finalized)
		return EPERM;
	state.phase = implementation::phase::failed;
	try
	{
		active(state.connection, state.session);
		execute(state.connection, "RELEASE SAVEPOINT " + state.checkpoint);
		std::array<uint8_t, CURRENCY_RESULT_PAYLOAD_BYTES> payload = {};
		require(currency_command_encode_result(state.result, &payload));
		const auto retained_error = economic_sql_bank_verify_retained(
			state.connection, state.command, state.code, payload);
		require(!retained_error, retained_error);
		economic_sql_authority_snapshot authority;
		const auto authority_error = economic_sql_lock_authority(
			state.connection, state.authority.lineage, state.authority.epoch,
			state.requests, &authority);
		require(!authority_error, authority_error);
		require(authority.lineage_revision == state.authority.lineage_revision &&
				authority.mappings.size() == state.authority.mappings.size(),
			ESTALE);
		for (size_t index = 0; index < authority.mappings.size(); ++index)
			require(authority.mappings[index].revision ==
					state.authority.mappings[index].revision,
				ESTALE);
		require(equal(balances(state.connection, state.identity, state.bank_id),
			      state.result));
		const auto where = "operation_id=" + id(state.command.operation_id);
		count(state.connection, "critical_outbox", where, state.prepared ? 1 : 0);
		if (state.prepared)
			count(state.connection, "critical_outbox",
			      predicate({ { "operation_id", id(state.command.operation_id) },
					  { "event_index", "0" },
					  { "destination", "3" },
					  { "event_type", "1" },
					  { "payload_version", "1" },
					  { "payload", hex(payload) },
					  { "status", "0" },
					  { "attempt_count", "0" },
					  { "last_error_code", "0" } }) +
				      " AND delivered_at IS NULL AND dead_lettered_at IS NULL AND next_attempt_at<=CURRENT_TIMESTAMP(6)",
			      1);
		active(state.connection, state.session);
		state.phase = implementation::phase::verified;
		return 0;
	}
	catch (const failure &error)
	{
		return error.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}

unsigned int economic_sql_bank_verify_retained(MYSQL *connection, const critical_command &command,
					       unsigned int result_code,
					       std::span<const uint8_t> result_payload)
{
	try
	{
		require(connection, EINVAL);
		const auto identity = decode(command);
		inbox(connection, command, false);
		count(connection, "critical_operation_inbox",
		      predicate({ { "operation_id", id(command.operation_id) },
				  { "result_code", std::to_string(result_code) },
				  { "result_payload", hex(result_payload) } }),
		      1);
		currency_command_result result = {};
		require(currency_command_decode_result(result_payload.data(), result_payload.size(),
						       &result));
		count(connection, "critical_operation_inbox",
		      predicate({ { "operation_id", id(command.operation_id) },
				  { "durable_revision",
				    std::to_string(std::max(result.wallet_revision,
							    result.bank_revision)) } }),
		      1);
		const auto row = read(
			connection,
			"SELECT canonical_plan FROM economic_accounting_operation WHERE operation_id=" +
				id(command.operation_id),
			1);
		std::optional<economic_prepared_currency> prepared;
		economic_accounting_plan plan;
		if (!result_code)
		{
			require(row[0].has_value() &&
				row[0]->size() <= ECONOMIC_ACCOUNTING_MAX_PLAN_BYTES);
			checked(economic_plan_decode(
				std::span<const uint8_t>(
					reinterpret_cast<const uint8_t *>(row[0]->data()),
					row[0]->size()),
				&plan));
			require(plan.accounts.size() == 2);
			economic_currency_authority authority = {
				identity.intent.admission.metadata.epoch,
				identity.wallet,
				identity.bank,
				command.keys[0],
				command.keys[1],
				{}
			};
			for (const auto &account : plan.accounts)
			{
				if (economic_account_key_equal(account.key, identity.wallet))
				{
					authority.state.wallet.amount = account.before;
					authority.state.wallet_revision = account.before_revision;
				}
				else if (economic_account_key_equal(account.key, identity.bank))
				{
					authority.state.bank.amount = account.before;
					authority.state.bank_revision = account.before_revision;
				}
				else
					throw failure{ EILSEQ };
			}
			checked(economic_bank_transfer_prepare(command, identity.intent, authority,
							       currency_revision_policy::sql_legacy,
							       &prepared));
			checked(prepared->agrees_with(plan));
			require(equal(prepared->mutation().after(), result));
		}
		else
		{
			require(!row[0].has_value() && business_error(result_code));
			// A rejection publishes its unchanged before-state. Reproduce the
			// original native decision from that witness, never current balances.
			// This also rejects codec-valid negative/oversized holdings before
			// revision checks can make them look like a valid stale decision.
			std::optional<currency_prepared_mutation> rejected;
			require(currency_prepare_mutation(identity.payload, result,
							  command.expected_revisions[0].revision,
							  command.expected_revisions[1].revision,
							  currency_revision_policy::sql_legacy,
							  &rejected) == result_code);
		}
		const auto where = "operation_id=" + id(command.operation_id);
		const auto bank_id =
			prepared ?
				integer<uint64_t>(
					read(connection,
					     "SELECT bank_id FROM currency_ledger WHERE " + where,
					     1)[0]) :
				mapping_hint(connection, identity.bank.authority_id);
		require(bank_id && bank_id <= UINT32_MAX);
		if (prepared)
			count(connection, "currency_ledger",
			      predicate(ledger(command, identity, bank_id, result)), 1);
		else
			count(connection, "currency_ledger", where, 0);
		count(connection, "economic_account_mapping",
		      predicate({ { "mapping_id", std::to_string(identity.wallet.authority_id) },
				  { "lineage", id(identity.wallet.lineage) },
				  { "account_kind", "1" },
				  { "context_id", "0" },
				  { "backend_kind", "1" },
				  { "locator_kind", std::to_string(PLAYER_LOCATOR) },
				  { "native_id", std::to_string(identity.payload.pid) } }),
		      1);
		// Retained native bindings survive retirement; no active-epoch,
		// active mapping or current balance is consulted during replay.
		count(connection, "economic_account_mapping",
		      predicate({ { "mapping_id", std::to_string(identity.bank.authority_id) },
				  { "lineage", id(identity.bank.lineage) },
				  { "account_kind", "2" },
				  { "context_id", std::to_string(identity.bank.context_id) },
				  { "backend_kind", "1" },
				  { "locator_kind", std::to_string(BANK_LOCATOR) },
				  { "native_id", std::to_string(bank_id) } }),
		      1);
		evidence(connection, command, identity, result_code,
			 prepared ? &prepared->plan() : nullptr, false);
		return 0;
	}
	catch (const failure &error)
	{
		return error.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}
#endif
