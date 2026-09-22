#include "persistence/economic_sql_coin_transaction.h"
#include "persistence/economic_sql_wallet_internal.h"

using namespace economic_sql_wallet_detail;

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
	bool failed = false, finalized = false;
#ifndef __NO_MYSQL__
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
	void child_receipt(size_t i)
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
		count(connection, "critical_outbox", "operation_id=" + id(child.operation_id), 1);
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
		count(connection, "economic_accounting_operation",
		      "operation_id=" + id(child.operation_id), 0);
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
		state->command = command;
		economic_frozen_intent intent;
		checked(economic_intent_decode(command.accounting_intent, &intent));
		require(intent.admission.facts.size() == ECONOMIC_COIN_WALLET_FACT_BYTES, EINVAL);
		coin_transfer_payload payload;
		require(coin_transfer_command_decode_payload(command, &payload), EINVAL);
		state->children = { payload.source.change, payload.destination.change };
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
		inbox(connection, command, true);
		for (size_t i = 0; i < 2; ++i)
		{
			auto &identity = state->identities[i];
			identity.intent = intent;
			identity.wallet = wallets[i];
			identity.bank = banks[i];
			require(currency_command_decode_payload(state->children[i],
								&identity.payload),
				EINVAL);
			state->bank_ids[i] = mapping_hint(connection, banks[i].authority_id);
			require(state->bank_ids[i] && state->bank_ids[i] <= UINT32_MAX, EILSEQ);
			state->requests.push_back(
				{ wallets[i], PLAYER_LOCATOR, identity.payload.pid });
			if (!i || !economic_account_key_equal(banks[0], banks[1]))
				state->requests.push_back(
					{ banks[i], BANK_LOCATOR, state->bank_ids[i] });
		}
		auto code = economic_sql_lock_authority(connection, meta.lineage, meta.epoch,
							state->requests, &state->authority);
		require(!code, code);
		economic_coin_wallet_authority authority;
		for (size_t i = 0; i < 2; ++i)
			authority[i] = { meta.epoch,
					 wallets[i],
					 banks[i],
					 state->children[i].keys[0],
					 state->children[i].keys[1],
					 balances(connection, state->identities[i],
						  state->bank_ids[i]) };
		checked(economic_coin_wallets_prepare(command, intent, authority,
						      currency_revision_policy::sql_legacy,
						      &state->prepared));
		for (size_t i = 0; i < 2; ++i)
			state->result.wallets[i] = state->prepared->mutations()[i].after();
		require(coin_transfer_command_destination_after_source(payload, state->result,
								       &state->children[1]),
			ESTALE);
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
	if (state.failed || state.finalized || index != state.next || index >= 2)
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
	if (state.failed || state.finalized || state.next != 2)
		return EPERM;
	state.failed = true;
	try
	{
		state.marker();
		state.verify_effects();
		evidence(state.connection, state.command, state.identities[0], 0,
			 &state.prepared->plan(), true, true);
		evidence(state.connection, state.command, state.identities[0], 0,
			 &state.prepared->plan(), false, true);
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
#endif
