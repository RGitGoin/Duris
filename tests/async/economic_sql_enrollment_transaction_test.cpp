#include "persistence/economic_sql_enrollment_transaction.h"
#include "persistence/economic_sql_baseline_transaction.h"
#include "economy/economic_sql_source_normalize.h"
#include <algorithm>
#include <atomic>
#include <barrier>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>
#include <thread>
#include <type_traits>

class economic_sql_enrollment_test_access
{
    public:
	static constexpr auto apply = &economic_sql_enrollment_transaction::apply;
	static constexpr auto reconcile = &economic_sql_enrollment_transaction::reconcile;
};
class economic_sql_baseline_test_access
{
    public:
	static constexpr auto initialize = &economic_sql_baseline_transaction::initialize;
	static constexpr auto apply = &economic_sql_baseline_transaction::apply;
};
using owner = economic_sql_enrollment_test_access;
critical_operation_id ident(uint64_t n)
{
	critical_operation_id r = {};
	for (size_t i = 0; i < 8; ++i)
		r.bytes[i] = static_cast<uint8_t>(n >> (i * 8));
	return r;
}
critical_command command(const economic_enrollment_request &r)
{
	critical_operation_id op;
	assert(critical_operation_id_generate(&op));
	critical_command c;
	assert(economic_enrollment_command_build(op, r, 123456, &c) ==
	       economic_accounting_error::ok);
	assert(critical_command_envelope_valid(c) && !critical_command_valid(c) &&
	       !critical_command_legacy_execution_supported(c));
	return c;
}
economic_enrollment_request input()
{
	economic_enrollment_request r;
	r.lineage = ident(889001);
	r.epoch = ident(889002);
	r.epoch_operation = ident(889003);
	r.actor_id = 17;
	r.native_id = 222;
	r.boundary_digest[0] = 1;
	r.source_digest[0] = 2;
	return r;
}
void codec()
{
	auto r = input();
	auto c = command(r);
	economic_enrollment_request decoded;
	assert(economic_enrollment_command_decode(c, &decoded) == economic_accounting_error::ok);
	auto old = decoded;
	for (size_t n : { size_t(0), size_t(4), size_t(6), size_t(74), size_t(79) })
	{
		auto bad = c;
		bad.payload[n] ^= 1;
		assert(economic_enrollment_command_decode(bad, &decoded) !=
		       economic_accounting_error::ok);
		assert(decoded.native_id == old.native_id);
	}
	for (auto kind : { economic_account_kind::pile, economic_account_kind::auction_escrow,
			   economic_account_kind::pending_claim, economic_account_kind::treasury })
	{
		r.kind = kind;
		auto saved = c;
		assert(economic_enrollment_command_build(c.operation_id, r, 123456, &c) !=
		       economic_accounting_error::ok);
		assert(critical_command_equal(c, saved));
	}
	r = input();
	r.native_id = uint64_t(INT32_MAX) + 1;
	assert(economic_enrollment_command_build(c.operation_id, r, 123456, &c) !=
	       economic_accounting_error::ok);
	r.kind = economic_account_kind::bank;
	r.native_id = uint64_t(UINT32_MAX) + 1;
	assert(economic_enrollment_command_build(c.operation_id, r, 123456, &c) !=
	       economic_accounting_error::ok);
	r = input();
	r.balance[3] = INT64_MAX;
	assert(economic_enrollment_command_build(c.operation_id, r, 123456, &c) ==
	       economic_accounting_error::overflow);
}
#ifdef __NO_MYSQL__
int main()
{
	codec();
	auto c = command(input());
	assert(owner::apply(nullptr, c).error_code == ENOTSUP &&
	       owner::reconcile(nullptr, c).error_code == ENOTSUP);
	assert(economic_sql_verify_holding_source(nullptr, false, 222, {}) == ENOTSUP);
	std::cout << "client-free enrollment refusal and codec PASS\n";
}
#else
bool instrument = false, after_write = false, faulted = false;
size_t query_seen = 0, query_target = 0, allocation_seen = 0, allocation_target = 0;
std::vector<size_t> writes;
extern "C" int __real_mysql_real_query(MYSQL *, const char *, unsigned long);
extern "C" unsigned int __real_mysql_errno(MYSQL *);
extern "C" int __wrap_mysql_real_query(MYSQL *c, const char *sql, unsigned long size)
{
	if (!instrument || (size == 8 && !memcmp(sql, "ROLLBACK", 8)))
		return __real_mysql_real_query(c, sql, size);
	faulted = false;
	++query_seen;
	const bool write = size >= 6 && (!memcmp(sql, "INSERT", 6) || !memcmp(sql, "UPDATE", 6) ||
					 !memcmp(sql, "COMMIT", 6));
	if (write && !query_target && !allocation_target)
		writes.push_back(query_seen);
	const bool fail = query_target && query_seen == query_target;
	if (fail && !(after_write && write))
	{
		faulted = true;
		return 1;
	}
	const auto result = __real_mysql_real_query(c, sql, size);
	if (fail)
	{
		faulted = true;
		return 1;
	}
	return result;
}
extern "C" unsigned int __wrap_mysql_errno(MYSQL *c)
{
	return faulted ? 2013 : __real_mysql_errno(c);
}
extern "C" void *__real__Znwm(size_t);
extern "C" void *__real__Znam(size_t);
extern "C" void *__wrap__Znwm(size_t n)
{
	if (allocation_target && ++allocation_seen == allocation_target)
		throw std::bad_alloc();
	return __real__Znwm(n);
}
extern "C" void *__wrap__Znam(size_t n)
{
	if (allocation_target && ++allocation_seen == allocation_target)
		throw std::bad_alloc();
	return __real__Znam(n);
}

