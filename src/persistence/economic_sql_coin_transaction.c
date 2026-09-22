#include "persistence/economic_sql_coin_transaction.h"
#include "persistence/economic_sql_wallet_internal.h"
#include "persistence/critical_outbox.h"

using namespace economic_sql_wallet_detail;
#ifndef __NO_MYSQL__
namespace
{
unsigned int rejection_code(economic_accounting_error error)
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
}
#endif

struct economic_sql_coin_transaction::implementation
{
	MYSQL *connection = nullptr;
	unsigned long session = 0;
	critical_command command;
	std::array<critical_command, 2> children;
	std::array<bank_identity, 2> identities;
	std::array<uint64_t, 2> bank_ids = {};
	std::vector<economic_sql_mapping_request> requests;
	economic_sql_authority_snapshot authority;
	std::optional<economic_prepared_coin_wallets> prepared;
	coin_transfer_result result;
	std::string checkpoint;
	size_t next = 0;
	unsigned int code = 0;
	critical_failure_stage stage = critical_failure_stage::none;
	bool failed = false, finalized = false, verified = false;
#ifndef __NO_MYSQL__
	coin_transfer_payload bind_command(const critical_command &source)
	{
		require(source.schema_version == CRITICAL_COMMAND_ACCOUNTING_SCHEMA_VERSION &&
				critical_command_envelope_valid(source),
			EPROTONOSUPPORT);
		command = source;
		economic_frozen_intent intent;
		checked(economic_intent_decode(command.accounting_intent, &intent));
		require(intent.admission.facts.size() == ECONOMIC_COIN_WALLET_FACT_BYTES, EINVAL);
		coin_transfer_payload payload;
		require(coin_transfer_command_decode_payload(command, &payload), EINVAL);
		children = { payload.source.change, payload.destination.change };
		std::array<economic_account_key, 2> wallets, banks;
		const auto &meta = intent.admission.metadata;
		for (size_t i = 0; i < 2; ++i)
		{
			wallets[i] = { meta.lineage, economic_account_kind::wallet,
				       little_u64(intent.admission.facts, i * 24), 0 };
			banks[i] = { meta.lineage, economic_account_kind::bank,
				     little_u64(intent.admission.facts, i * 24 + 8),
				     little_u64(intent.admission.facts, i * 24 + 16) };
		}
		auto admission = command;
		admission.schema_version = CRITICAL_COMMAND_SCHEMA_VERSION;
		admission.accounting_intent.clear();
		std::vector<uint8_t> expected;
		checked(economic_coin_wallets_intent(admission, meta.epoch, wallets, banks,
						     &expected));
		require(expected == command.accounting_intent, EACCES);
		for (size_t i = 0; i < 2; ++i)
		{
			identities[i].intent = intent;
			identities[i].wallet = wallets[i];
			identities[i].bank = banks[i];
			require(currency_command_decode_payload(children[i],
								&identities[i].payload),
				EINVAL);
		}
		return payload;
	}
	void marker()
	{
		active(connection, session);
		execute(connection, "RELEASE SAVEPOINT " + checkpoint);
		execute(connection, "SAVEPOINT " + checkpoint);
		inbox(connection, command, true);
	}
	void verify_authority()
	{
		economic_sql_authority_snapshot current;
		const auto code = economic_sql_lock_authority(connection, authority.lineage,
							      authority.epoch, requests, &current);
		require(!code, code);
		require(current.lineage_revision == authority.lineage_revision &&
				current.mappings.size() == authority.mappings.size(),
			ESTALE);
		for (size_t i = 0; i < current.mappings.size(); ++i)
			require(current.mappings[i].revision == authority.mappings[i].revision,
				ESTALE);
	}
	void child_receipt(size_t i, bool fresh = true)
	{
		const auto &child = children[i];
		const auto &after = result.wallets[i];
		inbox(connection, child, false);
		std::array<uint8_t, CURRENCY_RESULT_PAYLOAD_BYTES> encoded;
		require(currency_command_encode_result(after, &encoded));
		count(connection, "critical_operation_inbox",
		      predicate({ { "operation_id", id(child.operation_id) },
				  { "result_code", "0" },
				  { "durable_revision",
				    std::to_string(
					    std::max(after.wallet_revision, after.bank_revision)) },
				  { "result_payload", hex(encoded) } }),
		      1);
		count(connection, "currency_ledger", "operation_id=" + id(child.operation_id), 1);
		count(connection, "currency_ledger",
		      predicate(ledger(child, identities[i], bank_ids[i], after)), 1);
		if (fresh)
		{
			count(connection, "critical_outbox",
			      "operation_id=" + id(child.operation_id), 1);
			count(connection, "critical_outbox",
			      predicate({ { "operation_id", id(child.operation_id) },
					  { "event_index", "0" },
					  { "destination", "3" },
					  { "event_type", "1" },
					  { "payload_version", "1" },
					  { "payload", hex(encoded) },
					  { "status", "0" },
					  { "attempt_count", "0" },
					  { "last_error_code", "0" } }) +
				      " AND delivered_at IS NULL AND dead_lettered_at IS NULL AND next_attempt_at<=CURRENT_TIMESTAMP(6)",
			      1);
		}
		count(connection, "economic_accounting_operation",
		      "operation_id=" + id(child.operation_id), 0);
	}
	economic_coin_wallet_authority rejection_authority() const
	{
		economic_coin_wallet_authority before;
		for (size_t i = 0; i < 2; ++i)
			before[i] = { identities[i].intent.admission.metadata.epoch,
				      identities[i].wallet,
				      identities[i].bank,
				      children[i].keys[0],
				      children[i].keys[1],
				      result.wallets[i] };
		return before;
	}
	void verify_rejection()
	{
		require(business_error(code));
		verify_authority();
		for (size_t i = 0; i < 2; ++i)
		{
			require(equal(balances(connection, identities[i], bank_ids[i]),
				      result.wallets[i]),
				ESTALE);
			const auto where = "operation_id=" + id(children[i].operation_id);
			for (const char *table :
			     { "critical_operation_inbox", "critical_outbox", "currency_ledger",
			       "economic_accounting_operation" })
				count(connection, table, where, 0);
		}
		count(connection, "currency_ledger", "operation_id=" + id(command.operation_id), 0);
	}
	void retained_mappings()
	{
		for (size_t i = 0; i < 2; ++i)
		{
			const auto &identity = identities[i];
			bank_ids[i] =
				code ? mapping_hint(connection, identity.bank.authority_id) :
				       integer<uint64_t>(read(
					       connection,
					       "SELECT bank_id FROM currency_ledger WHERE operation_id=" +
						       id(children[i].operation_id),
					       1)[0]);
			require(bank_ids[i] && bank_ids[i] <= UINT32_MAX);
			for (bool bank : { false, true })
			{
				const auto &key = bank ? identity.bank : identity.wallet;
				count(connection, "economic_account_mapping",
				      predicate(
					      { { "mapping_id", std::to_string(key.authority_id) },
						{ "lineage", id(key.lineage) },
						{ "account_kind", bank ? "2" : "1" },
						{ "context_id", std::to_string(key.context_id) },
						{ "backend_kind", "1" },
						{ "locator_kind",
						  std::to_string(bank ? BANK_LOCATOR :
									PLAYER_LOCATOR) },
						{ "native_id",
						  std::to_string(bank ? bank_ids[i] :
									identity.payload.pid) } }),
				      1);
			}
			if (!code)
				child_receipt(i, false);
		}
	}
	void verify_effects()
	{
		verify_authority();
		for (size_t i = 0; i < 2; ++i)
		{
			auto expected = result.wallets[i];
			if (!i && bank_ids[0] == bank_ids[1])
			{
				expected.bank = result.wallets[1].bank;
				expected.bank_revision = result.wallets[1].bank_revision;
			}
			require(equal(balances(connection, identities[i], bank_ids[i]), expected),
				ESTALE);
			child_receipt(i);
		}
		count(connection, "currency_ledger", "operation_id=" + id(command.operation_id), 0);
	}
#endif
};

