#!/usr/bin/env python3
"""Native grant-boundary checks; submission is stubbed, not durable completion."""
import subprocess
import tempfile
from pathlib import Path
from _paths import extract_function

PRELUDE = r"""
#include <cassert>
#include <string>
struct character {};
struct object { bool released = false; };
using P_char = character *;
using P_obj = object *;
constexpr int FALSE = 0;
static bool accept;
static int submissions, releases;
static P_char recipient, initiator;
static P_obj submitted;
static std::string feedback;
bool item_creation_grant_submit_to_player(P_char ch, P_obj obj, P_char actor) {
    ++submissions; recipient = ch; submitted = obj; initiator = actor;
    assert(!obj->released);
    return accept;
}
void extract_obj(P_obj obj, int mode) {
    assert(mode == FALSE && !obj->released);
    obj->released = true; ++releases;
}
void send_to_char(const char *message, P_char ch) {
    assert(ch == recipient); feedback += message;
}
"""
DRIVER = r"""
int main() {
    character actor; object accepted, refused;
    accept = true;
    assert(grant_tradeskill_item(&actor, &accepted));
    assert(submissions == 1 && recipient == &actor && initiator == &actor);
    assert(submitted == &accepted && !accepted.released && releases == 0);
    assert(feedback.empty());
    // Queue acceptance transfers responsibility; helper must not retire output.
    accept = false;
    assert(!grant_tradeskill_item(&actor, &refused));
    assert(submissions == 2 && submitted == &refused);
    assert(refused.released && releases == 1 && !accepted.released);
    assert(feedback.find("no tradeskill item was created") != std::string::npos);
    feedback.clear();
    assert(!grant_tradeskill_item(&actor, nullptr));
    assert(submissions == 2 && releases == 1 && !feedback.empty());
}
"""
CASES = (
    ("economy/tradeskill.c", "bool grant_tradeskill_item(", "grant_tradeskill_item",
     "no tradeskill item was created", False),
    ("world/world_quest.c", "static bool grant_world_quest_reward(", "grant_world_quest_reward",
     "your quest reward was not created", True),
)
for path, signature, name, refusal_text, silent_null in CASES:
    driver = DRIVER.replace("grant_tradeskill_item", name).replace(
        "no tradeskill item was created", refusal_text)
    if silent_null:
        driver = driver.replace("releases == 1 && !feedback.empty()",
                                "releases == 1 && feedback.empty()")
    with tempfile.TemporaryDirectory(prefix="grant-boundary-") as directory:
        source = Path(directory) / "grant.cpp"
        binary = Path(directory) / "grant"
        source.write_text(PRELUDE + extract_function(path, signature) + driver)
        subprocess.run(["g++", "-std=c++20", "-fsanitize=address,undefined",
                        "-fno-omit-frame-pointer", str(source), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    print(name + ": acceptance, refusal cleanup and null output passed")
