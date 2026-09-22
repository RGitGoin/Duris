# Foundation acceptance audit for #474

Audited 2026-09-21 against original issues #475-#478; live #475 requirements rechecked after raw classification.
This is an acceptance gap report, not closure or merge evidence. No new PR is
needed for each remaining component. Finish an issue-sized batch first.

| Issue | Established within original scope | Remaining acceptance work |
| --- | --- | --- |
| #475 inventory/contracts | All 2,716 lexical coordinates classified: 2,668 mapped across 852 routes and 48 declarations; 13 golden fixtures and 45 contract tests pass | Review indirect/aliased mutations against current master; reconcile integration ownership; retain completed source/destination descriptions and finish executable test links (641 routes currently have no test candidates); freeze registry and refresh delivery status. Runtime enforcement is not required to close this contract issue. |
| #476 pure plans/links | Bounded types and plans, canonical encoding, deterministic links, typed intent comparison, rejection tests, legacy replay fixtures and coordinator interfaces | Repeat qualification on the final delivery head and satisfy #475 merge prerequisite. Replay/publication ownership is now explicit in the public contract; current pure-module checks passed at 623ae3803. Later gameplay integration is not a prerequisite for this pure module. |
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

## Shop payment, stock and completion review

All current lexical economy/shop.c coordinates are mapped. Produced-item
allocation differs from transfer of existing stock. Flatfile linked trade
submission differs from payment-then-grant sequencing; callback publication,
unadmitted cleanup and committed stock retirement have separate classifications.

Existing-stock purchase currently pays, detaches stock, then uses a creation-grant
interface whose refusal extracts the item. Integration must retain prior ownership
and recover the payment/stock together. Produced-purchase refunds credit the player
before keeper lookup/debit, requiring retained reversal identity and reconciliation
of partial failure. Callback failures after durable commit retain a publication
obligation; they must not generate new economic effects.

Invalid-stock removal can be triggered by buy, peruse or list. Keeper death moves
artifacts to a valid room and destroys other stock. Repair probes a template and
can mutate attack type before payment; its item change must remain linked to cost.
Peruse has a separate inspection fee. The barter branch is dormant because transact
sets merchandise to null; its lexical sites are not active runtime coverage.

Current inventory: 172 routes, 2,745 raw matches, 2,687 unique coordinates,
500 writer-mapped coordinates, 42 reviewed declarations, 2,145 unclassified.
Census completion and runtime qualification remain false. No runtime code changed.

## Currency helpers and shared-bank publication

All current lexical core/utility.c coordinates are mapped. Persisted-player
ADD_MONEY/SUB_MONEY paths submit asynchronous wallet changes; NPC and nonpersistent
paths mutate native denominations. Returned change belongs to the same debit.
Generic wallet reward/spend reasons do not establish caller-specific transfer,
refund or issuance provenance. SUB_BALANCE likewise reports accepted submission,
not completed payment.

ADD_MONEY can fall back to an auction-house pickup claim after submission failure.
The claim and wallet credit require one retained original identity; a log or staff
message is not recovery evidence. The generic completion callback only reports
rejection, so callers cannot assume their item/service outcome is reversible.

Shared-bank publication copies one holding to matching online account/racewar
characters. It is not a separate grant per character. Single-denomination updates
have no revision fence. Full snapshots reject older explicit revisions, while the
compatibility UINT64_MAX path bypasses the fence and revision update; caller
ordering/lifetime evidence remains necessary.

Current inventory: 175 routes, 2,745 raw matches, 2,687 unique coordinates,
527 writer-mapped coordinates, 42 reviewed declarations, 2,118 unclassified.
Census and runtime qualification remain incomplete. Inventory/documentation only.

## Item movement and creation publication review

All current lexical item/item_movement_transaction.c coordinates are mapped.
Separate routes cover bounded movement submission, creation queue adapters,
unadmitted disposal, committed reconstruction, live grant publication and corpse
batch projection. Reconstruction can temporarily displace conflicting live UIDs;
failure restores them, while success removes superseded live copies. Neither is
an independent economic destruction or creation.

Single-root submission can adopt an absent registry item through a creation
command before continuing its requested transfer. Absence is not proof of fresh
issuance; baseline/adoption provenance and continuation linkage remain required.
Mobile claims reject absent registry ownership. Single creation grants with
existing registry ownership choose operator_repair instead of creation.

Journal-uncertain submissions retain the original pending identity. Before-entry
cancellation retains active journaled roots and disposes only unsubmitted tails.
Committed publication failure retains its queue. Dirty component marking after
publication is not durable save acknowledgment; existing helpers are not proof
that #474 accounting integration is qualified.

Current inventory: 179 routes, 2,745 raw matches, 2,687 unique coordinates,
552 writer-mapped coordinates, 42 reviewed declarations, 2,093 unclassified.
Census and runtime qualification remain incomplete. Inventory/documentation only.

## Locker custody, projection and fees

Live issue #475 was rechecked: it remains open and requires every identified writer
to have an integration owner/classification, with unsupported paths visible; its
scope explicitly excludes gameplay changes. This census remains required work.

All current lexical item/storage_lockers.c coordinates are mapped. Sorting shells,
private-chest materialization, child-preserving shell teardown, temporary access
probes and save/load staging are distinct from authoritative asset creation.
Stable locker/chest identity must span temporary character, room and container
representations. Money piles stay visible on the floor when sorting into chests
is inappropriate. Private saves must succeed before public-item staging proceeds.

Room reuse currently extracts leftovers; prior-session save/disposition must prove
they are disposable copies. Corpse ejection and artifact storage refusal transfer
existing roots rather than create replacements. Synthetic nontransferable chest
shells need explicit representation policy, not silent repeated issuance.

Entry fees submit wallet/bank payments after entry. Private chest metadata is
created before payment submission, with attempted deletion on bank submission
refusal; wallet return is ignored. Link service/entitlement and payment outcomes,
including uncertain commit or failed compensation, in the owning integration.

Current inventory: 188 routes, 2,745 raw matches, 2,687 unique coordinates,
578 writer-mapped coordinates, 42 reviewed declarations, 2,067 unclassified.
Census and runtime qualification remain incomplete. Inventory/documentation only.

## Generated zones, relics and reset disposition

All current lexical world/random.zone.c coordinates are mapped. Generated signs
and entrances need an explicit nontransferable-feature policy; takeable no-rent
sigils remain transferable assets. Loot construction includes nested chest
contents, key payload, guardian custody and epic stones. Moving a populated chest
from room to guardian does not issue its contents again. The chest level-potion
branch is explicitly disabled; the relic activation reward branch is active.

Quest turn-in consumes a sigil before group epic awards and random item rewards.
Retain the recipient set, rolled payloads and reward-loop children under the
original turn-in. Labyrinth generation probes existing artifact tracking before
creating a guardian relic; probe cleanup and rejected candidates are not sinks for
the tracked relic. Reset destroys selected objects but transfers surviving corpse
or artifact roots when the entrance exists.

Semantic follow-up includes object value-array assignments (notably template 3
value[3]) and data-backed transferable status. Lexical completeness for this file
does not resolve these questions or supply durable authority for transient NPCs.

Current inventory: 197 routes, 2,745 raw matches, 2,687 unique coordinates,
624 writer-mapped coordinates, 42 reviewed declarations, 2,021 unclassified.
Census and runtime qualification remain incomplete. Inventory/documentation only.

## Enhancement transformations and probes

All current lexical item/enhance.c coordinates are mapped. Replacement enhancement
combines cash, old equipment, material and a newly selected output; modifier and
superior enhancement mutate the same item UID instead. Submission of a debit does
not establish atomic payment with either transformation. Chaos-pouch generated
materials must remain distinct from physical inputs.

Base-stat, name, plan and index-building probes allocate and dispose templates
without player ownership. Death essence is placed on the dying NPC, not directly
on the reported killer. Seasonal-mobile items and skipped-reset material fallback
need original event provenance and explicit transient custody policy.

Semantic gap confirmed: vnum_from_inv consumes inventory indirectly and is absent
from the current item-lifecycle pattern. Its definition is in classes/drannak.c
and callers include superior enhancement, drannak and ethermancer paths; the header
declaration must also be handled when discovery is extended. Do not freeze census
coverage before this helper and object payload mutations are reviewed.

Current inventory: 205 routes, 2,745 raw matches, 2,687 unique coordinates,
656 writer-mapped coordinates, 42 reviewed declarations, 1,989 unclassified.
Census and runtime qualification remain incomplete. Inventory/documentation only.

## Indirect inventory consumption discovery

The lifecycle scanner now includes vnum_from_inv. A regression test proves that
calls/declarations are discovered while comments, strings and read-only
vnum_in_inv are ignored. All 38 contract tests pass. The full scan adds 11 sites:
the consuming definition, nine callers and one declaration. Every new site is
classified; the declaration is excluded with its reviewed source hash.

The helper checks a template/count then extracts concrete carried instances.
Exact UIDs and child disposition must belong to the enclosing operation.
Reviewed callers cover shard-to-orb exchange, conjuration orb cost, faerie-sight
dust, superior enhancement and salvage tools. The three descent ingredient calls
are after an unconditional disabled-command return and are explicitly dormant.
Four previously discovered helper-body/exchange sites were also mapped.

Current inventory: 211 routes, 2,756 raw matches, 2,698 unique coordinates,
670 writer-mapped coordinates, 43 reviewed declarations, 1,985 unclassified.
This closes the named helper-discovery gap, not overall semantic completeness.
Runtime behavior and qualification remain unchanged.

## Salvage inputs, outcomes and partial delivery

All current lexical item/salvage.c coordinates are mapped. Material downgrade
submits two lower-quality outputs then retires the input without checking both
grant outcomes. Ordinary salvage can submit luck/guaranteed essence, one or two
materials with rolled cost payloads, and a recipe scroll before retiring the
source. Scientific-tool consumption remains a linked input.

Separate routes distinguish intended skill/quality breakage from technical grant
refusal. The grant helper disposes only a provisional rejected output. Retain
every rolled child, input UID and tool under the original salvage operation;
independent successful children do not prove complete delivery. Rolled item cost
is payload rather than issued wallet currency. No gameplay behavior changed.

Current inventory: 215 routes, 2,756 raw matches, 2,698 unique coordinates,
685 writer-mapped coordinates, 43 reviewed declarations, 1,970 unclassified.
Census and runtime qualification remain incomplete.

## Guild-hall fixtures and provisional golem cash

All current lexical guild/guildhall_rooms.c coordinates are mapped. The file
materializes fixtures from hall configuration rather than executing purchases.
Door/board/heartstone/window/fountain/counter/portal/tome objects require explicit
data-backed representation policy; do not assume every load is economic issuance
or every fixture is nontransferable merely from its name.

Configured golems have template coins zeroed before room placement. Classify this
at the admission boundary rather than as destruction of an established wallet.
Guild and racewar metadata do not provide durable NPC holding identity by
themselves.

Older fixture teardown unlinks objects and clears pointers without extraction;
workshop teardown separately extracts its matching prop. Unlinking is not proof of
durable retirement or even object disposal. Preserve hall lifetime, possible
children and actual transferable status when implementing the owning slice.

Current inventory: 218 routes, 2,756 raw matches, 2,698 unique coordinates,
717 writer-mapped coordinates, 43 reviewed declarations, 1,938 unclassified.
Census and runtime qualification remain incomplete. Inventory/documentation only.


## Currency transaction adapters and publication

All current lexical currency_transaction.c coordinates are mapped. Wallet
publication installs a committed four-denomination snapshot with a revision guard;
it is not another issuance. Bank payment denomination change belongs to the same
payment. Endpoint and identify builders construct plans without committing them.
Generic, wallet-value, reward and coin submission adapters need caller provenance
and preserve the distinction between accepted submission and committed result.

Prepared submission returns true for an already-pending operation ID before
comparing the new payload at that shortcut. Record same-ID changed-intent
qualification as an integration obligation; this inventory does not prove it.
Native save acknowledgement and retained publication obligations also remain
separate from snapshot installation. No gameplay behavior changed.

Draft validator passed all 13 fixtures. Current inventory: 223 routes, 2,756 raw
matches, 2,698 unique coordinates, 737 mapped, 43 reviewed declarations and 1,918
unclassified. This is contract validity only; census and runtime qualification
remain incomplete.


## Currency and coin command construction

The currency builder and coin builder/decode/revision-adjustment coordinates are
classified as plan construction, not additional economic events. Coin endpoints
derive deterministic child IDs from the parent and endpoint index. Validation
checks distinct endpoint identities, conserved copper value, wallet deltas, pile
UID/payload/topology and bounded encoding. Reconstruction during decoding does
not replay a transfer. Shared bank or custody revision adjustment carries the
source result into the destination command without publishing live balances.

These are observed builder semantics, not proof of atomic backend execution,
retained publication or save acknowledgement. The existing currency transaction
contract test is source-level evidence; it cannot establish those runtime
properties. No runtime code or test behavior changed in this batch.

Draft validator passed 13 fixtures: 226 routes, 2,756 raw matches, 2,698 unique
coordinates, 741 mapped, 43 reviewed declarations and 1,914 unclassified.
Census completion, contract freeze and runtime qualification remain outstanding.


## Mining harvests and nexus object lifetimes

All current lexical mining.c and nexus_stones.c coordinates are classified.
Mining reserves a charge and can extract the exhausted mine before its scheduled
harvest completes. Interrupted attempts and failed item loads therefore differ
from admitted ore/gems. Rolled item cost is payload, not issued currency. Pick
fumbles move the existing equipment to inventory. Parchment and DamageOneItem
remain linked helper outcomes rather than evidence of an atomic harvest bundle.
Mine placement failure disposes a provisional object; trusted purge/reset and
resource depletion need explicit fixture/resource lifetime policy.

Nexus guardians and sages zero template money before placement. Guardian mace
allocation is optional and requires its own item/lifetime policy. Configured stone
identity does not establish durable economic object identity or transferability.
Reset/reload/expiration remove stone and NPC representations and rebuild them.
Expiration has different ordering: flat-file authority update precedes teardown;
SQL updates alignment after teardown and can fail before replacement. Preserve
these observed outcomes when integrating; this census makes no gameplay changes.

Draft validator passed 13 fixtures: 237 routes, 2,756 raw matches, 2,698 unique
coordinates, 769 mapped, 43 reviewed declarations and 1,886 unclassified.
Census completion and runtime qualification remain outstanding.


## Blackjack and boon delivery boundaries

All current lexical cardgames.c and boon.c candidates are mapped. Blackjack
accepts SUB_MONEY submission before proceeding with table state; for persisted
players this is asynchronous acceptance, not a committed wager. Win/push payouts
write denominations directly. Fold/bust/loss reset without a payout. The player
hand owner is attached on deal, after the wager, so durable wager/round/participant
binding and interruption policy cannot be inferred from live table fields.

Five boon candidates are inside #if 0 and are explicitly disabled legacy paths.
Active completed-boon publication separately submits cash rewards or allocates
items. Cash rejection may attempt auction pickup fallback: the SQL-build helper
inserts/increments a claim without an operation-ID argument, while the
__NO_MYSQL__ helper refuses. These branches require original reward identity and
proof of which leg committed, not two issuances. Item allocation failure yields
no item. The flat pending-reward caller acknowledges after publication returns,
which does not prove an asynchronous cash child committed or an item was saved.
These are recorded integration obligations; no runtime behavior changed.

Draft validator passed 13 fixtures: 241 routes, 2,756 raw matches, 2,698 unique
coordinates, 782 mapped, 43 reviewed declarations and 1,873 unclassified.
Census completion and runtime qualification remain outstanding.


## Collector custody and trade submission

Collector live detachment promotes contents to the original parent/room before
extracting the selected representation, after committed custody publication.
The selected UID remains in collector authority; extraction is not destruction
of that holding. A failed relocation can leave partial live publication, which
requires recovery rather than inventing a rolled-back durable transaction.

Repository classifications separate revision-guarded custody state, its legacy
ledger evidence and purchase debit. Purchase canonicalizes the wallet and advances
bank revision without changing bank denominations. The scanner match on local
bank[6] initialization is explicitly a SQL parsing buffer, not an economic write;
it remains mapped because prototype-only exclusions cannot represent this case.
Known player_data and legacy baseline writes are included in the route semantics
even though this census pattern does not match those tables.

Collector and shop submission wrappers only accept queued work. Collector callback
can report durable success with a publication error; shop callback combines commit
and publication success. Neither publication error proves rollback. These adapter
classifications do not qualify native save acknowledgement or restart recovery.

Draft validator passed 13 fixtures: 248 routes, 2,756 raw matches, 2,698 unique
coordinates, 793 mapped, 43 reviewed declarations and 1,862 unclassified. No
runtime code changed; census completion and runtime qualification remain open.


## Auction SQL claims and command publication

Auction repository coordinates distinguish wallet changes, custody transitions,
listing snapshots, staged money/item claims and claim consumption. A rebid by the
existing winner debits only the increase; a replacement bidder funds the full
bid and stages the prior bidder refund. Seller proceeds subtract the closing fee.
Claim-to-wallet credit and claim zeroing belong to one transfer, not issuance plus
a separate sink. Custody ledger rows are evidence of the same item movement.

The existing auction.sql_apply support text claimed frozen recipient materialization
that this checkout's auction_repository_execute does not substantiate. It now
marks that behavior unverified. The remove branch with an existing winner stages
the seller item claim without staging a bidder refund; preserve this explicit
behavior question for #485 rather than silently assume balanced cancellation.

The auction command candidate is a length-checked object-blob memcpy in decoding,
not a coin mutation. Builder reconstruction checks fences without economic side
effects. Submission accepts queued work; publication can erase pending and report
failure after commit. Offline actor completion remains pending in memory. Native
save/restart qualification is not established by this mapping.

Draft validator passed 13 fixtures after correcting a mistaken builder-coordinate
assignment to the actual decoder match: 257 routes, 2,756 raw matches, 2,698 unique
coordinates, 805 mapped, 43 reviewed declarations and 1,850 unclassified. No
runtime behavior changed; auction_houses.c and other census gaps remain open.


## Auction-house callers and live representations

All 51 current lexical auction_houses.c candidates are mapped, completing current
lexical mapping under src/economy (not semantic or runtime qualification). Active
SQL and __NO_MYSQL__ paths separately cover listing, bid, removal, finalization
and money/item pickup. Listing and claim blob copies are payload construction,
not coin writes. Listing callbacks remove existing live representations after
custody commit; claim callbacks deserialize and assign committed UIDs. Dirty
marking is not durable save acknowledgement, and skipped missing/failed children
can leave publication incomplete.

Listings serialize the first root once while quantity selection compares template
number. Claim callbacks reconstruct every root from the shared blob. This source
does not establish exact payload preservation for distinct same-template items.
Temporary objects for info/resort/backfill are catalog representations, not fresh
items or authoritative item destruction.

Repository symbol searches found only definitions for the retained offer/bid/
pickup/finalize legacy handlers. They are mapped as retained compiled paths, not
active coverage. Their separate money helpers, SQL edits and compensation cannot
be assumed atomic. Legacy pickup restoration messages are not proof of successful
restoration. SQL aggregate claim helper remains callable by other rewards; the
__NO_MYSQL__ helper refuses, and neither signature accepts an original operation ID.

Draft validator passed 13 fixtures: 263 routes, 2,756 raw matches, 2,698 unique
coordinates, 856 mapped, 43 reviewed declarations and 1,799 unclassified. No
runtime changes. Complete census, semantic blind spots and qualification remain.


## Object-command coin transfer and pile publication

Twenty-one actobj.c candidates now distinguish compound PC-to-PC give and supported
container put from debit-then-publication floor drop or fallback give. NPC/live
recipient identity is not durable holding authority. Compensation uses a separate
rebasable wallet_reward submission without an original debit-ID argument; a
missing sender or rejected refund can leave only an alert. Restoration messages
therefore do not establish delivery.

Pile endpoint construction renders temporary descriptions without live value
mutation. Rejected new piles are provisional cleanup. Committed pile publication
checks revisions and may consume, restore or discard a detached offline copy;
only the first changes pile lifetime as part of the transfer. Offline cleanup is
not destruction of the authoritative holding. Newer revisions suppress stale
publication. Native save/restart qualification is still separate.

Transient put merges via add_coins or creates and places a pile after debit; its
existing-pile add_coins call is a semantic mutation not matched by this scanner.
PC-corpse wrapper calls writeCorpse, which alone is not acknowledgement evidence.
The inventory records these helper effects without asserting the scanner is
semantically exhaustive. No runtime behavior was changed.

Draft validator passed 13 fixtures: 268 routes, 2,756 raw matches, 2,698 unique
coordinates, 877 mapped, 43 reviewed declarations and 1,778 unclassified.
Census and runtime qualification remain incomplete.


## Existing-pile admission, pickup and trusted junk

An absent runtime pile is first admitted through a same-owner item transaction;
that callback rechecks location, containing boundary and owner before pickup.
Admission does not credit the wallet, and failed continuation leaves the pile
available. Missing runtime authority is not proof the economic value is newly
issued. Tracked pickup computes whole coins within wallet limits and pairs pile
change with wallet credit; bulk continuation and corpse persistence calls remain
separate from acknowledged native saving.

PC pickup returns before the older NPC money branch. That branch clears value[]
fields, credits live money and consumes the pile. Its notall display branch has a
local initialized to zero and no subsequent assignment. NPC holding identity,
integer arithmetic and refusal outcomes still require explicit treatment. Tracked
NPC item pickup uses mobile_claim while retaining source authority. Condition-zero
items instead call MakeScrap before ordinary transfer submission.

Junk is restricted to alive trusted characters. Visible nontransient bulk items
invoke two ten-copper reward calls, whereas ordinary single-item junk invokes one;
transient destruction gives no reward. Record these existing distinctions rather
than silently changing economics during accounting integration. Item retirement,
container descendants and asynchronous reward delivery need linked identities.

Draft validator passed 13 fixtures: 273 routes, 2,756 raw matches, 2,698 unique
coordinates, 895 mapped, 43 reviewed declarations and 1,760 unclassified. Eighteen
additional sites classified; no runtime changes. Census/qualification remain open.

## Gift, container and drop publication boundaries

Classified eleven command movement sites. Generic put records parent/root topology
including same-owner nesting; an already-owned locker root has a distinct live-only
exception. Submission acceptance and the put callback's ignored result do not prove
native publication or save acknowledgement. Container-get and drop helpers relocate
existing items; Redis hints, dirty marks and corpse writes are not save receipts.

Player gift completion checks source carriage and recipient PID but not shared room,
then invokes hooks after the live handoff. Pet gift completion additionally checks
pet identity, ownership/charm and original room. Its stale-topology extraction
removes a live representation after committed custody, not the economic item.
Recovery advice does not itself qualify restart recovery. These distinctions remain
implementation/test obligations for #482; this inventory changes no game behavior.

Draft validator passed 13 fixtures: 277 routes, 2,756 raw matches, 2,698 unique
coordinates, 906 mapped, 43 reviewed declarations and 1,749 unclassified.
Census and runtime qualification remain incomplete.

## Bulk pickup and drop operation boundaries

Eight more command sites are classified. Bulk pickup admits missing stock roots to
source custody individually before transferring the durable forest. Earlier source
admissions can survive later pickup refusal, so they are neither delivered items
nor proof of issuance. Deferred coin pickup and MakeScrap run separately after the
forest transfer; a whole get command is not therefore one atomic economic event.

