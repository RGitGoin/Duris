#include "persistence/economic_sql_enrollment_transaction.h"
#include "persistence/economic_sql_source_snapshot.h"
#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cstring>
#include <memory>
#include <new>
#include <openssl/sha.h>
#include <type_traits>

namespace
{
constexpr size_t receipt_bytes = 24 + ECONOMIC_ENROLLMENT_COMMAND_BYTES;
static_assert(receipt_bytes <= CRITICAL_COMPLETION_RESULT_MAX_BYTES);
uint64_t little(const uint8_t *p)
{
	uint64_t v = 0;
	for (size_t i = 0; i < 8; ++i)
		v |= uint64_t(p[i]) << (8 * i);
	return v;
}
critical_apply_result result(const critical_command &command, uint64_t mapping, uint64_t revision,
			     bool replay)
{
	critical_apply_result r{ replay ? critical_apply_outcome::already_applied :
					  critical_apply_outcome::applied,
				 revision, 0 };
	r.result_size = receipt_bytes;
	const std::array<uint8_t, 8> magic = { 'E', 'L', 'R', '1', 1, 0, 0, 0 };
	std::copy(magic.begin(), magic.end(), r.result_payload.begin());
	for (size_t i = 0; i < 8; ++i)
	{
		r.result_payload[8 + i] = static_cast<uint8_t>(mapping >> (8 * i));
		r.result_payload[16 + i] = static_cast<uint8_t>(revision >> (8 * i));
	}
	std::copy(command.payload.begin(), command.payload.end(), r.result_payload.begin() + 24);
	return r;
}
}
economic_accounting_error economic_enrollment_result_account(const critical_command &command,
							     const critical_apply_result &value,
							     economic_account_key *out)
{
	using error = economic_accounting_error;
	if (!out || value.error_code || value.failure_stage != critical_failure_stage::none ||
	    (value.outcome != critical_apply_outcome::applied &&
	     value.outcome != critical_apply_outcome::already_applied) ||
	    value.result_size != receipt_bytes || !value.durable_revision)
		return error::corrupt_evidence;
	economic_enrollment_request request;
	auto status = economic_enrollment_command_decode(command, &request);
	if (status != error::ok)
		return status;
	auto mapping = little(value.result_payload.data() + 8);
	auto revision = little(value.result_payload.data() + 16);
	if (!mapping || request.expected_lineage_revision == UINT64_MAX ||
	    revision != request.expected_lineage_revision + 1 || revision != value.durable_revision)
		return error::corrupt_evidence;
	auto expected = result(command, mapping, revision,
			       value.outcome == critical_apply_outcome::already_applied);
	if (!std::equal(expected.result_payload.begin(),
			expected.result_payload.begin() + receipt_bytes,
			value.result_payload.begin()))
		return error::corrupt_evidence;
	economic_account_key key{ request.lineage, request.kind, mapping, request.context_id };
	if (!economic_account_key_valid(key))
		return error::corrupt_evidence;
	*out = key;
	return error::ok;
}
#ifdef __NO_MYSQL__
critical_apply_result economic_sql_enrollment_transaction::run(MYSQL *, const critical_command &,
							       bool)
{
	return { critical_apply_outcome::retryable_failure, 0, ENOTSUP };
}
#else
namespace
{
struct failure
{
	unsigned int code;
};
void need(bool okay, unsigned int code = EILSEQ)
{
	if (!okay)
		throw failure{ code };
}
void checked(economic_accounting_error e)
{
	need(e == economic_accounting_error::ok,
	     e == economic_accounting_error::capacity ? ENOMEM : EINVAL);
}
using cells = std::vector<std::optional<std::string>>;
std::string hex(std::span<const uint8_t> bytes)
{
	static constexpr char digits[] = "0123456789abcdef";
	std::string s = "X'";
	s.reserve(3 + bytes.size() * 2);
	for (auto b : bytes)
	{
		s += digits[b >> 4];
		s += digits[b & 15];
	}
	return s + "'";
}
std::string id(const critical_operation_id &v)
{
	return hex(v.bytes);
}
std::string hash(std::span<const uint8_t> bytes)
{
	economic_digest v;
	SHA256(bytes.data(), bytes.size(), v.data());
	return hex(v);
}
void execute(MYSQL *c, const std::string &s)
{
	if (mysql_real_query(c, s.data(), s.size()))
		throw failure{ mysql_errno(c) };
}
cells read(MYSQL *c, const std::string &s, size_t n)
{
	execute(c, s);
	std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)> rows(mysql_store_result(c),
								      mysql_free_result);
	need(bool(rows), mysql_errno(c) ? mysql_errno(c) : EIO);
	need(mysql_num_rows(rows.get()) == 1, ENOENT);
	need(mysql_num_fields(rows.get()) == n);
	auto row = mysql_fetch_row(rows.get());
	auto sizes = mysql_fetch_lengths(rows.get());
	need(row && sizes);
	cells values;
	for (size_t i = 0; i < n; ++i)
	{
		need(sizes[i] <= 4096, E2BIG);
		values.push_back(row[i] ?
					 std::optional<std::string>(std::string(row[i], sizes[i])) :
					 std::nullopt);
	}
	return values;
}
template <typename T> T number(const std::optional<std::string> &cell)
{
	need(cell.has_value());
	T n = 0;
	auto converted = std::from_chars(cell->data(), cell->data() + cell->size(), n);
	need(converted.ec == std::errc{} && converted.ptr == cell->data() + cell->size());
	return n;
}
void count(MYSQL *c, const std::string &table, const std::string &where, uint64_t expected)
{
	need(expected <= 1, EINVAL);
	// Inspect only enough matching rows to prove absence or detect a duplicate.
	need(number<uint64_t>(read(c,
				   "SELECT COUNT(*) FROM (SELECT 1 FROM " + table + " WHERE " +
					   where + " LIMIT " + std::to_string(expected + 1) +
					   ") AS matched",
				   1)[0]) == expected);
}
void active(MYSQL *c, unsigned long session)
{
	need((c->server_status & SERVER_STATUS_IN_TRANS) && mysql_thread_id(c) == session,
	     ENOTCONN);
	using flag = std::remove_pointer_t<decltype(MYSQL_BIND{}.is_null)>;
	flag reconnect = false;
	need(!mysql_get_option(c, MYSQL_OPT_RECONNECT, &reconnect) && !reconnect, EPERM);
}
struct transaction
{
	MYSQL *connection;
	unsigned long session;
	bool started = false;
	~transaction()
	{
		if (started && mysql_thread_id(connection) == session)
			mysql_real_query(connection, "ROLLBACK", 8);
	}
};
std::string receipt_identity(const critical_command &command)
{
	std::vector<uint8_t> wire, keys;
	auto status = critical_command_encode(command, &wire);
	need(status == critical_command_codec_result::ok,
	     status == critical_command_codec_result::overflow ? ENOMEM : EINVAL);
	for (const auto &key : command.keys)
	{
		keys.push_back(static_cast<uint8_t>(key.type));
		for (size_t n = 0; n < 8; ++n)
			keys.push_back(static_cast<uint8_t>(key.id >> (8 * n)));
	}
	return "operation_id=" + id(command.operation_id) + " AND command_hash=" + hash(wire) +
	       " AND keys_hash=" + hash(keys) +
	       " AND command_type=" + std::to_string(static_cast<uint16_t>(command.type)) +
	       " AND schema_version=1 AND payload_version=1";
}
std::string assignments(std::string predicate)
{
	// Values are exclusively canonical numbers and binary hex, never source SQL.
	for (size_t at = 0; (at = predicate.find(" AND ", at)) != std::string::npos;)
		predicate.replace(at, 5, ",");
	return predicate;
}
void reserve(MYSQL *c, const critical_command &command)
{
	execute(c, "INSERT INTO critical_operation_inbox SET " +
			   assignments(receipt_identity(command)) + ",status=0,result_payload=X''");
	need(mysql_affected_rows(c) == 1);
}
void epoch(MYSQL *c, const economic_enrollment_request &r)
{
	count(c, "economic_epoch",
	      "lineage=" + id(r.lineage) + " AND epoch=" + id(r.epoch) +
		      " AND creating_operation_id=" + id(r.epoch_operation),
	      1);
	count(c, "critical_operation_inbox",
	      "operation_id=" + id(r.epoch_operation) +
		      " AND status=1 AND result_code=0 AND failure_stage=0 AND committed_at IS NOT NULL",
	      1);
}
std::string locator(const economic_enrollment_request &r)
{
	return "lineage=" + id(r.lineage) + " AND backend_kind=1 AND locator_kind=" +
	       std::to_string(r.kind == economic_account_kind::wallet ? 1 : 2) +
	       " AND native_id=" + std::to_string(r.native_id);
}
std::string mapping_identity(const critical_command &command, const economic_enrollment_request &r,
			     uint64_t mapping)
{
	return "mapping_id=" + std::to_string(mapping) + " AND " + locator(r) +
	       " AND account_kind=" + std::to_string(static_cast<uint16_t>(r.kind)) +
	       " AND context_id=" + std::to_string(r.context_id) +
	       " AND creating_operation_id=" + id(command.operation_id);
}
critical_apply_result verify(MYSQL *c, const critical_command &command,
			     const economic_enrollment_request &r, bool replay)
{
	auto row = read(
		c,
		"SELECT status,result_code,failure_stage,durable_revision,result_payload,committed_at "
		"FROM critical_operation_inbox WHERE " +
			receipt_identity(command) + " FOR UPDATE",
		6);
	need(number<uint64_t>(row[0]) == 1 && !number<uint64_t>(row[1]) &&
	     !number<uint64_t>(row[2]) && row[5]);
	need(row[4] && row[4]->size() == receipt_bytes);
	auto mapping = little(reinterpret_cast<const uint8_t *>(row[4]->data()) + 8);
	auto revision = number<uint64_t>(row[3]);
	auto value = result(command, mapping, revision, replay);
	need(!memcmp(value.result_payload.data(), row[4]->data(), receipt_bytes));
	economic_account_key key;
	checked(economic_enrollment_result_account(command, value, &key));
	auto lineage = read(c,
			    "SELECT revision FROM economic_lineage_state WHERE lineage=" +
				    id(r.lineage) + " LOCK IN SHARE MODE",
			    1);
	need(number<uint64_t>(lineage[0]) >= revision);
	epoch(c, r);
	auto state = read(
		c,
		"SELECT active_native_id,retiring_operation_id,revision FROM economic_account_mapping WHERE " +
			mapping_identity(command, r, mapping) + " LOCK IN SHARE MODE",
		3);
	if (state[0])
		need(number<uint64_t>(state[0]) == r.native_id && !state[1]);
	else
		need(state[1] && state[1]->size() == 16 && number<uint64_t>(state[2]) > 0);
	count(c, "economic_account_mapping", "creating_operation_id=" + id(command.operation_id),
	      1);
	for (const char *table : { "economic_accounting_operation", "currency_ledger",
				   "item_ownership_ledger", "critical_outbox" })
		count(c, table, "operation_id=" + id(command.operation_id), 0);
	return value;
}
void native(MYSQL *c, const economic_enrollment_request &r)
{
	const bool bank = r.kind == economic_account_kind::bank;
	auto verified = economic_sql_verify_holding_source(c, bank, r.native_id, r.source_digest);
	need(!verified, verified);
	const std::string selection =
		bank ? "bank_revision,bank_copper,bank_silver,bank_gold,bank_platinum,racewar" :
		       "wallet_revision,copper,silver,gold,platinum,0";
	auto values = read(c,
			   "SELECT " + selection + " FROM " +
				   (bank ? "account_banks WHERE id=" : "player_data WHERE pid=") +
				   std::to_string(r.native_id) + " FOR UPDATE",
			   6);
	need(number<uint64_t>(values[0]) == r.native_revision &&
		     number<uint64_t>(values[5]) == r.context_id,
	     ESTALE);
	for (size_t n = 0; n < 4; ++n)
		need(number<int64_t>(values[n + 1]) == r.balance[n], ESTALE);
}
}
critical_apply_result
economic_sql_enrollment_transaction::run(MYSQL *c, const critical_command &command, bool reconcile)
{
	bool committing = false;
	try
	{
		need(c, EINVAL);
		economic_enrollment_request request;
		checked(economic_enrollment_command_decode(command, &request));
		need(!(c->server_status & SERVER_STATUS_IN_TRANS), EBUSY);
		using flag = std::remove_pointer_t<decltype(MYSQL_BIND{}.is_null)>;
		flag reconnect = false;
		need(!mysql_get_option(c, MYSQL_OPT_RECONNECT, &reconnect) && !reconnect, EPERM);
		auto server = mysql_get_server_info(c);
		need(server, ENOTCONN);
		auto isolation = read(c,
				      strstr(server, "MariaDB") ?
					      "SELECT @@SESSION.tx_isolation" :
					      "SELECT @@SESSION.transaction_isolation",
				      1);
		need(isolation[0] && *isolation[0] == "READ-COMMITTED", EPERM);
		transaction owner{ c, mysql_thread_id(c) };
		owner.started = true;
		execute(c, "START TRANSACTION");
		active(c, owner.session);
		bool replay = reconcile;
		if (!reconcile)
		{
			try
			{
				reserve(c, command);
			}
			catch (const failure &e)
			{
				if (e.code != 1062)
					throw;
				replay = true;
			}
		}
		if (replay)
		{
			(void)read(
				c,
				"SELECT operation_id FROM critical_operation_inbox WHERE operation_id=" +
					id(command.operation_id) + " FOR UPDATE",
				1);
			need(number<uint64_t>(
				     read(c,
					  "SELECT COUNT(*) FROM critical_operation_inbox WHERE " +
						  receipt_identity(command),
					  1)[0]) == 1,
			     EEXIST);
			auto value = verify(c, command, request, true);
			active(c, owner.session);
			return value;
		}
		auto lineage = read(
			c,
			"SELECT active_epoch,revision FROM economic_lineage_state WHERE lineage=" +
				id(request.lineage) + " FOR UPDATE",
			2);
		need(!lineage[0], EBUSY);
		need(number<uint64_t>(lineage[1]) == request.expected_lineage_revision, ESTALE);
		need(request.expected_lineage_revision < UINT64_MAX, EOVERFLOW);
		epoch(c, request);
		// No generation inference from names, balances, digests, or retirement alone.
		need(!number<uint64_t>(
			     read(c,
				  "SELECT EXISTS(SELECT 1 FROM economic_account_mapping WHERE " +
					  locator(request) + ")",
				  1)[0]),
		     EEXIST);
		native(c, request);
		execute(c, "INSERT INTO economic_account_mapping SET " +
				   assignments(locator(request)) + ",account_kind=" +
				   std::to_string(static_cast<uint16_t>(request.kind)) +
				   ",context_id=" + std::to_string(request.context_id) +
				   ",active_native_id=" + std::to_string(request.native_id) +
				   ",creating_operation_id=" + id(command.operation_id) +
				   ",revision=0");
		auto mapping = mysql_insert_id(c);
		need(mapping && mysql_affected_rows(c) == 1);
		execute(c, "UPDATE economic_lineage_state SET revision=revision+1 WHERE lineage=" +
				   id(request.lineage) + " AND active_epoch IS NULL AND revision=" +
				   std::to_string(request.expected_lineage_revision));
		need(mysql_affected_rows(c) == 1);
		auto value = result(command, mapping, request.expected_lineage_revision + 1, false);
		execute(c,
			"UPDATE critical_operation_inbox SET status=1,result_code=0,failure_stage=0,durable_revision=" +
				std::to_string(value.durable_revision) + ",result_payload=" +
				hex(std::span(value.result_payload.data(), value.result_size)) +
				",committed_at=CURRENT_TIMESTAMP(6) WHERE " +
				receipt_identity(command) + " AND status=0");
		need(mysql_affected_rows(c) == 1);
		value = verify(c, command, request, false);
		active(c, owner.session);
		committing = true;
		execute(c, "COMMIT");
		owner.started = false;
		need(mysql_thread_id(c) == owner.session &&
			     !(c->server_status & SERVER_STATUS_IN_TRANS),
		     ENOTCONN);
		return value;
	}
	catch (const failure &e)
	{
		return { committing ? critical_apply_outcome::ambiguous_commit :
				      critical_apply_outcome::retryable_failure,
			 0, e.code == ENOENT ? unsigned(EAGAIN) : e.code };
	}
	catch (const std::bad_alloc &)
	{
		return { committing ? critical_apply_outcome::ambiguous_commit :
				      critical_apply_outcome::retryable_failure,
			 0, ENOMEM };
	}
}
#endif
critical_apply_result economic_sql_enrollment_transaction::apply(MYSQL *c,
								 const critical_command &command)
{
	return run(c, command, false);
}
critical_apply_result
economic_sql_enrollment_transaction::reconcile(MYSQL *c, const critical_command &command)
{
	return run(c, command, true);
}
