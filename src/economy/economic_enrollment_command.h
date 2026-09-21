#ifndef DURIS_ECONOMIC_ENROLLMENT_COMMAND_H
#define DURIS_ECONOMIC_ENROLLMENT_COMMAND_H
#include "economy/economic_accounting_plan.h"

// Initial observed-incarnation enrollment, not historical provenance or authority
// to freeze/activate. Reused native identities require a separate typed lifecycle.
struct economic_enrollment_request
{
	critical_operation_id lineage = {}, epoch = {}, epoch_operation = {};
	uint64_t actor_id = 0, expected_lineage_revision = 0;
	economic_account_kind kind = economic_account_kind::wallet;
	uint64_t native_id = 0, context_id = 0, native_revision = 0;
	economic_coin_vector balance = {};
	economic_digest boundary_digest = {}, source_digest = {};
};
constexpr size_t ECONOMIC_ENROLLMENT_COMMAND_BYTES = 200;
constexpr uint64_t ECONOMIC_ENROLLMENT_FENCE = 0x45434f4e454e524c;
// Metadata-only schema-1 envelope, deliberately refused by legacy execution.
// Canonical fields and accepted_at_usec must be retained unchanged on retry.
economic_accounting_error economic_enrollment_command_build(const critical_operation_id &,
							    const economic_enrollment_request &,
							    uint64_t accepted_at_usec,
							    critical_command *);
economic_accounting_error economic_enrollment_command_decode(const critical_command &,
							     economic_enrollment_request *);
#endif