Bulk drop publishes durable roots only after rechecking the actor's original room
and all root carriage. Stale committed publication alerts and clears live batch
state without undoing custody. Synchronous candidates are selected from current
inventory afterwards (or directly if no durable roots exist), not from the durable
snapshot. Coin insertion can merge/free a representation without destroying value.
Room-get publication, floor hints and corpse writes likewise do not prove native
save acknowledgement. These are inventory findings, not changed gameplay.

Draft validator passed 13 fixtures: 282 routes, 2,756 raw matches, 2,698 unique
coordinates, 914 mapped, 43 reviewed declarations and 1,741 unclassified.
Census and runtime qualification remain incomplete.

## Give fallback, weight relocation and consumable retirement

Fourteen command sites are now classified. Durable pet routing is CMD_GIVE-gated
and checks matching runtime custody; distinct-PC durable gifts use their own
submission branch. Remaining give paths move synchronously and invoke hooks.
Existing insertion/crumble pointer risk remains explicitly deferred in source.
Weight adjustment relocates the same item to recompute carried/equipment/container
effects, not to issue another item; invalid wearer recovery can orphan its live
location and helper side effects still require treatment.

Drinking can retire an owned empty transient container after liquid/effect changes.
Eating applies effects before retirement; level mushroom attempts a silent save
before extraction and consumes even if that save reports failure. Privileged
non-food eating and optional artifact registry clearing are existing behavior.
These paths need benefit/payload/retirement boundaries without claiming atomicity
or changing gameplay in the #475 inventory.

Draft validator passed 13 fixtures: 286 routes, 2,756 raw matches, 2,698 unique
coordinates, 928 mapped, 43 reviewed declarations and 1,727 unclassified.
Census and runtime qualification remain incomplete.

## Equipment placement and broken-item wear

Sixteen equipment sites are classified. Common wear plus wrist/belt branches move
existing carried items into slots; removal returns items to the same actor and can
also return three belt attachments. These are placement/effect changes, not fresh
issuance. Initial carry checks do not make the multi-item removal one atomic saved
group. Wear refuses detached pending grants, while nonpositive-condition carried
items invoke MakeScrap instead of equipping. Existing insertion/crumble pointer
risk remains explicitly deferred in source. No runtime behavior changed.

Draft validator passed 13 fixtures: 289 routes, 2,756 raw matches, 2,698 unique
coordinates, 944 mapped, 43 reviewed declarations and 1,711 unclassified.
Census and runtime qualification remain incomplete.

## Empty-container publication and poison consumption

Twelve sites classified: empty submits durable roots separately from live movement,
revalidates selected objects and destination graph, and attempts local rollback on
insertion failure. That rollback cannot undo committed custody. Failed publication
retains live operation state and returns false; storage writes/dirty marks on
success are not native save receipts or restart-safe retention evidence.

Poison application changes weapon payload and liquid quantity/weight before retiring
an empty transient container. Belt containers are selectable although the final
detach calls obj_from_char; helper behavior remains a qualification obligation.
Food catalog diagnostic allocates prototypes without placement or cleanup in its
body; no caller was found in searched src C files. It is not proven active issuance.
No runtime behavior changed.

Draft validator passed 13 fixtures: 293 routes, 2,756 raw matches, 2,698 unique
coordinates, 956 mapped, 43 reviewed declarations and 1,699 unclassified.
Census and runtime qualification remain incomplete.

## Bulk put and container publication

Nine sites classified. Bulk put submits selected durable roots, revalidates capacity
and live source, then publishes sequentially. A later publication refusal can follow
earlier live insertions and clear batch state without undoing custody. The remaining
pass skips exact batch UIDs, can defer newly durable candidates independently, and
counts actual insertion rather than put's accepted/handled return value.

Quiver and ordinary-container put paths serve both acknowledged and synchronous
movement. Carried versus room fallback has distinct count/space updates; an old
comment does not prove the room branch unreachable. Nesting, payload updates and
native save acknowledgement must remain distinct. No runtime behavior changed.

Draft validator passed 13 fixtures: 295 routes, 2,756 raw matches, 2,698 unique
coordinates, 965 mapped, 43 reviewed declarations and 1,690 unclassified.
Census and runtime qualification remain incomplete.

## Remaining actobj drop and trusted recovery sites

Seven final lexical candidates in actobj.c classified. Trusted get nowhere forces
invalid/detached live objects into staff inventory outside ordinary pickup submission;
missing location is not proof of abandoned authority or fresh issuance. NPC all-drop
fallbacks and single synchronous drops move existing items, with branch-specific
restrictions preserved. Room pile merging and subsequent reads remain qualification
concerns, not proof that disappearing representations destroy economic value.

All current scanner candidates in actobj.c now map to reviewed routes. This is not
semantic exhaustiveness: indirect helpers, direct payload/value[] changes, authority
proof and executable integration evidence remain required. No runtime change.
Draft validator passed 13 fixtures: 298 routes, 2,756 raw matches, 2,698 unique
coordinates, 972 mapped, 43 reviewed declarations and 1,683 unclassified.

## Sector forage creation

Reviewed the full forage_sect table and publication body, classifying 90 scanner
sites as one issuance route with sector-specific outcomes. Allocation is provisional;
optional poison changes the item payload before inventory insertion. Attempt identity,
chosen outcome and poison payload must survive retries without duplicate issuance.
The helper does not demonstrate allocation-null handling or durable saving, and
post-insertion messages are not receipts. Existing distributions remain unchanged.

Draft validator passed 13 fixtures: 299 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,062 mapped, 43 reviewed declarations and 1,593 unclassified.
Census and runtime qualification remain incomplete; no game behavior changed.

## Forage display/tree branches and legacy quit

Ten sites classified. Forage while memorizing/scribing creates and removes a display
prototype without admission; successful giant fallback instead issues a tree to the
room. Neither should be conflated with normal inventory forage. Legacy quit's
lower-level branch drops nontransients and destroys transients before terminal save;
a save failure does not reverse those mutations. Source comments/restrictions limit
ordinary reachability but do not justify declaring compiled branches absent.

Draft validator passed 13 fixtures: 302 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,072 mapped, 43 reviewed declarations and 1,583 unclassified.
No runtime changes; census and qualification remain incomplete.

## ATM and coin theft endpoints

Five sites classified. Deposit/withdraw submit paired opposite denomination vectors
for wallet and shared bank; callback balance display is not native save evidence.
Coin theft instead decrements victim live cash before a separate ADD_MONEY call for
the thief. Its credit is a transfer endpoint, not fresh reward issuance. Selected
random denominations, both owners and original attempt must be retained for common
accounting; the shown path does not establish paired commit or compensation.

Draft validator passed 13 fixtures: 303 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,077 mapped, 43 reviewed declarations and 1,578 unclassified.
No gameplay change; census and runtime qualification remain incomplete.

## Group split ordering

Both split helper sites classified. Eligible same-room visible PC/morph recipients
receive independent ADD_MONEY calls before sender SUB_MONEY. Given increments
without checking recipient credit outcomes; sender debit refusal only logs. Preserve
membership, denomination and integer remainder in the future participant bundle.
Seven-character input does not prove copper conversion fits int. No atomic transfer
or compensation is demonstrated by this command; gameplay remains unchanged.

Draft validator passed 13 fixtures: 303 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,079 mapped, 43 reviewed declarations and 1,576 unclassified.
Census and runtime qualification remain incomplete.

## Potion and legacy scroll consumption

Eight sites classified. Quaff consumes on spill, epic gain, lethal self-damage or
normal spell completion, including no-magic outcomes, without a device-action gate.
Recite reaches its old unequip/spell/extraction path only when begin_device_action
returns legacy. These paths distinguish retirement with/without benefit and cannot
serve as proof of durable device replay or compound save acknowledgement.

Draft validator passed 13 fixtures: 305 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,087 mapped, 43 reviewed declarations and 1,568 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Theft fallback, donation and environmental retirement

Ten sites classified: fallback item theft bypasses handled trusted-PC transfer;
donation transfers to well or destroys excess duplicates after acceptance; hide
removes track objects and vampire lick removes blood before healing. Environmental
lifetime must be resolved without inventing durable holdings. Committed ascension
returns equipment to the same actor; offering commit is not saved item placement.

Draft validator passed 13 fixtures: 310 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,097 mapped, 43 reviewed declarations and 1,558 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## actoth lexical inventory completed

The final old-descent equipment site is classified as compiled legacy same-owner
placement. Its entry rejects non-NPC actors, and src search finds only the definition;
it is not proven active player behavior. All current actoth.c scanner coordinates
are now mapped. This does not establish semantic exhaustiveness or runtime coverage.

Draft validator passed 13 fixtures: 311 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,098 mapped, 43 reviewed declarations and 1,557 unclassified.
No gameplay change; census and runtime qualification remain incomplete.

## Mobile salesman stock sale

Two sales_spec sites classified. Wielded or first carried stock is priced at twice
cost plus three; both seller-selection paths call transact and change NPC behavior
on helper success. Exact stock UID, seller lifetime and quote need settlement
identity. Helper success does not itself prove atomic common-accounting delivery.

Draft validator passed 13 fixtures: 312 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,100 mapped, 43 reviewed declarations and 1,555 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Replacement skeleton starting balances

Eight cash assignments classified. Animated skeleton death independently allocates
two replacement NPCs and clears prototype cash before room placement. This is
initialization, not established destruction of admitted currency or a transfer from
the dying skeleton. Original death disposition and replacement lifetime/admission
remain separate obligations; allocation failures can produce partial spawning.

Draft validator passed 13 fixtures: 313 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,108 mapped, 43 reviewed declarations and 1,547 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Paid cleric and carnival services

Four coordinates classified. Cleric payment is followed by clearing the NPC's whole
cash balance and attempting a spell, not a demonstrated atomic fee/service bundle.
Carnival purchase charges 500 copper then relocates the customer; ticket narration
creates no item. Preserve pricing and trusted passage behavior. Source commands do
not prove common settlement, effect delivery or native save acknowledgement.

Draft validator passed 13 fixtures: 315 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,112 mapped, 43 reviewed declarations and 1,543 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## World quest fee and refund linkage

Four sites classified. Fees commit before callback quest/map/abandon effects; context
has action, fee and giver but no quest-instance version. Stale-state checks do not
prove original quest identity. Refund helper accepts only player/fee and announces
return before unchecked ADD_MONEY; original debit linkage and replay remain open.
New-quest quote permits zero while callback rejects nonpositive fee, an unresolved
boundary recorded without gameplay changes.

Draft validator passed 13 fixtures: 317 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,116 mapped, 43 reviewed declarations and 1,539 unclassified.
Census and runtime qualification remain incomplete.

## Smelter pooled deposits and conversion

Eleven sites classified. Give hook directly transfers coins or ore into NPC-held
pooled stock, then separate fee debits precede conversion. finish_smelt detaches two
matching inputs, allocates next-size ore and gives output to current player; it does
not extract detached inputs in its body. Depositor identity, actual retirement and
fee/output failure handling remain unqualified, rather than assumed durable escrow.

Draft validator passed 13 fixtures: 319 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,127 mapped, 43 reviewed declarations and 1,528 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Task clearing and monk remort fees

Two fee sites classified. Task clearing removes the affect before unchecked debit;
monk remort invokes unchecked debit then resets class/spells. Live affordability is
not committed payment, and no compensation callback is demonstrated. Preserve exact
task or original character state and existing eligibility; epic threshold is not
proof of an epic charge.

Draft validator passed 13 fixtures: 321 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,129 mapped, 43 reviewed declarations and 1,526 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Elixir effects and artifact information fees

Seven sites classified. Witch purchase applies an affect without creating a potion,
then clears NPC platinum. Artifact locator sells information without moving artifacts
and clears all NPC cash after payment. Those sinks can include prior holdings, not
only current fees; helper return and narration do not qualify atomic settlement.

Draft validator passed 13 fixtures: 323 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,136 mapped, 43 reviewed declarations and 1,519 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Assistant transformation item handoffs

Eight sites classified across shabo_butler and shabo_petre. Replacement NPCs receive
existing inventory and equipment from assistant NPCs; this is custody movement, not
fresh loot. Prototype starting state, source lifetime and partial graph publication
remain distinct from a qualified atomic transfer.

Draft validator passed 13 fixtures: 325 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,144 mapped, 43 reviewed declarations and 1,511 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Transformation and dracolich death release

Seven sites classified. Shabo Palle moves possessions to replacement then extracts
original NPC; two allocations overwrite the local replacement pointer, so allocation
lifetime remains unqualified. Dracolich death releases carried/worn items directly
to room after saved-corpse check. These move existing items, not newly issued loot;
partial publication and source lifetime require explicit accounting boundaries.

Draft validator passed 13 fixtures: 327 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,151 mapped, 43 reviewed declarations and 1,504 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Encounter objects and assistant gift creation

Ten sites classified. Eligoth creates a room rift, tentacler death selects one of
five objects for fixed room 89227, and assistant visitor detection creates a rose
before invoking give. Death/visit source identity, selected outcome and intermediate
NPC custody must survive retry; narration and command invocation are not receipts.

Draft validator passed 13 fixtures: 330 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,161 mapped, 43 reviewed declarations and 1,494 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Alignment offering and devour fallback

Five sites classified. Harpy choice consumes feather before character alignment
transformation. Devour first permits durable corpse release handling; fallback moves
corpse children to room before retiring root. Contents are transferred rather than
destroyed with root. Neither narration nor sequential live mutations prove receipt.

Draft validator passed 13 fixtures: 332 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,166 mapped, 43 reviewed declarations and 1,489 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Money changer fee and rounding sink

Four direct wallet sites classified. Exchange debits/credits customer denominations
without NPC credit or transaction submission. Lower conversion retains 90 percent;
higher conversion uses truncated rate/10 and whole-output rounding. Preserve actual
integer economics, not advertised nominal percentages; explicit fee sink and bounds
remain common-accounting obligations.

Draft validator passed 13 fixtures: 333 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,170 mapped, 43 reviewed declarations and 1,485 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Ship ticket retirement before entry

Four ticket-taker sites classified. Player-held ticket, or matching NPC-held ticket,
is consumed before returning to ordinary entry processing. Retirement does not prove
travel succeeded; NPC stock match lacks depositor identity in this handler. Preserve
exact ticket/traveler linkage without changing existing admission policy.

Draft validator passed 13 fixtures: 334 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,174 mapped, 43 reviewed declarations and 1,481 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Xexos form-change custody

Eight sites classified across combat transformation and idle reversion. Inventory
and equipment move to the opposite allocated NPC form before source extraction;
these are existing items, not repeated issuance. Cash is not explicitly transferred
in these blocks, so prototype balances and source disposition require separate review.

Draft validator passed 13 fixtures: 335 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,182 mapped, 43 reviewed declarations and 1,473 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Special death pile and heart publication

Twelve sites classified in stone_crumble and bahamut. Existing possessions move
without fresh issuance; stone-pile container and dragon heart are new objects.
Stone cash materialization has no local wallet debit: caller lifetime disposition
must establish conservation. Heart publication precedes decay payload initialization.
Neither handler proves durable publication or restart-safe exactly-once behavior.

Draft validator passed 13 fixtures: 337 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,194 mapped, 43 reviewed declarations and 1,461 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## NPC absorption, exchanges and rewards

Seventeen sites classified across phalanx, blob, rainbow-key exchange, trusted
fooquest transformation and newbie-paladin reward. Blob digestion preserves direct
children before retiring their container; absorption is a distinct custody change.
Rainbow input retirement precedes output allocation; newbie eligibility removal
precedes sword creation. The fooquest transfer comment actually describes fresh
prototype allocation. These ordering distinctions remain integration obligations.

Draft validator passed 13 fixtures: 342 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,211 mapped, 43 reviewed declarations and 1,444 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Remaining mobile custody paths

Twenty-four sites classified: janitor pickup, spiritist disarm, ice-wolf and
assistant transformations, ice-malice death transfer, Malevolence transformation,
and butler source-form transfer. These preserve existing item identities across
room/equipment/NPC custody. Source death and prototype cash disposition remain
separate obligations; assistant allocation failures permit partial transformation.
All current scanner coordinates in specs.mobile.c are mapped. This is lexical
inventory completion only, not semantic completeness or runtime qualification.

Draft validator passed 13 fixtures: 348 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,235 mapped, 43 reviewed declarations and 1,420 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Alchemy pouch and altar issuance

Twenty-nine sites classified. Alchemy pouch advances its timer before sequential
creation of eight children; allocation failure leaves a partial set. Llyms altar
retires the held treasure before blessing or unchecked random wallet credit and
optional bonus creation. Persist chosen outcomes without changing distributions;
partial failure and pet lifetime behavior require explicit integration treatment.

Draft validator passed 13 fixtures: 352 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,264 mapped, 43 reviewed declarations and 1,391 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Charge exhaustion and object conversion

Ten sites classified in monolith absorption, elemental ring exhaustion, Menden
figurine consumption, banana-to-peel conversion and crystal spike exhaustion.
Charge/item consumption does not establish successful charm or spell execution.
Monolith charge aggregation and banana replacement involve payload/creation effects
beyond retirement. Figurine unequip receives local pos=-1; detachment and pet
lifetime require qualification rather than assuming success from the helper call.

Draft validator passed 13 fixtures: 357 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,274 mapped, 43 reviewed declarations and 1,381 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Ferry and pool relocation

Twelve sites classified in ferry movement/unloading and floating/teleporting pools.
Existing vessel, cargo and pool identities survive room changes; no issuance occurs.
Ferry route progress and random pool destinations need consistent retained state.
Sequential cargo unloading is not proven atomic with passenger movement.

Draft validator passed 13 fixtures: 362 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,286 mapped, 43 reviewed declarations and 1,369 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Chest reward, hostile issuance and imprisonment

Seven sites classified. Treasure chest grants up to four potions but only detaches
itself despite crumbling narration; retirement must not be inferred. Zarbon curse
objects are fresh issuance into victim inventory. Imprison retirement couples PID,
struggle payload and flag clearing, without proving offline player save completion.

Draft validator passed 13 fixtures: 365 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,293 mapped, 43 reviewed declarations and 1,362 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Ward and beacon retirement

Six sites classified: huntsman ward trigger retirement after arming-state cleanup
and alarm/damage/affect scheduling; inactive skill beacon random decay; thought
beacon dispel using explicit false extraction mode. Disarm and active beacon skill
changes are payload paths not established by the extraction census. Historical
not-in-game comments do not establish current deployment status.

Draft validator passed 13 fixtures: 368 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,299 mapped, 43 reviewed declarations and 1,356 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Badge breakage and forced equipment changes

Five sites classified. Guild badge random breakage retires a worn object.
Dragonslayer rejection accepts worn or carried source but invokes obj_from_char
without explicit unequip; helper behavior needs qualification. Disarm gloves move
the victim weapon to that same victim inventory despite drops narration, retaining
ownership and coupling holy-sword state cleanup.

Draft validator passed 13 fixtures: 371 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,304 mapped, 43 reviewed declarations and 1,351 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Reset relocation and disabled citadel movement

Five sites classified. Ruby monocle relocates the same object while zone age is
zero; the reset condition is not a unique invocation identifier. Flying citadel
movement is unreachable after an unconditional return and must remain disabled.
Die roll moves the existing equipped die into the room without wager or payout.

Draft validator passed 13 fixtures: 374 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,309 mapped, 43 reviewed declarations and 1,346 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Artifact sword rejection

Five sites classified. Good/evil sword poof subtracts hit points before extraction.
Holy weapon alignment rejection rehomes to the first matching player or retires
with no recipient, before its potentially lethal bolt. Preserve this ordering,
artifact identity and exemptions; detached-item death must not be reintroduced.

Draft validator passed 13 fixtures: 376 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,314 mapped, 43 reviewed declarations and 1,341 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Slot settlement and bound cap rejection

Six coordinates classified: slot jackpot coupon allocation precedes net wallet
submission, with result narration and machine payload already changed. Net-zero
results skip submission; no callback establishes save completion. Bound Xmas cap
rejection damages wearer before retirement, requiring lifetime qualification.

Draft validator passed 13 fixtures: 379 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,320 mapped, 43 reviewed declarations and 1,335 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Revenant equipment transitions

Two coordinates classified for transformation and reversion equipment removal.
Items remain with the same character; object remove procs run before unequip and
may have additional effects. Preserve helm exception and race/affect ordering;
post-movement liveness checking does not prove callback object validity.

Draft validator passed 13 fixtures: 381 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,322 mapped, 43 reviewed declarations and 1,333 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Artifact forced wield

Seven coordinates classified for good/evil sword forced primary equipment.
Displaced items stay in the same owner inventory. Native-owned path reacquires
actor runtime ID and sword UID after equip; that lifetime check is not durable
accounting acknowledgement. Preserve slot conditions and holder exemptions.

Draft validator passed 13 fixtures: 382 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,329 mapped, 43 reviewed declarations and 1,326 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Olympus linked portal relocation

Two coordinates classified. Existing portal moves before updating matching origin
portal destination and timer; missing counterpart does not prevent relocation.
Retain selected destination and both identities, and preserve sector predicate.
Linked payload publication is not established by the room-movement census alone.

Draft validator passed 13 fixtures: 383 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,331 mapped, 43 reviewed declarations and 1,324 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Dragonlord equipment transitions

Three coordinates classified for plate conflict removal, race reversion and legacy
transformation equipment removal. Ownership remains unchanged; remove callbacks,
race effects and event rescheduling need independent qualification. Legacy naming
does not establish registration or justify excluding a writer.

Draft validator passed 13 fixtures: 385 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,334 mapped, 43 reviewed declarations and 1,321 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Necroplasm and vapor custody

Ten coordinates classified for necroplasm conflict removal, room pickup and body
slot equipment, plus vapor forced equipment. Existing identities persist; native
and legacy slot selection differ and must not be silently normalized. All current
specs.object.c scanner coordinates are mapped; payload blind spots and runtime
qualification remain, so this is lexical inventory completion only.

Draft validator passed 13 fixtures: 387 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,344 mapped, 43 reviewed declarations and 1,311 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Spell component consumption helper

One coordinate classified. Matching top-level inventory components are immediately
consumed up to the requested maximum, including partial availability. Returned count
does not establish spell success; artifact exclusion is a comment, not a predicate.

Draft validator passed 13 fixtures: 388 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,345 mapped, 43 reviewed declarations and 1,310 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Shield and food creation spells

Four coordinates classified. Shield output carries timer180 into caster inventory;
food output carries nosell into the room. Both check allocation but neither helper
publication nor preceding narration proves durable save or replay-safe admission.

Draft validator passed 13 fixtures: 390 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,349 mapped, 43 reviewed declarations and 1,306 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Spring and divine font creation

Four coordinates classified. Both creations set caster-level payload and decay
before room publication; only spring applies the sector refusal. Indoor restriction
is commented out. Initial admission and later decay remain separate obligations.

Draft validator passed 13 fixtures: 392 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,353 mapped, 43 reviewed declarations and 1,302 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Insect spell component ordering

One coordinate classified. Selected mandrake is destroyed before duration250 room
affect installation. Existing room effect or missing component returns without
consumption; root identity and effect publication need retained cast linkage.

