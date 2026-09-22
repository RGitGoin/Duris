#ifndef DURIS_ECONOMIC_BANK_PUBLICATION_H
#define DURIS_ECONOMIC_BANK_PUBLICATION_H
#include "persistence/critical_command_coordinator.h"
#include <cstddef>
// Game-thread owner of accounting bank publication and subsequent status save.
// Replay restores the immutable obligation, never an old actor callback.
bool economic_bank_publication_restore(const critical_command &, void *context);
critical_submit_result economic_bank_publication_submit(const critical_command &);
void economic_bank_publication_pulse();
size_t economic_bank_publication_pending();
void economic_bank_publication_reset();
#endif
