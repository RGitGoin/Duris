#!/usr/bin/env python3
"""Publication journal metadata, corruption and mixed-version rewrite checks."""
from pathlib import Path
import subprocess
import tempfile
from _paths import ROOT, rel
with tempfile.TemporaryDirectory(prefix="duris-publication-journal-") as temporary:
    work = Path(temporary)
    binary = work / "test"
    subprocess.run(["g++", "-std=c++20", "-Wall", "-Wextra", "-Werror", "-Isrc",
                    "-fsanitize=address,undefined", "-fno-pie", "-no-pie",
                    "tests/async/critical_command_publication_journal_test.cpp",
                    rel("critical_command.c"), rel("critical_command_journal.c"),
                    "-pthread", "-lcrypto", "-lz", "-o", str(binary)], cwd=ROOT, check=True)
    subprocess.run([str(binary), str(work / "state")], check=True, timeout=30)
print("publication journal mixed versions, duplicate policy, corruption and checkpoint preservation passed")
