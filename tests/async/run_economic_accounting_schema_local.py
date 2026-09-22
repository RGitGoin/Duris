#!/usr/bin/env python3
"""Run accounting qualification in a private disposable local MariaDB instance.

Requires mariadb-server-core and mariadb-client-core; installs nothing and does
not use checkout configuration or an existing database. No system service starts.
"""
import argparse
import os
from pathlib import Path
import secrets
import shutil
import socket
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]

def run_local(suite="accounting"):
    for name in ("mariadb-install-db", "mariadbd", "mysql", "mysql_config"):
        if not shutil.which(name):
            raise SystemExit(f"missing prerequisite: {name}")
    if os.name != "posix":
        raise SystemExit("run inside the Linux build environment")
    with tempfile.TemporaryDirectory(prefix="duris-accounting-local-") as directory:
        root = Path(directory)
        data = root / "data"
        log = root / "server.log"
        with log.open("wb") as output:
            subprocess.run(["mariadb-install-db", "--no-defaults", f"--datadir={data}",
                            "--auth-root-authentication-method=normal", "--skip-test-db"],
                           stdout=output, stderr=subprocess.STDOUT, check=True)
            with socket.socket() as listener:
                listener.bind(("127.0.0.1", 0))
                port = listener.getsockname()[1]
            server = subprocess.Popen(["mariadbd", "--no-defaults", f"--datadir={data}",
                f"--socket={root / 'server.sock'}", f"--pid-file={root / 'server.pid'}",
                "--bind-address=127.0.0.1", f"--port={port}", "--skip-name-resolve",
                "--max-allowed-packet=64M", *(["--user=root"] if os.geteuid() == 0 else [])],
                stdout=output, stderr=subprocess.STDOUT)
            try:
                client = ["mysql", "--no-defaults", "--skip-ssl", "--protocol=tcp",
                          "-h", "127.0.0.1", "-P", str(port), "-u", "root", "-N", "-B"]
                env = os.environ.copy()
                for key in tuple(env):
                    if key.startswith("DB_") or key in ("MYSQL_PWD", "MYSQL_UNIX_PORT"):
                        env.pop(key)
                for attempt in range(90):
                    if server.poll() is not None:
                        raise RuntimeError("private MariaDB exited during startup")
                    ready = subprocess.run(client + ["-e", "SELECT @@datadir"], env=env,
                                           capture_output=True, timeout=5)
                    if ready.returncode == 0:
                        if Path(ready.stdout.decode().strip()).resolve() != data.resolve():
                            raise RuntimeError("loopback port does not belong to the private instance")
                        break
                    time.sleep(1)
                else:
                    raise RuntimeError("private MariaDB startup timed out")
                password = secrets.token_hex(24)
                database = "economic_schema_test_" + secrets.token_hex(6)
                sql = ("CREATE USER 'accounting_test'@'127.0.0.1' IDENTIFIED BY '" + password +
                       "'; GRANT ALL PRIVILEGES ON *.* TO 'accounting_test'@'127.0.0.1' WITH GRANT OPTION;" +
                       " CREATE DATABASE " + database + " CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;")
                subprocess.run(client, input=sql, text=True, env=env, check=True,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                env.update(ENVIRONMENT="test", DB_HOST="127.0.0.1", DB_PORT=str(port),
                    DB_USER="accounting_test", DB_PASSWD=password, MYSQL_PWD=password,
                    DB_NAME=database, ECONOMIC_ACCOUNTING_DISPOSABLE_SCHEMA="1",
                    DB_ALLOWED_TARGETS=f"127.0.0.1:{port}/{database}")
                client[client.index("root")] = "accounting_test"
                def sql_file(path):
                    with (ROOT / path).open("rb") as source:
                        subprocess.run(client + [database], stdin=source, env=env,
                                       cwd=ROOT, check=True)
                def run(*args):
                    print("Running " + " ".join(args), flush=True)
                    subprocess.run(args, cwd=ROOT, env=env, check=True)
                sql_file("migrations/bootstrap_multithread_safe.sql")
                run("python3", "scripts/migration_runner.py", "adopt", "--kind", "fresh_bootstrap")
                run("python3", "scripts/migration_runner.py", "run")
                run("python3", "scripts/migration_runner.py", "run")
                run("bash", "migrations/verify_runtime_compatibility.sh")
                sql_file("migrations/immutable/0031_economy_accounting.sql")
                run("bash", "migrations/immutable/0031_economy_accounting.sh")
                if suite == "currency":
                    run("python3", "tests/async/run_currency_transaction_local.py")
                elif suite == "wallet":
                    run("python3", "tests/async/run_economic_sql_bank_transaction_mysql.py")
                else:
                    run("python3", "tests/async/test_economic_accounting_schema_mysql.py", "-v")
                    run("python3", "migrations/verify_economic_baseline_schema.py")
                    run("python3", "tests/async/test_economic_baseline_schema_mysql.py", "-v")
                    run("bash", "tests/async/run_economic_accounting_authority_mysql.sh")
                    for test in ("test_economic_sql_bank_transaction_flatfile.py",
                                 "run_economic_sql_bank_transaction_mysql.py",
                                 "run_economic_sql_baseline_transaction_mysql.py",
                                 "run_economic_sql_source_snapshot_mysql.py",
                                 "run_economic_sql_enrollment_transaction_mysql.py"):
                        run("python3", "tests/async/" + test)
            finally:
                if server.poll() is None:
                    server.terminate()
                    try:
                        server.wait(timeout=30)
                    except subprocess.TimeoutExpired:
                        server.kill()
                        server.wait(timeout=10)
    print("Disposable local MariaDB qualification passed", flush=True)

def main():
    global ROOT
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--suite", choices=("accounting", "currency", "wallet"), default="accounting")
    args = parser.parse_args()
    # Some tracked helper scripts contain CRLF. Normalize an execution copy,
    # never the source checkout or sealed immutable migration bytes.
    helper = ROOT / "migrations/verify_runtime_compatibility.sh"
    if b"\r\n" not in helper.read_bytes():
        return run_local(args.suite)
    source = ROOT
    with tempfile.TemporaryDirectory(prefix="duris-accounting-source-") as directory:
        ROOT = Path(directory)
        try:
            for name in ("src", "migrations", "scripts", "tests", "docs"):
                shutil.copytree(source / name, ROOT / name,
                                ignore=shutil.ignore_patterns("__pycache__"))
            for script in ROOT.rglob("*.sh"):
                if "immutable" not in script.relative_to(ROOT).parts:
                    script.write_bytes(script.read_bytes().replace(b"\r\n", b"\n"))
            print("Using temporary LF helper scripts; sealed migrations unchanged", flush=True)
            run_local(args.suite)
        finally:
            ROOT = source

if __name__ == "__main__":
    main()
