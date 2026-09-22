"""Characterize the legacy slot machine's economic settlement routes."""

import json
from pathlib import Path

from _paths import extract_function


ROOT = Path(__file__).resolve().parents[2]
INVENTORY = json.loads(
    (ROOT / "docs/persistence/economy_accounting/writers.json").read_text(
        encoding="utf-8"
    )
)


def test_slot_machine_records_one_signed_net_wallet_settlement() -> None:
    slot = extract_function("specs/specs.object.c", "int slot_machine(")

    # Denomination conversion is shared by wager and payout, so the submitted amount
    # is the player's net result in copper, including multi-denomination bets.
    assert "static const int coin_values[] = { 1, 10, 100, 1000 };" in slot
    assert "payout_value = (int64_t)coins * coin_values[type];" in slot
    assert "net_value = payout_value - (int64_t)coinamt * coin_values[type];" in slot
    assert slot.count("currency_transaction_submit_wallet_value(") == 1
    assert "net_value > 0 ? currency_reason_type::wallet_reward" in slot
    assert "currency_reason_type::wallet_spend" in slot
    assert "OBJ_VNUM(obj), critical_source_site::command" in slot
    assert '"The slot result could not be recorded.' in slot

    route = next(row for row in INVENTORY["writers"] if row["id"] == "coin.slot_net_submission")
    assert route["integration_issue"] == 481 and route["reason"] == "coin_transfer"
    assert route["test_candidates"] == ["tests/async/test_slot_machine_economy_contract.py"]


def test_slot_machine_coupon_jackpot_remains_an_explicit_legacy_item_route() -> None:
    slot = extract_function("specs/specs.object.c", "int slot_machine(")

    assert "coins = 50000 * coinamt;" in slot
    assert "obj_to_char(read_object(44, VIRTUAL), ch);" in slot
    assert "obj_to_char(read_object(43, VIRTUAL), ch);" in slot

    route = next(row for row in INVENTORY["writers"] if row["id"] == "item.slot_jackpot_coupons")
    assert route["integration_issue"] == 482 and route["reason"] == "first_admission"
    assert route["coverage"] == "legacy"
    assert route["test_candidates"] == ["tests/async/test_slot_machine_economy_contract.py"]


if __name__ == "__main__":
    test_slot_machine_records_one_signed_net_wallet_settlement()
    test_slot_machine_coupon_jackpot_remains_an_explicit_legacy_item_route()
    print("slot machine economy contract passed (source characterization only)")
