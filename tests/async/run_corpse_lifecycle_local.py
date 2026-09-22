#!/usr/bin/env python3
"""Run the native corpse lifecycle owner in an explicitly disposable local schema."""
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
if (os.environ.get("ECONOMIC_ACCOUNTING_DISPOSABLE_SCHEMA") != "1" or
    os.environ.get("DB_HOST") != "127.0.0.1" or os.environ.get("DB_SOCKET") or
    not re.fullmatch(r"economic_schema_test_[A-Za-z0-9_]+", os.environ.get("DB_NAME", ""))):
    raise SystemExit("explicit disposable loopback schema required")
script = (ROOT / "tests/async/run_corpse_lifecycle_repository_schema_mysql.sh").read_text()
chunk = script.split("g++ -std=c++20", 1)[1].split('"$ROOT/bin/tests/corpse_lifecycle_repository_mysql_harness"', 1)[0]
files = re.findall(r"(?:tests|src)/[A-Za-z0-9_/.-]+\.(?:cpp|c)", chunk)
assert files[0] == "tests/async/corpse_lifecycle_repository_mysql_harness.cpp"
with tempfile.TemporaryDirectory(prefix="duris-corpse-owner-") as directory:
    executable = Path(directory) / "corpse"
    flags = ["g++", "-std=c++20", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
             "-pthread", "-O1", "-g", "-fsanitize=address,undefined",
             "-fno-omit-frame-pointer", "-fno-pie", "-no-pie", "-Isrc"]
    flags += shlex.split(subprocess.check_output(["mysql_config", "--cflags"], text=True))
    flags += files + shlex.split(subprocess.check_output(["mysql_config", "--libs"], text=True))
    subprocess.run(flags + ["-lcrypto", "-o", str(executable)], cwd=ROOT, check=True)
    subprocess.run([str(executable)], cwd=ROOT, check=True,
                   env=dict(os.environ, CORPSE_LIFECYCLE_TEST_DB_NAME=os.environ["DB_NAME"],
                            ASAN_OPTIONS="detect_leaks=1:halt_on_error=1",
                            UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1"))
print("Native corpse lifecycle owner qualification passed", flush=True)