std::string id(const critical_operation_id &value)
{
	char b[CRITICAL_COMMAND_ID_HEX_SIZE];
	assert(critical_operation_id_to_hex(value, b, sizeof(b)));
	return std::string("UNHEX('") + b + "')";
}
void sql(MYSQL *c, const std::string &s)
{
	if (mysql_real_query(c, s.data(), s.size()))
	{
		std::cerr << "synthetic SQL error " << mysql_errno(c) << "\n";
		abort();
	}
}
uint64_t scalar(MYSQL *c, const std::string &s)
{
	sql(c, s);
	auto result = mysql_store_result(c);
	assert(result);
	auto row = mysql_fetch_row(result);
	assert(row && row[0]);
	auto v = strtoull(row[0], nullptr, 10);
	mysql_free_result(result);
	return v;
}
MYSQL *connect_fixture()
{
	const auto env = [](const char *key)
	{
		auto v = getenv(key);
		assert(v && *v);
		return v;
	};
	assert(!strcmp(env("ECONOMIC_ACCOUNTING_DISPOSABLE_SCHEMA"), "1") &&
	       !strcmp(env("DB_HOST"), "127.0.0.1"));
	assert(!getenv("DB_SOCKET") || !*getenv("DB_SOCKET"));
	const std::string name = env("DB_NAME");
	assert(name.starts_with("economic_schema_test_enrollment_") &&
	       name.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_") ==
		       std::string::npos);
	auto c = mysql_init(nullptr);
	assert(c);
	unsigned timeout = 10, protocol = MYSQL_PROTOCOL_TCP;
	assert(!mysql_options(c, MYSQL_OPT_CONNECT_TIMEOUT, &timeout));
	assert(!mysql_options(c, MYSQL_OPT_READ_TIMEOUT, &timeout));
	assert(!mysql_options(c, MYSQL_OPT_WRITE_TIMEOUT, &timeout));
	assert(!mysql_options(c, MYSQL_OPT_PROTOCOL, &protocol));
	using flag = std::remove_pointer_t<decltype(MYSQL_BIND{}.is_null)>;
	flag reconnect = false;
	assert(!mysql_options(c, MYSQL_OPT_RECONNECT, &reconnect));
	assert(mysql_real_connect(c, "127.0.0.1", env("DB_USER"), env("DB_PASSWD"), name.c_str(),
				  strtoul(env("DB_PORT"), nullptr, 10), nullptr, 0));
	sql(c, "SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED");
	return c;
}
void setup(MYSQL *c)
{
	instrument = false;
	allocation_target = 0;
	query_target = 0;
	faulted = false;
	after_write = false;
	sql(c, "ROLLBACK");
	// This entire unique schema is owned by this synthetic runner.
	for (const char *table :
	     { "economic_baseline_reservation", "economic_baseline_witness",
	       "economic_baseline_control", "economic_accounting_coin_posting",
	       "economic_accounting_account_effect", "economic_accounting_child",
	       "economic_accounting_item_reference", "economic_accounting_source_claim",
	       "economic_accounting_operation", "economic_account_mapping",
	       "economic_lineage_state", "economic_epoch", "critical_operation_inbox" })
		sql(c, std::string("DELETE FROM ") + table);
	auto r = input();
	sql(c,
	    "INSERT INTO critical_operation_inbox(operation_id,command_hash,keys_hash,command_type,schema_version,payload_version,status,result_code,durable_revision,result_payload,committed_at) VALUES(" +
		    id(r.epoch_operation) +
		    ",REPEAT(CHAR(1),32),REPEAT(CHAR(2),32),1,1,1,1,0,0,X'',CURRENT_TIMESTAMP(6))");
	sql(c,
	    "INSERT INTO economic_epoch(lineage,epoch,ordinal,transition_kind,transition_digest,creating_operation_id) VALUES(" +
		    id(r.lineage) + "," + id(r.epoch) + ",1,1,REPEAT(CHAR(1),32)," +
		    id(r.epoch_operation) + ")");
	sql(c, "INSERT INTO economic_lineage_state(lineage,active_epoch) VALUES(" + id(r.lineage) +
		       ",NULL)");
}
std::vector<economic_enrollment_request> observations(MYSQL *c)
{
	economic_sql_source_snapshot snapshot;
	assert(!economic_sql_capture_sources(c, {}, &snapshot));
	economic_sql_normalized_sources normalized;
	assert(economic_sql_normalize_sources(snapshot, 512, &normalized) ==
	       economic_accounting_error::ok);
	std::vector<economic_enrollment_request> requests;
	for (const auto &holding : normalized.holdings)
	{
		if (holding.kind != economic_sql_holding_kind::wallet &&
		    holding.kind != economic_sql_holding_kind::bank)
			continue;
		assert(holding.disposition == economic_sql_holding_disposition::current &&
		       holding.balance && holding.native_revision);
		auto r = input();
		r.kind = holding.kind == economic_sql_holding_kind::bank ?
				 economic_account_kind::bank :
				 economic_account_kind::wallet;
		r.native_id = holding.native_id;
		r.context_id = holding.native_context;
		r.balance = *holding.balance;
		r.native_revision = *holding.native_revision;
		r.source_digest = holding.source.digest;
		r.boundary_digest = snapshot.digest;
		requests.push_back(r);
	}
	assert(requests.size() == 3);
	return requests;
}
economic_account_key success(const critical_command &c, const critical_apply_result &r,
			     bool replay = false)
{
	if (r.error_code)
		std::cerr << "enrollment error " << r.error_code << "\n";
	assert(r.outcome == (replay ? critical_apply_outcome::already_applied :
				      critical_apply_outcome::applied) &&
	       !r.error_code);
	economic_account_key key;
	assert(economic_enrollment_result_account(c, r, &key) == economic_accounting_error::ok);
	return key;
}
void empty(MYSQL *c)
{
	assert(!scalar(c, "SELECT COUNT(*) FROM economic_account_mapping"));
	assert(!scalar(c, "SELECT revision FROM economic_lineage_state"));
	assert(scalar(c, "SELECT COUNT(*) FROM critical_operation_inbox") == 1);
	assert(!scalar(c, "SELECT COUNT(*) FROM critical_outbox"));
}
void failed(MYSQL *c, const critical_command &op, unsigned error)
{
	auto r = owner::apply(c, op);
	assert(r.outcome == critical_apply_outcome::retryable_failure && r.error_code == error);
	empty(c);
}
void basic(MYSQL *c, const std::vector<economic_enrollment_request> &captured)
{
	setup(c);
	economic_baseline_batch batch;
	batch.lineage = input().lineage;
	batch.epoch = input().epoch;
	batch.preparation_id = ident(889004);
	batch.actor_id = 17;
	batch.opening_account = { batch.lineage, economic_account_kind::opening, 999001, 0 };
	batch.boundary_digest = captured[0].boundary_digest;
	batch.coverage_digest[0] = 1;
	std::vector<critical_command> operations;
	for (auto request : captured)
	{
		request.expected_lineage_revision = operations.size();
		auto op = command(request);
		auto applied = owner::apply(c, op);
		auto key = success(op, applied);
		auto repeat = owner::apply(c, op);
		success(op, repeat, true);
		assert(repeat.result_payload == applied.result_payload);
		batch.holdings.push_back(
			{ key, request.balance, request.native_revision, request.source_digest });
		operations.push_back(op);
	}
	assert(scalar(c, "SELECT COUNT(*) FROM economic_account_mapping WHERE account_kind=2") ==
	       1);
	sql(c, "START TRANSACTION");
	assert(!economic_sql_baseline_test_access::initialize(
		c, batch.lineage, batch.epoch, batch.opening_account, input().epoch_operation));
	sql(c, "COMMIT");
	std::optional<economic_prepared_baseline> prepared;
	assert(economic_baseline_prepare(batch, &prepared) == economic_accounting_error::ok);
	critical_command opening;
	assert(economic_baseline_command_build(*prepared, 123456, &opening) ==
	       economic_accounting_error::ok);
	auto opened = economic_sql_baseline_test_access::apply(c, opening, *prepared);
	assert(opened.outcome == critical_apply_outcome::applied);
	assert(scalar(c,
		      "SELECT COUNT(*) FROM economic_baseline_reservation WHERE identity_kind=1") ==
	       3);
	assert(scalar(c, "SELECT active_epoch IS NULL FROM economic_lineage_state") == 1);
	sql(c, "UPDATE player_data SET name='RenamedEnrollment',copper=copper+1 WHERE pid=222");
	auto reconnect = connect_fixture();
	success(operations.front(), owner::reconcile(reconnect, operations.front()), true);
	mysql_close(reconnect);
	sql(c, "UPDATE player_data SET name='EnrollOne',copper=copper-1 WHERE pid=222");
	auto changed = operations.front();
	changed.accepted_at_usec++;
	assert(owner::apply(c, changed).error_code == EEXIST);
	sql(c, "UPDATE economic_account_mapping SET active_native_id=NULL,retiring_operation_id=" +
		       id(input().epoch_operation) + ",revision=1");
	success(operations.front(), owner::reconcile(c, operations.front()), true);
	auto request = captured.front();
	request.expected_lineage_revision = 3;
	assert(owner::apply(c, command(request)).error_code == EEXIST);
}
void refusals(MYSQL *c, const economic_enrollment_request &good)
{
	setup(c);
	auto op = command(good);
	assert(owner::reconcile(c, op).error_code == EAGAIN);
	empty(c);
	sql(c, "START TRANSACTION");
	assert(owner::apply(c, op).error_code == EBUSY);
	assert(c->server_status & SERVER_STATUS_IN_TRANS);
	sql(c, "ROLLBACK");
	auto bad = good;
	bad.expected_lineage_revision = 1;
	failed(c, command(bad), ESTALE);
	bad = good;
	bad.source_digest[0] ^= 1;
	failed(c, command(bad), ESTALE);
	bad = good;
	bad.balance[0]++;
	failed(c, command(bad), ESTALE);
	bad = good;
	bad.native_revision++;
	failed(c, command(bad), ESTALE);
	// Capture binds account_name, not player name or a generation token.
	// This proves captured-row divergence only; it does not prove name/generation identity.
	sql(c, "UPDATE player_data SET account_name='changed_enrollment' WHERE pid=222");
	failed(c, op, ESTALE);
	sql(c, "UPDATE player_data SET account_name='enrollment_account' WHERE pid=222");
	sql(c, "UPDATE economic_lineage_state SET active_epoch=" + id(good.epoch));
	assert(owner::apply(c, op).error_code == EBUSY);
	sql(c, "UPDATE economic_lineage_state SET active_epoch=NULL");
	empty(c);
	sql(c, "UPDATE critical_operation_inbox SET status=0");
	assert(owner::apply(c, op).error_code);
	sql(c, "UPDATE critical_operation_inbox SET status=1");
	empty(c);
	bad = good;
	bad.lineage = ident(5555);
	assert(owner::apply(c, command(bad)).error_code == EAGAIN);
	empty(c);
	bad = good;
	bad.epoch = ident(6666);
	assert(owner::apply(c, command(bad)).error_code);
	empty(c);
	auto applied = owner::apply(c, op);
	auto key = success(op, applied);
	// A valid selected mapping does not excuse a second creation under its root.
	sql(c,
	    "INSERT INTO economic_account_mapping(lineage,account_kind,context_id,backend_kind,locator_kind,native_id,active_native_id,creating_operation_id,retiring_operation_id,revision) "
	    "SELECT lineage,account_kind,context_id,backend_kind,locator_kind,native_id+1000000,active_native_id+1000000,creating_operation_id,retiring_operation_id,revision FROM economic_account_mapping");
	assert(owner::reconcile(c, op).error_code == EILSEQ);
	sql(c, "DELETE FROM economic_account_mapping WHERE native_id>=1000000");
	success(op, owner::reconcile(c, op), true);
	sql(c, "UPDATE economic_account_mapping SET context_id=77");
	assert(owner::reconcile(c, op).error_code);
	sql(c, "UPDATE economic_account_mapping SET context_id=0");
	sql(c,
	    "UPDATE critical_operation_inbox SET result_payload=SUBSTRING(result_payload,1,10) WHERE operation_id=" +
		    id(op.operation_id));
	assert(owner::reconcile(c, op).error_code);
	(void)key;
}
void faults(MYSQL *c, const economic_enrollment_request &r)
{
	setup(c);
	auto op = command(r);
	query_seen = 0;
	writes.clear();
	instrument = true;
	success(op, owner::apply(c, op));
	instrument = false;
	auto queries = query_seen;
	auto write_points = writes;
	for (size_t n = 1; n <= queries; ++n)
	{
		setup(c);
		query_target = n;
		query_seen = 0;
		instrument = true;
		auto failed_result = owner::apply(c, op);
		instrument = false;
		faulted = false;
		assert(failed_result.error_code);
		empty(c);
	}
	for (auto n : write_points)
	{
		setup(c);
		query_target = n;
		query_seen = 0;
		after_write = true;
		instrument = true;
		auto failed_result = owner::apply(c, op);
		instrument = false;
		faulted = false;
		assert(failed_result.error_code);
		if (failed_result.outcome == critical_apply_outcome::ambiguous_commit)
		{
			auto fresh = connect_fixture();
			success(op, owner::reconcile(fresh, op), true);
			mysql_close(fresh);
		}
		else
			empty(c);
	}
	size_t n = 1;
	for (; n < 3000; ++n)
	{
		setup(c);
		allocation_target = n;
		allocation_seen = 0;
		auto outcome = owner::apply(c, op);
		allocation_target = 0;
		if (!outcome.error_code)
		{
			success(op, outcome);
			break;
		}
		assert(outcome.error_code == ENOMEM);
		empty(c);
	}
	assert(n < 3000);
	std::cout << queries << " query faults, " << write_points.size() << " post-write faults, "
		  << n - 1 << " allocation faults PASS\n";
}
void concurrency(MYSQL *c, const economic_enrollment_request &r, bool same)
{
	setup(c);
	auto a = command(r), b = same ? a : command(r);
	std::barrier start(2);
	critical_apply_result x, y;
	std::thread first(
		[&]
		{
			auto db = connect_fixture();
			start.arrive_and_wait();
			x = owner::apply(db, a);
			mysql_close(db);
		});
	std::thread second(
		[&]
		{
			auto db = connect_fixture();
			start.arrive_and_wait();
			y = owner::apply(db, b);
			mysql_close(db);
		});
	first.join();
	second.join();
	if ((same && (x.error_code || y.error_code)) ||
	    (!same && !((!x.error_code && y.error_code == ESTALE) ||
			(!y.error_code && x.error_code == ESTALE))))
		std::cerr << "enrollment concurrency same=" << same
			  << " left(outcome/code/revision)=" << static_cast<unsigned>(x.outcome)
			  << "/" << x.error_code << "/" << x.durable_revision
			  << " right=" << static_cast<unsigned>(y.outcome) << "/" << y.error_code
			  << "/" << y.durable_revision << std::endl;
	if (same)
	{
		assert(!x.error_code && !y.error_code && x.outcome != y.outcome);
		success(a, x, x.outcome == critical_apply_outcome::already_applied);
		success(b, y, y.outcome == critical_apply_outcome::already_applied);
		assert(x.result_payload == y.result_payload);
	}
	else
	{
		assert((!x.error_code) != (!y.error_code));
		// The lineage lock sees the winner's revision before native-history checks.
		const auto &loser = x.error_code ? x : y;
		assert(loser.error_code == ESTALE && !loser.durable_revision && !loser.result_size);
		success(x.error_code ? b : a, x.error_code ? y : x);
	}
	assert(scalar(c, "SELECT COUNT(*) FROM economic_account_mapping") == 1);
	assert(scalar(c, "SELECT revision FROM economic_lineage_state") == 1);
}
void retained_and_verifier(MYSQL *c, const economic_enrollment_request &r)
{
	setup(c);
	assert(economic_sql_verify_holding_source(c, false, r.native_id, r.source_digest) == EPERM);
	sql(c, "START TRANSACTION");
	using flag = std::remove_pointer_t<decltype(MYSQL_BIND{}.is_null)>;
	flag enabled = true;
	assert(!mysql_options(c, MYSQL_OPT_RECONNECT, &enabled));
	assert(economic_sql_verify_holding_source(c, false, r.native_id, r.source_digest) == EPERM);
	enabled = false;
	assert(!mysql_options(c, MYSQL_OPT_RECONNECT, &enabled));
	assert(!economic_sql_verify_holding_source(c, false, r.native_id, r.source_digest));
	assert(economic_sql_verify_holding_source(c, false, 444444, r.source_digest) == ENOENT);
	assert(c->server_status & SERVER_STATUS_IN_TRANS);
	sql(c, "ROLLBACK");
	auto op = command(r);
	auto applied = owner::apply(c, op);
	success(op, applied);
	sql(c, "UPDATE economic_lineage_state SET active_epoch=" + id(r.epoch));
	success(op, owner::reconcile(c, op), true);
	sql(c, "UPDATE economic_lineage_state SET active_epoch=NULL");
	sql(c, "DELETE FROM player_data WHERE pid=222");
	success(op, owner::reconcile(c, op), true);
	sql(c,
	    "INSERT INTO player_data(pid,name,account_name,racewar,copper,silver,wallet_revision) VALUES(222,'EnrollOne','enrollment_account',1,10,2,7)");
	auto second = r;
	second.expected_lineage_revision = 1;
	assert(owner::apply(c, command(second)).error_code == EEXIST);
	sql(c, "UPDATE economic_account_mapping SET account_kind=2,context_id=77");
	assert(owner::apply(c, command(second)).error_code == EEXIST);
	for (size_t offset :
	     { size_t(0), size_t(8), size_t(16), size_t(24), size_t(160), size_t(223) })
	{
		setup(c);
		op = command(r);
		success(op, owner::apply(c, op));
		auto at = std::to_string(offset + 1);
		sql(c, "UPDATE critical_operation_inbox SET result_payload=INSERT(result_payload," +
			       at + ",1,CHAR(ORD(SUBSTRING(result_payload," + at +
			       ",1)) ^ 1)) WHERE operation_id=" + id(op.operation_id));
		assert(owner::reconcile(c, op).error_code);
	}
	setup(c);
	sql(c, "UPDATE player_data SET account_name=NULL WHERE pid=222");
	auto null_observed = observations(c).front();
	sql(c, "START TRANSACTION");
	assert(!economic_sql_verify_holding_source(c, false, 222, null_observed.source_digest));
	sql(c, "UPDATE player_data SET account_name='' WHERE pid=222");
	assert(economic_sql_verify_holding_source(c, false, 222, null_observed.source_digest) ==
	       ESTALE);
	sql(c, "ROLLBACK");
	sql(c, "UPDATE player_data SET account_name='enrollment_account' WHERE pid=222");
}
int main()
{
	codec();
	auto c = connect_fixture();
	sql(c, "INSERT INTO accounts(account_name) VALUES('enrollment_account')");
	sql(c,
	    "INSERT INTO player_data(pid,name,account_name,racewar,copper,silver,wallet_revision) VALUES(222,'EnrollOne','enrollment_account',1,10,2,7),(223,'EnrollTwo','enrollment_account',1,0,0,0)");
	sql(c,
	    "INSERT INTO account_banks(id,account_name,racewar,bank_copper,bank_gold,bank_revision) VALUES(333,'enrollment_account',1,50,1,9)");
	setup(c);
	auto captured = observations(c);
	basic(c, captured);
	refusals(c, captured.front());
	faults(c, captured.front());
	unsigned long rounds = 1;
	if (const auto *value = getenv("DURIS_ENROLLMENT_CONCURRENCY_ROUNDS"))
	{
		char *end = nullptr;
		rounds = strtoul(value, &end, 10);
		assert(end && !*end && rounds >= 1 && rounds <= 100);
	}
	for (unsigned long round = 0; round < rounds; ++round)
	{
		std::cout << "enrollment concurrency round " << round + 1 << "/" << rounds
			  << std::endl;
		concurrency(c, captured.front(), false);
		concurrency(c, captured.front(), true);
	}
	retained_and_verifier(c, captured.front());
	assert(scalar(c, "SELECT copper FROM player_data WHERE pid=222") == 10);
	assert(scalar(c, "SELECT bank_copper FROM account_banks WHERE id=333") == 50);
	mysql_close(c);
	std::cout << "SQL initial enrollment, source binding, opening bridge and replay PASS\n";
}
#endif