Draft validator passed 13 fixtures: 393 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,354 mapped, 43 reviewed declarations and 1,301 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Unmaking fallback corpse release

Three coordinates classified. Persistence deferral gets first opportunity; fallback
releases existing children, heals caster, then retires corpse. Preserve child UIDs,
room/type refusals and corpse-level conversion. Deferral handling must exclude
fallback, and sequential publication does not establish durable completion.

Draft validator passed 13 fixtures: 394 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,357 mapped, 43 reviewed declarations and 1,298 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Sticks-to-snakes arrow consumption

Two coordinates classified. Selected arrows detach into a temporary reversed list;
each attack precedes retirement. Victim death stops consumption and the function
has no cleanup for remaining detached arrows. This is a custody/retirement gap to
qualify during integration, not proof of full batch destruction or restoration.

Draft validator passed 13 fixtures: 395 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,359 mapped, 43 reviewed declarations and 1,296 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Channel orb retirement

One coordinate classified. Successful leader morph precedes orb retirement and
avatar affect/helper handling. Failed morph preserves orb with advanced timer;
retirement alone does not establish completion of the broader channel operation.

Draft validator passed 13 fixtures: 396 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,360 mapped, 43 reviewed declarations and 1,295 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Flame blade and minor creation publication

Three coordinates classified. Flame blade creates level-dependent temporary weapon
payload with caster PID or NPC sentinel before inventory publication. Minor creation
instead receives an existing object, publishes to room, then sets height; caller
allocation/admission remains distinct from this helper.

Draft validator passed 13 fixtures: 398 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,363 mapped, 43 reviewed declarations and 1,292 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Doom blade creation branches

Three coordinates classified. Class selects prototype426 or352; only the latter
branch explicitly sets timer1800. Preserve prototype-derived payload and branch
semantics, recording fresh admission separately from narration and placement.

Draft validator passed 13 fixtures: 399 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,366 mapped, 43 reviewed declarations and 1,289 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Moonstone replacement

Three coordinates classified. Prototype lookup retires prior stone without local
PID check, then affect binding changes before new allocation. Allocation failure
can leave prior retirement/binding changes; existing affect duration is unchanged.

Draft validator passed 13 fixtures: 400 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,369 mapped, 43 reviewed declarations and 1,286 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Tree spell track cleanup

One coordinate classified. Tree spell extracts all matching room track prototypes,
without owner filtering despite the comment. Track economic enrollment needs
qualification before assuming ledger retirement; preserve existing refusals.

Draft validator passed 13 fixtures: 401 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,370 mapped, 43 reviewed declarations and 1,285 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## General portal endpoint creation

Five coordinates classified. Optional second allocation failure cleans unpublished
first portal; successful endpoints receive links/timers before sequential room
publication. Retain paired UIDs and distinguish cleanup from admitted retirement.

Draft validator passed 13 fixtures: 402 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,375 mapped, 43 reviewed declarations and 1,280 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Soulbind global cleanup

One coordinate classified. Global soulbind cleanup uses name keywords, retaining
keep_uid and excluding kingdom-store-bound objects. Preserve exact retired identities
across custody locations; helper does not prove replacement or player save completion.

Draft validator passed 13 fixtures: 403 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,376 mapped, 43 reviewed declarations and 1,279 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Soulbind recreation publication

Four coordinates classified. Reload allocates a fresh customized object; NPC direct
publication/cleanup differs from PC creation submission/completion. Rejected PC
submission cleans unpublished output. Submission acceptance is not save acknowledgement.

Draft validator passed 13 fixtures: 404 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,380 mapped, 43 reviewed declarations and 1,275 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Soulbind player movement submission

One coordinate classified. Cross-player submission checks active source ownership
and recipient weight, retaining UID/PIDs/rooms/replacement context for publication.
Accepted submission remains pending; callback and save acknowledgement require
separate qualification. Same-player and nonplayer helpers are distinct paths.

Draft validator passed 13 fixtures: 405 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,381 mapped, 43 reviewed declarations and 1,274 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Conjured weapon submission

Three coordinates classified. NPC publication is direct; PC creation captures
UID/PID/kind/health cost and cleans unpublished output on rejection. Completion
identity/live checks precede effects but do not prove save or restart-safe replay.

Draft validator passed 13 fixtures: 406 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,384 mapped, 43 reviewed declarations and 1,271 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Conjured weapon allocation callers

Three coordinates classified for ensis, lancea and simulacrum prototypes. Each
configures level/timer/PID payload then delegates shared submission. Preserve
prototype defaults where branches do not override; allocation is not admission.

Draft validator passed 13 fixtures: 409 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,387 mapped, 43 reviewed declarations and 1,268 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Corpse portal relocation

Two coordinates classified. First prototype/name match is relocated to caster room;
no new portal is allocated. Nonroom first match refuses rather than continuing.
Preserve selected UID; name lookup does not prove newest corpse or durable owner.

Draft validator passed 13 fixtures: 410 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,389 mapped, 43 reviewed declarations and 1,266 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Soulbind live publication helpers

Four coordinates classified for direct nonplayer transfer and committed player
publication. Detached recovery and same-UID reacquisition differ from a new economic
transfer; live custody checks alone do not establish metadata/save completion.

Draft validator passed 13 fixtures: 412 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,393 mapped, 43 reviewed declarations and 1,262 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Teleport item charge exhaustion

Two coordinates classified. Positive charge decrement reaches retirement after
travel/follower processing; worn item detaches before extraction. Retain charge
and UID without treating item exhaustion as durable travel or save proof.

Draft validator passed 13 fixtures: 413 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,395 mapped, 43 reviewed declarations and 1,260 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Committed resurrection inventory publication

Nine coordinates classified. Old possessions release/retire before corpse children
transfer; money objects are removed as post-commit projection. Inventory transient
predicate checks corpse flags, unlike equipment item flags; this observed distinction
requires integration review. No correction or runtime qualification claimed.

Draft validator passed 13 fixtures: 414 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,404 mapped, 43 reviewed declarations and 1,251 unclassified.
No gameplay changes; census and runtime qualification remain incomplete.

## Committed resurrection corpse retirement

Reviewed the helper through its final corpse extraction. writeCharacter failure
logs and warns but does not prevent corpse retirement; integration must preserve
recovery authority until durable save acknowledgement. This is a documented
existing behavior, not a runtime correction.

Draft validation: 13 fixtures, 414 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,405 mapped, 43 reviewed declarations, 1,250 unclassified.
Release readiness remains false; no gameplay change or runtime qualification.

## Regular and lesser resurrection fallback transfers

Classified 28 coordinates in two fallback routes. Existing wallet denominations
become a room coin pile before inventory/equipment release; corpse coin payload
becomes wallet value and other children transfer to the resurrected character.
These are value transfers, not fresh issuance. The player branch offers deferred
persistence first. Fallback coin creation has no local allocation guard before
wallet debit; inventory transient checks use corpse flags rather than item flags.
Both routines retire the corpse despite player save failure. Deferred and NPC
behavior, exact value conservation and restart/save acknowledgement remain
unqualified; no gameplay fixes were made.

Draft validator passed 13 fixtures: 416 routes, 2,756 raw matches, 2,698 unique
coordinates, 1,433 mapped, 43 reviewed declarations, 1,222 unclassified.
Release readiness remains false.

## Remaining magic.c item-drop candidates

Classified the final14 unique lexical coordinates in magic.c across seven routes:
curse-removal drops, recall weight drops, disabled fire/frost breath and crimson
light drops, disabled wraithform drops, and unequip_char_dale. Breath/crimson
branches follow unconditional returns; wraithform also uses #if 0. These are
not current runtime enforcement evidence. Active drops preserve existing UIDs;
curse-removal direct object flag mutation and other payload-only writes remain
semantic coverage work beyond this lexical census.

Draft validation passed13 fixtures:423 routes,2756 raw matches,2698 unique
coordinates,1447 mapped,43 reviewed declarations,1208 unclassified.
All current magic.c lexical matches are mapped; broader census and runtime
qualification remain incomplete. No gameplay changes.

## Necromancy corpse conversion fallbacks

Nine coordinates classified for wall of bones and corpse compaction. Wall success
releases children and retires its source; implicit scale components may already
be consumed before wall creation fails. Compaction unloads children before bone
pile allocation, so allocation failure preserves an emptied corpse rather than
undoing child transfers. Successful output copies corpse level and replaces the
source. Deferred persistence and durable acknowledgement remain unqualified.

Draft validation: 13 fixtures, 425 routes, 2756 raw matches, 2698 unique
coordinates, 1456 mapped, 43 reviewed declarations, 1199 unclassified.
No runtime changes; release readiness remains false.

## Raised-item recipient and pet wallet reset

Six coordinates classified. place_raised_item routes non-money objects to a PC
caster, otherwise to follower; compare current routing to master baseline before
claiming gameplay compatibility. setup_pet PET_NOCASH zeroes all denominations
after linked affect application without a local restore exclusion or NPC guard.
Caller provenance must distinguish prototype initialization from existing-value
retirement. These are classifications, not runtime fixes or qualified behavior.

Draft validation passed 13 fixtures: 427 routes, 2756 raw matches, 2698 unique
coordinates, 1462 mapped, 43 reviewed declarations, 1193 unclassified.
Release readiness remains false.

## Cadaver creation and corpseform consumption

Seven coordinates classified. Exhume and summon host allocate prototype2, set
randomized capped corpse level and description, publish, then apply decay/embalm.
Allocation has no local null guard; host tool/terrain checks are commented out.
Corpseform applies transformation and follower changes before releasing children
and retiring its source corpse. Admission and replay must preserve generated
payloads and existing child UIDs without repeating transformation effects.

Draft validation passed13 fixtures:430 routes,2756 raw matches,2698 unique
coordinates,1469 mapped,43 reviewed declarations,1186 unclassified.
No gameplay changes or runtime qualification; release readiness remains false.

## Committed raise publication and saved corpse copies

Eleven coordinates classified across saved-copy cleanup/publication, recursive
money/transient removal and committed raise inventory publication. Saved clones
need original/copy lineage rather than duplicate economic admission. Committed
raise routes children by pet_uid, handles destroy-equipment, retires corpse and
publishes/configures follower before caster checkpoint. Failed writeCharacter
only logs; void completion is not durable acknowledgement. Nested transient
container disposal and retained recovery remain qualification requirements.

Draft validation passed13 fixtures:434 routes,2756 raw matches,2698 unique
coordinates,1480 mapped,43 reviewed declarations,1175 unclassified.
No runtime changes; release readiness remains false.

## Legacy raise contents: remaining necromancy matches

Twelve coordinates classified across raise_undead, call titan, dracolich, golem,
avatar and greater dracolich. Each reviewed transfer block saves a corpse copy
when contents exist, detaches children through place_raised_item and retires the
source after reading decay. Preserve child identities and backup lineage; each
spell's complete follower setup, gameplay baseline and save/recovery ordering
still require qualification. Current necromancy.c lexical matches are all mapped,
which does not establish semantic or runtime completeness.

Draft validation passed13 fixtures:440 routes,2756 raw matches,2698 unique
coordinates,1492 mapped,43 reviewed declarations,1163 unclassified.
No runtime changes; release readiness remains false.

## Ranged gathering, throwing and reloading

Fifteen coordinates classified. Gather transfers PID-tagged projectiles into a
quiver with capacity checked after insertion; temporary unequip/reequip is not
issuance. Non-returning throws transfer existing weapon to selected room. Reload
converts stack quantity to weapon payload, retiring exhausted ammo; partial-stack
payload mutation is documented despite lacking its own lexical extraction site.
Exact quantities, UID custody, combat ordering and durable save remain unqualified.

Draft validation passed13 fixtures:443 routes,2756 raw matches,2698 unique
coordinates,1507 mapped,43 reviewed declarations,1148 unclassified.
No gameplay changes; release readiness remains false.

## Fired projectile disposition

Six coordinates classified: non-cursed arrows detach and decrement quiver count,
then hit transfers to victim or invokes MakeScrap, while misses publish to computed
room. Shield blocks may also scrap shields. Cursed arrows retain custody but still
receive PID/enchantment payload mutations. Scrapping precedes later damage and
enchantment use, requiring object-lifetime and recovery qualification.
All current range.c lexical coordinates are mapped, not runtime qualified.

Draft validation passed13 fixtures:444 routes,2756 raw matches,2698 unique
coordinates,1513 mapped,43 reviewed declarations,1142 unclassified.
No gameplay changes; release readiness remains false.

## Room dump rewards, prayer delivery and key consumption

Seven coordinates classified. Dump removes preexisting non-artifact room objects
before command dispatch; dropped money sets reward accumulator to1, making reward
order-sensitive. Prayer moves first matching named item before charging cost+1,
without local affordability check. Feed-lock consumes key after do_unlock without
checking outcome; historical inactive comment is not registration evidence.
Compound retirement/reward, delivery/charge and unlock/consumption remain unqualified.

Draft validation passed13 fixtures:447 routes,2756 raw matches,2698 unique
coordinates,1520 mapped,43 reviewed declarations,1135 unclassified.
No gameplay changes; release readiness remains false.

## Patrol, hireling and stable transactions

Eight coordinates classified. Patrol/hireling allocation precedes fees and follower
publication. Stabling charges before writePet and ticket allocation with no local
refund on failure. Redemption charges and destroys the ticket while petrestore is
commented out; narration is not restoration evidence. Preserve exact fee and
pet/ticket lineage and qualify compound failures. All current specs.room.c lexical
matches mapped; runtime integration remains incomplete.

Draft validation passed13 fixtures:451 routes,2756 raw matches,2698 unique
coordinates,1528 mapped,43 reviewed declarations,1127 unclassified.
No gameplay changes; release readiness remains false.

## Quest inputs, rewards, previews and disappearance

Eighteen coordinates classified in quest.c. Inputs consume NPC-held items/coins;
item reward submission and coin issuance are separate sequential effects. Accepted
grant submission is followed by narration without completion acknowledgement here.
Description-only prototype loads are not economic admission. Disappearance cleanup
follows reward calls even when completion tracking only logs failure. Input/reward
identity, partial completion and NPC cleanup remain runtime qualification work.
All current quest.c lexical matches mapped, not semantic completeness.

Draft validation passed13 fixtures:456 routes,2756 raw matches,2698 unique
coordinates,1546 mapped,43 reviewed declarations,1109 unclassified.
No gameplay changes; release readiness remains false.

## Encounter-generated loot

Sixteen coordinates classified across randomized magical/equipment allocation and
NPC publication. Magical output can be a money object, requiring exact generated
coin payload preservation. Random descriptions/stats and conversion must not reroll
on replay. Equipment extra_flags uses logical negation in its existing expression;
recorded for integration review without changing behavior. World-generation owner
is #486. Allocation and CheckEqWorthUsing are distinct from economic admission.

Draft validation passed13 fixtures:459 routes,2756 raw matches,2698 unique
coordinates,1562 mapped,43 reviewed declarations,1093 unclassified.
No gameplay changes; release readiness remains false.

## NPC cadavers and equipment selection

Fourteen coordinates classified. NPC cadaver generation preserves random payload
and decay; allocation lacks a local null guard. MobThief detaches selected weapon
before slot checks; cursed two-handed primary can return without local restoration.
IsBetterObject relocates old equipment and recursively reuses it after wear helper.
UID custody and partial-failure/replay behavior remain unqualified.

Draft validation passed13 fixtures:462 routes,2756 raw matches,2698 unique
coordinates,1576 mapped,43 reviewed declarations,1079 unclassified.
No gameplay changes; release readiness remains false.

## NPC hunt weapon swaps

Sixteen coordinates classified in MobHuntCheck. Hunt backstab selection detaches
only within viable free/replacement slot branches, unlike MobThief early detach;
blocked cursed slots leave selected weapon carried. Existing UID custody changes,
subsequent movement/combat and save acknowledgement require separate qualification.
All current mobact.c lexical matches mapped; semantic/runtime work remains.

Draft validation passed13 fixtures:463 routes,2756 raw matches,2698 unique
coordinates,1592 mapped,43 reviewed declarations,1063 unclassified.
No gameplay changes; release readiness remains false.

## Underworld orb, weapons and dragon loot

Twelve coordinates classified. Sea orb slips to inventory, not ground. Gith weapon
usage can retire the object; separate return proc transfers existing ownership.
Tiamat releases existing equipment before unchecked heart allocation, publishes
heart before timer assignment, and needs one death/output identity for recovery.
No runtime correction or durable save qualification is claimed.

Draft validation passed13 fixtures:467 routes,2756 raw matches,2698 unique
coordinates,1604 mapped,43 reviewed declarations,1051 unclassified.
Release readiness remains false.

## Purple worm corpse custody

Six coordinates classified. Periodic release saves PC corpse; death drops all
carried items. Swallow selects room objects by value3 before corpse-type check,
moves them while traversing the mutated list, and saves PC corpse before setting
digestion timer. Exact selected graph and death/custody/save ordering remain
unqualified. All current underworld lexical matches mapped, not runtime coverage.

Draft validation passed13 fixtures:468 routes,2756 raw matches,2698 unique
coordinates,1610 mapped,43 reviewed declarations,1045 unclassified.
No gameplay changes; release readiness remains false.

## Undermountain corpse release and death loot

Twelve coordinates classified. Flying dagger and ochre jelly offer deferred corpse
release before fallback child transfer and retirement. Dagger death creates loot;
animated sword and helmed horror allocate before event checks, leaving non-death
allocations without local cleanup. Provisional allocation is not economic admission;
NPC death identity, failure and replay remain unqualified.

Draft validation passed13 fixtures:473 routes,2756 raw matches,2698 unique
coordinates,1622 mapped,43 reviewed declarations,1033 unclassified.
No gameplay changes; release readiness remains false.

## Remaining Undermountain disarm and decay

Five coordinates classified: flindbar disarm preserves existing weapon identity;
lightning sword selects first room corpse after death before deferred/fallback
release, not an explicit victim-bound corpse; drow equipment retires on actual
sunlit/non-underworld predicate. Compound death/effect/save behavior unqualified.
All current Undermountain lexical matches mapped, not semantic/runtime completion.

Draft validation passed13 fixtures:476 routes,2756 raw matches,2698 unique
coordinates,1627 mapped,43 reviewed declarations,1028 unclassified.
No gameplay changes; release readiness remains false.

## CTF flag creation and retirement

Nine coordinates classified: initial flags, missing-primary recreation, explicit
reload and deletion. Logical event identity and carrier TAG_CTF state extend beyond
ordinary inventory custody. Missing runtime object alone cannot prove new admission;
recreation needs predecessor lineage and deletion/allocation-failure qualification.
Reload uses i for room versus id for output, recorded for integration review.

Draft validation passed13 fixtures:480 routes,2756 raw matches,2698 unique
coordinates,1636 mapped,43 reviewed declarations,1019 unclassified.
No gameplay changes; release readiness remains false.

## CTF pickup, drop and capture custody

Six coordinates classified. Pickup uses detached flag plus TAG_CTF, reclaim returns
home, drop publishes before affect removal, and capture updates bonuses before
relocation. Boon/nonpositive destinations can retain detached event-held objects.
Logical custody and exactly-once bonus/save require qualification beyond inventory.
All current ctf.c lexical matches mapped, not semantic/runtime completion.

Draft validation passed13 fixtures:483 routes,2756 raw matches,2698 unique
coordinates,1642 mapped,43 reviewed declarations,1013 unclassified.
No gameplay changes; release readiness remains false.

## Crafting template removal and pick breakage

Six coordinates classified. yank_make_item removes only held/carried matching
input; local pointer clearing is not caller invalidation. make_key changes lock
and existing key payload before optional pick retirement. Input/output identity,
partial effects and save recovery remain qualification work.

Draft validation passed13 fixtures:485 routes,2756 raw matches,2698 unique
coordinates,1648 mapped,43 reviewed declarations,1007 unclassified.
No gameplay changes; release readiness remains false.

## Thrown potion disposition

Ten coordinates classified. Slips may retire or drop potion; normal throw unequips
before effect/refusal checks. Actor disappearance consumes early; successful NPC
remix retains same object without allocation. Too-complex spell can return after
unequip; no-magic/blocked sight still reaches final consumption/remix. Retained
custody and ordered effects/save/replay remain unqualified.

Draft validation passed13 fixtures:486 routes,2756 raw matches,2698 unique
coordinates,1658 mapped,43 reviewed declarations,997 unclassified.
No gameplay changes; release readiness remains false.

## Disarm, morph prototype cash and home fee

Seven coordinates classified. Disarm moves existing weapons into same-owner
inventory. Morph zeroes newly allocated NPC prototype cash, not player wallet.
Home changes save fields before fee debit; failed save restores fields, while
successful save has no second local save after charge. Compound durability and
replay remain unqualified. All current actnew.c lexical matches mapped.

Draft validation passed13 fixtures:489 routes,2756 raw matches,2698 unique
coordinates,1665 mapped,43 reviewed declarations,990 unclassified.
No gameplay changes; release readiness remains false.

## Movement ice, unlock keys and dragging

Ten coordinates classified. Ice allocation/decay/publication distinguishes rejected
provisional objects; current-room contents access precedes NOWHERE fallback check.
Key breakage follows local unlock effects; dragging follows command-interpreter
movement and locker/object checks. Exact UID, destination and compound save/replay
ordering remain qualification work.

Draft validation passed13 fixtures:492 routes,2756 raw matches,2698 unique
coordinates,1675 mapped,43 reviewed declarations,980 unclassified.
No gameplay changes; release readiness remains false.

## Reward containers and lockpick consumption

Seventeen coordinates classified. Reward opening selects/filters provisional
objects, clears flags, publishes output then consumes source; replay must preserve
selected output. Lockpick failure/success/refusal fallthrough paths can consume
pick with actual wear and expression precedence preserved. Compound source/output
and lock/tool durability remain unqualified. Current actmove.c lexical matches mapped.

Draft validation passed13 fixtures:494 routes,2756 raw matches,2698 unique
coordinates,1692 mapped,43 reviewed declarations,963 unclassified.
No gameplay changes; release readiness remains false.

## Death-generated loot admission

Six fight.c coordinates classified under #486. Dragon scales, eligible soul
shards and independently selected random loot are new outputs placed on the
dying NPC before corpse handling. Replay must preserve selected identities and
payloads without rerolling or counting later corpse movement as issuance.
Scale/shard allocation failure and compound death publication remain unqualified.

Draft validation passed 13 fixtures: 495 routes, 2756 raw matches, 2698 unique
coordinates, 1698 mapped, 43 reviewed declarations, 957 unclassified.
No gameplay changes; release readiness remains false.

## Combat disarms, blood replacement and corpse portals

Fourteen coordinates classified across five routes. Mangle, quickstep fumble and
critical inventory disarm transfer existing weapons to the same owner's inventory;
ground-drop helper branches remain distinct. Blood replacement retires room blood
before checked replacement allocation, with provisional cleanup on no destination.
Corpse portals allocate/configure then publish on eligible PC death. Transient
policy and compound publication/save ordering remain unqualified.

Draft validation passed 13 fixtures: 500 routes, 2756 raw matches, 2698 unique
coordinates, 1712 mapped, 43 reviewed declarations, 943 unclassified.
No gameplay changes; release readiness remains false.

## Corpse construction, transfer callbacks and disputed wallet snapshot

