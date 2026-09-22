#!/usr/bin/env python3
"""Real bank publisher, currency publication and revision state; controlled save/coordinator endpoints."""
from pathlib import Path
import subprocess
import tempfile
from test_flatfile_accounting_store import ROOT, SOURCES
with tempfile.TemporaryDirectory(prefix="duris-bank-publication-") as temporary:
    for mode in ("sql", "flatfile"):
        binary = Path(temporary) / mode
        flags = ["-D__NO_MYSQL__", "-Isrc/no_mysql"] if mode == "flatfile" else []
        subprocess.run(["g++", "-std=c++20", "-Wall", "-Wextra", "-Werror", "-O1", "-g",
            "-fsanitize=address,undefined", "-fno-pie", "-no-pie", "-ffunction-sections", "-fdata-sections",
            "-Isrc", "-I/usr/include/mysql", *flags,
            "tests/async/economic_bank_publication_test.cpp", "src/economy/economic_bank_publication.c",
            "src/economy/currency_transaction.c", "src/economy/economic_command_admission.c",
            "src/player/player_revision_state.c", *SOURCES[4:],
            "-Wl,--gc-sections", "-lcrypto", "-pthread", "-o", str(binary)], cwd=ROOT, check=True)
        subprocess.run([str(binary)], check=True, timeout=30)
        print(mode + " bank publication save-ack ordering passed", flush=True)
