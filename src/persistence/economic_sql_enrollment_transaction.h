#ifndef DURIS_ECONOMIC_SQL_ENROLLMENT_TRANSACTION_H
#define DURIS_ECONOMIC_SQL_ENROLLMENT_TRANSACTION_H
#include "economy/economic_enrollment_command.h"
#include "persistence/critical_command_completion.h"
#include <mysql/mysql.h>

// Initial wallet/bank enrollment only. The lifecycle caller must independently
// authorize and retain the frozen boundary. NULL active_epoch is not proof of
// never-activated state. No production admission or activation is provided here.
class economic_sql_enrollment_transaction
{
	friend class economic_sql_accounting_lifecycle_transaction;
#ifdef DURIS_ECONOMIC_SQL_ENROLLMENT_TEST
	friend class economic_sql_enrollment_test_access;
#endif
	// Own the reconnect-disabled READ COMMITTED transaction; refuse nested use.
	// All retained native history prevents a new lifetime, including retirement.
	// Original-ID replay checks retained creation fields, not today's native row.
	static critical_apply_result apply(MYSQL *, const critical_command &);
	static critical_apply_result reconcile(MYSQL *, const critical_command &);
	static critical_apply_result run(MYSQL *, const critical_command &, bool);
};
// Validates the canonical retained request and decodes its allocated lifetime.
// Does not authenticate the database or authorize any operation.
economic_accounting_error economic_enrollment_result_account(const critical_command &,
							     const critical_apply_result &,
							     economic_account_key *);
#endif
