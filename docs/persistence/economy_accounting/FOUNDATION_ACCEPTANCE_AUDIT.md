# Foundation acceptance audit for #474

Audited 2026-09-21 against original issues #475â€“#478 and current local work.
This is an acceptance gap report, not closure or merge evidence. No new PR is
needed for each remaining component. Finish an issue-sized batch first.

| Issue | Established within original scope | Remaining acceptance work |
| --- | --- | --- |
| #475 inventory/contracts | 115 named routes; identity, arithmetic, conservation, topology, admission and publication contracts; 13 golden fixtures | Complete candidate review/mapping, review search blind spots, freeze the bounded registry and refresh estimates/status. Only 33 unique sites of 2,731 candidates are mapped. The other 2,698 are unclassified candidates, not proven writers. |
| #476 pure plans/links | Bounded types and plans, canonical encoding, deterministic links, typed intent comparison, rejection tests, legacy replay fixtures and coordinator interfaces | Qualify the closing head, settle coordinator replay/publication-retention ownership explicitly, and satisfy #475 merge prerequisite. Later gameplay integration is not a prerequisite for this pure module. |
| #477 SQL storage | Additive tables/indexes; bank transaction-local evidence and rollback/lost-ACK coverage; local MySQL/MariaDB migration qualification | Common item/child/source append and finalization, compound savepoint rollback, application-role UPDATE/DELETE refusal with separate migration privileges, and common-adapter fault/invariant/original-ID deadlock coverage on both engines. Bank lock-timeout tests do not prove deadlock recovery. |
| #478 flat-file storage | Shared native-afterimage/evidence protocol, process/syscall crash tests, cold replay beyond cache, corruption/capacity refusal, sealed retention, lifecycle and backup registration | Bounded item/child bundle coverage with maximum accepted/first refused cases; shared native SQL/flat-file golden output and error parity. A baseline witness is not a native item mutation; pure SQL policy comparison is not native backend parity. |

## Evidence anchors

- #475: `../ECONOMY_ACCOUNTING.md`, `writers.json`, `registry.json`,
  `scripts/validate_economy_accounting.py`, and
  `tests/async/test_economy_accounting_contract.py`.
- #476: `src/economy/economic_accounting_types.h`, plan and currency-adapter sources/tests,
  `tests/async/test_economic_accounting_plan.py`, admission and replay fixtures.
- #477: `migrations/economy_accounting.sql`, `SQL_STORAGE.md`, `SQL_BANK.md`,
  SQL bank transaction and repository harnesses. Existing bank-only restrictions
  and missing application-role protection are explicit, not inferred readiness.
- #478: `FLATFILE_STORAGE.md`, `FLATFILE_BANK.md`, flat-file store/bank tests,
  `tests/async/test_flatfile_backup_manifest.py`, and backup manifest authority locks.

## Immediate deliverable

Complete #475 first, then qualify #476 as its dependent review batch. Do not
require backend enforcement to close the inventory or pure planning issue.
Storage issues need their own common item/compound acceptance work; they do not
implicitly require every later gameplay route or activation journey.

A separate `--census` validator gate now requires complete candidate mapping
without requiring runtime backend qualification. Draft validation remains useful
while incomplete; claiming `census_complete=true` with unmapped sites is rejected.
The current census correctly fails this gate with `writer census not complete`.
All 29 contract tests pass after the gate change. No registry was frozen and no
candidate was marked reviewed merely to pass validation.

The SQL enrollment component is preserved locally at b6805ca78738ff80968689c4589b6c1219372798,
with MySQL/MariaDB sanitizer/fault/race tests, client-free/admission checks and
both builds passed. It remains unpublished and does not complete #479.

No issue is closed by this audit. Distinguish local implementation, qualification,
review readiness, merge and closure in subsequent reports.

## First inventory review increment

After the initial audit, 41 function declarations were individually reviewed and
recorded as nonwriters with full declaration hashes. Two had previously been
incorrectly counted as writer mappings and were moved to declaration reviews.
Nine unique `mobconv.c` coordinates now map to `world.mobile_scaling`. Callers in
`world/db.c:2755`, `combat/mobcombat.c:1196` and `core/files.c:4783` establish that
this helper is used for both fresh NPCs and restored pets: it cannot establish
fresh issuance by itself. The four CLEAR_MONEY macro assignments in core/utils.h
remain writer candidates, not declaration exclusions.