Fifteen coordinates classified. Corpse shell creation differs from existing NPC
inventory directly relinked through contains/loc pointers, a semantic blind spot
for helper-only scanning. NOCORPSE cleanup releases children or retires them when
no room exists. Player transfers distinguish single-root adoption and registered
batch submission, committed callback publication, and subsequent corpse save.
Disputed death's temporary wallet pile is a save snapshot, not independent coin
issuance; database acknowledgement and durable journal handoff remain distinct.
Recovery and compound save/publication qualification remain outstanding.

Draft validation passed 13 fixtures: 504 routes, 2756 raw matches, 2698 unique
coordinates, 1727 mapped, 43 reviewed declarations, 928 unclassified.
No gameplay changes; release readiness remains false.

## Flat-file corpse and room restoration

Sixteen coordinates classified across staged cleanup, corpse materialization,
room materialization and catalog publication. These reconstruct saved identities
and denomination quantities, not new issuance; failed staging releases runtime
copies rather than retiring saved assets. Direct attach_root pointer linkage is
also documented. Corpse and item placement are checked; final money placement
lacks equivalent local verification. Restore test linked as a candidate only,
not executed runtime or accounting release evidence.

Draft validation passed 13 fixtures: 508 routes, 2756 raw matches, 2698 unique
coordinates, 1743 mapped, 43 reviewed declarations, 912 unclassified.
No gameplay changes; release readiness remains false.

## Transport signs, duplicate inventory and tickets

Twelve unique coordinates classified. Sign reconciliation releases duplicate
contents and retires duplicate shells, admitting a new sign only when missing.
Duplicate transport NPC equipment/inventory moves to the selected survivor.
Ticket purchase allocates before fare debit and publication; redemption consumes
before route/rider setup. Its route read after extraction is a lifetime concern
for qualification. Compound fee/ticket and consumption/service recovery remain
unverified. All current transport.c lexical candidates mapped.

Draft validation passed 13 fixtures: 512 routes, 2756 raw matches, 2698 unique
coordinates, 1755 mapped, 43 reviewed declarations, 900 unclassified.
No gameplay changes; release readiness remains false.

## Epic refunds, stat purchases and stone absorption

Thirteen coordinates classified. Epic skill refund stages pickup credit with
wallet-transaction fallback after skill reset; stages/commit/save remain distinct.
Nine stat shop branches debit before permanent-stat spell effects and need compound
recovery. Stone absorption retires matching objects without local source quantity
increase; reads after extraction and loop advancement need lifetime qualification.
All current epic.c lexical candidates mapped; integration remains unverified.

Draft validation passed 13 fixtures: 515 routes, 2756 raw matches, 2698 unique
coordinates, 1768 mapped, 43 reviewed declarations, 887 unclassified.
No gameplay changes; release readiness remains false.

## Condition damage and scrap conversion

Eleven coordinates classified including MakeScrap definition/body. Condition damage
is itself a semantic mutation. Normal destruction replaces source with allocated
scrap and releases children; forced destruction extracts directly. Failed scrap
allocation or missing room can leave source despite caller destruction result.
Actual wearer resolution and direct stale location clearing require identity-aware
qualification. Children retained in room are neither new issuance nor retirement.

Draft validation passed 13 fixtures: 517 routes, 2756 raw matches, 2698 unique
coordinates, 1779 mapped, 43 reviewed declarations, 876 unclassified.
No gameplay changes; release readiness remains false.

## Kingdom node generation and personal gathering

Six coordinates classified under #486. Node generation preserves selected room,
richness and charge payload. Personal gathering spends shared charge before output
allocation/capacity checks, allowing partial or zero yield; rejected provisional
material differs from admitted material retirement. Exhaustion retires node and
surviving node records worked time. Compound charge/output recovery unqualified.

Draft validation passed 13 fixtures: 519 routes, 2756 raw matches, 2698 unique
coordinates, 1785 mapped, 43 reviewed declarations, 870 unclassified.
No gameplay changes; release readiness remains false.

## Kingdom node cleanup and realm harvesting

Five remaining kingdom_harvest.c coordinates classified: room/world reaping,
periodic exhaustion, shutdown cleanup and realm harvest exhaustion. Reaping covers
expired/out-of-region/owned-land nodes. Shutdown requires transient-policy review.
Realm harvest updates capped abstract resource counters via dirty marking, not
wallet or material inventory, and spends node charge even when nothing is banked.
This documents the semantic boundary without adding a currency category.

Draft validation passed 13 fixtures: 524 routes, 2756 raw matches, 2698 unique
coordinates, 1790 mapped, 43 reviewed declarations, 865 unclassified.
No gameplay changes; release readiness remains false.

## Highway special item mutations

Fifteen coordinates classified: wand random gems, ankh/sword exchange, spider
web creation, fire smoke scheduling and smoke/fire conversion. Wand rerolls loop
bound and clears source flags; sword exchange consumes ankh and moves existing
sword. Smoke conversion detaches smoke without local extraction. Event/object
publication and compound input/output durability remain qualification work.

Draft validation passed 13 fixtures: 529 routes, 2756 raw matches, 2698 unique
coordinates, 1805 mapped, 43 reviewed declarations, 850 unclassified.
No gameplay changes; release readiness remains false.

## Shaman spell debris, death drops and summoned cash

Eleven unique coordinates classified. Iceball creates checked decaying debris.
Transfer-wellness explosion moves inventory/equipment to room and creates wallet
pile without local debit, then invokes death; duplicate-money interaction needs
qualification. Summoned beast zeroes freshly loaded NPC cash after room placement,
not player wallet. Current smagic.c lexical candidates mapped.

Draft validation passed 13 fixtures: 532 routes, 2756 raw matches, 2698 unique
coordinates, 1816 mapped, 43 reviewed declarations, 839 unclassified.
No gameplay changes; release readiness remains false.

## Racial object generation and falling movement

Twelve affects.c coordinates classified. Racial droppings allocate before goblin's
second chance, leaving an unplaced allocation on that refusal path; valid outputs
publish to current/prior room and invalid destinations discard provisional output.
Falling transfers existing object across room boundary under recursion guard;
height-only movement changes z_cord. No new issuance for existing falling items.
Transient and durable event/publication handling remain unqualified.

Draft validation passed 13 fixtures: 534 routes, 2756 raw matches, 2698 unique
coordinates, 1828 mapped, 43 reviewed declarations, 827 unclassified.
No gameplay changes; release readiness remains false.

## Summoned replacement routing and rope consumption

Eight coordinates classified. Replacement helper separates direct NPC publication
from PC grant submission; immediate refusal discards provisional output. Global
matching retirement must bind exact old UIDs. Bind/capture effects precede held
rope extraction with artifact retirement enabled. Compound effects, replacement
retirement and save acknowledgement remain unqualified.

Draft validation passed 13 fixtures: 538 routes, 2756 raw matches, 2698 unique
coordinates, 1836 mapped, 43 reviewed declarations, 819 unclassified.
No gameplay changes; release readiness remains false.

## Summoned payloads and corpse carving

Four remaining new_skills.c coordinates classified. Book/totem allocation precedes
payload configuration and replacement grant. Carving consumes part availability
before failure checks; successful output changes corpse weight before limb weight
overrides and insertion. Preserve exact payload and original failure semantics.
All current lexical candidates in this file mapped; runtime qualification remains.

Draft validation passed 13 fixtures: 541 routes, 2756 raw matches, 2698 unique
coordinates, 1840 mapped, 43 reviewed declarations, 815 unclassified.
No gameplay changes; release readiness remains false.

## Ethermancer creation, consumption and relocation

Ten coordinates classified: faerie dust consumption, wind blade grant, frost
beacon/debris creation and cosmic-rift relocation. Beacon flag follows publication;
frost debris allocation precedes NOWHERE refusal. Rift matches value3 before corpse
type and traverses after moving object; exact destinations and save acknowledgement
remain unqualified. All current ethermancer.c lexical candidates mapped.

Draft validation passed 13 fixtures: 546 routes, 2756 raw matches, 2698 unique
coordinates, 1850 mapped, 43 reviewed declarations, 805 unclassified.
No gameplay changes; release readiness remains false.

## Preview objects, recipe outputs and seasonal NPC item

Ten drannak.c coordinates classified. Store display and random recipe templates
are provisional objects, not admitted wealth. Recipe scroll creation is separate
from template cleanup and targets supplied character (victim in random_recipe).
Zone selection rejects potion then falls back to random equipment. Seasonal NPC
receives unchecked item allocation before room placement. Replay/admission remains
unqualified; all current lexical file candidates mapped.

Draft validation passed 13 fixtures: 551 routes, 2756 raw matches, 2698 unique
coordinates, 1860 mapped, 43 reviewed declarations, 795 unclassified.
No gameplay changes; release readiness remains false.

## NPC ship treasure generation

Twelve ship_npc.c coordinates classified under #486. Chest publication precedes
key checks; later null return can leave published chest. Generated platinum,
materials and stones form nested reward outputs, while key belongs to captain.
Cyric crew adds fragment/key without local chest/allocation checks. One retained
ship-generation manifest must prevent rerolled or duplicate rewards on recovery.

Draft validation passed 13 fixtures: 553 routes, 2756 raw matches, 2698 unique
coordinates, 1872 mapped, 43 reviewed declarations, 783 unclassified.
No gameplay changes; release readiness remains false.

## Login bank reward, zero baseline and provisional kit cleanup

Six coordinates classified. CHAOS bank grant uses identified operation and pending
intent for million-platinum bank delta; queue acceptance is not saved completion.
Fresh character zero wallet/bank baseline differs from economic retirement. Kit
destructor owns unpublished roots until successful coordinator handoff. Remaining
kit materialization, ordinary newbie grant and wipe paths still require review.

Draft validation passed 13 fixtures: 556 routes, 2756 raw matches, 2698 unique
coordinates, 1878 mapped, 43 reviewed declarations, 777 unclassified.
No gameplay changes; release readiness remains false.

## Starter kit forests and equipment wipe

Fifteen remaining nanny.c coordinates classified. CHAOS builder filters/configures
provisional roots and bag contents, handing forest ownership over only on accepted
batch submission. Ordinary newbie materialization distinguishes direct NPC placement
from deferred PC grants. Equipment wipe retires roots and deletes locker files;
commented ship/wallet resets are not active behavior. Recovery remains unqualified.

Draft validation passed 13 fixtures: 561 routes, 2756 raw matches, 2698 unique
coordinates, 1893 mapped, 43 reviewed declarations, 762 unclassified.
No gameplay changes; release readiness remains false.

## Ship shells and rejected insurance delivery

Seven coordinates classified. Ship constructor registers runtime hash before panel
allocation; allocation is not proven admission and can serve restore callers.
Deletion detaches/extracts panel and representation after layout cleanup; caller
lifecycle must distinguish runtime cleanup from retirement. Rejected bank insurance
stages auction pickup from captured entitlement; repeat-safe fallback unqualified.

Draft validation passed 13 fixtures: 564 routes, 2756 raw matches, 2698 unique
coordinates, 1900 mapped, 43 reviewed declarations, 755 unclassified.
No gameplay changes; release readiness remains false.

## Ship movement and sinking insurance

Sixteen coordinates classified across shutdown evacuation, loading, panel reset,
navigation, docking fallback, sinking and summon arrival. Existing representation
movement is not issuance; cargo helper effects remain separate qualification.
Insurance bank path truncates to platinum while fallback uses full amount and can
mutate ship coffers directly. Void docking fallback later overwrites ship location
with original destination; state agreement requires review.

Draft validation passed 13 fixtures: 572 routes, 2756 raw matches, 2698 unique
coordinates, 1916 mapped, 43 reviewed declarations, 739 unclassified.
No gameplay changes; release readiness remains false.

## Ship shop hull cash, summon fees and sale credits

Eight coordinates classified. Hull callback pairs epic outcome with later cash and
hull effects; summon fee precedes delayed service. Cargo/contraband sale credit
follows slot/market helpers and precedes separately queued ship/market persistence.
Slot-sale ordering differs for weapons versus equipment/diplomat. Compound durable
outcomes and replay remain unqualified.

Draft validation passed 13 fixtures: 577 routes, 2756 raw matches, 2698 unique
coordinates, 1924 mapped, 43 reviewed declarations, 731 unclassified.
No gameplay changes; release readiness remains false.

## Disabled ship sale and repair charges

Seven coordinates classified. Whole-ship sale payout is unreachable after explicit
refusal. Repair-all, sail, armor and internal fees precede state/time changes.
All-armor branch lacks local save/status call used by single arc. Internal price
is1000 per point despite comment suggesting greater cost. Preserve actual formulas
and disabled behavior; compound durability remains unqualified.

Draft validation passed 13 fixtures: 582 routes, 2756 raw matches, 2698 unique
coordinates, 1931 mapped, 43 reviewed declarations, 724 unclassified.
No gameplay changes; release readiness remains false.

## Ship weapon repair, ammunition and rename fee

Three payment coordinates classified. Weapon repair and selected/all ammunition
reload debit before state/time changes and queued save. Reload is abstract ammo
counter refill, not item allocation. Rename helper reports success before wallet
debit, so cross-state durable ordering still requires qualification.

Draft validation passed 13 fixtures: 585 routes, 2756 raw matches, 2698 unique
coordinates, 1934 mapped, 43 reviewed declarations, 721 unclassified.
No gameplay changes; release readiness remains false.

## Cargo and contraband purchases

Two payment coordinates classified with coupled semantic slot/market writers.
Cargo reuses same-port slot and applies epic discount; contraband selects empty
slot and uses full price. Both mutate slot quantity/basis before wallet debit,
then market adjustment and queued ship save. These abstract slots are not item
allocations. Atomic durability/replay remain unqualified.

Draft validation passed 13 fixtures: 587 routes, 2756 raw matches, 2698 unique
coordinates, 1936 mapped, 43 reviewed declarations, 719 unclassified.
No gameplay changes; release readiness remains false.

## Ship weapon and equipment installation

Two payment coordinates classified with slot/maintenance mutations. Weapon
progression gate uses conjunction and has inactive PvP build multiplier; equipment
uses active PvP multiplier and listed weight despite scaled-weight prose. Both
debit before installation; exact eligibility/pricing and compound save/replay
remain qualification work.

Draft validation passed 13 fixtures: 589 routes, 2756 raw matches, 2698 unique
coordinates, 1938 mapped, 43 reviewed declarations, 717 unclassified.
No gameplay changes; release readiness remains false.

## Moonstone assembly, quest spawn and exchanges

Sixteen coordinates classified across assembly, fragment generation, automaton
crew purchase and ring reward. Assembly and reward consume inputs before output
prototype/allocation succeeds; fragment generation can publish one item before
returning failure. Crew purchase couples wallet debit, item retirement and
semantic ship mutation before queued save. Atomicity and restart/retry behavior
remain qualification work; no incidental gameplay fixes were introduced.

Draft validation passed 13 fixtures: 593 routes, 2756 raw matches, 2698 unique
coordinates, 1954 mapped, 43 reviewed nonwriters, 701 unclassified.
No gameplay changes; release readiness remains false.

## Remaining ship-shop candidates

Five coordinates classified: crew and chief hiring debit before semantic ship
changes and queued save; summon moves the existing ship item into transit and
schedules arrival after optional cargo clearing. The old hull debit is unreachable
because the current branch submits an asynchronous purchase and returns first.
It remains a dormant classified site, not a second active payment route.

Draft validation passed 13 fixtures: 597 routes, 2756 raw matches, 2698 unique
coordinates, 1959 mapped, 43 reviewed nonwriters, 696 unclassified.
No gameplay changes; release readiness remains false.

## Shopkeeper singleton reconciliation

Eight coordinates classified separately for existing stock transfer/template
cleanup and missing produced-template regeneration. Recovery policy must avoid
counting templates as fresh finite-stock issuance. Duplicate character extraction
is an indirect cleanup boundary requiring qualification, including branches that
skip stock transfer. Listed singleton test is a candidate, not executed evidence.

Draft validation passed 13 fixtures: 599 routes, 2756 raw matches, 2698 unique
coordinates, 1967 mapped, 43 reviewed nonwriters, 688 unclassified.
No gameplay changes; release readiness remains false.

## Forced weapon drop

Seven coordinates classified as same-owner inventory staging, durable submission
and floor publication, or direct legacy/NPC movement. Refused durable submission
retains an unequipped weapon in inventory. Publication retry accepts existing
floor placement before dirty/hint side effects; restart/save behavior remains
unqualified. Existing forced-drop test listed as candidate, not executed evidence.

Draft validation passed 13 fixtures: 602 routes, 2756 raw matches, 2698 unique
coordinates, 1974 mapped, 43 reviewed nonwriters, 681 unclassified.
No gameplay changes; release readiness remains false.

## Chaos material prefetch and collection

Four coordinates classified: temporary description prefetch is not economic
admission, while collection extracts existing material UIDs after committed
batch destruction. Collection scoreboard mutates before submit; rollback,
in-memory pending state, context validation and restart/save ordering remain
qualification concerns. Prefetch test inspected and listed, not executed.

Draft validation passed 13 fixtures: 604 routes, 2756 raw matches, 2698 unique
coordinates, 1978 mapped, 43 reviewed nonwriters, 677 unclassified.
No gameplay changes; release readiness remains false.

## Static material-rarity reporting

Nine coordinates classified across template existence/name helpers and report
iteration. All allocate temporary objects and extract without publication;
report counts are static composition, not economic issuance or live scarcity.
Allocation/extraction side effects remain unqualified. Existing report test is
listed as a candidate, not executed evidence.

Draft validation passed 13 fixtures: 607 routes, 2756 raw matches, 2698 unique
coordinates, 1987 mapped, 43 reviewed nonwriters, 668 unclassified.
No gameplay changes; release readiness remains false.

## Disabled breath-weapon item drops

Six coordinates in fire, frost and crimson breath are inside #if FALSE blocks
(FALSE is defined as false in core/config.h). They remain traceable dormant
candidate routes, not active destruction or transfer coverage. Re-enabling them
would require custody/environmental-placement qualification.

Draft validation passed 13 fixtures: 610 routes, 2756 raw matches, 2698 unique
coordinates, 1993 mapped, 43 reviewed nonwriters, 662 unclassified.
No gameplay changes; release readiness remains false.

## Smoking item consumption

Four coordinates classified: environmental herb loss before effects, rolling
paper/pipe retirement after effects, and herb consumption restricted to PCs or
PC pets. Surviving pipe durability decrement is a semantic writer. FALSE herb
extraction is actual consumption here, unlike temporary-object cleanup.
Replay and durable ordering remain unqualified.

Draft validation passed 13 fixtures: 613 routes, 2756 raw matches, 2698 unique
coordinates, 1997 mapped, 43 reviewed nonwriters, 658 unclassified.
No gameplay changes; release readiness remains false.

## Account display temporary inventory cleanup

Four coordinates classified in cleanup_temp_char, called after temporary
restoreCharOnly display extraction. Equipment/inventory extraction disposes
loaded copies, not intended saved-item retirement. Helper side effects and
restore-failure cleanup remain qualification work.

Draft validation passed 13 fixtures: 614 routes, 2756 raw matches, 2698 unique
coordinates, 2001 mapped, 43 reviewed nonwriters, 654 unclassified.
No gameplay changes; release readiness remains false.

## Rogue Slip custody paths

Seven coordinates classified: durable PC-to-PC success, direct legacy success,
and failed-skill floor drop. Failure still mutates custody without the success
branch transaction. Committed publication accepts already-delivered inventory
before character saves/notching; restart and save acknowledgments remain
qualification work. No behavior changes were made.

Draft validation passed 13 fixtures: 617 routes, 2756 raw matches, 2698 unique
coordinates, 2008 mapped, 43 reviewed nonwriters, 647 unclassified.
Release readiness remains false.

## Disguise kit consumption

Four coordinates classified for probabilistic failed-roll kit retirement and
successful disguise kit retirement. Held kits are unequipped before extraction;
size refusal skips consumption. Kit requirement and consumption exemption use
different boolean conditions; preserve existing behavior during integration.

Draft validation passed 13 fixtures: 619 routes, 2756 raw matches, 2698 unique
coordinates, 2012 mapped, 43 reviewed nonwriters, 643 unclassified.
No gameplay changes; release readiness remains false.

## Innate spring and foundry admission

Four coordinates classified: delayed innate events allocate checked prototypes,
publish into actor current room, then attach timed decay. Timed decay does not
by itself prove a transient accounting exemption. Event retry/recovery, room
changes and allocation/placement failure remain qualification work.

Draft validation passed 13 fixtures: 621 routes, 2756 raw matches, 2698 unique
coordinates, 2016 mapped, 43 reviewed nonwriters, 639 unclassified.
No gameplay changes; release readiness remains false.

## Ferry initialization and movement

Six coordinates classified: ferry/station object generation can partially
publish before initialization failure; route movement relocates an existing
ferry identity and advances route state before destination validation.
Infrastructure policy, repeat initialization and recovery remain unqualified.

Draft validation passed 13 fixtures: 623 routes, 2756 raw matches, 2698 unique
coordinates, 2022 mapped, 43 reviewed nonwriters, 633 unclassified.
No gameplay changes; release readiness remains false.

## Ferry ticket purchase and undead ferry admission

Six coordinates classified: provisional ticket allocation/insufficient-funds
cleanup, machine-price debit then ticket publication, and undead ferry room
admission. Displayed ferry price and charged machine field are distinct sources.
Compound durability and repeated initialization remain unqualified.

Draft validation passed 13 fixtures: 625 routes, 2756 raw matches, 2698 unique
coordinates, 2028 mapped, 43 reviewed nonwriters, 627 unclassified.
No gameplay changes; release readiness remains false.

## Generated NPC wallet restoration

Four denomination assignments classified as decoded snapshot restoration.
Empty state leaves wallet unchanged; capture can intentionally encode zero
currency. Caller authority selection and repeated recovery must be qualified
without treating restored balances as new issuance. Listed tests are candidates,
not executed evidence.

Draft validation passed 13 fixtures: 626 routes, 2756 raw matches, 2698 unique
coordinates, 2032 mapped, 43 reviewed nonwriters, 623 unclassified.
No gameplay changes; release readiness remains false.

## Random NPC currency and loot generation

Four coordinates classified for generated platinum/gold and random equipment
attachment. Local generation level differs from capped displayed level; loot
loop has no explicit three-item cap despite comment. Map placement failure
extracts NPC after generation, requiring indirect cleanup qualification.

Draft validation passed 13 fixtures: 628 routes, 2756 raw matches, 2698 unique
coordinates, 2036 mapped, 43 reviewed nonwriters, 619 unclassified.
No gameplay changes; release readiness remains false.

## Achievement item and currency rewards

Four coordinates classified for level-five gift and two ship-owner currency
rewards. Reward mutation precedes progress marker. Chaos tattoo helper does not
locally gate ship-owner credit on existing level progress; caller once-only
control remains to qualify. No incidental behavior fixes introduced.

Draft validation passed 13 fixtures: 631 routes, 2756 raw matches, 2698 unique
coordinates, 2040 mapped, 43 reviewed nonwriters, 615 unclassified.
Release readiness remains false.

## Disabled wagon initialization

Two coordinates classified behind unconditional init_wagons entry return.
Dormant body allocates wagon after horse placement and links it after room
publication. These are not active issuance sites; reactivation needs qualification.