economic_sql_coin_transaction::economic_sql_coin_transaction(std::unique_ptr<implementation> state)
	: state_(std::move(state))
{
}
economic_sql_coin_transaction::~economic_sql_coin_transaction() = default;
const critical_command &economic_sql_coin_transaction::child_command(size_t index) const
{
	return state_->children.at(index);
}
const coin_transfer_result &economic_sql_coin_transaction::result() const
{
	return state_->result;
}

unsigned int economic_sql_coin_transaction::result_code() const
{
	return state_->code;
}

critical_failure_stage economic_sql_coin_transaction::failure_stage() const
{
	return state_->stage;
}

#ifdef __NO_MYSQL__
unsigned int
economic_sql_coin_transaction::prepare(MYSQL *, const critical_command &,
				       std::unique_ptr<economic_sql_coin_transaction> *)
{
	return ENOTSUP;
}
unsigned int economic_sql_coin_transaction::apply_endpoint(size_t)
{
	return ENOTSUP;
}
unsigned int economic_sql_coin_transaction::finalize()
{
	return ENOTSUP;
}
unsigned int economic_sql_coin_transaction::verify_root_completion()
{
	return ENOTSUP;
}
unsigned int economic_sql_coin_transaction::verify_retained(MYSQL *, const critical_command &,
							    unsigned int, std::span<const uint8_t>)
{
	return ENOTSUP;
}
#else
unsigned int
economic_sql_coin_transaction::prepare(MYSQL *connection, const critical_command &command,
				       std::unique_ptr<economic_sql_coin_transaction> *transaction)
{
	try
	{
		require(connection && transaction, EINVAL);
		require(command.schema_version == CRITICAL_COMMAND_ACCOUNTING_SCHEMA_VERSION &&
				critical_command_envelope_valid(command),
			EPROTONOSUPPORT);
		auto state = std::make_unique<implementation>();
		state->connection = connection;
		state->session = mysql_thread_id(connection);
		active(connection, state->session);
		const auto payload = state->bind_command(command);
		const auto &intent = state->identities[0].intent;
		const auto &meta = intent.admission.metadata;
		inbox(connection, command, true);
		for (size_t i = 0; i < 2; ++i)
		{
			auto &identity = state->identities[i];
			state->bank_ids[i] =
				mapping_hint(connection, state->identities[i].bank.authority_id);
			require(state->bank_ids[i] && state->bank_ids[i] <= UINT32_MAX, EILSEQ);
			state->requests.push_back({ state->identities[i].wallet, PLAYER_LOCATOR,
						    identity.payload.pid });
			if (!i || !economic_account_key_equal(state->identities[0].bank,
							      state->identities[1].bank))
				state->requests.push_back({ state->identities[i].bank, BANK_LOCATOR,
							    state->bank_ids[i] });
		}
		auto code = economic_sql_lock_authority(connection, meta.lineage, meta.epoch,
							state->requests, &state->authority);
		require(!code, code);
		economic_coin_wallet_authority authority;
		for (size_t i = 0; i < 2; ++i)
			authority[i] = { meta.epoch,
					 state->identities[i].wallet,
					 state->identities[i].bank,
					 state->children[i].keys[0],
					 state->children[i].keys[1],
					 balances(connection, state->identities[i],
						  state->bank_ids[i]) };
		const auto preparation = economic_coin_wallets_prepare(
			command, intent, authority, currency_revision_policy::sql_legacy,
			&state->prepared, &state->stage);
		if (preparation != economic_accounting_error::ok)
		{
			state->code = rejection_code(preparation);
			if (!state->code)
				checked(preparation);
			for (size_t i = 0; i < 2; ++i)
				state->result.wallets[i] = authority[i].state;
		}
		else
		{
			for (size_t i = 0; i < 2; ++i)
				state->result.wallets[i] = state->prepared->mutations()[i].after();
			require(coin_transfer_command_destination_after_source(
					payload, state->result, &state->children[1]),
				ESTALE);
		}
		critical_operation_id marker_id{};
		require(critical_operation_id_generate(&marker_id), ENOMEM);
		char marker_hex[CRITICAL_COMMAND_ID_HEX_SIZE]{};
		require(critical_operation_id_to_hex(marker_id, marker_hex, sizeof(marker_hex)));
		state->checkpoint = std::string("economic_coin_") + marker_hex;
		active(connection, state->session);
		execute(connection, "SAVEPOINT " + state->checkpoint);
		*transaction = std::unique_ptr<economic_sql_coin_transaction>(
			new economic_sql_coin_transaction(std::move(state)));
		return 0;
	}
	catch (const failure &e)
	{
		return e.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}
unsigned int economic_sql_coin_transaction::apply_endpoint(size_t index)
{
	auto &state = *state_;
	if (state.failed || state.finalized || state.code || index != state.next || index >= 2)
		return EPERM;
	state.failed = true;
	try
	{
		state.marker();
		state.verify_authority();
		if (index)
			state.child_receipt(0);
		const auto &child = state.children[index];
		inbox(state.connection, child, true);
		const auto &mutation = state.prepared->mutations()[index];
		require(equal(balances(state.connection, state.identities[index],
				       state.bank_ids[index]),
			      mutation.before()),
			ESTALE);
		count(state.connection, "currency_ledger", "operation_id=" + id(child.operation_id),
		      0);
		count(state.connection, "economic_accounting_operation",
		      "operation_id=" + id(state.command.operation_id), 0);
		const auto code = currency_sql_mutation_writer::write(
			state.connection, child, mutation,
			static_cast<uint32_t>(state.bank_ids[index]));
		require(!code, code);
		require(equal(balances(state.connection, state.identities[index],
				       state.bank_ids[index]),
			      mutation.after()),
			ESTALE);
		active(state.connection, state.session);
		++state.next;
		state.failed = false;
		return 0;
	}
	catch (const failure &e)
	{
		return e.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}
unsigned int economic_sql_coin_transaction::finalize()
{
	auto &state = *state_;
	if (state.failed || state.finalized || (state.code ? state.next != 0 : state.next != 2))
		return EPERM;
	state.failed = true;
	try
	{
		state.marker();
		if (state.code)
			state.verify_rejection();
		else
			state.verify_effects();
		evidence(state.connection, state.command, state.identities[0], state.code,
			 state.prepared ? &state.prepared->plan() : nullptr, true, true);
		evidence(state.connection, state.command, state.identities[0], state.code,
			 state.prepared ? &state.prepared->plan() : nullptr, false, true);
		if (state.code)
			state.verify_rejection();
		else
			state.verify_effects();
		inbox(state.connection, state.command, true);
		active(state.connection, state.session);
		state.finalized = true;
		state.failed = false;
		return 0;
	}
	catch (const failure &e)
	{
		return e.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}
unsigned int economic_sql_coin_transaction::verify_retained(MYSQL *connection,
							    const critical_command &command,
							    unsigned int result_code,
							    std::span<const uint8_t> result_payload)
{
	try
	{
		require(connection, EINVAL);
		const auto session = mysql_thread_id(connection);
		active(connection, session);
		require(!result_code || business_error(result_code), EPROTONOSUPPORT);
		implementation state;
		state.connection = connection;
		state.code = result_code;
		const auto payload = state.bind_command(command);
		const auto root_where = "operation_id=" + id(command.operation_id);
		const auto row =
			read(connection,
			     "SELECT canonical_plan FROM economic_accounting_operation WHERE " +
				     root_where,
			     1);
		if (result_code)
		{
			require(!row[0].has_value());
			require(coin_transfer_command_decode_result(payload, result_payload.data(),
								    result_payload.size(),
								    &state.result));
			std::array<uint8_t, COIN_TRANSFER_RESULT_BYTES> encoded;
			require(coin_transfer_command_encode_result(payload, state.result,
								    &encoded));
			require(result_payload.size() == encoded.size() &&
				std::equal(encoded.begin(), encoded.end(), result_payload.begin()));
			std::optional<economic_prepared_coin_wallets> candidate;
			const auto decision = economic_coin_wallets_prepare(
				command, state.identities[0].intent, state.rejection_authority(),
				currency_revision_policy::sql_legacy, &candidate, &state.stage);
			require(decision != economic_accounting_error::capacity, ENOMEM);
			require(rejection_code(decision) == result_code && !candidate);
			inbox(connection, command, false, state.stage);
			count(connection, "critical_operation_inbox",
			      predicate({ { "operation_id", id(command.operation_id) },
					  { "result_code", std::to_string(result_code) },
					  { "result_payload", hex(encoded) },
					  { "durable_revision", "0" } }),
			      1);
			state.retained_mappings();
			count(connection, "currency_ledger", root_where, 0);
			evidence(connection, command, state.identities[0], result_code, nullptr,
				 false, true);
			active(connection, session);
			return 0;
		}
		require(row[0].has_value());
		economic_accounting_plan plan;
		checked(economic_plan_decode(
			std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(row[0]->data()),
						 row[0]->size()),
			&plan));
		economic_coin_wallet_authority authority;
		for (size_t i = 0; i < 2; ++i)
		{
			const auto &identity = state.identities[i];
			authority[i] = { identity.intent.admission.metadata.epoch,
					 identity.wallet,
					 identity.bank,
					 state.children[i].keys[0],
					 state.children[i].keys[1],
					 {} };
			bool wallet_found = false, bank_found = false;
			for (const auto &effect : plan.accounts)
			{
				if (economic_account_key_equal(effect.key, identity.wallet))
				{
					require(!wallet_found);
					wallet_found = true;
					authority[i].state.wallet.amount = effect.before;
					authority[i].state.wallet_revision = effect.before_revision;
				}
				if (economic_account_key_equal(effect.key, identity.bank))
				{
					require(!bank_found);
					bank_found = true;
					authority[i].state.bank.amount = effect.before;
					authority[i].state.bank_revision = effect.before_revision;
				}
			}
			require(wallet_found && bank_found);
		}
		checked(economic_coin_wallets_prepare(
			command, state.identities[0].intent, authority,
			currency_revision_policy::sql_legacy, &state.prepared));
		inbox(connection, command, false);
		checked(state.prepared->agrees_with(plan));
		for (size_t i = 0; i < 2; ++i)
			state.result.wallets[i] = state.prepared->mutations()[i].after();
		require(coin_transfer_command_destination_after_source(payload, state.result,
								       &state.children[1]));
		std::array<uint8_t, COIN_TRANSFER_RESULT_BYTES> encoded;
		require(coin_transfer_command_encode_result(payload, state.result, &encoded));
		require(result_payload.size() == encoded.size() &&
			std::equal(encoded.begin(), encoded.end(), result_payload.begin()));
		const auto revision = std::max({ state.result.wallets[0].wallet_revision,
						 state.result.wallets[0].bank_revision,
						 state.result.wallets[1].wallet_revision,
						 state.result.wallets[1].bank_revision });
		count(connection, "critical_operation_inbox",
		      predicate({ { "operation_id", id(command.operation_id) },
				  { "result_code", "0" },
				  { "result_payload", hex(encoded) },
				  { "durable_revision", std::to_string(revision) } }),
		      1);
		state.retained_mappings();
		count(connection, "currency_ledger", root_where, 0);
		evidence(connection, command, state.identities[0], 0, &plan, false, true);
		active(connection, session);
		return 0;
	}
	catch (const failure &e)
	{
		return e.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}

unsigned int economic_sql_coin_transaction::verify_root_completion()
{
	auto &state = *state_;
	if (state.failed || !state.finalized || state.verified)
		return EPERM;
	state.failed = true;
	try
	{
		active(state.connection, state.session);
		execute(state.connection, "RELEASE SAVEPOINT " + state.checkpoint);
		coin_transfer_payload payload;
		require(coin_transfer_command_decode_payload(state.command, &payload));
		std::array<uint8_t, COIN_TRANSFER_RESULT_BYTES> encoded;
		require(coin_transfer_command_encode_result(payload, state.result, &encoded));
		const auto code =
			verify_retained(state.connection, state.command, state.code, encoded);
		require(!code, code);
		if (state.code)
		{
			state.verify_rejection();
			count(state.connection, "critical_outbox",
			      "operation_id=" + id(state.command.operation_id), 0);
			active(state.connection, state.session);
			state.verified = true;
			state.failed = false;
			return 0;
		}
		state.verify_effects();
		std::array<uint8_t, CRITICAL_OUTBOX_COIN_RECEIPT_BYTES> receipt;
		std::copy(state.children[0].operation_id.bytes.begin(),
			  state.children[0].operation_id.bytes.end(), receipt.begin());
		std::copy(state.children[1].operation_id.bytes.begin(),
			  state.children[1].operation_id.bytes.end(), receipt.begin() + 16);
		count(state.connection, "critical_outbox",
		      "operation_id=" + id(state.command.operation_id), 1);
		count(state.connection, "critical_outbox",
		      predicate({ { "operation_id", id(state.command.operation_id) },
				  { "event_index", "0" },
				  { "destination",
				    std::to_string(CRITICAL_OUTBOX_COIN_RECEIPT_DESTINATION) },
				  { "event_type",
				    std::to_string(CRITICAL_OUTBOX_COIN_RECEIPT_EVENT) },
				  { "payload_version", "1" },
				  { "payload", hex(receipt) },
				  { "status", "0" },
				  { "attempt_count", "0" },
				  { "last_error_code", "0" } }) +
			      " AND delivered_at IS NULL AND dead_lettered_at IS NULL AND next_attempt_at<=CURRENT_TIMESTAMP(6)",
		      1);
		active(state.connection, state.session);
		state.verified = true;
		state.failed = false;
		return 0;
	}
	catch (const failure &e)
	{
		return e.code;
	}
	catch (const std::bad_alloc &)
	{
		return ENOMEM;
	}
}
#endif
