#!/usr/bin/env python3
"""Native typed flatfile compound coin root journeys."""
import os
from pathlib import Path
import subprocess
import tempfile
from test_flatfile_accounting_store import ROOT, SOURCES


def main():
    sources = ["tests/async/flatfile_accounting_coin_test.cpp",
               "src/flatfile/flatfile_accounting_coin_transaction.c",
               "src/flatfile/flatfile_item_repository.c", "src/player/player_snapshot_codec.c",
               "src/economy/economic_coin_adapter.c", "src/economy/coin_transfer_command.c",
               "src/flatfile/flatfile_accounting_authority.c",
               "src/flatfile/flatfile_accounting_bank_transaction.c",
               "src/flatfile/flatfile_identity_repository.c",
               "src/flatfile/flatfile_player_domain_repository.c",
               "src/world/epic_command.c", "src/combat/combat_outcome_command.c", *SOURCES[1:]]
    with tempfile.TemporaryDirectory(prefix="duris-flat-coin-") as temporary:
        binary = Path(temporary) / "coin"
        subprocess.run([
            "g++", "-std=c++20", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
            "-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
            "-fno-pie", "-no-pie", "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections", "-D__NO_MYSQL__", "-DDURIS_FLATFILE_ACCOUNTING_TEST",
            "-DDURIS_FLATFILE_AUTHORITY_FAULT_TEST", "-Isrc", "-Isrc/no_mysql", *sources,
            "-Wl,--wrap=_Znwm,--wrap=_Znam", "-lcrypto", "-pthread", "-o", str(binary),
        ], cwd=ROOT, check=True)
        subprocess.run([str(binary), str(Path(temporary) / "state")], cwd=ROOT, check=True,
                       timeout=660,
                       env=dict(os.environ, ASAN_OPTIONS="detect_leaks=1:halt_on_error=1",
                                UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1"))


if __name__ == "__main__":
    main()