The earlier 2,731 figure counts lexical hits, including repeated matches on one
line. There are 2,673 distinct path/line/family coordinates. Current classification
is 40 mapped writer coordinates, 41 reviewed declarations and 2,592 unclassified
coordinates. No completeness or backend status was promoted. This remains a
partial census, and the search's semantic blind spots still require review.

## World allocation, reset and money-helper review

The next review maps all 56 existing distinct census coordinates in world/db.c:
prototype NPC coin parsing, provisional object construction, and reset branches
B/C/A/O/P/G/E. Object allocation sets a candidate UID with no destination owner;
failed artifact/load/destination checks can discard candidates. Successful reset
placement is the proposed admission boundary, while replacing existing equipment
also moves an already-existing item. Neither allocation nor cleanup by itself
proves issuance/destruction. These are classifications for integration, not
claims that accounting is already enforced.

The scanner now also catches CLEAR_MONEY: its definition and all three current
callers. Justice guards and zombie-game NPCs are placed in a room before their
wallets are cleared, so an enclosing admission boundary must cover the clearing
or refuse it. Patrol setup also clears the wallet. Header macro assignments are
mapped as executable behavior, not excluded with prototypes.

Reviewed handler routes now include player/NPC wallet-to-pile paths and completion
cleanup, provisional pile construction, checked pile addition, both room-pile
merge branches, and recursive extraction. Player and NPC authority differ;
retiring a merged pile UID must not destroy its transferred value. Recursive
extract_obj and gone_for_good do not prove a genuine accounting destruction.
The helper review also observes pile->value mutation inside add_coins; broader
object-value aliases remain a semantic-search coverage question before freezing
the census. This review does not declare the lexical patterns exhaustive.

Verification for this increment: 35 contract tests and the full draft validator
pass. Current inventory: 117 routes, 2,735 raw hits, 2,677 distinct coordinates,
128 mapped writer coordinates, 41 reviewed declarations, 2,508 unclassified.
No census-complete flag, registry freeze or runtime coverage was promoted.

## Remaining handler publication and teardown review

All current lexical coordinates in src/world/handler.c are now mapped (this is
not a claim that lexical scanning proves complete semantic coverage). Separate
routes cover prototype-weight probes, NPC social flower creation, general live
inventory/equipment/container linking, corpse release/raise/resurrection and
nested-release publication, committed corpse destruction, legacy decay, and
character/pet teardown. Existing currency and extraction routes remain distinct.

Specific integration constraints retained from source review:

- obj_to_char can submit creation, refuse ownership, discard a provisional
  candidate, or crumble an item; it is not uniformly a projection helper.
- obj_to_char_at_end and obj_to_obj_at_end preserve load order but cannot be
  assumed to have the same authority checks as normal player publication.
- A committed corpse result owns its wallet/item effects. Callback cleanup of
  stale money or corpse objects must not append a second accounting event.
- Legacy decay can transfer child items out and destroy only the root; recursive
  no-destination cleanup has a different disposition.
- Durable-pet and terminal-save teardown retain durable ownership while freeing
  live objects. Other character-removal branches can drop or destroy items.

The scanner additionally includes obj_to_obj_at_end; all six hits were added,
with its single prototype explicitly reviewed as a declaration. Other new call
sites remain unclassified until their callers are reviewed. Current totals are
126 routes, 2,741 raw matches, 2,683 unique coordinates, 199 writer-mapped
coordinates, 42 reviewed declarations, and 2,442 unclassified coordinates.
36 contract tests pass; census completion and runtime qualification remain false.

Follow-up review covers all four newly discovered at-end caller sites: reward
container promotion/rollback and the legacy restore publication loop. Reward
children move out before container removal, with failed placements restored to
the original container; they are not newly issued rewards. Legacy deserialization
must retain identity or use explicit baseline policy before claiming projection.
The unreviewed remainder of restoreObjects is explicitly left open.
Final batch totals: 128 routes, 2,683 unique coordinates, 213 mapped writer
coordinates, 42 declarations, 2,428 unclassified. The raw census remains 2,741.

## Character files and initial SQL loader review

