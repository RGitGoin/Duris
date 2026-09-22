#!/usr/bin/env python3
"""Native NPC potion factory, with successful controlled allocations."""
from pathlib import Path
import subprocess
import tempfile
from _paths import ROOT, extract_function
PRELUDE = r'''
#include "core/prototypes.h"
#include "core/utils.h"
#include "core/utility.h"
#include "core/config.h"
#include "classes/salchemist.h"
#include <cassert>
potion potion_data[1]{};
obj_data objects[8]{};
static int allocations=0,published=0,waits=0;
static P_char expected=nullptr;
P_obj read_object(int vnum,int mode) {
    assert(vnum==850 && mode==VIRTUAL && allocations<8);
    return &objects[allocations++];
}
void obj_to_char(P_obj obj,P_char ch) {
    assert(ch==expected && obj==&objects[published++]);
    assert(obj->value[0]==MIN(50,GET_LEVEL(ch)));
}
void CharWait(P_char ch,int pulse) { assert(ch==expected && pulse==PULSE_VIOLENCE); ++waits; }
'''
DRIVER = r'''
int main() {
    char_data npc{}; npc.specials.act=ACT_ISNPC; expected=&npc; potion_data[0].vnum=850;
    for(int level:{1,25,50,60}) {
        npc.player.level=level; allocations=published=waits=0;
        assert(MobAlchemistGetPotions(&npc,0,3));
        assert(allocations==3 && published==3 && waits==1);
    }
    for(int count:{0,-1}) {
        allocations=published=waits=0;
        assert(MobAlchemistGetPotions(&npc,0,count));
        assert(allocations==0 && published==0 && waits==1);
    }
}
'''
with tempfile.TemporaryDirectory(prefix="alchemy-npc-potions-") as directory:
    cpp=Path(directory)/"test.cpp"; binary=Path(directory)/"test"
    cpp.write_text(PRELUDE+extract_function("salchemist.c","bool MobAlchemistGetPotions(")+DRIVER)
    subprocess.run(["g++","-std=c++20","-Wall","-Wextra","-Werror","-O1","-g",
                    "-fsanitize=address,undefined","-fno-omit-frame-pointer","-no-pie",
                    "-Isrc",str(cpp),"-o",str(binary)],cwd=ROOT,check=True)
    subprocess.run([str(binary)],check=True)
print("Native NPC potion factory success-path checks passed")
