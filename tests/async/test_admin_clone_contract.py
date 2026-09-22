"""Characterize trusted clone creation and publication surfaces for accounting census."""

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ACTWIZ = (ROOT / "src/cmd/actwiz.c").read_text(encoding="utf-8")
INTERP = (ROOT / "src/cmd/interp.c").read_text(encoding="utf-8")
INVENTORY = json.loads(
    (ROOT / "docs/persistence/economy_accounting/writers.json").read_text(encoding="utf-8")
)


def body(source: str, start: str, end: str) -> str:
    begin = source.index(start)
    return source[begin : source.index(end, begin)]


clone_factory = body(ACTWIZ, "struct obj_data *clone_obj(P_obj obj)", "void clone_container_obj")
container_clone = body(ACTWIZ, "void clone_container_obj(P_obj to, P_obj obj)", "void do_clone")
clone_command = body(ACTWIZ, "void do_clone(P_char ch, char *argument, int", "void do_knock")

# The trust threshold is supplied by command registration, not inferred from do_clone.
assert "CMD_GRT(CMD_CLONE, STAT_DEAD + POS_PRONE, do_clone, LESSER_G);" in INTERP
assert "if (IS_NPC(ch))" in clone_command
assert 'count = 1;' in clone_command and "if (count > 100)" in clone_command

# Cloning allocates new instances and copies mutable fields; it does not unlink the source.
assert "read_object(obj->R_num, REAL)" in clone_factory
assert "ocopy->value[i] = obj->value[i]" in clone_factory
assert "ocopy->timer[i] = obj->timer[i]" in clone_factory
assert "obj_from_char(" not in clone_factory and "obj_from_room(" not in clone_factory

# Nested contents are recursively copied and published into the new container tree.
assert "ocopy = clone_obj(tmp);" in container_clone
assert "clone_container_obj(ocopy, tmp);" in container_clone
assert "obj_to_obj(ocopy, to);" in container_clone

# A cloned mob receives copied equipment/carrying; object clones go to the actor or room.
assert "ocopy = clone_obj(mob->equipment[j]);" in clone_command
assert "equip_char(mcopy, ocopy, j, 0);" in clone_command
assert "obj_to_char(ocopy, mcopy);" in clone_command
assert "obj_to_char(ocopy, ch);" in clone_command
assert "obj_to_room(ocopy, ch->in_room);" in clone_command

route = next(row for row in INVENTORY["writers"] if row["id"] == "item.admin_clone")
assert route["integration_issue"] == 486 and route["reason"] == "first_admission"
assert route["test_candidates"] == ["tests/async/test_admin_clone_contract.py"]
assert len(route["sites"]) == 6

print("admin clone census contract passed (source characterization; accounting enforcement remains unverified)")
