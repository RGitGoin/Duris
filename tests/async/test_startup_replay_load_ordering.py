#!/usr/bin/env python3
"""Reproduce replay/load ordering with real pipelines and a synthetic repository.

This qualifies the ordering requirement, not SQL/flat-file durability or startup wiring.
"""
import shlex
import subprocess
import tempfile
from pathlib import Path
from _paths import rel

ROOT = Path(__file__).resolve().parents[2]
HARNESS = r"""
#include "persistence/critical_command_coordinator.h"
#include "persistence/critical_command_journal.h"
#include "player/player_load_pipeline.h"
#include "persistence/persistence_observability.h"
#include "sql/sql_pool.h"
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <mutex>

bool player_save_pipeline_save_admitted(int) { return true; }
extern "C" MYSQL *sql_pool_acquire(void) { return nullptr; }
extern "C" void sql_pool_release(MYSQL *) {}
bool player_load_request_valid(const player_load_request &r, uint64_t now) {
    return r.schema_version == PLAYER_LOAD_SCHEMA_VERSION && r.request_id &&
           r.pid > 0 && !r.account_name.empty() && r.deadline_usec > now;
}
player_load_result player_load_repository_execute(MYSQL *, const player_load_request &) {
    assert(false); return {};
}
struct repository {
    std::mutex mutex;
    std::condition_variable changed;
    bool applying = false, release = false;
    uint64_t wallet = 100, revision = 1;
};
critical_apply_result apply(const critical_command &, void *raw) {
    auto &db = *static_cast<repository *>(raw);
    std::unique_lock<std::mutex> lock(db.mutex);
    db.applying = true;
    db.changed.notify_all();
    db.changed.wait(lock, [&] { return db.release; });
    db.wallet = 75;
    db.revision = 2;
    return {critical_apply_outcome::applied, 1, 0};
}
player_load_result load(const player_load_request &r, void *raw) {
    auto &db = *static_cast<repository *>(raw);
    std::lock_guard<std::mutex> lock(db.mutex);
    player_load_result result = {};
    result.request_id = r.request_id; result.pid = r.pid;
    result.outcome = player_load_outcome::applied;
    result.domains.wallet[0] = db.wallet;
    result.domains.wallet_revision = db.revision;
    return result;
}
player_load_request request(uint64_t id) {
    player_load_request r = {};
    r.request_id = id; r.pid = 10; r.account_name = "synthetic-account";
    r.deadline_usec = persistence_observability_now_usec() + PLAYER_LOAD_TIMEOUT_USEC;
    return r;
}
int main(int argc, char **argv) {
    assert(argc == 2);
    critical_command command = {};
    command.schema_version = CRITICAL_COMMAND_SCHEMA_VERSION;
    assert(critical_operation_id_generate(&command.operation_id));
    command.type = critical_command_type::test;
    command.payload_version = 1;
    command.source_site = critical_source_site::command;
    command.deadline_class = critical_deadline_class::interactive;
    command.keys = {{critical_entity_type::player, 10}};
    command.payload = {1}; command.accepted_at_usec = 1700000000000010ULL;
    assert(critical_command_normalize(&command));
    assert(critical_command_journal_init(argv[1]));
    assert(critical_command_journal_append(command) == critical_command_journal_result::ok);
    critical_command_journal_shutdown();
    repository db;
    assert(player_load_pipeline_init(load, &db));
    assert(critical_command_coordinator_init(argv[1], apply, &db, 1));
    {
        std::unique_lock<std::mutex> lock(db.mutex);
        assert(db.changed.wait_for(lock, std::chrono::seconds(5), [&] { return db.applying; }));
    }
    // Successful coordinator initialization does not order a load after replay.
    player_load_result early = {}, fallback = {};
    assert(player_load_pipeline_wait(request(1), &early, 5000));
    assert(player_load_pipeline_execute_sync(request(2), &fallback));
    assert(early.domains.wallet[0] == 100 && early.domains.wallet_revision == 1);
    assert(fallback.domains.wallet[0] == 100 && fallback.domains.wallet_revision == 1);
    assert(!critical_command_coordinator_drain(0));
    {
        std::lock_guard<std::mutex> lock(db.mutex);
        db.release = true;
    }
    db.changed.notify_all();
    assert(critical_command_coordinator_drain(5000));
    // A completed read stays stale even after replay finishes: it must be discarded.
    assert(early.domains.wallet[0] == 100 && early.domains.wallet_revision == 1);
    player_load_result fresh = {}, fresh_fallback = {};
    assert(player_load_pipeline_wait(request(3), &fresh, 5000));
    assert(player_load_pipeline_execute_sync(request(4), &fresh_fallback));
    assert(fresh.domains.wallet[0] == 75 && fresh.domains.wallet_revision == 2);
    assert(fresh_fallback.domains.wallet[0] == 75 && fresh_fallback.domains.wallet_revision == 2);
    player_load_pipeline_shutdown();
    critical_command_coordinator_shutdown();
}
"""
with tempfile.TemporaryDirectory(prefix="duris-startup-order-") as directory:
    temp = Path(directory)
    source, binary = temp / "test.cpp", temp / "test"
    source.write_text(HARNESS)
    cflags = shlex.split(subprocess.check_output(["mysql_config", "--cflags"], text=True))
    libs = shlex.split(subprocess.check_output(["mysql_config", "--libs"], text=True))
    subprocess.run(["g++", "-std=c++20", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                    "-pthread", "-Isrc", *cflags, str(source),
                    *(rel(name) for name in ("critical_command.c", "critical_command_journal.c",
                      "critical_command_coordinator.c", "player_load_pipeline.c",
                      "persistence_observability.c")),
                    *libs, "-lz", "-lcrypto", "-o", str(binary)], cwd=ROOT, check=True)
    subprocess.run([str(binary), str(temp / "journal")], check=True, timeout=15)
print("replay/load ordering reproduced; post-drain fresh reads observe committed revision")
