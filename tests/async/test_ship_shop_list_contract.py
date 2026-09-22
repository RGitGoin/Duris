#!/usr/bin/env python3
from _paths import SRC, extract_function
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
source = (SRC / "ships" / "ship_shop.c").read_text()

shop = source[source.index("int ship_shop_proc(") :]
list_branch = shop[shop.index("if (cmd == CMD_LIST)") : shop.index("if (cmd == CMD_BUY)")]

assert "if (!*arg1)" in list_branch
assert "return list_hulls(ch, ship, owned);" in list_branch
assert list_branch.index("if (!*arg1)") < list_branch.index("if (*arg1)")

print("[PASS] bare list at a ship shop defaults to the purchasable hull catalog")

# Whole-ship sale deliberately has no reachable payout/destruction path. Match
# the complete entry prefix, not merely the presence of a return somewhere in
# the function: moving the refusal below a mutation must fail this contract.
import re
sale = extract_function("ships/ship_shop.c", "int sell_ship(")
entry = sale[sale.index("{") + 1:sale.index("return TRUE;") + len("return TRUE;")]
assert re.fullmatch(
    r'\s*int i = 0, k = 0, j;\s*'
    r'send_to_char\("&\+RSelling ships completely is not allowed\.&N\\r\\n", ch\);\s*'
    r'return TRUE;', entry
), "Whole-ship sale must refuse before any mutation or argument-dependent branch"
assert sale.index("return TRUE;") < sale.index("ADD_MONEY(ch, cost);")
assert sale.index("return TRUE;") < sale.index("shipObjHash.erase(ship);")
assert sale.index("return TRUE;") < sale.index("delete_ship(ship);")
print("[PASS] whole-ship sale refuses unconditionally before payout and destruction")
