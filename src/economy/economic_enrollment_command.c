#include "economy/economic_enrollment_command.h"
#include <algorithm>
#include <climits>
#include <new>

namespace
{
using error = economic_accounting_error;
void put(std::vector<uint8_t> &b, size_t at, uint64_t value, size_t width = 8)
{
	for (size_t n = 0; n < width; ++n)
		b[at + n] = static_cast<uint8_t>(value >> (8 * n));
}
uint64_t get(const std::vector<uint8_t> &b, size_t at, size_t width = 8)
{
	uint64_t value = 0;
	for (size_t n = 0; n < width; ++n)
		value |= uint64_t(b[at + n]) << (8 * n);
	return value;
}
bool nonzero(const economic_digest &value)
{
	return std::any_of(value.begin(), value.end(), [](uint8_t b) { return b != 0; });
}
}
economic_accounting_error economic_enrollment_command_build(const critical_operation_id &operation,
							    const economic_enrollment_request &r,
							    uint64_t accepted,
							    critical_command *out)
{
	if (!out || !accepted || critical_operation_id_is_zero(operation) ||
	    critical_operation_id_is_zero(r.lineage) || critical_operation_id_is_zero(r.epoch) ||
	    critical_operation_id_is_zero(r.epoch_operation) ||
	    operation.bytes == r.epoch_operation.bytes || !r.actor_id || !r.native_id ||
	    !nonzero(r.boundary_digest) || !nonzero(r.source_digest) ||
	    (r.kind != economic_account_kind::wallet && r.kind != economic_account_kind::bank) ||
	    (r.kind == economic_account_kind::wallet &&
	     (r.context_id || r.native_id > INT32_MAX)) ||
	    (r.kind == economic_account_kind::bank &&
	     (r.context_id > INT8_MAX || r.native_id > UINT32_MAX)))
		return error::invalid_identity;
	for (auto amount : r.balance)
		if (amount < 0)
			return error::invalid_identity;
	int64_t total = 0;
	auto value_error = economic_coin_value(r.balance, &total);
	if (value_error != error::ok)
		return value_error;
	try
	{
		critical_command result = {};
		result.schema_version = CRITICAL_COMMAND_SCHEMA_VERSION;
		result.operation_id = operation;
		result.type = critical_command_type::economic_enrollment;
		result.payload_version = 1;
		result.source_site = critical_source_site::operator_repair;
		result.deadline_class = critical_deadline_class::recovery;
		result.accepted_at_usec = accepted;
		result.keys.push_back({ critical_entity_type::system, ECONOMIC_ENROLLMENT_FENCE });
		result.payload.resize(ECONOMIC_ENROLLMENT_COMMAND_BYTES);
		const std::array<uint8_t, 4> magic = { 'E', 'L', 'C', '1' };
		std::copy(magic.begin(), magic.end(), result.payload.begin());
		put(result.payload, 4, 1, 2);
		put(result.payload, 6, ECONOMIC_ENROLLMENT_COMMAND_BYTES, 2);
		std::copy(r.lineage.bytes.begin(), r.lineage.bytes.end(),
			  result.payload.begin() + 8);
		std::copy(r.epoch.bytes.begin(), r.epoch.bytes.end(), result.payload.begin() + 24);
		std::copy(r.epoch_operation.bytes.begin(), r.epoch_operation.bytes.end(),
			  result.payload.begin() + 40);
		put(result.payload, 56, r.actor_id);
		put(result.payload, 64, r.expected_lineage_revision);
		put(result.payload, 72, static_cast<uint16_t>(r.kind), 2);
		put(result.payload, 80, r.native_id);
		put(result.payload, 88, r.context_id);
		put(result.payload, 96, r.native_revision);
		for (size_t i = 0; i < 4; ++i)
			put(result.payload, 104 + 8 * i, static_cast<uint64_t>(r.balance[i]));
		std::copy(r.boundary_digest.begin(), r.boundary_digest.end(),
			  result.payload.begin() + 136);
		std::copy(r.source_digest.begin(), r.source_digest.end(),
			  result.payload.begin() + 168);
		if (!critical_command_envelope_valid(result))
			return error::corrupt_evidence;
		*out = std::move(result);
		return error::ok;
	}
	catch (const std::bad_alloc &)
	{
		return error::capacity;
	}
}
economic_accounting_error economic_enrollment_command_decode(const critical_command &command,
							     economic_enrollment_request *out)
{
	if (!out || !critical_command_envelope_valid(command) ||
	    command.type != critical_command_type::economic_enrollment ||
	    command.payload.size() != ECONOMIC_ENROLLMENT_COMMAND_BYTES)
		return error::corrupt_evidence;
	try
	{
		economic_enrollment_request r;
		std::copy_n(command.payload.begin() + 8, 16, r.lineage.bytes.begin());
		std::copy_n(command.payload.begin() + 24, 16, r.epoch.bytes.begin());
		std::copy_n(command.payload.begin() + 40, 16, r.epoch_operation.bytes.begin());
		r.actor_id = get(command.payload, 56);
		r.expected_lineage_revision = get(command.payload, 64);
		r.kind = static_cast<economic_account_kind>(get(command.payload, 72, 2));
		r.native_id = get(command.payload, 80);
		r.context_id = get(command.payload, 88);
		r.native_revision = get(command.payload, 96);
		for (size_t i = 0; i < 4; ++i)
		{
			auto amount = get(command.payload, 104 + 8 * i);
			if (amount > uint64_t(INT64_MAX))
				return error::corrupt_evidence;
			r.balance[i] = static_cast<int64_t>(amount);
		}
		std::copy_n(command.payload.begin() + 136, 32, r.boundary_digest.begin());
		std::copy_n(command.payload.begin() + 168, 32, r.source_digest.begin());
		critical_command expected;
		auto status = economic_enrollment_command_build(
			command.operation_id, r, command.accepted_at_usec, &expected);
		if (status != error::ok)
			return status;
		std::vector<uint8_t> a, b;
		if (critical_command_encode(command, &a) != critical_command_codec_result::ok ||
		    critical_command_encode(expected, &b) != critical_command_codec_result::ok)
			return error::capacity;
		if (a != b)
			return error::corrupt_evidence;
		*out = std::move(r);
		return error::ok;
	}
	catch (const std::bad_alloc &)
	{
		return error::capacity;
	}
}
