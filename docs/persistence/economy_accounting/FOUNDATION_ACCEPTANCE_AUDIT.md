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
