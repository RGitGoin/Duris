#!/usr/bin/env python3
"""Native flatfile save pipeline; controlled capture and accounting completion fixtures."""
from _paths import rel
from pathlib import Path
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[2]
NAMES = [
    'flatfile_player_repository.c',
    'player_load_topology.c',
    'flatfile_identity_repository.c',
    'flatfile_item_repository.c',
    'flatfile_collector_repository.c',
    'collector_command.c',
    'collector_codec.c',
    'collector_policy.c',
    'coin_transfer_command.c',
    'flatfile_player_snapshot_file.c',
    'flatfile_corpse_repository.c',
    'flatfile_locker_repository.c',
    'flatfile_world_item_repository.c',
    'flatfile_artifact_repository.c',
    'flatfile_shop_trade_repository.c',
    'flatfile_shop_trade_materialization.c',
    'flatfile_shopkeeper_repository.c',
    'flatfile_auction_repository.c',
    'flatfile_boon_repository.c',
    'flatfile_player_domain_repository.c',
    'flatfile_authority_transaction.c',
    'player_snapshot_codec.c',
    'flatfile_store.c',
    'item_transfer_command.c',
    'corpse_lifecycle_command.c',
    'shop_trade_command.c',
    'critical_command.c',
    'epic_command.c',
    'currency_command.c',
    'auction_command.c',
    'combat_outcome_command.c',
    'boon_reward_command.c',
    'boon_shop_command.c',
    'persistence_observability.c',
    'flatfile_ip_activity_repository.c',
    'economic_bank_publication.c',
    'currency_transaction.c',
    'economic_command_admission.c',
    'player_revision_state.c',
    'player_save_pipeline.c',
    'player_save_worker.c',
    'player_save_journal.c',
    'economic_accounting_types.c',
    'economic_accounting_plan.c',
    'economic_accounting_intent.c',
    'economic_currency_adapter.c',
]
with tempfile.TemporaryDirectory(prefix="duris-bank-native-save-") as temporary:
    binary = Path(temporary) / "test"
    subprocess.run(["g++", "-std=c++20", "-Wall", "-Wextra", "-Werror", "-O1", "-g",
        "-D__NO_MYSQL__", "-Isrc", "-Isrc/no_mysql", "-ffunction-sections", "-fdata-sections",
        "tests/async/economic_bank_native_save_test.cpp", *[rel(name) for name in NAMES],
        "-Wl,--gc-sections", "-lcrypto", "-pthread", "-o", str(binary)], cwd=ROOT, check=True)
    for mode in ("normal", "replay", "failure"):
        subprocess.run([str(binary), str(Path(temporary) / mode), mode], cwd=ROOT, check=True, timeout=30)
    print("Native bank save pipeline acknowledgement ordering passed", flush=True)