All current lexical coordinates in core/files.c are mapped. New routes distinguish
serializer comparison prototypes, acknowledged-save projection and terminal
unload, player/pet status decoding, single-object decoding, confiscation, and pet
snapshot equipment staging. The #if 0 confiscation-content and pet-extraction
blocks are recorded as dormant source, not claimed executable behavior.

- writeCharacter restores live equipment on failed/nonterminal save; successful
  terminal cleanup does not retire saved ownership. Flat-file new-player domain
  balances publish only after the required authoritative reads succeed.
- Legacy pet status reads saved currency then zeroes it; later convertMob can
  recalculate coins. A retained holding/baseline policy remains necessary.
- Confiscation combines item destruction with a cost-derived debt credit. It must
  not invent wallet currency; any promoted contents need linked transfer effects.
- SQL serializer probes, player status/items and shared-bank loading are now
  classified. Loading zeros into the bank before the account query is not a sink.
  Prototype allocation and rejection cleanup are not independent issuance events.
- SQL discovery now ignores keyword case. This exposes four existing lowercase
  account_banks statements: legacy row creation and direct deposit/withdrawals.
  Their transactions do not carry #474 identity/evidence and cannot bypass future
  activation. A current source search found no callers of the legacy deposit or
  withdrawal helpers outside sql_player.c/header; that is not dead-code proof.

Other SQL loader/storage coordinates remain unclassified; this does not declare
sql_player.c fully reviewed. Current inventory: 140 routes, 2,745 raw matches,
2,687 unique coordinates, 288 writer-mapped coordinates, 42 declarations, and
2,357 unclassified. All 37 contract tests pass. Census completion remains false.

## Remaining SQL storage and reward lifecycle review

All current lexical sql_player.c and account_reward.c coordinates are now mapped;
this does not prove semantic completeness. SQL routes distinguish locker/corpse
reconstruction, temporary migration cleanup, shop staging, produced-stock
representatives, saved-item snapshot replacement, and acknowledged source
retirement. A produced-stock representative requires an explicit authority policy;
source-row deletion after handoff must not retire the destination item.

Reward routes distinguish summon allocation/reservation/submission, expired-grant
revocation and duplicate cleanup, dismissal with retained entitlement, and corpse
container dissolution with child promotion. Existing eligibility-update failure
can precede extraction; future integration must resolve this persistence gap.
Grant markers are not proof of duplicate durable item identity.

Semantic follow-up remains necessary: reward cleanup writes player_items through
SQL outside the current table-pattern census. Lexical coverage is therefore not
the acceptance criterion by itself. Runtime coverage remains legacy/unverified.

Current inventory: 152 routes, 2,745 raw matches, 2,687 unique coordinates,
341 writer-mapped coordinates, 42 reviewed declarations, and 2,304 unclassified.
Census completion remains false. This batch changes inventory/documentation only.

## Crafting and tradeskill ownership review

All current lexical coordinates in economy/crafting.c and economy/tradeskill.c
are mapped. Recipe descriptions, entitlement lookup, list/stat/info previews,
material-name probes and post-refinement message probes are distinct from owned
ingredients and provisional outputs. Commented legacy forging is absent from
the executable census; parchment learning after unconditional return is explicitly
classified as dormant source.

Craft and forge consume ingredients before output grant submission. Chaos-pouch
variants require generated usage rather than nonexistent material UIDs, while
actual essence/tools/flux remain inputs. Smith links ore, cash and rolled output;
fishing and parchment publication are separate issuance routes. Bandage use
consumes an item before a delayed healing event. Recipe learning records an
entitlement before retiring its physical scroll. Epic-store purchase commits its
epic debit before item allocation/publication and uses a separate failure refund.

Integration constraints discovered in source, not fixed in this census batch:

- Smith searches ch inventory but rollback attaches selected ore to pl; preserve
  actual source ownership rather than assume rollback is neutral.
- Refinement reads the ore vnum after extract_obj; capture input metadata before
  retirement and distinguish random gameplay loss from technical output failure.
- Parchment rarity rejection returns with a provisional allocation outstanding.
- Successful grant submission is not proof of durable publication/save.

Current inventory: 163 routes, 2,745 raw matches, 2,687 unique coordinates,
451 writer-mapped coordinates, 42 reviewed declarations, 2,194 unclassified.
Draft validation passes 13 fixtures. Census and runtime qualification remain
incomplete; this batch changes inventory/documentation only.
