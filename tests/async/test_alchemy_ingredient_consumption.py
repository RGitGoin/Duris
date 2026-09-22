#!/usr/bin/env python3
"""Native ingredient selectors with production identity/preflight and controlled extraction endpoints."""
from pathlib import Path
import subprocess
import tempfile
from _paths import ROOT, extract_function, source
import re

PRELUDE = r'''
#include "core/prototypes.h"
#include "classes/salchemist.h"
#include "world/vnum.obj.h"
#include "magic/spells.h"
#include <cstring>
#include <cassert>
#include <vector>
index_data indexes[4]{};
P_index obj_index=indexes;
static std::vector<P_obj> consumed;
bool notch_skill(P_char, int skill, float chance) {
    assert(skill==SKILL_MIX && chance==6.25f); return true;
}
void extract_obj(P_obj obj, int) {
    for (auto prior:consumed) assert(prior!=obj);
    consumed.push_back(obj);
}
'''
DRIVER = r'''
int main() {
    char_data actor{};
    obj_data first{},unrelated{},second{},surplus{};
    first.R_num=second.R_num=surplus.R_num=0; unrelated.R_num=1;
    first.next_content=&unrelated; unrelated.next_content=&second;
    second.next_content=&surplus; actor.carrying=&first;
    indexes[0].virtual_number=VOBJ_FORAGE_NIGHTSHADE; indexes[1].virtual_number=200;
    assert(get_id_for(&first)==NIGHTSHADE && get_id_for(&unrelated)==WRONG_INGREDIENT);
    for (bool poison:{false,true}) {
        int requirements[MAX_INGREDIENTS+1]{};
        requirements[0]=requirements[1]=poison?VOBJ_FORAGE_NIGHTSHADE:NIGHTSHADE;
        assert(got_all_poison_ingredients(&actor,requirements)==(poison?1:0));
        assert(got_all_ingredients(&actor,requirements)==(poison?0:1));
        consumed.clear();
        if(poison) extract_used_poison_ingredients(&actor,requirements);
        else extract_used_ingredients(&actor,requirements);
        assert((consumed==std::vector<P_obj>{&first,&second}));
        assert(requirements[0]==(poison?VOBJ_FORAGE_NIGHTSHADE:NIGHTSHADE) && requirements[1]==requirements[0]);
        // These helpers consume available matches; callers must preflight sufficiency.
        requirements[2]=requirements[3]=requirements[0];
        assert(!got_all_poison_ingredients(&actor,requirements));
        assert(!got_all_ingredients(&actor,requirements));
        consumed.clear();
        if(poison) extract_used_poison_ingredients(&actor,requirements);
        else extract_used_ingredients(&actor,requirements);
        assert((consumed==std::vector<P_obj>{&first,&second,&surplus}));
        actor.carrying=nullptr; consumed.clear();
        if(poison) extract_used_poison_ingredients(&actor,requirements);
        else extract_used_ingredients(&actor,requirements);
        assert(consumed.empty()); actor.carrying=&first;
    }
}
'''
with tempfile.TemporaryDirectory(prefix="alchemy-ingredients-") as directory:
    cpp=Path(directory)/"test.cpp"; binary=Path(directory)/"test"
    table=re.search(r"int basic_ingredients\[\] = .*?;", source("salchemist.c").read_text(), re.S)
    assert table
    cpp.write_text(PRELUDE+table.group(0)+extract_function("salchemist.c","int get_id_for(")+
                   extract_function("salchemist.c","int got_all_ingredients(")+
                   extract_function("salchemist.c","int got_all_poison_ingredients(")+extract_function("salchemist.c","void extract_used_poison_ingredients(")+
                   extract_function("salchemist.c","void extract_used_ingredients(")+DRIVER)
    subprocess.run(["g++","-std=c++20","-Wall","-Wextra","-Werror","-O1","-g",
                    "-fsanitize=address,undefined","-fno-omit-frame-pointer","-no-pie",
                    "-Isrc",str(cpp),"-o",str(binary)],cwd=ROOT,check=True)
    subprocess.run([str(binary)],check=True)
print("Native alchemy ingredient selection and consumption checks passed")