Draft validation passed 13 fixtures: 632 routes, 2756 raw matches, 2698 unique
coordinates, 2042 mapped, 43 reviewed nonwriters, 613 unclassified.
No gameplay changes; release readiness remains false.

## Weather item transitions

Five coordinates classified: timed flower replacement allocates before old
flower extraction and room publication; wind stows existing worn light into
same character inventory. Paired replacement/recovery and movement persistence
remain unqualified.

Draft validation passed 13 fixtures: 634 routes, 2756 raw matches, 2698 unique
coordinates, 2047 mapped, 43 reviewed nonwriters, 608 unclassified.
No gameplay changes; release readiness remains false.

## Training dummy economic-state cleanup

Eight coordinates classified: profile removes prototype equipment/inventory and
zeros wallet before caller places dummy in room. Distinguish pre-admission
cleanup from destruction of admitted assets. Tests listed as candidates only;
helper side effects and lifecycle qualification remain open.

Draft validation passed 13 fixtures: 636 routes, 2756 raw matches, 2698 unique
coordinates, 2055 mapped, 43 reviewed nonwriters, 600 unclassified.
No gameplay changes; release readiness remains false.

## Thought beacon replacement

Three coordinates classified: prior beacon removal and affect update precede
replacement allocation, decay/PID configuration and room admission. Prior-object
lookup does not locally compare PID. Failure, ownership selection and replay
qualification remain open; no behavior change introduced.

Draft validation passed 13 fixtures: 637 routes, 2756 raw matches, 2698 unique
coordinates, 2058 mapped, 43 reviewed nonwriters, 597 unclassified.
Release readiness remains false.

## Imprisonment shell lifecycle

Three coordinates classified: shell allocation precedes possible disbelieve
return without local cleanup; accepted cast changes victim state before room
publication. Damage retires shell only when strictly greater than remaining HP,
otherwise mutating HP. Failure/recovery/effect coupling remain unqualified.

Draft validation passed 13 fixtures: 639 routes, 2756 raw matches, 2698 unique
coordinates, 2061 mapped, 43 reviewed nonwriters, 594 unclassified.
No gameplay changes; release readiness remains false.

## Epic skill payment and magical device infusion

Three coordinates classified: committed epic purchase callback debits wallet
before skill update/save or submits refund on changed state; infusion consumes
stone before semantic device charge/wear changes and cooldown. Compound replay,
refund and save acknowledgment remain unqualified.

Draft validation passed 13 fixtures: 641 routes, 2756 raw matches, 2698 unique
coordinates, 2064 mapped, 43 reviewed nonwriters, 591 unclassified.
No gameplay changes; release readiness remains false.

## Mail postage and letter admission

Five coordinates classified: historical date-gated gift, postage charged before
composition, and stored-message deletion before letter publication. Allocation
failure, composition abort and publication/restart ordering remain unqualified.

Draft validation passed 13 fixtures: 644 routes, 2756 raw matches, 2698 unique
coordinates, 2069 mapped, 43 reviewed nonwriters, 586 unclassified.
No gameplay changes; release readiness remains false.

## Track object replacement

Three coordinates classified: sector-limit cleanup precedes unchecked new track
allocation, configuration, room placement and decay. Explicit transient policy
and failure/recovery qualification remain open.

Draft validation passed 13 fixtures: 645 routes, 2756 raw matches, 2698 unique
coordinates, 2072 mapped, 43 reviewed nonwriters, 583 unclassified.
No gameplay changes; release readiness remains false.

## Ship maneuver and coffer withdrawal

Three coordinates classified: maneuver updates semantic location before moving
existing ship representation; owner coffer claim credits wallet before zeroing
ship money without local save. Caller durability/replay remain unqualified.

Draft validation passed 13 fixtures: 647 routes, 2756 raw matches, 2698 unique
coordinates, 2075 mapped, 43 reviewed nonwriters, 580 unclassified.
No gameplay changes; release readiness remains false.

## Guild association transfers and MOTD paper

Four coordinates classified: deposit/withdrawal couple player and guild balances
with sequential saves; deposit also changes debt state. MOTD paper consumption
follows unchecked local file write. Atomic durability/replay remain unqualified.

Draft validation passed 13 fixtures: 650 routes, 2756 raw matches, 2698 unique
coordinates, 2079 mapped, 43 reviewed nonwriters, 576 unclassified.
No gameplay changes; release readiness remains false.

## Training split payment and practice

Three wallet coordinates classified: RobCash debits bank shortfall before wallet
remainder, while practice debits before skill increment. Wallet return handling,
compound debit and caller save/replay remain qualification work.

Draft validation passed 13 fixtures: 652 routes, 2756 raw matches, 2698 unique
coordinates, 2082 mapped, 43 reviewed nonwriters, 573 unclassified.
No gameplay changes; release readiness remains false.

## World quest item and mercenary rewards

Five coordinates classified: provisional reward allocation/grant/refusal cleanup
and mercenary currency credits. Other rewards and quest reset continue after
failed item submission. Accepted submission is not publication acknowledgment;
compound completion/replay qualification remains open.

Draft validation passed 13 fixtures: 654 routes, 2756 raw matches, 2698 unique
coordinates, 2087 mapped, 43 reviewed nonwriters, 568 unclassified.
No gameplay changes; release readiness remains false.

## Quest reward profile probes

Two coordinates classified as temporary prototype inspection with RAII cleanup;
only reward metadata survives and no item is published. Allocation/extraction
helper side effects remain unqualified independently of actual quest grants.

Draft validation passed 13 fixtures: 655 routes, 2756 raw matches, 2698 unique
coordinates, 2089 mapped, 43 reviewed nonwriters, 566 unclassified.
No gameplay changes; release readiness remains false.

## Allocator bulk-copy false positives

Two mm.c matches copy allocator next pointers, not economic state. Explicitly
classified non-economic in route inventory because nonwriter schema accepts
only declarations. They must not become required currency integration sites;
route counts include classified false positives as well as actual writers.

Draft validation passed 13 fixtures: 656 routes, 2756 raw matches, 2698 unique
coordinates, 2091 mapped, 43 reviewed nonwriters, 564 unclassified.
No gameplay changes; release readiness remains false.

## Scribing scroll consumption

One extraction coordinate classified with adjacent book-page and scroll-slot
mutations. One-spell scroll retires, multi-spell scroll loses matching spell,
and zero-spell error returns after page progress. Replay/save qualification open.

Draft validation passed 13 fixtures: 657 routes, 2756 raw matches, 2698 unique
coordinates, 2092 mapped, 43 reviewed nonwriters, 563 unclassified.
No gameplay changes; release readiness remains false.

## Guild creation confirmation payment

One coordinate classified: confirmation rechecks affordability/founder, calls
association creation, then charges only on helper success and clears confirmation.
Creation/payment atomicity and restart qualification remain open.

Draft validation passed 13 fixtures: 658 routes, 2756 raw matches, 2698 unique
coordinates, 2093 mapped, 43 reviewed nonwriters, 562 unclassified.
No gameplay changes; release readiness remains false.

## Locker identification prepared payment

One submit coordinate classified with prepared-operation identity, owner checks,
uncertain-result retry, paid receipt recording and display/delivery marker phases.
No item transfer occurs. Duplicate-charge and receipt failure/recovery behavior
remain qualification work.

Draft validation passed 13 fixtures: 659 routes, 2756 raw matches, 2698 unique
coordinates, 2094 mapped, 43 reviewed nonwriters, 561 unclassified.
No gameplay changes; release readiness remains false.

## Deferred device scroll cleanup

One extraction coordinate classified with prior spell-slot consumption and
in-memory UID reservation. Finish queues physical cleanup for game-thread pulse;
local action commit does not establish durable accounting. Abort/restart/save
qualification remains open.

Draft validation passed 13 fixtures: 660 routes, 2756 raw matches, 2698 unique
coordinates, 2095 mapped, 43 reviewed nonwriters, 560 unclassified.
No gameplay changes; release readiness remains false.

## Wand-of-wonder gem admission

Three coordinates classified: preselected gem allocation, invalid-action
provisional cleanup, and room publication. Local commit consumes a wand charge;
it does not prove durable accounting. Preserve repeated-bound random selection,
partial publication and selected-count damage on allocation failure. Replay,
publication/save acknowledgement and cancellation qualification remain open.

Draft validation passed 13 fixtures: 661 routes, 2756 raw matches, 2698 unique
coordinates, 2098 mapped, 43 reviewed nonwriters, 557 unclassified.
No gameplay changes; release readiness remains false.

## Vecna special item custody

Ten coordinates grouped into four existing-item movement routes: north/down
corpse movement, oaken-staff pre-death inventory drop, and Krindor container
reset. Include indirect unequip/death effects and device semantic mutation in
qualification. Krindor traverses next_content after relocating each item;
complete traversal and unsupported-location behavior remain concerns, not fixes.
No dedicated runtime test evidence added.

Draft validation passed 13 fixtures: 665 routes, 2756 raw matches, 2698 unique
coordinates, 2108 mapped, 43 reviewed nonwriters, 547 unclassified.
No gameplay changes; release readiness remains false.

## Claw Caverns golem death bundle

Nine unique coordinates (ten raw matches) classified as one compound NPC death
route: new shard shell, existing inventory/equipment, and wallet-backed money
pile. Invalid fallback destroys a container already holding existing assets;
it is not solely provisional cleanup. This helper does not zero the wallet.
Caller death/default-corpse handling, retirement, repeated invocation and save/
replay conservation require qualification; no incidental fix or test claim.

Draft validation passed 13 fixtures: 666 routes, 2756 raw matches, 2698 unique
coordinates, 2117 mapped, 43 reviewed nonwriters, 538 unclassified.
No gameplay changes; release readiness remains false.

## Twin Towers forest death and decay

Eight coordinates classified into death-object admission and periodic corpse
replacement. Replacement publishes before original destruction; unsupported
location destroys both. Allocation failure leaves the post-decrement counter
below zero, and contents are not explicitly transferred. These are qualification
concerns only. Caller death behavior, compound replacement and restart/save
conservation remain unverified.

Draft validation passed 13 fixtures: 668 routes, 2756 raw matches, 2698 unique
coordinates, 2125 mapped, 43 reviewed nonwriters, 530 unclassified.
No gameplay changes; release readiness remains false.

## Dragonnia disarm, false key and remains

Seven unique coordinates (eight raw matches) classified as disarm custody,
false-key stow/destruction and customized remains admission. Key selection uses
first carried match after stowing; remains name alone does not prove wallet
conversion. Prototype semantics, caller lifetime and save/replay qualification
remain open. No incidental fixes or runtime evidence added.

Draft validation passed 13 fixtures: 671 routes, 2756 raw matches, 2698 unique
coordinates, 2132 mapped, 43 reviewed nonwriters, 523 unclassified.
No gameplay changes; release readiness remains false.

## Ailvio map grants and stock replacement

Seven coordinates classified into repeatable map admission and existing pickup
with secret room-stock replenishment. Preserve separate command grants while
deduplicating replay of the same operation; replacement allocation refusal
leaves original stock unchanged. Partial placement and save/restart behavior
remain qualification work. No gameplay changes or runtime tests.

Draft validation passed 13 fixtures: 673 routes, 2756 raw matches, 2698 unique
coordinates, 2139 mapped, 43 reviewed nonwriters, 516 unclassified.
Release readiness remains false.

## Juiblex death-object admission

Six coordinates classified as paired lake objects and wormhole admission.
Second lake allocation failure leaves the first provisional object without
local cleanup; wormhole publication precedes NPC relocation. Both handlers
return false after success. Caller lifetime, partial admission and indirect
custody/save/replay effects remain qualification work, not incidental fixes.

Draft validation passed 13 fixtures: 675 routes, 2756 raw matches, 2698 unique
coordinates, 2145 mapped, 43 reviewed nonwriters, 510 unclassified.
No gameplay changes; release readiness remains false.

## Verzanan dog consumption

Six coordinates classified in two handlers: eligible corpse/food contents
release followed by shell destruction. Preserve age, awake and combat gates.
The persistence defer helper precedes direct mutation; its completion path and
direct fallback both require qualification. Existing contents are not issuance.

Draft validation passed 13 fixtures: 677 routes, 2756 raw matches, 2698 unique
coordinates, 2151 mapped, 43 reviewed nonwriters, 504 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Lohrr scripted death assets

Five coordinates classified as one legacy scripted death route: headgear
retention/hiding, new head admission, indirect death and corpse/content sink.
Deferred corpse destruction precedes direct extraction. Name-based corpse
selection, partial effects and save/replay qualification remain open; no change
to targeting or gameplay policy.

Draft validation passed 13 fixtures: 678 routes, 2756 raw matches, 2698 unique
coordinates, 2156 mapped, 43 reviewed nonwriters, 499 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Studio script object actions

Five coordinates classified into room load, actor gift and self-object purge.
GIVE creates an asset rather than transferring script-owner stock. Record
trigger/action identity and oneof selection without suppressing legitimate
repeat invocations. Ordered partial execution, indirect mobile purge, contents
and save/replay remain qualification work.

Draft validation passed 13 fixtures: 681 routes, 2756 raw matches, 2698 unique
coordinates, 2161 mapped, 43 reviewed nonwriters, 494 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Kingdom guard weapons and banners

Eight coordinates classified into five routes: checked weapon equip, banner
admission, realm cleanup, combat destruction and orphan cleanup. Preserve
occupied-hand issuance gate, banner HP/association and distinction between
zero-association bulk cleanup and orphan-only selection. Test file is a
candidate, not executed coverage; lifetime/save/replay qualification remains.

Draft validation passed 13 fixtures: 686 routes, 2756 raw matches, 2698 unique
coordinates, 2169 mapped, 43 reviewed nonwriters, 486 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Ioun overuse destruction

Seven coordinates classified as multi-owner equipment destruction with retained
contents and source retirement. Preserve weekly/use-count/level gates and
artifact/trusted/NPC exemptions. Source may be encountered before its final
extraction; lifetime and carried/inside branch reachability remain unverified.
No incidental fix; compound saves and replay still need qualification.

Draft validation passed 13 fixtures: 687 routes, 2756 raw matches, 2698 unique
coordinates, 2176 mapped, 43 reviewed nonwriters, 479 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Random equipment factories and fountains

Six coordinates classified into conditional fountain publication and material,
stone and equipment factories. Factory returns are provisional allocations,
not admission; caller placement and chosen random semantics need qualification.
Fountain retry exhaustion and build reachability remain explicit concerns.

Draft validation passed 13 fixtures: 691 routes, 2756 raw matches, 2698 unique
coordinates, 2182 mapped, 43 reviewed nonwriters, 473 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Beholder disintegration assets

Six coordinates classified as condition loss, retained-content release and
equipment shell destruction. Artifact and save gates preserved. Presence scan
includes slot zero while selection excludes it; termination and branch
reachability are qualification concerns, not incidental fixes.

Draft validation passed 13 fixtures: 692 routes, 2756 raw matches, 2698 unique
coordinates, 2188 mapped, 43 reviewed nonwriters, 467 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## NPC special theft

Six coordinates classified into item custody and wallet transfer. Caught item
attempt handling may fall through into coin theft despite the choice comment.
Successful debit precedes unchecked credit; two-owner conservation, NPC
lifetime and save/replay qualification remain open. Preserve existing gates.

Draft validation passed 13 fixtures: 694 routes, 2756 raw matches, 2698 unique
coordinates, 2194 mapped, 43 reviewed nonwriters, 461 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Guildhall construction payments

Six wallet sinks classified with construction points and service state. Most
helpers construct first, then charge; overmax charges before increment. Preserve
overmax price units and trusted construction-point behavior, which differ from
other commands. Debit results, helper persistence and save/replay atomicity
remain unqualified. No incidental price or exemption changes.

Draft validation passed 13 fixtures: 700 routes, 2756 raw matches, 2698 unique
coordinates, 2200 mapped, 43 reviewed nonwriters, 455 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Outpost rubble and evacuation

Five coordinates classified as repair rubble retirement and death rubble
admission with existing gate-room item evacuation. Preserve building identity
and lifecycle ordering. Repeated gate_room loop does not prove all-interior
coverage; ownership-before-refusal and partial effects need qualification.

Draft validation passed 13 fixtures: 702 routes, 2756 raw matches, 2698 unique
coordinates, 2205 mapped, 43 reviewed nonwriters, 450 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Justice guard lifecycle assets

Five coordinates classified as torch admission and mission-end inventory/
equipment destruction. Retirement can destroy received assets, not just spawn
equipment. Adjacent CLEAR_MONEY is recorded as an initialization blindspot.
Equip refusal, NPC wallet lifetime and save/replay qualification remain open.

Draft validation passed 13 fixtures: 704 routes, 2756 raw matches, 2698 unique
coordinates, 2210 mapped, 43 reviewed nonwriters, 445 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Chaos test and admin issuance

Five coordinates classified as fixture-item grants, restricted quest-room
wallet reward and trusted chaos funds. Preserve authorization and distinguish
partial grant admission or committed callback from native publication/save
completion. Room movement precedes wallet submission. No test account data read.

Draft validation passed 13 fixtures: 707 routes, 2756 raw matches, 2698 unique
coordinates, 2215 mapped, 43 reviewed nonwriters, 440 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Pond and bloodstone spell assets

Five coordinates classified as pond admission and bloodstone replacement.
Bloodstone selects previous object by prototype without local owner check;
old retirement and affect mutation precede replacement allocation. Preserve
existing fields and gates; partial replacement/save/replay remain unqualified.

Draft validation passed 13 fixtures: 709 routes, 2756 raw matches, 2698 unique
coordinates, 2220 mapped, 43 reviewed nonwriters, 435 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Ship evacuation and cargo conversion

Eight coordinates classified into evacuation, destructive clearing, jettison
crate publication and salvage retirement. Preserve player-corpse/panel rules
and cargo survival distribution. Cargo caller reduces slots despite helper
failure; salvage adds cargo before crate destruction. Partial conversions,
indirect NPC assets and save/replay qualification remain open.

Draft validation passed 13 fixtures: 713 routes, 2756 raw matches, 2698 unique
coordinates, 2228 mapped, 43 reviewed nonwriters, 427 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Githyanki first-level weapon

Six coordinates classified as one class-selected level-50 reward. Caller
checks highest level below 50, then advances that marker even after failed
allocation or unsupported class. Preserve branch priority; admission and
progression save/replay qualification remain open.

Draft validation passed 13 fixtures: 714 routes, 2756 raw matches, 2698 unique
coordinates, 2234 mapped, 43 reviewed nonwriters, 421 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Random-object admin and preview paths

Seven coordinates classified as trusted grants, temporary preview cleanup and
provisional base factory. Recipe helper effects remain separate from disposal
of its input object. Grant null/refusal handling and caller publication/save/
replay qualification remain open.

Draft validation passed 13 fixtures: 717 routes, 2756 raw matches, 2698 unique
coordinates, 2241 mapped, 43 reviewed nonwriters, 414 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Player-file utility stubs

Six candidates belong to standalone pfile utility stubs (src/Makefile
PFILE_OBJS_NAMES). Allocation/free, no-op room/container placement and local
carrying-list linkage are not gameplay issuance/transfer/sinks. The memset
match is not wallet mutation. Recorded as an infrastructure route because
the nonwriter schema only supports declarations; exclude from active gameplay
integration scope and qualify actual utility persistence separately.

Draft validation passed 13 fixtures: 718 routes, 2756 raw matches, 2698 unique
coordinates, 2247 mapped, 43 reviewed nonwriters, 408 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Spell dispatch and paired walls

Nine coordinates classified into minor-creation delegation, avatar-focus
admission/reuse, vines herbs and paired walls. Preserve PC/pet ingredient cost
and NPC exemption. Walls set exit state before paired publication; allocation
partner cleanup and partial save/replay qualification remain open.

Draft validation passed 13 fixtures: 722 routes, 2756 raw matches, 2698 unique
coordinates, 2256 mapped, 43 reviewed nonwriters, 399 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## NQ quest payments and rewards

Nine coordinates classified into provisional item factory, rounded platinum
reward, player/NPC item publication and compound component/tag/wallet payment.
The action-test helper mutates assets; reward cash truncates to platinum.
Preserve economics and qualify partial effects, NPC lifetime and save/replay.

Draft validation passed 13 fixtures: 726 routes, 2756 raw matches, 2698 unique
coordinates, 2265 mapped, 43 reviewed nonwriters, 390 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Test-command prototype reports

Eight coordinates are temporary prototype copies for weapon-dice, missile and
two-pass shield reports. They are never published by these loops; exclude
from gameplay issuance/sink integration. Infrastructure route records this
classification under the current declaration-only nonwriter schema.

Draft validation passed 13 fixtures: 727 routes, 2756 raw matches, 2698 unique
coordinates, 2273 mapped, 43 reviewed nonwriters, 382 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Player item graph materialization

Five coordinates classified as staged rollback, identity-preserving graph
materialization and local discard. Shared graph helper serves restore and
creation payload callers; allocation is not a second issuance. UID clearing
and memory publication flags do not prove persisted deletion or commit.
Ownership hydration, caller lifecycle and save/replay qualification remain open.

Draft validation passed 13 fixtures: 730 routes, 2756 raw matches, 2698 unique
coordinates, 2278 mapped, 43 reviewed nonwriters, 377 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Player wallet and bank restoration

Eight assignments restore validated snapshot denominations with domain
revisions. They are not issuance or deposit. Snapshot freshness, pending
publication, partial character materialization and caller admission require
qualification; no cutover or acknowledgement proof inferred.

Draft validation passed 13 fixtures: 731 routes, 2756 raw matches, 2698 unique
coordinates, 2286 mapped, 43 reviewed nonwriters, 369 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Recovered NPC reset stock

Six coordinates classified as recovery-triggered reset replenishment, not
snapshot UID restoration. Preserve artifact, population, chance and existing
count/slot gates. Loaded count does not verify placement. Admission authority,
repeat recovery and save/replay qualification remain open; test is candidate only.

Draft validation passed 13 fixtures: 732 routes, 2756 raw matches, 2698 unique
coordinates, 2292 mapped, 43 reviewed nonwriters, 363 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## World recovery object graph

Nine coordinates classified as snapshot graph restoration and reverse rollback.
Existing roots may be reused; new representations preserve snapshot UIDs.
Failure cleanup does not locally clear those UIDs, so extraction side effects
and rollback membership require qualification. No new issuance inferred.

Draft validation passed 13 fixtures: 734 routes, 2756 raw matches, 2698 unique
coordinates, 2301 mapped, 43 reviewed nonwriters, 354 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Item ownership repository mutations

Five coordinates classified as current-owner insert/update, ownership ledger,
coin payload and owner-destruction command construction. These are storage
components, not independent gameplay issuance. Enclosing transaction, revision,
ambiguous retry and native publication qualification remain open.

Draft validation passed 13 fixtures: 739 routes, 2756 raw matches, 2698 unique
coordinates, 2306 mapped, 43 reviewed nonwriters, 349 unclassified.
No gameplay changes or runtime tests; release readiness remains false.

## Corpse repository compound mutations

Seven coordinates classified across bank preparation, wallet delta, nested
item transfers, physical row materialization and world-root retirement.
Shared operation IDs/event offsets and savepoint release require enclosing
transaction qualification; no separate issuance or native publication inferred.

Draft validation passed 13 fixtures: 745 routes, 2756 raw matches, 2698 unique
coordinates, 2313 mapped, 43 reviewed nonwriters, 342 unclassified.
No gameplay changes or runtime tests; release readiness remains false.


