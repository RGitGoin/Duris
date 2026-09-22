"""Characterize legacy coin routes that remain outside accounting enforcement."""

import json

from _paths import ROOT, SRC
from _source_contract import function_body


INVENTORY = json.loads(
    (ROOT / "docs/persistence/economy_accounting/writers.json").read_text(
        encoding="utf-8"
    )
)
TEST_PATH = "tests/async/test_legacy_coin_route_contract.py"


def body(path: str, signature: str) -> str:
    result = function_body((SRC / path).read_text(encoding="utf-8"), signature)
    assert result is not None, f"missing route definition: {path} {signature}"
    return result


def route(writer_id: str) -> dict:
    return next(row for row in INVENTORY["writers"] if row["id"] == writer_id)


def test_money_changer_tracks_both_conversion_directions_and_fees() -> None:
    exchange = body("specs/specs.mobile.c", r"\bint\s+money_changer\s*\(")

    assert "if (from > to)" in exchange and "if (from < to)" in exchange
    assert "RATE_TO_LOWER" in exchange
    assert all(rate in exchange for rate in ("RATE_TO_SILVER", "RATE_TO_GOLD", "RATE_TO_PLATINUM"))
    assert "ch->points.cash[from] -=" in exchange
    assert "ch->points.cash[to] +=" in exchange
    assert "currency_transaction_submit" not in exchange

    for writer_id in ("special.money_changer", "currency.mobile_exchange"):
        writer = route(writer_id)
        assert writer["integration_issue"] == 480
        assert writer["test_candidates"] == [TEST_PATH]


def test_smelter_separates_coin_handoff_from_paid_ore_processing() -> None:
    smelter = body("specs/specs.mobile.c", r"\bint\s+smelter\s*\(")

    assert "pl->points.cash[type] -= amount;" in smelter
    assert "ch->points.cash[type] += amount;" in smelter
    assert "SUB_MONEY(ch, amount * 1000, 0)" in smelter
    assert "finish_smelt(ch, pl, OBJ_VNUM(obj))" in smelter

    writer = route("special.smelter")
    assert writer["integration_issue"] == 480
    assert writer["test_candidates"] == [TEST_PATH]


def test_split_characterizes_group_credits_and_the_later_sender_debit() -> None:
    split = body("cmd/actoth.c", r"\bvoid\s+do_split\s*\(")

    assert "share = gold / group_size;" in split
    assert "ADD_MONEY(gl->ch, (int)(share * coin_value));" in split
    assert "SUB_MONEY(ch, (int)(given * split_coin_value), 0)" in split
    assert split.index("ADD_MONEY(gl->ch") < split.index("SUB_MONEY(ch")

    writer = route("currency.split")
    assert writer["integration_issue"] == 480
    assert writer["test_candidates"] == [TEST_PATH]


def test_npc_coin_pickup_is_live_only_and_destroys_the_source_pile() -> None:
    get = body("cmd/actobj.c", r"\bvoid\s+get\s*\(")

    pc_branch = get.index("if (IS_PC(ch) && o_obj->type == ITEM_MONEY)")
    npc_coin_branch = get.index("if ((o_obj->type == ITEM_MONEY)", pc_branch)
    branch = get[npc_coin_branch:]
    for field in ("o_obj->value[3] = 0;", "o_obj->value[2] = 0;",
                  "o_obj->value[1] = 0;", "o_obj->value[0] = 0;"):
        assert field in branch
    assert "got_p * 1000 + got_g * 100 + got_s * 10 + got_c" in branch
    assert branch.index("if (total_value <= 0)") < branch.index("ADD_MONEY(ch, total_value);")
    assert branch.index("ADD_MONEY(ch, total_value);") < branch.index("extract_obj(o_obj);")
    assert "currency_transaction_submit" not in branch

    writer = route("coin.npc_pile_pickup")
    assert writer["integration_issue"] == 480
    assert writer["test_candidates"] == [TEST_PATH]


def test_ship_coffer_claim_requires_owner_and_moves_the_full_balance() -> None:
    claim = body("ships/ship_control.c", r"\bint\s+claim_coffer\s*\(")

    assert claim.index("isname(GET_NAME(ch), SHIP_OWNER(ship))") < claim.index("ADD_MONEY(ch, ship->money)")
    assert "ADD_MONEY(ch, ship->money);" in claim
    assert claim.index("ADD_MONEY(ch, ship->money);") < claim.index("ship->money = 0;")

    writer = route("coin.ship_coffer_claim")
    assert writer["integration_issue"] == 480
    assert writer["test_candidates"] == [TEST_PATH]


def test_guild_deposit_and_withdraw_preserve_the_cross_owner_route_order() -> None:
    deposit = body("guild/assocs.c", r"\bvoid\s+Guild::deposit\s*\(")
    withdraw = body("guild/assocs.c", r"\bvoid\s+Guild::withdraw\s*\(")

    assert deposit.index("SUB_MONEY(member") < deposit.index("platinum += p;")
    assert deposit.index("platinum += p;") < deposit.index("writeCharacter(member")
    assert deposit.index("writeCharacter(member") < deposit.index("save();")

    assert "HAS_FINE(member)" in withdraw and "GT_OFFICER(GET_A_BITS(member))" in withdraw
    assert withdraw.index("ADD_MONEY(member") < withdraw.index("platinum -= p;")
    assert withdraw.index("platinum -= p;") < withdraw.index("writeCharacter(member")
    assert withdraw.index("writeCharacter(member") < withdraw.index("save();")

    for writer_id in ("coin.guild_deposit", "coin.guild_withdraw"):
        writer = route(writer_id)
        assert writer["integration_issue"] == 480
        assert writer["test_candidates"] == [TEST_PATH]


if __name__ == "__main__":
    test_money_changer_tracks_both_conversion_directions_and_fees()
    test_smelter_separates_coin_handoff_from_paid_ore_processing()
    test_split_characterizes_group_credits_and_the_later_sender_debit()
    test_npc_coin_pickup_is_live_only_and_destroys_the_source_pile()
    test_ship_coffer_claim_requires_owner_and_moves_the_full_balance()
    test_guild_deposit_and_withdraw_preserve_the_cross_owner_route_order()
    print("legacy coin route contracts passed (source characterization only)")
