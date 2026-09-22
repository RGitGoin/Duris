#!/usr/bin/env python3
"""Production money helper bodies; controlled submission/claim endpoints, no DB."""
from pathlib import Path
import subprocess
import tempfile
from _paths import ROOT, extract_function

PRELUDE = r''' 
#include "core/prototypes.h"
#include "core/utils.h"
#include "core/utility.h"
#include "economy/currency_transaction.h"
#include "economy/auction_houses.h"
#include "persistence/persistence_checkpoint.h"
#include <cassert>
#include <cstdlib>
#include <string>
static bool accepted=true, claim_accepted=true;
static int submissions=0, claims=0;
static int64_t delta=0;
static std::string message;
[[noreturn]] int panic_corruption_int(const char *,const char *,...) { std::abort(); }
void logit(const char *,const char *,...) {}
void gmcp_char_vitals(P_char) {}
void act(const char *,int,P_char,P_obj,void *,int) {}
char *coin_stringv(int,int) { static char text[]="coins"; return text; }
void send_to_char(const char *text,P_char) { message=text; }
void mark_player_dirty_components(int,player_component_mask_t) {}
static void currency_adjustment_committed(P_char,bool,const currency_command_result &,
    unsigned int,const uint8_t *,size_t) {}
bool currency_transaction_submit_wallet_value(P_char,int64_t value,currency_reason_type reason,
    int64_t,critical_source_site,critical_deadline_class,currency_completion_fn completion,
    const void *context,size_t size) {
    assert(completion && !context && !size);
    assert(reason==(value>0?currency_reason_type::wallet_reward:currency_reason_type::wallet_spend));
    ++submissions; delta=value; return accepted;
}
bool insert_money_pickup(int pid,int amount) {
    assert(pid==42 && amount==17); ++claims; return claim_accepted;
}
'''
DRIVER = r'''
int main() {
    char_data actor{}; pc_only_data pc{}; actor.only.pc=&pc; pc.pid=42;
    GET_COPPER(&actor)=100;
    ADD_MONEY(&actor,17);
    assert(submissions==1 && delta==17 && claims==0 && GET_MONEY(&actor)==100);
    accepted=false; ADD_MONEY(&actor,17);
    assert(submissions==2 && claims==1 && GET_MONEY(&actor)==100);
    assert(message.find("auction house")!=std::string::npos);
    claim_accepted=false; ADD_MONEY(&actor,17);
    assert(submissions==3 && claims==2 && GET_MONEY(&actor)==100);
    assert(message.find("staff review")!=std::string::npos);
    ADD_MONEY(&actor,0); ADD_MONEY(&actor,-1);
    assert(submissions==3 && claims==2);
    assert(SUB_MONEY(&actor,17,0)==-1 && delta==-17 && GET_MONEY(&actor)==100);
    accepted=true; assert(SUB_MONEY(&actor,17,0)==0 && GET_MONEY(&actor)==100);
    const int before=submissions;
    assert(SUB_MONEY(&actor,101,0)==-1 && SUB_MONEY(&actor,0,0)==-1);
    assert(SUB_MONEY(&actor,1,1)==-1 && submissions==before && claims==2);
    // Transient NPC credits and change must conserve value across denomination boundaries.
    char_data npc{}; npc.specials.act=ACT_ISNPC;
    for(int amount:{1,9,10,11,99,100,101,999,1000,1001,1111}) {
        for(auto &coin:npc.points.cash) coin=0;
        ADD_MONEY(&npc,amount);
        assert(GET_MONEY(&npc)==amount);
        assert(SUB_MONEY(&npc,amount+1,0)==-1 && GET_MONEY(&npc)==amount);
        assert(SUB_MONEY(&npc,1,0)==0 && GET_MONEY(&npc)==amount-1);
        if(amount>1) assert(SUB_MONEY(&npc,amount-1,0)==0 && GET_MONEY(&npc)==0);
    }
    assert(submissions==before && claims==2);
    char name[]="fixture"; npc.player.short_descr=name; actor.player.name=name;
    actor.in_room=1; npc.in_room=2;
    assert(!transact(&actor,nullptr,&npc,17) && submissions==before);
    npc.in_room=1; accepted=false;
    assert(!transact(&actor,nullptr,&npc,17) && GET_MONEY(&npc)==0);
    accepted=true;
    obj_data merchandise{}; merchandise.cost=1000;
    assert(transact(&actor,&merchandise,&npc,17));
    assert(delta==-17 && GET_MONEY(&npc)==17 && GET_MONEY(&actor)==100);
    // Passing merchandise still charges cash: the production branch disables barter.
    assert(merchandise.cost==1000 && claims==2);
}
'''
with tempfile.TemporaryDirectory(prefix="money-helpers-") as directory:
    cpp=Path(directory)/"test.cpp"; binary=Path(directory)/"test"
    cpp.write_text(PRELUDE+extract_function("core/utility.c","void ADD_MONEY(")+
                   extract_function("core/utility.c","int SUB_MONEY(")+
                   extract_function("economy/shop.c","bool transact(")+DRIVER)
    subprocess.run(["g++","-std=c++20","-Wall","-Wextra","-Werror","-g","-O1",
                    "-fsanitize=address,undefined","-fno-omit-frame-pointer","-no-pie",
                    "-Isrc",str(cpp),"-o",str(binary)],cwd=ROOT,check=True)
    subprocess.run([str(binary)],check=True)
print("Native money helper submission, fallback and denomination checks passed")