## NPC copyover asset recovery

Ten coordinates classified across file and buffer NPC wallet/item restoration.
Gold assignment restores saved balances. Equipment and inventory reconstruction
uses prototype vnums rather than retained object UIDs or complete item graphs;
recovery intent is not evidence of identity-preserving accounting. Duplicate
replay, missing prototypes, allocation failure and partial publication remain
qualification requirements under #489. No gameplay changes or runtime tests.
Draft validation passed 13 fixtures: 749 routes, 2756 raw matches, 2698 unique coordinates, 2323 mapped, 43 reviewed nonwriters, 332 unclassified. Release readiness remains false.


## Building evacuation and portals

Eight coordinates classified as existing room-object evacuation, portal
retirement and paired portal admission. Portal allocations publish independently
and generation returns success even on partial allocation; an existing partial
pair blocks retry. These are qualification findings, not additional bug fixes.
Indirect building NPC extraction remains a semantic census consideration.
No gameplay changes or runtime tests; release readiness remains false.
Draft validation passed 13 fixtures: 752 routes, 2756 raw matches, 2698 unique coordinates, 2331 mapped, 43 reviewed nonwriters, 324 unclassified.


## Remaining small scripted item routes

Classified tree detachment, soul quest issuance, mallet and paired quest sinks,
race-restoration unequip, corpse relocation, forced leggings equip and zombie
generator error retirement. Tree detachment has no destination/extraction at
the call; callbacks and partially published quest entities require qualification.
Incidental failures are recorded, not expanded into unrelated implementation.
No runtime changes or runtime tests; release readiness remains false.
Draft validation passed 13 fixtures: 760 routes, 2756 raw matches, 2698 unique coordinates, 2345 mapped, 43 reviewed nonwriters, 310 unclassified.


## Blackjack stakes and settlement

Two coordinates classified as stake debit and payout/refund credit. A table
message does not establish a house wallet transfer. Normal wins credit twice
the stake, pushes return it, and losses have already paid; the periodic path
uses a separate single-stake payout. Actor, durable wager identity, interruption
and repeat settlement require qualification. Credit messaging ignores outcome.
No gameplay changes or runtime tests; release readiness remains false.
Draft validation passed 13 fixtures: 762 routes, 2756 raw matches, 2698 unique coordinates, 2347 mapped, 43 reviewed nonwriters, 308 unclassified.


## Remaining storage and season-reset candidates

Eleven coordinates classified across combat reward storage, prepared currency
writes/bank row creation, restitution ownership and ledger, bank load results,
death quarantine, and auction/saved-item season reset. Read-result assignment
is not issuance; quarantine is not destruction; reset is a lifecycle boundary.
Transaction, restart and publication qualification remain unverified.
No runtime changes or runtime tests; release readiness remains false.
Draft validation passed 13 fixtures: 771 routes, 2756 raw matches, 2698 unique coordinates, 2358 mapped, 43 reviewed nonwriters, 297 unclassified.


## Remaining small command and item helpers

Eight coordinates classified as temporary world-list inspection, transfer
construction/submission, random-exit retirement, rename payment, web auction
removal submission and zone purge. Zone purge actually skips empty corpses;
nonempty nonartifact corpses reach extraction, contrary to nearby comment.
Rename service precedes unchecked debit; web success acknowledges submission.
These are qualification observations, not incidental gameplay fixes.
No runtime changes or runtime tests; release readiness remains false.
Draft validation passed 13 fixtures: 778 routes, 2756 raw matches, 2698 unique coordinates, 2366 mapped, 43 reviewed nonwriters, 289 unclassified.


## Alchemist mixing, repair and smelting

Grouped ingredient sinks, temporary recipe previews, poison/potion crafting,
repair material consumption, furnace conversion and NPC potion grants. Preserve
actual quantities: repair traversal consumes every matching material; potion
components are spent under loop conditions. Smelt failure can overwrite a
provisional result pointer. These are qualification findings, not recipe changes.
Encrust/enchant classifications remain pending. No runtime changes or tests.
Draft validation passed 13 fixtures: 789 routes, 2756 raw matches, 2698 unique coordinates, 2412 mapped, 43 reviewed nonwriters, 243 unclassified. Release readiness remains false.


## Alchemist encrust and enchant

Remaining fifteen alchemist coordinates classified: virtual jewel preparation,
paired failure sinks, replacement item conversion, owner-absence retirement,
enchant payment and delayed destruction. Pouch usage precedes output allocation;
enchant payment precedes delayed refusal/failure with no local refund. Preserve
existing economics while qualifying identity, interruption and publication.
No runtime changes or runtime tests; release readiness remains false.
Draft validation passed 13 fixtures: 795 routes, 2756 raw matches, 2698 unique coordinates, 2427 mapped, 43 reviewed nonwriters, 228 unclassified.


## Wizard grants and purges

Ten coordinates classified across provisional load, ownership establishment,
completion publication, named purge and room purge. UID-less direct admission
and UID-bearing asynchronous admission differ. Committed stale publication is
not successful native delivery; named artifact purge retains artifact registry.
NPC teardown remains an indirect asset boundary. No runtime changes or tests.
Draft validation passed 13 fixtures: 800 routes, 2756 raw matches, 2698 unique coordinates, 2437 mapped, 43 reviewed nonwriters, 218 unclassified. Release readiness remains false.


## Administrative storage lifecycle

Seventeen coordinates grouped across establishment, destruction, child removal
and native completion. Flatfile removal advances through separately submitted
children before root retirement; interrupted progress is not all-or-nothing.
Other modes publish new storage before writeSavedItem or mutate directly.
Backend parity and postcommit publication recovery remain qualification gaps.
No runtime changes or runtime tests; release readiness remains false.
Draft validation passed 13 fixtures: 807 routes, 2756 raw matches, 2698 unique coordinates, 2454 mapped, 43 reviewed nonwriters, 201 unclassified.


## Administrative prototype inspection exclusions

Twenty-four coordinates classified as temporary stat/list/rating copies or
unreachable disabled death-object listing. These have no gameplay admission;
allocation alone must not generate issuance. This completes raw actwiz matches,
not semantic coverage of every indirect administrative mutation.
No runtime changes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 815 routes, 2756 raw matches, 2698 unique coordinates, 2478 mapped, 43 reviewed nonwriters, 177 unclassified.


## Artifact recovery and retirement

Twenty-three coordinates grouped as ground/NPC reconstruction, artifact poof
and unsaved-character equipment teardown. Recovery uses prototype/location
records rather than prior item UIDs at these calls. Poof moves nested items
then extracts before owner/corpse saves. Teardown deliberately preserves
artifact registry and must not be assumed to destroy durable assets.
No runtime changes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 819 routes, 2756 raw matches, 2698 unique coordinates, 2501 mapped, 43 reviewed nonwriters, 154 unclassified.


## Artifact report exclusions

Sixteen coordinates grouped as JSON cache, flatfile artifact list and SQL/flat
player artifact list temporary prototypes. Display copies have no owner
publication and must not create gameplay issuance/destruction events.
No runtime changes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 822 routes, 2756 raw matches, 2698 unique coordinates, 2517 mapped, 43 reviewed nonwriters, 138 unclassified.


## Artifact swap

Ten unique coordinates grouped as validation copy, replacement preparation
and publication/old-artifact retirement. Existing replacement-vnum copy removal
precedes original lookup; replacement publication precedes registry update and
owner/corpse saves. Offline failed terminal save retains dummy for recovery.
Refusal cleanup and compound durability require qualification, not scope expansion.
No runtime changes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 825 routes, 2756 raw matches, 2698 unique coordinates, 2527 mapped, 43 reviewed nonwriters, 128 unclassified.


## Artifact clear, poof and timer commands

Seven coordinates classify temporary validation copies in commands that erase
registry entries, delegate real artifact poof, or edit expiry metadata. Registry
clear is not live-item extraction; timer change is not issuance. SQL main/mortal
partial deletion and delegated offline persistence remain qualification concerns.
No runtime changes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 828 routes, 2756 raw matches, 2698 unique coordinates, 2534 mapped, 43 reviewed nonwriters, 121 unclassified.


## Artifact maintenance movement

Eleven coordinates grouped as invalid-location Limbo salvage, corpse-expiry
preparation and artifact-wars forced drops. Salvage may move an outer container;
corpse save can precede poof; wars timer failure does not prevent native drop.
Compound custody/registry/publication qualification remains open.
No runtime changes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 831 routes, 2756 raw matches, 2698 unique coordinates, 2545 mapped, 43 reviewed nonwriters, 110 unclassified.


## Artifact legacy import and offline hunt

Eleven coordinates grouped as legacy import and offline ownership repair.
Import can remove live copies and create missing offline inventory, with registry
update before optional save; hunt can retire zone copies while correcting
registry ownership. These are not pure read-only scans or ordinary rewards.
No runtime changes or tests; lifecycle qualification remains open.
Draft validation passed 13 fixtures: 833 routes, 2756 raw matches, 2698 unique coordinates, 2556 mapped, 43 reviewed nonwriters, 99 unclassified. Release readiness remains false.


## Artifact binding maintenance

Seven coordinates classified as temporary description copies around binding
and timer reconciliation. SQL repair mismatch path accesses then extracts an
already extracted prototype; record lifetime concern without expanding scope.
Physical item ownership is distinct from binding metadata.
No runtime changes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 835 routes, 2756 raw matches, 2698 unique coordinates, 2563 mapped, 43 reviewed nonwriters, 92 unclassified.


## Winterhaven death reward groups

Fifty-six coordinates grouped into Cerberus optional/guaranteed rolls and
Tiamat, Dragonnia and Lanella death rewards. Cerberus branches can produce four
and five objects, not uniformly one; preserve actual probabilities and counts.
Stable death identity and partial publication need qualification.
No runtime changes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 840 routes, 2756 raw matches, 2698 unique coordinates, 2619 mapped, 43 reviewed nonwriters, 36 unclassified.


## Winterhaven decay and gifts

Twenty-one coordinates grouped as death object, corpse/heart replacement,
gem factory and wrapper opening. Heart countdown advances before allocation;
gem factory allocates but returns zero, so intended gift delivery is not proven.
Input contents, worn topology and partial publication require qualification.
No runtime fixes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 845 routes, 2756 raw matches, 2698 unique coordinates, 2640 mapped, 43 reviewed nonwriters, 15 unclassified.


## Winterhaven janitor and consumable transformations

Fourteen coordinates grouped as pickup, well creation, residual disposal and
three random item transformations. Consumables are destroyed before replacement
allocation; janitor donation/unequip helpers remain semantic coverage concerns.
All Winterhaven raw candidates mapped. Remaining raw candidate is an extern
ship money-helper declaration unsupported by current exclusion syntax.
No runtime changes or tests; release readiness remains false.
Draft validation passed 13 fixtures: 851 routes, 2756 raw matches, 2698 unique coordinates, 2654 mapped, 43 reviewed nonwriters, 1 unclassified.


## Raw scanner classification complete

All 2698 unique coordinates now classified: 2654 mapped across 851 routes and
44 reviewed declarations. Added optional extern declaration recognition with
a regression proving an extern function body remains ineligible for exclusion.
39 contract tests and 13 draft fixtures pass. census_complete remains false
pending semantic indirect-writer review and acceptance metadata reconciliation;
zero raw matches is not issue closure or runtime qualification.


## Post-census acceptance reconciliation

Live #475 scope rechecked after commit 157c707f6. Raw classification is complete,
but acceptance remains open. Current inventory has 851 named routes and 726
empty test_candidates arrays; candidate paths themselves are not executed
coverage. Source/destination descriptions are embedded in prose rather than
dedicated fields. Named integration ownership must be checked against those
classifications instead of trusting inherited owner metadata.

Remaining bounded #475 work:
- Verify source baseline against current master and review indirect/aliased
  mutations (money-bearing object values, materialization and helper callers).
- Reconcile each economic route source/destination, mode, slice and test link;
  distinguish pure construction/inspection from integration work.
- Review/freeze registry and fixtures, run strict census and contract checks,
  and refresh issue-level delivery status with explicit unsupported routes.

Do not gate #475 on later runtime integration. No gameplay changes, new PRs
or issue-closure claims follow from this reconciliation.


## Current-master source drift check

On 2026-09-21 canonical Community-Duris/Duris master was
65d9d2ceb4688d7db4b702c7c86861d41ff870cd, fetched by explicit repository URL.
Reviewed branch head was ada1a1e361203e37a7129a499339fa746ef2812a; last source
change was b6805ca78738ff80968689c4589b6c1219372798. Shared merge base was
48c0aedd8e094eee37285111e46e735e4cf12320.

`git diff HEAD...FETCH_HEAD -- src` and its --stat variant were empty.
Upstream-only commits through this master concern world data; no upstream
source mutation changes need additional mapping at this checked head. A direct
HEAD-to-master diff also includes this branch unfinished accounting work and
must not be mistaken for upstream drift. This check does not merge upstream,
prove world-data economic equivalence, or resolve semantic scanner blind spots.

## Single-item decoder scanner coverage (2026-09-21)

Added `read_one_object` to lifecycle scanning. Seven auction callers map to existing claim-publication, legacy-pickup, and temporary catalog routes; the decoder definition maps to retained-item reconstruction and its prototype is a hashed declaration exclusion. No additional gameplay writer or runtime change is introduced. A scanner regression checks that the call is detected while comment/string lookalikes are ignored. Validation: 40 tests, 13 fixtures, 2,765 raw matches, zero unclassified coordinates, and `git diff --check` pass. Indirect coin payload review and acceptance metadata remain unfinished; census/release flags remain false.

## Coin-pile indirect writes (2026-09-21)

Reviewed `add_coins` denomination addition (`handler.c:6263`), `create_money` prototype clearing (`handler.c:6395`), committed `std::copy` publication (`actobj.c:4001`), and legacy NPC pickup. These are already classified routes; added explicit source/destination fields for their four entries. Linked the existing coin-custody test to committed publication, with its structural-only publication coverage limitation retained. The similarly named `pile->value[CORPSE_LEVEL]` writes in necromancy and corpse processing are corpse metadata, not coin denominations. Generic template copying (`db.c:3058`) initializes provisional object data and does not alone prove economic admission. This bounded review does not certify every generic value-field write; source/destination metadata and remaining semantic review are still incomplete. No gameplay change or additional route was needed.

## Required census metadata gate (2026-09-21)

Completed census validation now rejects routes without explicit nonblank source/destination strings or executable test candidate links. Draft validation still permits incomplete metadata; backend enforcement remains a separate release gate. Regression tests cover absent/blank/non-string endpoints, empty test links, and permitted draft state. All 41 contract tests and 13 fixtures pass; source counts are unchanged. Existing file links still require relevance review and actual execution before claiming coverage.

## Auction endpoint and test metadata (2026-09-21)

All 25 auction routes now have explicit source/destination classifications, including command preparation, live projections, legacy definitions and temporary catalog copies. Linked the existing SQL journey harness to seven SQL helper routes and the codec/publication tests to their corresponding routes, retaining partial-coverage limits. Moved `test_auction_finalize_claim.py` from current item claim to legacy finalization because its assertions target the legacy SQL finalization guard. There are now 715 routes without test candidates. Draft contract validation (13 fixtures) and diff checks pass; no native auction suite was run for this metadata change and no backend status advanced.

## Recovery endpoint metadata (2026-09-21)

Added explicit source/destination classifications for all 23 recovery routes, preserving identity gaps, temporary staging and source-retirement distinctions. Test review rejected superficial matches: `test_copyover_custody.py` explicitly has no NPCs, `test_shopkeeper_copyover_compat.py` checks format/version gates, and `test_player_load_items.py` targets the newer player-load path rather than legacy `sql_load_player_items`. These were not assigned as coverage for legacy NPC/item restoration. Missing test candidates remain 715; metadata is still incomplete overall. Draft validation passes with unchanged source counts; no runtime behavior changed.

## Shop endpoint and coverage correction (2026-09-21)

Added explicit endpoints for all 20 shop routes. Replaced six overly broad payload-builder test links with the relevant live-route structural test, removed four unrelated links for refund/death/payment/dormant barter, and linked the transaction harness to submission/publication. Missing test candidates rise to 718 because misleading links were removed. `test_shop_trade_live_route.py` passes; this is source-structure validation, not native callback qualification. Draft accounting validation and diff checks pass; backend states remain unchanged.

## Collector endpoint metadata (2026-09-21)

Completed explicit endpoints for 12 collector routes. Linked successful live detachment/child promotion to its actual preparation harness, SQL custody/evidence/purchase helpers to the repository journey, and submission to the transaction harness. Corrected cancellation linkage to repository cases that exercise unchanged custody and quarantine. The bank parsing buffer remains an inspection-only candidate without invented mutation coverage. Missing test candidates: 713. Existing harnesses were inspected, not executed in this metadata pass; draft contract validation and diff checks pass without backend qualification.

## Currency endpoints and validation limit (2026-09-21)

All 31 currency routes now have explicit endpoints (29 completed here, two retained from prior coin review). Removed five repository-only test links from utility helpers and live bank projections; linked command-builder structural checks. Missing candidates: 717. Accounting draft validation and diff checks pass. Currency transaction contract suite: 9/10 pass; operator-script executable-bit assertion fails under Windows `stat().st_mode`. Git index verifies all three checked scripts are mode 100755. This records a platform validation limit, not a successful full suite; no test or script behavior was changed.

## Coin transfer endpoint metadata (2026-09-21)

Added explicit endpoints for 14 remaining core coin-transfer routes; the two previously reviewed publication/NPC-pickup routes retain their endpoints. Linked five relevant source-structure checks for debit dispatch, floor drop, fallback give, pile planning and rejected provisional cleanup. `test_coin_command_transaction_contract.py` and draft accounting validation pass. These checks establish routing/order structure, not end-to-end native settlement or restart safety. Missing test candidates: 712. Game-specific reward/payment coin routes remain separate pending metadata work.

## Gameplay coin endpoints (2026-09-21)

Added explicit endpoints for the remaining 52 `coin.*` reward, service, ship, guild, theft and recovery routes from their reviewed classifications. Existing ordering/refund/replay limitations remain intact; disabled whole-ship sale remains unreachable. Across the inventory, 179/851 routes now have explicit source/destination fields and 672 lack them; 712 lack test candidates. Draft validator (13 fixtures) and diff checks pass. This metadata completion is not runtime integration, payment atomicity, or test qualification.

## Core item endpoint and test correction (2026-09-21)

Added endpoints for 20 core item routes, distinguishing transfer, staging, reconstruction, decay and destruction. Replaced unrelated shop/kingdom links on four creation-queue routes with actual reconciliation/batch source contracts; removed unrelated links from core live publication and corpse batch projection. Both creation contract scripts pass, along with draft accounting validation and diff checks. There are 199 explicit endpoint routes, 652 missing endpoints and 714 missing test candidates. Structural checks do not establish native publication/restart qualification.

## Item command endpoints (2026-09-21)

Added 25 endpoints for pickup/drop/give, bulk/container movement, wear/remove, consumption and forage routes. Linked the actual empty publication/planning harness to two routes (inspected, not executed here) and bulk drop/put source contracts to two submission routes. Bulk source contracts and draft accounting validation pass, as does diff checking. Explicit endpoints: 224; missing endpoints: 627; missing test candidates: 710. Source contract success does not qualify durable execution or all failure paths.

## Original requirement and ID validator recheck (2026-09-21)

Re-read live #475: executable test links are required, but one bespoke test per route is not. Shared tests are valid only with demonstrated relevance; inventory counts are not implementation-task counts. Fixed the explicit missing-ID validation requirement: named writer/fixture/account/reason IDs now reject absent, blank and non-string values before duplicate detection. Regression coverage includes malformed IDs and registry integration. All 43 contract tests, 13 fixtures and diff checks pass. Endpoint/test gaps remain 627/710; no runtime or closure claim.

## Currency integration ownership correction (2026-09-21)

Live #481 scope explicitly owns quest/NPC rewards, starter grants, services, locker identification, ship/cargo/insurance/guild costs and refunds. Corrected 45 `coin.*` routes incorrectly inherited from item-history #482: 39 to #481, three existing-value transfers (ship coffer and guild deposit/withdraw) to #480, two NPC initialization/reset routes to #486, and fresh-character zero baseline to #479. Each entry retains one matching owner label. This fixes responsibility metadata without changing gameplay or declaring those integrations complete. Draft validation and diff checks pass; endpoint/test gaps remain 627/710.

## Death and lifecycle endpoints (2026-09-21)

Added explicit endpoints for all seven death and 15 lifecycle routes, preserving committed-result projection, retained-custody unloading, tombstone-only visibility, uncertain deletion acknowledgement, and disabled destructive policy distinctions. No production action or gameplay change. Explicit endpoints: 246; missing endpoints: 605; test-link gaps remain 710. Draft contract validation and diff checks pass; this does not qualify lifecycle backend behavior.

## Storage and locker endpoints (2026-09-21)

Added endpoints for 20 storage helpers and nine locker routes. Retained distinctions include bank-row ensure versus deposit, recovery projection versus issuance, savepoint release versus commit, quarantine versus destruction, and synthetic chest teardown versus contained-asset retirement. Explicit endpoints: 275; missing endpoints: 576; test-link gaps remain 710. Draft contract validation and diff checks pass. No backend or payment behavior changed or qualified.

## Import, backup, restore and mixed economy endpoints (2026-09-22)

Added 30 endpoint classifications spanning import/openings, backup/restore, utility-only representations and mixed quest/service routes. Preserved disabled pet restoration, nonrefunded rent failure, post-save hometown charge and ambiguous recovery publication limits. Explicit endpoints: 305; missing endpoints: 546; test-link gaps: 710. Draft validation and diff checks pass; no gameplay, production restoration or runtime qualification performed.

## Crafting, enhancement and mining endpoints (2026-09-22)

Added 37 endpoint classifications preserving real inputs versus probes, provisional grants, pouch substitutions, intended random loss, separate output submissions, same-item enhancement and delayed harvest. Explicit endpoints: 342; missing endpoints: 509; test-link gaps remain 710. Draft validation and diff checks pass. No prices, distributions, input-loss behavior or backend qualification changed.

## Alchemy endpoints (2026-09-22)

Added 17 alchemy endpoints distinguishing preview copies, physical/generated materials, intended craft failure, replacement identities and paid delayed enchantment. Existing loop consumption and partial-allocation limits remain visible. Explicit endpoints: 359; missing endpoints: 492; test-link gaps remain 710. Draft validation and diff checks pass; gameplay economics and runtime qualification are unchanged.

## Artifact endpoints (2026-09-22)

Added 17 artifact endpoints separating registry/binding/timer changes, temporary inspection, duplicate cleanup, reconstruction, replacement and expiry. Retained missing-UID recovery, partial cleanup and separate-save limits. Explicit endpoints: 376; missing endpoints: 475; test-link gaps remain 710. Draft validation and diff checks pass; no artifact mechanics or runtime qualification changed.

## Wizard and inspection endpoints (2026-09-22)

Added endpoints for 12 wizard and 11 inspection routes. Actual grants/purges/storage graph changes remain distinct from report prototypes and disabled commands. Sequential storage removal and backend differences remain visible. Explicit endpoints: 399; missing endpoints: 452; test-link gaps remain 710. Draft validation and diff checks pass; no administrator behavior or native qualification changed.

