#ifndef DURIS_ECONOMIC_SQL_COIN_TRANSACTION_H
#define DURIS_ECONOMIC_SQL_COIN_TRANSACTION_H
#include "economy/economic_coin_adapter.h"
#include <mysql/mysql.h>
#include <memory>

// Borrowed root transaction; no commit, retry, inbox insertion or publication.
// The root owner reserves each child, invokes apply_endpoint in source order,
// completes its receipt/outbox, then calls finalize. Only this component's own
// successful native writes authorize appending the typed accounting evidence.
// A nonzero result_code skips children and finalizes an unchanged-state rejection.
// Runtime dispatch remains disabled until retained/root completion integration.
class economic_sql_coin_transaction
{
    public:
	~economic_sql_coin_transaction();
	economic_sql_coin_transaction(const economic_sql_coin_transaction &) = delete;
	economic_sql_coin_transaction &operator=(const economic_sql_coin_transaction &) = delete;
	static unsigned int prepare(MYSQL *, const critical_command &,
				    std::unique_ptr<economic_sql_coin_transaction> *);
	const critical_command &child_command(size_t index) const;
	const coin_transfer_result &result() const;
	unsigned int result_code() const;
	unsigned int apply_endpoint(size_t index);
	unsigned int finalize();
	// After root inbox/outbox completion, immediately before its owner commits.
	unsigned int verify_root_completion();
	// Caller supplies an active transaction with reconnect disabled. Retained
	// success and rejection receipts; no active epoch, current balances or outbox needed.
	static unsigned int verify_retained(MYSQL *, const critical_command &,
					    unsigned int result_code,
					    std::span<const uint8_t> result_payload);

    private:
	struct implementation;
	std::unique_ptr<implementation> state_;
	explicit economic_sql_coin_transaction(std::unique_ptr<implementation>);
};
#endif
