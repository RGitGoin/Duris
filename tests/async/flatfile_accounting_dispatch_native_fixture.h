#pragma once
#include "flatfile/flatfile_accounting_dispatch.h"
#include <cassert>
#include <cerrno>

// The real typed owners and dispatcher are linked. Only the unrelated legacy
// dispatcher is wrapped; entering it fails the native accounting journey.
const char *persistence_mode_flatfile_root()
{
	return nullptr;
}
extern "C" critical_apply_result accounting_unexpected_legacy(const critical_command &, void *) asm(
	"__wrap__Z51flatfile_critical_command_repository_apply_selectedRK16critical_commandPv");
extern "C" critical_apply_result accounting_unexpected_legacy(const critical_command &, void *)
{
	assert(false && "native accounting journey must never enter legacy dispatcher");
	return { critical_apply_outcome::terminal_failure, 0, EINVAL };
}
