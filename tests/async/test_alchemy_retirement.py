#!/usr/bin/env python3
"""Native timed item retirement with controlled world/extraction endpoints."""
from pathlib import Path
import subprocess
import tempfile
from _paths import ROOT, extract_function
PRELUDE = r'''
#include "core/prototypes.h"
#include "core/utils.h"
#include "core/utility.h"
#include "cmd/interp.h"
#include "net/comm.h"
#include <cassert>
room_data rooms[1]{};
room_data *world=rooms;
static int extractions=0;
static P_obj extracted=nullptr;
void extract_obj(P_obj obj,int cascade) { assert(cascade==TRUE); extracted=obj; ++extractions; }
void act(const char *,int,P_char,P_obj,void *,int) {}
int number(int low,int high) { assert(low==20 && high==35); return 20; }
'''
DRIVER = r'''
int main() {
    obj_data item{}; item.loc_p=LOC_ROOM; item.loc.room=0; item.value[5]=42;
    assert(thrusted_eq_proc(&item,nullptr,CMD_SET_PERIODIC,nullptr)==TRUE);
    assert(!thrusted_eq_proc(&item,nullptr,CMD_LOOK,nullptr) && extractions==0);
    item.loc_p=0;
    assert(!thrusted_eq_proc(&item,nullptr,CMD_PERIODIC,nullptr) && extractions==0);
    item.loc_p=LOC_ROOM;
    char_data owner{},other{},npc{}; pc_only_data owner_pc{},other_pc{};
    owner.only.pc=&owner_pc; other.only.pc=&other_pc; owner_pc.pid=42; other_pc.pid=43;
    npc.specials.act=ACT_ISNPC;
    rooms[0].people=&other; other.next_in_room=&npc; npc.next_in_room=&owner;
    owner.points.hit=95; owner.points.max_hit=100;
    other.points.hit=10; other.points.max_hit=100;
    npc.points.hit=20; npc.points.max_hit=100;
    assert(!thrusted_eq_proc(&item,nullptr,CMD_PERIODIC,nullptr) && extractions==0);
    assert(owner.points.hit==100 && other.points.hit==30 && npc.points.hit==40);
    npc.next_in_room=nullptr;
    assert(!thrusted_eq_proc(&item,nullptr,CMD_PERIODIC,nullptr));
    assert(extractions==1 && extracted==&item);
    rooms[0].people=nullptr;
    obj_data empty_room_item{}; empty_room_item.loc_p=LOC_ROOM; empty_room_item.loc.room=0;
    assert(!thrusted_eq_proc(&empty_room_item,nullptr,CMD_PERIODIC,nullptr));
    assert(extractions==2 && extracted==&empty_room_item);
}
'''
with tempfile.TemporaryDirectory(prefix="alchemy-retirement-") as directory:
    cpp=Path(directory)/"test.cpp"; binary=Path(directory)/"test"
    cpp.write_text(PRELUDE+extract_function("utility.c","int BOUNDED(")+extract_function("salchemist.c","int thrusted_eq_proc(")+DRIVER)
    subprocess.run(["g++","-std=c++20","-Wall","-Wextra","-Werror","-O1","-g",
                    "-fsanitize=address,undefined","-fno-omit-frame-pointer","-no-pie",
                    "-Isrc",str(cpp),"-o",str(binary)],cwd=ROOT,check=True)
    subprocess.run([str(binary)],check=True)
print("Native alchemy timed retirement checks passed")