## Service and entitlement endpoints (2026-09-22)

Added 28 staff, boon, kingdom, reward, guildhall and service endpoints. Preserved entitlement versus live instance, non-coin realm resources, temporary fixture lifecycle, disabled boon code, and unchecked/post-payment service limits. Explicit endpoints: 427; missing endpoints: 424; test-link gaps remain 710. Draft validation and diff checks pass; no reward/service behavior or backend qualification changed.

## Helper, building and remaining service endpoints (2026-09-22)

Added 29 helper/building/ship/gambling/quest/spell endpoints, preserving transfers versus issuance, independent portal allocations, unreachable legacy hull debit and acceptance-versus-commit limits. Explicit endpoints: 456; missing endpoints: 395; test-link gaps remain 710. Draft validation and diff checks pass; no gameplay or runtime qualification changed.

## Scripted economy endpoints (2026-09-22)

Added 36 special, nexus, script and Winterhaven endpoints. Preserved independent random outputs, same-owner equipment swaps, paired sinks, partial allocations and the gem-factory null-return limitation without changing mechanics. Explicit endpoints: 492; missing endpoints: 359; test-link gaps remain 710. Draft validation and diff checks pass; no native execution or gameplay qualification claimed.

## World admission endpoints (2026-09-22)

Added 25 world endpoints covering reset/prototype preparation, reconstruction, random loot, death rewards and kingdom nodes. Preserved same-owner moves, rejected provisional copies, disabled potion code and charge-before-partial-yield behavior. Explicit endpoints: 517; missing endpoints: 334; test-link gaps remain 710. Draft validation and diff checks pass; no runtime admission or backend qualification claimed.

## Remaining world endpoints (2026-09-22)

Added 30 remaining world endpoints, including NPC/ship loot, keeper reconciliation, ferries, death outputs, nodes and spell fixtures. Preserved reuse, partial allocation, indirect carried holdings and unresolved death-wallet conversion. All world routes now have explicit endpoints. Inventory total: 547 explicit, 304 missing; test-link gaps remain 710. Draft validation and diff checks pass; no runtime qualification claimed.

## Consumption and NPC transformation endpoints (2026-09-22)

Added 25 item endpoints covering consumption, fallback theft, donations, same-owner equipment moves, NPC-form transfers and death rewards. Preserved normal-death continuation, separate existing-item movement/new output, and known allocation limits. Explicit endpoints: 572; missing endpoints: 279; test-link gaps remain 710. Draft validation and diff checks pass; gameplay and runtime qualification unchanged.

## Scripted item movement and consumption endpoints (2026-09-22)

Added 25 item endpoints for transformations, offerings, charge exhaustion, ferry/pool relocation and random outputs. Preserved same-object transfers, parent-only digestion, sequential output creation and detached chest state. Explicit endpoints: 597; missing endpoints: 254; test-link gaps remain 710. Draft validation and diff checks pass; no gameplay or runtime qualification changed.

## Item effect and spell endpoints (2026-09-22)

Added 25 endpoints for ward/charge/periodic retirement, alignment rejection, same-owner equipment changes, portals, components and spell creation. Explicit endpoints: 622; missing endpoints: 229; test-link gaps remain 710. Draft validation and diff checks pass; no effect policy, randomness or runtime qualification changed.

## Spell materialization and transfer endpoints (2026-09-22)

Added 25 endpoints for spell objects, soulbinding, portals, components and resurrection publication. Retained provisional allocation, captured-context submission, duplicate cleanup and same-object relocation distinctions. Explicit endpoints: 647; missing endpoints: 204; test-link gaps remain 710. Draft validation and diff checks pass; no spell economics or runtime qualification changed.

## Corpse, disabled spell and ammunition endpoints (2026-09-22)

Added 25 endpoints preserving disabled branches, existing-child transfers, saved-corpse copies, partial compaction and ammunition quantity conversion. Explicit endpoints: 672; missing endpoints: 179; test-link gaps remain 710. Draft validation and diff checks pass; no corpse/ammunition behavior or runtime qualification changed.

## Combat, encounter and flag endpoints (2026-09-22)

Added 25 item endpoints for projectiles, random encounter outputs, NPC equipment selection, corpse handling and CTF flags. Preserved provisional leaks, blocked-slot behavior, same-owner moves and separate corpse saves. Explicit endpoints: 697; missing endpoints: 154; test-link gaps remain 710. Draft validation and diff checks pass; no runtime qualification changed.

## Command, corpse and transport endpoints (2026-09-22)

Added 25 item endpoints including CTF carrier state, breakage, potions, combat disarm, corpse transactions and transport tickets. Preserved compound-service and object-lifetime concerns rather than asserting completed behavior. Explicit endpoints: 722; missing endpoints: 129; test-link gaps remain 710. Draft validation and diff checks pass; no gameplay or runtime qualification changed.

### 2026-09-22: condition, summon and spell endpoints

Specified source/destination metadata for 25 existing routes after checking their source sites: condition/scrap conversion, scripted web/fire and gems, spell drops, summoned replacements, consumed materials, previews and recipe grants. Temporary previews and provisional allocations remain distinct from admission; detached smoke is not claimed retired, and wallet projection is not claimed a debit. No gameplay, ownership, test links or backend qualification changed. Explicit endpoints now cover 747 of 851 routes, leaving 104; 710 routes still lack test candidates. Draft contract validation and whitespace validation passed; these checks do not establish runtime acceptance.

### 2026-09-22: starter, ship and material endpoints

Added verified source/destination descriptions for 30 existing starter-kit, ship, reward, forced-drop, material-report and disabled breath routes. Provisional cleanup and report allocations are not admission; ship restoration intent remains caller-dependent; disabled blocks remain dormant. Explicit endpoints now cover 777/851 routes, leaving 74. The 710 test-candidate gaps and all backend qualification statuses are unchanged. Draft validation passed with 13 fixtures and the unchanged census; whitespace validation passed. No gameplay changes.

### 2026-09-22: consumption, class and scripted endpoints

Specified 35 source-verified endpoint pairs for smoking, slip/disguise, innate objects, tickets, mail, spell devices and scripted transfers. Temporary restored copies/probes remain distinct from retirement; dormant branches remain labeled; compound effects and save attempts do not imply durable completion. Endpoint coverage is 812/851, leaving 39. Test-candidate gaps remain 710 and backend qualification is unchanged. Draft validator (13 fixtures, unchanged census) and whitespace checks passed. No gameplay changes.

### 2026-09-22: endpoint description pass complete

Added the final 39 source/destination pairs, checking scripted gifts/retirement, factories, NPC theft, ship cargo conversion and recovery source sites. All 851 inventory routes now have explicit endpoints. Restoration and temporary cleanup remain distinct from issuance/destruction; recovered NPC reset replenishment is new stock. This completes only endpoint metadata: 710 routes still lack test candidates, ownership reconciliation and other foundation acceptance work remain, census_complete stays false, and no runtime/backend qualification changed. Draft validation passed (13 fixtures; unchanged 2765 matches, 2707 coordinates, 2662 mapped, 45 nonwriters, zero unclassified), as did whitespace validation.

### 2026-09-22: relevant conjuration and forced-drop test links

Inspected assertions and linked eight routes to existing conjuration, forced-weapon-drop and Wind Blade tests; three routes already had candidates, so empty test lists decreased from 710 to 705. The conjuration source-contract test passed and supports submission ordering, UID checks and deferred replacement retirement, not backend durability. Forced-drop harness exercises actual implementation with stubbed storage, while Wind Blade extracts its callback into a harness; those native harnesses were inspected but not run in this pass. Draft validator and whitespace checks passed. No backend evidence or release status changed.

### 2026-09-22: starter-kit test mapping

Linked chaos_kit_item_builder to the inspected extracted-production preparation/placement harness and generated-data/source contract, and chaos_kit_batch_builder to the source contract verifying single batch submission after preparation failure checks. The generated-kit contract passed against current data; the native harness was inspected, not executed. It stubs loading/eligibility and does not establish persistence. No test was assigned to provisional forest cleanup solely because its destructor is included in the harness. Empty test-candidate lists now total 703. Draft validation and whitespace checks passed; runtime/backend status remains unchanged.

### 2026-09-22: recovery test mapping

Linked player graph materialization and staging cleanup to test_player_load_items.py: its compiled production-source harness asserts restored UIDs/topology and failed-allocation/invalid-graph extraction counts. Linked recovered-NPC reset replenishment to test_world_recovery_npc_items.py, whose stubbed-world harness asserts stock/equipment counts and repeat-call population stability. These harnesses were inspected, not executed in this pass; neither proves backend durability. Did not assign transaction-order source assertions as proof of world rollback cleanup. Empty test lists now total 701. Draft validation and whitespace checks passed; qualification status unchanged.

### 2026-09-22: death and corpse integration ownership

Re-read live issues #482 and #486. #482 explicitly owns generic custody/topology and excludes death/world orchestration; #486 owns specialized combat/corpse writers, loot/restore, no-corpse outcomes and recovery projections. Reassigned 19 death/corpse orchestration routes from #482 to #486, including corpse transfer submission/completion, shell/NPC contents, disputed wallet snapshots, saved corpse handling and specialized death drops/releases. Generic transformation/carving and trusted-nowhere movement were not reassigned merely for matching a keyword. This changes integration responsibility only; generic #482 machinery remains a dependency. Endpoints, test candidates (701 empty), support and backend evidence are unchanged. Draft validation and whitespace checks passed. Sources: https://github.com/Community-Duris/Duris/issues/482 and https://github.com/Community-Duris/Duris/issues/486.

### 2026-09-22: generic reward and service ownership

Re-read live #481 (coin issuance, expenses and authorized adjustments). Corrected seven generic money-bearing routes from #482 to #481: trusted junk reward, dump reward, stable rent/redemption, quest coin reward, transport ticket purchase and ferry ticket purchase. Compound item mechanics still depend on #482; item-only grants/factories were not moved solely because their names contain reward or starter. This avoids assigning currency counterpart policy to the generic item slice. Endpoint/test/backend fields remain unchanged (701 empty test lists). Draft validation and whitespace checks passed. Source: https://github.com/Community-Duris/Duris/issues/481.

### 2026-09-22: cross-player handoff test mapping

Linked durable Slip and Soulbind submission/publication to the inspected issue-549 contract; all seven structural tests passed. Callback routes additionally link the extracted-function runtime harness covering rejection, mismatched UID, successful publication and repeated effects; that harness was inspected but not executed here. Legacy Slip and failed-skill drops were not credited with durable-branch coverage. Empty test lists total 698. Draft inventory and whitespace checks passed. No backend qualification changed.

### 2026-09-22: conjuration submission and retirement coverage

Mapped five Soulbind/conjured-weapon preparation/submission routes to the previously inspected issue-550 source contract. For the three spell allocation routes this checks delegation and absence of direct publication/damage, not every generated payload field. Mapped Soulbind duplicate cleanup and summoned replacement retirement to the extracted-production callback harness, whose assertions retain prior items on rejection and retire matching prior items after successful replacement. The native harness was inspected, not run; no storage durability claim. Empty test lists total 692. Draft inventory and whitespace checks passed.

### 2026-09-22: device consumption and wonder output test mapping

Linked deferred scroll cleanup to test_device_actions_runtime.py, whose assertions check cleared slots, delayed pulse extraction, cancellation and one extraction. Linked wonder gem admission to test_wonder_actions_runtime.py, which includes production wonder_actions.c and asserts deterministic gem count/types, placements, roll count and interruption during output creation. Both harnesses use synthetic runtime boundaries and were inspected, not run in this pass; backend durability remains unverified. Empty test lists total 691. Draft validator and whitespace checks passed.

### 2026-09-22: native harness verification at e903dd860

Windows execution of test_forced_weapon_drop.py could not launch g++; Docker daemon was unavailable. The existing WSL distribution duris-474-build provided g++ and python3 and successfully ran the current mounted worktree. All five commands passed using `wsl -d duris-474-build -- python3 tests/async/<script>`:

- test_forced_weapon_drop.py
- test_issue_549_cross_player_transfer_runtime.py (ASan/UBSan callbacks)
- test_issue_550_conjuration_runtime.py (ASan/UBSan callbacks)
- test_world_recovery_npc_items.py
- test_player_load_items.py

These results supersede earlier inspected-only status for these exact harnesses. They exercise synthetic/stubbed native boundaries, not MySQL/MariaDB or flat-file durability journeys. No backend support status is promoted. Source was unchanged during these runs; 691 empty test-candidate lists remain.

### 2026-09-22: remaining mapped native harnesses

At 80fff6fc5, ran four additional mapped harnesses with `wsl -d duris-474-build -- python3 tests/async/<script>`; all passed:

- test_chaos_kit_runtime.py: preparation, role filtering and placement.
- test_wind_blade_creation_runtime.py: completion callback and detached-object guard.
- test_device_actions_runtime.py: wrappers, charge/scroll consumption, identities, area lifetime and cancellation.
- test_wonder_actions_runtime.py: twenty native outcomes, captured random choices, legality, consumption and lifetime.

These supersede earlier inspected-only notes for these harnesses. Tests use synthetic runtime dependencies; they do not establish durable accounting or backend parity. Inventory counts and support flags remain unchanged. Whitespace validation passed.

### 2026-09-22: corpse batch and in-flight handoff verification

Mapped corpse_transfer_completion to test_corpse_creation_batch.py and corpse_shell_and_npc_contents to test_corpse_handoff_inflight_contract.py. Both passed under WSL at unchanged source 0b7426846. The native batch harness covers nested topology, rejection, pending coin conflicts, retention/replay and the extracted completion callback; it does not execute submit_next_corpse_item directly. Nine source checks cover deferred initial handoff, pending wallet fencing and holy-weapon placement before lethal effects. Shell coverage is limited to handoff ordering, not NPC shell creation. Empty test lists total 689. These are not durable backend journeys. Draft validator and whitespace checks passed.

### 2026-09-22: death wallet retry verification

Both test_death_wallet_retry.py and test_death_item_custody_contract.py passed under WSL. The former executes extracted retry logic with stubbed saves/disposition and checks pending conversion, failed publication/disposition/terminal-save retention. The latter checks source ordering for wallet snapshots and batched handoff plus native dispute tracking/event-admission fallback. Linked disputed wallet snapshot and corpse submission routes to its structural assertions; this does not prove actual database or journal durability despite stronger wording in test output. Empty test lists total 687. Draft validator and whitespace checks passed.

### 2026-09-22: account reward coverage verification

Existing mappings for reward.container_contents, reward.summon, reward.revocation_duplicates, reward.dismiss and reward.death_container were inspected against test_account_reward_container_contract.py. That source contract and test_account_reward_exact_contract.py both passed under WSL. Checks cover nonempty dismissal refusal, child promotion before extraction, corpse-hook ordering, exact-template markers and lifecycle configuration; these are source contracts, not backend journeys. No duplicate mappings were added. A separate training-dummy contract attempt via pytest could not start because the WSL Python environment lacks pytest; no pass or new cleanup coverage was claimed. Test-list count remains 687. Whitespace validation passed.

### 2026-09-22: training-dummy source checks available on Windows

The bundled Windows Python already provides pytest. Running `python -m pytest -q tests/async/test_training_dummy_contract.py` with that interpreter passed all 15 checks at 1fdfcf386, resolving the previous missing-pytest execution limitation without installation. Existing item.training_dummy_cleanup and coin.training_dummy_zero_wallet mappings were retained. Item checks assert equipment/inventory loop presence and admission refusals; wallet checks assert four zero assignments. They are structural checks, not execution of extraction or a backend journey; run_training_dummy_player_journey.py was not run here. Inventory counts remain unchanged. Whitespace validation passed.

### 2026-09-22: crafting coverage gap confirmed

Inspected test_crafting_enhancement_regressions.py, test_crafting_recipe_persistence_contract.py, test_crafting_module_contract.py and test_salvage_module_contract.py. They assert pricing/type tokens, recipe-provider selection, dispatch/configuration and salvage eligibility/module placement. None of those assertions exercises the tradeskill input-consumption/output-publication boundary. They therefore were not assigned to the eleven uncovered tradeskill routes merely because filenames match. Remaining routes: crafting.smith, crafting.refine, crafting.fishing, crafting.parchment, crafting.epic_store, crafting.tradeskill_grant, crafting.legacy_parchment_learning, crafting.legacy_forge_allocation, crafting.bandage, crafting.learn_recipe and crafting.refine_message_probe.

The next useful crafting coverage is a shared native grant/refusal harness around grant_tradeskill_item, followed by caller-specific input/output ordering checks where actual consumption occurs. Factory/probe tests must distinguish provisional cleanup from admitted-item retirement. No new per-route test requirement is inferred from this list. Existing test-link count stays 687 empty; no execution pass or backend qualification is claimed from this inspection.

### 2026-09-22: tradeskill grant native boundary test

Added test_tradeskill_grant_runtime.py, compiling the extracted production helper with ASan/UBSan. It passed under WSL: accepted submission preserves output and binds recipient/initiator; refusal releases only its provisional object once with FALSE; null output neither submits nor extracts. Linked only crafting.tradeskill_grant, reducing empty test lists to 686. Submission is stubbed; caller material consumption and backend durability remain untested by this harness. Draft validator and whitespace checks passed. No production behavior changed.

### 2026-09-22: tradeskill caller failure boundaries

Traced all three grant_tradeskill_item callers. Smith debits money and extracts ore before submission; refusal only cleans the new output. Refinement consumes inputs before its random result, so later output refusal is distinct from intended random failure. Fishing mutates its affect and grants experience before calling the helper and ignores its result. Updated these route authority boundaries to state the missing compound-operation acceptance cases. The helper test is deliberately not assigned as proof of caller rollback. This identifies integration work under the existing objective; no gameplay changes or new standalone issue/PR were made. Empty test lists remain 686. Draft validation and whitespace checks passed.

### 2026-09-22: canonical master drift recheck

Fetched canonical Community-Duris/Duris master without merging. Local head e57c728e51c9ae253845c803a9df8baa003e2e40 compared with master feed2939125c3e9a955b92adbf627a2ad568586e, merge base 48c0aedd8e094eee37285111e46e735e4cf12320. `git diff --stat HEAD...FETCH_HEAD -- src` was empty. Upstream-only changes are .gitignore, Earth-plane world files and their sector test; none introduces new source callsites for this inventory. This is a source-drift check, not world-data or final merge qualification. The worktree was clean before the audit.

The explicit `validate_economy_accounting.py --census` gate still fails with `writer census not complete`, as expected while test mapping and other acceptance work remain. No census/release flags were changed to bypass it. The 686 empty test lists remain unresolved.

### 2026-09-22: shared quest grant boundary coverage

Extended test_tradeskill_grant_runtime.py to compile both production grant helpers independently. Both passed ASan/UBSan acceptance, refused provisional cleanup and null-output cases under WSL. Quest null output correctly returns silently while tradeskill reports refusal. Linked item.world_quest_reward_grant; coverage is its grant helper, not reward selection or callers continuing quest reset/other rewards. Empty test lists now 685. Submission remains stubbed and does not prove commit/publication or backend durability. Draft validation and whitespace checks passed.

### 2026-09-22: replay publication retention contract clarified

Read enqueue_replayed and coordinator finalization alongside test_critical_command_coordinator.py. Replayed operation_state does not restore retain_until_publication; finalization checkpoints nonretained applied/already-applied/terminal results. The existing restart_publication test explicitly requires this: shutdown with a publication-held command, replay already_applied, then assert completed, no item fence and zero journal records. Therefore this is an intentional process-local callback contract, not evidence that assigning the bool alone is a safe fix.

For #479 restart-safe activation, determine which durable recovery projection or acknowledgement replaces the vanished callback before changing this behavior. Retaining all replayed operations would strand routes with no reconstructed acknowledger; releasing all cannot alone establish native save/publication acknowledgement. The #476/#479 interface decision and a restart journey spanning real projection reconstruction remain required. No coordinator code or existing tests changed in this inspection; no runtime pass claimed. Source anchors: src/persistence/critical_command_coordinator.c enqueue_replayed/finalization and tests/async/test_critical_command_coordinator.py restart_publication.

### 2026-09-22: boot observer recovery ownership

The actual coordinator initialization in src/net/comm.c registers only player_death_restitution_runtime_restore_replayed_command as replay observer. Its implementation in src/player/player_death_restitution_adapter.c returns true immediately for every non-restitution command. For restitution it decodes the original command, reserves status/submission slots and reconstructs runtime state through the specialized callback adapter. Thus this observer does not rebuild generic currency/item live-publication state. The player save pipeline is initialized before the coordinator; that ordering alone does not acknowledge a replayed economic projection.

The next implementation boundary is a domain-aware recovery observer/dispatcher that preserves the existing restitution adapter and establishes which economic routes need a reconstructed publication obligation before workers start. Persisted operation identity and projection/save evidence must drive that obligation; old actor pointers or repeating wallet deltas cannot. This is a design direction, not implemented support or proof that all replay routes require holds. Existing snapshot reconstruction paths still need checking before choosing per-domain policy. No source or support flag changed.

### 2026-09-22: SQL wallet reconstruction boundary

SQL wallet writes in critical_command_repository.c update all four player_data denomination columns and wallet_revision with a prior-revision predicate. player_load_repository.c reads those denominations and revision; player_load_materialize.c assigns them to live cash and wallet_revision. Therefore a fresh SQL login has an authoritative reconstruction path and must not reapply an old delta merely to replace a vanished callback. This source trace does not establish startup race safety, domain side-effect recovery or flat-file parity.

No direct critical_command_coordinator_is_fenced call was found in src/player, src/account or src/net. That scoped absence is not proof that all admission lacks indirect fencing. Next check is the boot-drain/admission ordering and fresh-load race with replay, followed by a native journey; the restitution-only observer alone does not settle it. No runtime or backend completion claimed.

### 2026-09-22: startup replay/login ordering gap

Verified comm.c enters game_loop after coordinator initialization without a startup drain; its visible coordinator drain is shutdown handling. The password login branch in nanny.c submits a player-load request and can fall back to synchronous execution. player_load_pipeline_login_admit currently returns only pid > 0, and repository-wide references show no callers of that helper or restitution login_admit beyond definitions/declarations. Consequently the inspected entry path does not itself establish exclusion between fresh loads and boot replay.

Required regression: hold a replay apply before its wallet write, start a fresh load that captures the old revision, release replay to commit/checkpoint, then attempt live materialization. The old snapshot must not enter gameplay or overwrite the newer authoritative wallet. Merely checking an entity fence at materialization is insufficient if replay finished between snapshot read and that check. Implement a bounded startup admission barrier or revision-aware load invalidation only after accounting for synchronous fallback and copyover; no global blocking drain or unbounded wait is assumed safe. This is a source-supported race candidate pending executable reproduction, not a demonstrated production incident.

### 2026-09-22: asynchronous boot replay reproduced in native harness

Extended test_critical_command_coordinator.py to block a journal-replayed command inside its apply callback. After init returns and the worker has entered apply, assertions require zero completed operations, the entity fence still held and one journal record. Releasing the callback permits existing completion/checkpoint assertions. The full coordinator test passed under WSL. This deterministically proves init success is not replay completion and verifies retained fencing during the window. It does not yet execute player loading or reproduce a stale wallet publication; that cross-component regression remains necessary. No coordinator implementation or inventory support flags changed. Whitespace validation passed.

### 2026-09-22: fresh materialization does not compare wallet revision

Inspected nanny_player_load_complete: it rejects descriptor/request-ID mismatch, retries selected load failures synchronously, and checks player_save_pipeline_save_admitted before allocating a fresh character. That predicate checks target save/login fences, not the critical-command wallet revision; refusal marks the result degraded rather than re-reading wallet state. player_load_materialize assigns wallet denominations/revision directly from the supplied result. Its player_revision_hydrate call concerns snapshot/save revision, not a fresh authoritative wallet comparison.

Existing test_player_load_pipeline.py compiles the queue and observability with a synthetic repository; materialization/nanny are source checks, so it cannot alone reproduce this race. The reproduction must bridge coordinator replay and actual fresh-load materialization, including account/copyover callers identified by that test, or validate a shared admission boundary used by all those callers. No new passing test or confirmed loss is asserted by this trace.

### 2026-09-22: copyover and world recovery join the replay boundary

Verified coordinator initialization precedes game_loop. Inside game_loop, after descriptor-pool creation, copyover_recover runs before Redis world recovery and before ordinary ticks. copyover_load_player uses player_load_pipeline_wait and falls back to execute_sync before materialization. Both execute after coordinator workers have started, without a replay-completion barrier in this inspected sequence. Account loading also calls the shared synchronous load/materialization path.

A guard limited to interactive login would therefore miss copyover and potentially world restoration. The recovery boundary must be established before copyover/Redis reconstruction or shared by every materialization route, while allowing the existing restitution observer to rebuild its offline state. Preserve retained recovery sources on failure; do not turn a timeout into permission to continue or consume recovery input. This narrows the implementation boundary for the pending regression and avoids adding a misleading login-only patch. No behavior changed or full race reproduced.

### 2026-09-22: bounded drain preserves held replay

Extended the held-replay coordinator test with drain(0): it returns false while retaining the entity fence and journal record, then normal completion proceeds after release. The full native coordinator suite passed under WSL. Drain checks queued/inflight/blocked/publication/admission work and dispatches completions through its observer. The installed observer invokes all domain completion handlers, including restitution, and locker_identify_pulse.

A startup caller must separately require successful coordinator initialization: an empty/uninitialized coordinator is not proof of recovered storage. The gate also must account for observer side effects before world restoration. This test qualifies timeout preservation only; no startup gate is installed by this commit. Whitespace validation passed.

### 2026-09-22: pre-restoration completion consumers

Checked startup-sensitive drain consumers. Currency completions without a process-local pending entry are ignored. locker_identify_init creates/locks its receipt directory but does not populate request work; pulse iterates that request map rather than scanning/replaying every receipt at boot. Restitution is different: the replay observer reconstructs its submission, and runtime_complete releases its target save/login fence only for applied, already_applied or terminal_failure outcomes; ambiguous/retryable results remain fenced. This supports delivering reconstructed restitution completions before loading its target instead of suppressing all startup completion dispatch.

These checks narrow the startup gate design but do not establish every domain callback safe or establish durable recovery for all economic side effects. The existing full dispatcher remains the reference; any startup integration must preserve its domain-specific handling and separately reject failed coordinator initialization. No production behavior or acceptance flag changed.

### 2026-09-22: startup boundary includes boot_db restoration

The candidate gate immediately before game_loop is insufficient for the full recovery requirement. In comm.c, boot_db runs before load/save pipeline and critical coordinator initialization. In world/db.c, non-mini boot_db already calls restoreCorpses, restoreSavedItems, conditionally restore_shopkeepers, remembers boot shopkeepers, and restores legacy artifact projections. Copyover/Redis restoration is only the later stage. Therefore draining replay before game_loop would not establish fresh custody for objects already restored by boot_db. This supersedes the earlier assumption that the pre-game-loop boundary covers all restoration; the complete stale-object journey still needs executable reproduction.

collector_catalog_cache_refresh also starts an asynchronous authoritative read before the proposed gate. Its source request must be ordered after replay or invalidated/reloaded before publication. The player_load_pipeline header explicitly preserves valid login admission despite secondary persistence fences; do not silently replace this with per-player blanket denial.

Implementation decision: do not install the incomplete pre-game-loop gate. First separate static world initialization from durable economic restoration, identify the dependencies needed by coordinator/restitution initialization, then order replay completion before affected restoration and cache reads. Preserve copyover/Redis inputs on failed recovery. Verify cold boot, copyover, Redis and mini-mode separately; a coordinator-only test does not qualify these journeys. Automatic approval review rejected the proposed global startup stop and reduced completion dispatcher; neither change was applied. Any revised startup policy still needs that approval resolved and full-dispatcher effects accounted for.

Native ordering evidence: tests/async/test_startup_replay_load_ordering.py links the production coordinator, journal, player-load pipeline and observability, with synthetic repository callbacks. Held replay permits both asynchronous wait and synchronous fallback to return wallet 100/revision 1; drain(0) refuses readiness. After releasing replay and completing drain, the retained result is still revision 1, while fresh reads return wallet 75/revision 2. This passed under WSL duris-474-build. It proves a fence check after read completion cannot itself refresh the snapshot; it does not run actual SQL/flat-file repositories, player materialization or boot orchestration. Startup ordering remains unresolved.

### 2026-09-22: pure-module qualification refresh

At source head 623ae3803, native test_economic_accounting_types.py, test_economic_accounting_plan.py, test_economic_currency_adapter.py and test_economic_accounting_replay.py all passed in WSL duris-474-build. Plan and adapter harnesses run SQL and client-free compilation variants under ASan/UBSan; plan bytes/output match across variants. Coverage includes 13 goldens, deterministic bytes/digests, malformed inputs, bounds, randomized transfers, legacy replay and unsupported mixed-journal preservation. These are pure/module tests, not native backend transaction or startup qualification. No C/C++ production source changed in this refresh; no full server build was rerun.

The public contract now explicitly assigns domain publication/reload obligations to integration slices rather than treating the process-local coordinator hold as a blanket replay bug. This resolves the ownership question for #476, not the pending domain implementation. #475 remains incomplete, and no child issue is declared closure-ready. Delivery instructions now preserve existing component PRs and require one final PR for xander-l.

### 2026-09-22: currency candidate test links

Linked nine currency/storage routes to inspected existing tests. test_currency_transaction_contract.py passed 10 source/schema checks; test_account_bank_delta_safety.py passed its source contracts. test_currency_completion_retention.py passed 34 native ASan/UBSan scenarios in SQL/client-free compilation variants, linking the real adapter/codecs with controlled coordinator/world endpoints. Each inventory link states its structural or native scope; backend qualification remains unverified. Routes without candidates decrease from 685 to 676. Draft validator and whitespace check passed; census_complete remains false.

Coin follow-up: test_coin_custody_lifecycle.py now directly tests shared-bank destination revision advancement while preserving the source request, child operation ID and accepted timestamp; mismatched initial bank revisions and null output are refused. Its native sanitizer suite passed, as did test_coin_get_completion.py and test_coin_command_transaction_contract.py. Linked coin.command_builder and coin.destination_revision_plan to the existing lifecycle harness, with item-owner revision branch coverage explicitly still unqualified. There are 674 routes without test candidates; no backend or completeness flags changed.

The same lifecycle harness now also executes the item-owner revision branch: shared-owner from/to revisions advance together, both source-owner matching paths are covered, mismatched initial revisions refuse, and the original request plus child ID/timestamp remain intact. The full native lifecycle suite passed again under WSL. This supersedes the preceding item-owner branch coverage gap only; backend transactions and end-to-end gameplay remain unqualified.

### 2026-09-22: maximum flat-file item evidence record

Extended flatfile_accounting_store_test.cpp with 6,000 before/after item witnesses and 3,000 item events in a structurally valid plan. The private test bridge stages and commits the evidence, then reacquires authority and verifies exact plan/result bytes. One extra witness or event returns capacity and preserves the previous encoded plan. The full test_flatfile_accounting_store.py ASan/UBSan run passed, including its existing 85 syscall/process-exit fault cases. Initial test setup omitted bucket initialization; corrected before the passing run.

This qualifies maximum item evidence storage, not a native item mutation owner or compound children. Child-bearing plans remain deliberately refused until unique child-ID reservation exists. The remaining #478 requirement still includes native item/child bundles and SQL/flat-file outcome parity. No production code or backend support flag changed; no full server build was needed for this test-only change.

## Current-head census repair and publication coverage

At `69b0a47be`, draft validation exposed seven moved source coordinates and one
stale declaration location. Each coordinate had exactly one unchanged excerpt in
its original file/family; the moved declaration retained its complete original
hash. The refresh changes locations without claiming new semantic review of those
routes. The scanner now discovers bank publication submit/restore/pulse entrypoints:
six definition/call sites belong to `currency.accounted_bank_publication`, and three
header prototypes are hash-verified declarations. The route records commit versus
publication/save boundaries, #480 ownership, original-ID retention and native test
candidates. Real wallet and revisioned shared-bank projection routes also link the
native journey. No backend status or completeness flag was promoted.

Validation: 13 golden fixtures, 852 routes, 2,774 lexical hits / 2,716 unique
coordinates; 2,668 mapped and 48 declarations, none unclassified. All 45 contract
tests pass. The strict census gate remains closed; 674 routes still lack executable
test candidates, and indirect/aliased writes need semantic review.

The issue repository master was verified as `feed2939125c3e9a955b92adbf627a2ad568586e`
through `Community-Duris/Duris`. The configured `origin` is the separate
`Community-Duris/DurisMUD` repository (`4b56d8a1ec3e1d64163d31f179665100326e0320`).
Use the issue repository explicitly for master comparisons and final PR publication.
This scoped refresh is not a whole-master census review or issue closure audit.

### Quest test candidate audit

Linked four world-quest reward/fee/refund routes to inspected source-contract tests, reducing missing candidates from 674 to 670. The reward rejection test existed but was omitted from its standalone runner; it is now included and passes. Both quest scripts and all 45 accounting contract tests pass. These candidates cover selected item publication, payment ordering and refund invocation contracts, not native backend atomicity, durable refund identity or currency issuance. Each route records that limit; support and completeness flags remain unchanged. Crafting/salvage symbol-only checks were not assigned to economic output routes.

### Native currency candidate audit

Linked identify preparation and wallet-value submission to inspected native adapter tests; 668 routes now lack candidates. The queue harness initially failed to link because its controlled item-movement stub lacked the current publication callback parameter. Updated that signature and asserted that this admission path passes no publication callback. The queue harness then passed SQL and client-free ASan/UBSan variants. Completion retention passed all 34 sanitizer scenarios, the accounting contract suite passed 45 tests, and draft validation passed. These use controlled coordinator/world endpoints, not database integration; identify bank-fallback arithmetic remains unqualified by this candidate. No production source, backend support status or completeness flag changed.

### Shop candidate audit

Linked SQL shopkeeper staging and derived stock to the inspected population harness: five ASan/UBSan scenarios passed (reset, shared template, duplicate snapshot, cleanup, invalid identity). It extracts production restore/reset code with fake DB/world I/O, not native database accounting. Linked creation refund to the passing multi-buy source contract, which checks refund invocation before sequence removal but does not execute or qualify the non-atomic refund helper. Dirty-shopkeeper retry checks were not assigned to cash/barter payment routes because they test persistence retries instead of those mutations. Missing candidates decrease from 668 to 665; no support or completeness flags changed.

### Retirement candidate audit

Linked four account-deletion/owner-destruction and zone-purge routes to inspected candidates; 661 routes lack candidates. Extended the native purge fixture to cover nonempty nonartifact corpse extraction, matching the actual branch rather than its misleading comment. All four ASan/UBSan lifetime scenarios and account-deletion source contracts pass. Controlled extractors prove dispatch, not recursive durable retirement; source checks prove neither SQL execution nor retry identity. Ship-shell retirement was not linked to the account test because that test only checks the runtime deletion delegation. No gameplay or backend status changed.

### Current-master lexical comparison

Re-read live #475 and fetched Community-Duris/Duris master at `feed2939125c3e9a955b92adbf627a2ad568586e`. Scanned an isolated temporary git archive using the current validator patterns: 2,765 hits, 2,707 unique coordinates and 1,892 distinct path/family/excerpt fragments. All 1,892 fragments occur in the branch scan; no master-only fragment was found. The branch adds nine distinct publication fragments (six definition/call sites and three prototypes), already inventoried. This fragment-set comparison does not verify duplicate-site multiplicity, surrounding control flow or indirect/aliased mutations; it is lexical drift evidence, not full semantic review or census completion. The issue explicitly requires executable tests in the inventory and does not require gameplay enforcement for contract closure. Continue the remaining 661 test gaps and semantic review without treating narrow candidates as backend qualification.

### NPC buffer recovery candidate audit

Extended the singleton harness with nonzero NPC gold preservation across five production buffer save/restore cycles; existing stock-count/equipment checks remain. ASan/UBSan harness and compatibility source checks pass. Linked the two buffer recovery routes, leaving 659 candidate gaps. The separate copyover custody fixture has no NPCs and telemetry serialization does not test NPC holdings, so neither was assigned to file/buffer NPC wallet routes. Controlled world I/O and prototype reconstruction do not qualify durable accounting or preservation of original item UIDs; file-based NPC recovery still has no candidate.

### Prefix-mutation census guard

The scanner now detects prefix increment/decrement on denomination macros and cash/bank array expressions, including member-qualified cash arrays. A regression covers those writes and excludes comments, strings and read-only arguments; all 46 contract tests pass. This addresses one syntactic blind spot, not general alias analysis. Reviewed reference-search examples include transaction bank-byte decoding and auction result-vector decoding (construction of decoded state rather than independent live issuance); mutable bank references in flat-file command application still require their full transaction context when qualifying authority. No route was reclassified from these searches alone.

### Scroll consumption candidate audit

The device harness now repeats scheduler/cleanup pulses after successful and cancelled scroll actions, asserting no repeated extraction or spell effects. Its full ASan/UBSan run passes. Linked legacy recitation consumption to this inspected native command/device/scheduler harness; controlled spell/extraction endpoints do not qualify durable retirement or backend replay. The scribing regression only exercises scheduling payload classification, so it was not linked to event-driven scroll destruction. There are 658 routes without candidates; no production behavior or coverage status changed.

### Native artifact candidate audit

Sword and wand-of-wonder ASan/UBSan suites pass. Linked sword poof (extraction/unequip for incompatible racewar in both action modes) and legacy wand gems (all twenty outcomes in both modes, gem count/placement and interruption checks). Controlled endpoints do not establish durable issuance or retirement. Studio ability tests exercise typed ability dispatch rather than object load/give/purge, so those routes remain without candidates. There are 656 missing candidate links; no production or support-status change.

### Unique source-site ownership validation

The validator now refuses repeated path/line/family assignments, whether repeated within one writer or assigned to a second writer with a different integration owner. Previously the mapped-site set silently collapsed both cases. A current-inventory audit found no duplicates; the new regression exercises both failures and all 47 contract tests pass. This enforces unambiguous ownership of identified census sites without claiming semantic completeness; 656 candidate gaps remain.

### Labyrinth and starter reward candidate audit

Both existing source-contract scripts pass. Linked reset_lab room-lookup safety (not executed custody/destruction) and paladin single-sword publication/no second starter kit (not durable once-only delivery); 654 candidate gaps remain. Paladin source removes eligibility before allocation and publication without an operation ID or allocation-failure recovery; recorded this authority gap. The relic potion reward is not exercised by the bounds test and remains unlinked. No gameplay change or backend promotion.

### Artifact repository candidate audit

The flat-file artifact repository, binding runtime and gameplay runtime harnesses plus source contracts pass (warning-clean native builds, not sanitizers). Linked ground/NPC restoration counts with corrupt-authority refusal as native controlled-world evidence; linked clear, binding maintenance and binding repair only to their inspected source delegation checks. Neither class qualifies common accounting admission, original UID preservation or correction replay. Missing candidates decrease from 654 to 649; no backend status changed.

### Native central money helper evidence

Added test_money_helpers_runtime.py extracting actual ADD_MONEY/SUB_MONEY bodies with production headers. ASan/UBSan passes accepted/rejected submission without live mutation, accepted/refused fallback claim, zero/negative credit no-op, invalid debit refusal and direct NPC credit/change conservation across eleven denomination boundaries. Initial linking required a fail-fast panic stub; the final harness passes. Linked three helper routes; 646 candidate gaps remain. Controlled submission/claim endpoints do not qualify persistence, replay identity or atomic compensation. No production changes.

### Native shop cash/dormant barter evidence

Extended the money helper harness with actual transact: room mismatch causes no submission, refused debit leaves recipient unchanged, and passing merchandise still charges cash because production explicitly disables barter. ASan/UBSan passes. Accepted persisted-payer submission credits the transient recipient while payer live balance remains unchanged; this demonstrates the existing completion boundary, not atomic payment. Two routes linked, leaving 644 candidate gaps. Existing shop live-route source contract also passes but was not treated as repair/peruse execution coverage. No production behavior changed.

### Required ownership text validation

Owner, authority boundary and source/sink classification now require nonblank strings. Truthy numbers, booleans, objects and whitespace previously passed; missing fields now produce a contract error rather than KeyError. Regression covers each malformed value and omitted field; all 48 contract tests pass. Current inventory audit found no invalid entries. This validates metadata shape, not semantic correctness or completion; 644 candidate gaps remain.

### Storage completion candidate audit

All three saved-item flat-file routing source contracts pass. Linked the two storage-completion inventory entries to refusal/commit ordering checks before root publication, child detachment and extraction. Submit/new/delete/remove routes were not linked solely because their names occur in the script: sequence execution, interruption recovery and backend parity remain unqualified. Missing candidates decrease from 644 to 642; no production behavior or backend status changed.

### Corpse repository contract refresh

Offline corpse contracts initially failed stale hardcoded table count (212 versus current 215) and migration head (0031 versus current 0032). Replaced those with consistency checks against the table list and highest registered migration, retaining explicit corpse authority registration/protection checks. All seven tests now pass. Linked compound corpse transfers to source savepoint/order evidence only, not native SQL execution; 641 candidate gaps remain. The older item-transfer SQL runner loads .env and was inspected but not executed. No migrations or production code changed.

### Private native corpse qualification

Added --suite corpse to the existing private local MariaDB runner and a guarded native harness launcher using the maintained disposable-container source list. The private instance verifies its datadir, never loads .env, and cleans up on exit. Fresh bootstrap/migration/runtime validation and native corpse lifecycle ASan/UBSan harness passed. This qualifies the existing corpse transaction harness on local MariaDB only, not MySQL or common accounting activation. Docker WSL integration was unavailable; Docker Desktop diagnosis continued separately.

### Native corpse inventory evidence

Mapped the successful private MariaDB ASan/UBSan run at 63746714f to five existing corpse routes after reading their assertions: world transfer, wallet application, nested room materialization, world-root retirement and compound rollback. Four previously empty candidate lists are now populated, leaving 637 gaps. Bank-row preparation remains without a candidate because the inspected assertions do not establish its row-creation semantics. Backend qualification statuses remain unverified: this is legacy repository evidence, not common accounting activation, MySQL parity or native save acknowledgement. Coinless raise records one currency ledger entry; it is not a no-ledger operation.

### Corpse bank creation and preservation regression

Extended the native corpse harness: coinless raise begins with no bank row and asserts exactly one zero-balance row at revision 1 after exact replay; resurrection begins with nonzero bank denominations and asserts they remain unchanged. Full private MariaDB bootstrap, migration replay, runtime/schema checks and native ASan/UBSan suite passed. Linked bank preparation to this executable candidate, leaving 636 missing candidates. No production behavior or backend qualification status changed.

### Disabled whole-ship sale contract

Rechecked live #474/#475 requirements: unsupported operations must remain visible and existing refusals preserved. Extended the ship-shop source test to require the complete unconditional entry refusal before legacy payout, hash removal and deletion. Both list and disabled-sale contracts pass. Linked the disabled sale writer to this limited executable source evidence, leaving 635 candidate gaps. This does not qualify enabled ship commerce or any accounting backend.

### Private native item transfer qualification

Added --suite item to the isolated local MariaDB runner and run_item_transfer_local.py, with the same explicit disposable loopback/schema guard and no .env loading as the corpse runner. Repaired the maintained item harness source list to link the dispatcher's restitution command/repository. Native execution then exposed outdated reduced restitution fixture tables/inserts (database error 1364); the fixture now uses the registered schema and supplies required synthetic receipt and classification fields. SQL failures report source line/error number without statement or credential contents. Fresh bootstrap, migration replay, runtime/accounting schema checks and the native item-transfer ASan/UBSan harness pass. This verifies existing creation, transfer, topology, refusal, replay, destruction, restitution runtime and allocator assertions on MariaDB only. No common accounting integration or MySQL parity is inferred; 635 candidate gaps remain pending assertion-to-route mapping.

### Native item inventory evidence

Read the passing native assertions and mapped three routes: ownership insertion (including multi-root creation/replay/collision), ownership update (transfer/topology/destruction/refusal), and ledger insertion (six rows and three outbox-linked operations across two UIDs after replay/refusal). Missing candidates decrease from 635 to 632. No coverage claimed for coin payload writes, whole-owner destruction, field-by-field ledger semantics, common economic links or other backends. Backend qualification flags remain unverified.

### Native coin payload evidence

At 50d0c81a2, run_economic_accounting_schema_local.py --suite currency passed private MariaDB bootstrap, migration replay, compatibility/schema checks and native ASan/UBSan currency/coin harness. Inspected pile_amount decoding and partial-pickup, merge/replay and depleted-pile NULL payload assertions; linked storage.item_coin_payload to the runner. Missing candidates decrease to 631. This is existing repository evidence, not common accounting integration, MySQL parity or native publication/save acknowledgement. No production changes.

### Candidate metadata validation

The validator now requires test_candidates to be a list of nonblank strings and rejects duplicates. Previously a dictionary could be iterated as evidence paths and malformed values could raise incidental TypeError/KeyError instead of a contract error. Regression includes dictionary/string/scalar/null containers, invalid elements, duplicates and a missing field. All 49 contract tests and current draft validation pass. Existing 631 candidate gaps remain; metadata validation does not establish runtime coverage.

### Current flat-file item transaction qualification

At ab6a7de77, test_flatfile_item_repository.py passed its warning-clean client-free native harness with authority fault injection. Both standard and area coin prototypes pass conversion, interrupted commit, replay/refusal, merge and retirement. The harness also passes its item, room, locker, corpse and shop custody/recovery assertions. Updated existing coin.flat_apply support evidence, without treating generic custody checks as complete domain economic integration. No sanitizer run, cross-backend parity, native game publication or #478 completion is claimed. Candidate gaps remain 631; no production changes.

### Alchemy ingredient helper regression

Added native ASan/UBSan test using production headers and both extracted ingredient-consumption helpers with controlled ID/extraction endpoints. Repeated requirements consume distinct objects in inventory order, surplus/unrelated inventory stays unselected, recipe arrays remain unchanged and empty inventory is safe. Insufficient requirements still consume available matches: callers own sufficiency checks, and no all-or-nothing craft behavior is claimed. Both routes now have candidates, leaving 629 gaps. Full recipe/vial/output lifecycle and durable accounting remain unqualified. No production changes.
