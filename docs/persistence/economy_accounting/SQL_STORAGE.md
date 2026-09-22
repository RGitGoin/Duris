# SQL accounting storage increment

Status: storage and identity-lock component on `codex/474-phase3-sql-storage`, based on guarded
intent PR #600. This is a partial delivery for #477, not an accounting activation
or completion claim. The source is the preserved `bb8c1e5c5` schema and
`0dc1079fd` identity-lock implementation, adapted to canonical master
`48c0aedd8e094eee37285111e46e735e4cf12320`.

## Storage and registration

The nine-table schema retains epochs, active lineage state, non-reused account
mappings, operation records, account effects, coin posting lines, child links,
item-ledger references, and source-event deduplication. The item reference uses
an additive unique index on the existing ledger so its composite foreign key
binds the exact legacy event, UID and revision. Existing balance/custody stores
remain authoritative; there is no mutable global mint/sink total.

`0030` belongs to telemetry quarantine on this baseline. The accounting migration
is provisionally `0031`; allocation must be checked again before merge. Bootstrap,
immutable verification, runtime metadata and lifecycle registration must agree
on both MySQL and MariaDB before this increment is review-ready. Historical
whole-schema fingerprints cannot be reused on the newer base. No baseline,
active epoch, mapping or gameplay value is seeded by this schema.

## Identity-lock helper

`economic_sql_lock_authority` borrows an already active transaction with automatic
reconnect disabled. It locks the lineage row shared, then ordinary account
mapping rows in ascending lifetime ID order. It checks lineage, epoch, account
kind/context, backend, native locator and active lifetime; duplicate mapping IDs
are rejected. Errors preserve the caller's output, but may leave locks held, so
the caller must roll back. It never starts/commits/retries transactions or writes
financial evidence. Client-free builds return `ENOTSUP` without changing output.

A successful snapshot proves identity under the caller's locks only. It does not
prove domain effects, inbox ownership or authorization to append accounting
records. There are no gameplay callsites in this increment. Existing schema-2
admission/replay guards remain in place.

## Verification scope

- Schema tests must exercise constraints, retained identities, child/item/source
  references and fresh bootstrap versus supported upgrade/replay on both engines.
- Native authority tests exercise transaction ownership, ordering, concurrent
  shared readers, exclusive-writer contention, inactive/stale epochs, retirement
  and native-ID reuse, output preservation, unsigned metadata, persisted foreign-lineage mismatch and
  connection loss after acquiring locks.
- The independently discovered client-free regression checks `ENOTSUP`, null
  output handling and preservation of an already populated output under ASan/UBSan.
- Both supported server builds, immutable migration validation and runtime/lifecycle
  consistency checks remain required. Results belong to the tested revision.

## Remaining #477 acceptance work

Typed transaction-local append/finalize adapters still must bind immutable intent
to actual locked domain effects and exact legacy rows, verify after-state and all
normalized entries, and finalize before every root commit. Existing compound
savepoint rejection paths must restore accumulated effects. Financial records
need append-only application-role protections. Fault tests must prove atomic
receipt/domain/evidence/outbox commits, original-ID replay after lost commit
acknowledgement, changed-payload rejection and specialized writer integration.
Neither a schema constraint suite nor the identity snapshot qualifies those gates.
## Local qualification results

MySQL 8.0.46 and MariaDB 10.11.14 passed fresh bootstrap, full migration run and
repeat execution. Separate upgrade databases started with predecessor
`511d04f16b613a000a857af4293fcd8b2d3fb48a` bootstrap and its 30-entry migration
manifest, then applied the target 0031 and replayed. All ten schema tests passed
on each fresh and upgraded database; final runtime compatibility verification
passed on all four. Fresh/upgrade metadata fingerprints matched per engine.

Both engines passed the final ASan/UBSan authority harness, including the added
foreign-lineage and lost-connection cases. The discovered client-free sanitizer
regression also passed. Static lifecycle/runtime validators and 37 existing
registration tests passed. MariaDB required a Linux temporary datadir after an
ALTER TABLE rename failed on the Windows-backed test filesystem; unchanged tests
passed on the fresh Linux directory. All owned database processes were stopped.

Run `tests/async/run_economic_accounting_schema_mysql.sh` with each supported
`ECONOMIC_ACCOUNTING_DB_IMAGE` for the disposable fresh-schema suite. The upgrade
journey must use a separate database adopted by the predecessor runner/manifest
before the target runner is used; replay on a target fresh bootstrap alone is
not upgrade evidence. Neither test flow may use an existing game database.

Full server build results are recorded with the PR revision; native SQL results
above qualify storage and identity locking only, not an accounting command commit.

## Common compound adapter implementation boundary

Source review on 2026-09-22 confirms this remains implementation work. The public
economic_accounting_repository API only locks identity mappings; its snapshot is
not append authority. economic_sql_bank_transaction::evidence requires exactly
two account effects and postings and refuses children/items. Flat-file
validate_record refuses children because no child reservation exists. Neither
backend currently supplies a general compound accounting transaction owner.

Implement the common #477 transaction path before its #478 counterpart: retain
the existing root inbox owner and authority locks, verify the complete typed
native effects, reserve child IDs, append child/item/source evidence, then
finalize within the same commit. Check collisions in both directions: a new
child against existing roots/children and a new root against reserved children.
The SQL child table has a unique child ID but that constraint alone does not
exclude a root using the same ID. Exact-ID retry must compare the retained root,
child relationships and canonical request; it must not mint replacement IDs.

Flat-file child reservations must share the authority journal with root evidence
and native after-images. Multiple children in one bucket must be combined into
one after-image, rather than staging duplicate filenames or rereading only the
pre-transaction index. Missing/corrupt reservation state must not become an empty
index. Include its format, bounds, compatibility and backup registration with
the implementation. Keep the current child refusal until these invariants are
implemented and qualified; structural codec acceptance is not an alternative.

### Reuse the existing compound SQL owner

The legacy coin branch in critical_command_repository_apply already creates the
coin_endpoints savepoint, inserts each derived child into critical_operation_inbox
with a plain INSERT before its native effect, and finalizes each child receipt and
outbox before the root commit. On endpoint failure it rolls back the compound
scope. Reuse this owner and its shared inbox identity namespace for accounting
integration; the missing work is typed accounting evidence validation/finalization,
not a second generic transaction framework. The child-link table's unique key
alone is still not an identity reservation mechanism. Flat-file reservations
require their own equivalent durable implementation.

### Disposable local MariaDB qualification

When Docker is unavailable, run
`python3 tests/async/run_economic_accounting_schema_local.py` in the Linux build
environment with MariaDB server/client core binaries and compiler dependencies.
The runner starts its own private temporary data directory on loopback, verifies
that the connected server owns that directory before writing, uses generated test
credentials, and stops the process and removes its data on completion or failure.
It never uses an existing database or installs/starts a system service. If tracked
helper scripts contain CRLF, it uses a temporary source copy and normalizes only
non-immutable shell helpers; sealed migration bytes and the checkout stay intact.

This runs the existing schema, authority, bank, baseline, source and enrollment
suites on MariaDB. It does not substitute for MySQL-engine qualification or prove
the still-missing common compound accounting path.

Verification on 2026-09-22: the completed local MariaDB 10.11.14 run passed
runtime compatibility, 10 accounting-schema tests, 10 baseline-schema tests,
authority locking/disconnect, typed bank, baseline retention, source capture and
initial enrollment suites, including their native fault/replay cases and
client-free refusal variants. The revised runner also completed successfully
with private-instance verification and automatic LF-copy handling. These results
qualify the existing components, not the missing compound accounting integration.

Use `python3 tests/async/run_economic_accounting_schema_local.py --suite currency`
to qualify the existing native currency/coin transaction owner separately. This
reuses its maintained production link set with ASan/UBSan. The collision matrix
seeds retained root/child inbox fixtures at the command's derived child IDs,
then checks both endpoints and exact-command retries. Balances, wallet/shared-bank
revisions, inbox, ledger and outbox counts must remain unchanged on collision;
the original completed transfer must remain replayable. This is evidence for
reusing the legacy transaction owner, not proof of compound accounting evidence
append or flat-file child reservations.

Verification on 2026-09-22: the currency suite passed on private MariaDB 10.11.14
with ASan/UBSan, including six seeded child-ID collision cases, two attempts per
case, and the existing native coin conversion/rollback/replay/reload/custody
matrix. The private instance and temporary source/executable directories were
cleaned up. MySQL-engine and compound accounting integration remain unqualified.

### Typed wallet-to-wallet coin evidence preparation

`economic_coin_adapter` now prepares the wallet-to-wallet part of the existing
coin command using writer ID 3 (`ECONOMIC_WRITER_COIN_WALLETS`) and the
`wallet_transfer` reason. Version-1 facts are 48 bytes: source then destination,
each with little-endian wallet lifetime ID, bank lifetime ID and bank context.
The command binding retains both native endpoints and their derived child IDs.

Preparation uses both pre-root authority snapshots and the shared native currency
arithmetic. It retains two child links and two denomination postings. A shared
bank gets one unchanged-balance effect spanning both revision advances; different
banks each get their own revision effect. The destination mutation starts from
the source's bank after-state only when the native fence and retained bank mapping
both identify the same bank. Divergent shared snapshots, mapping aliases, stale
fences and altered intent are rejected without replacing the prepared output.

This is a pure typed adapter, not a storage capability. Coin runtime admission
remains refused. SQL must acquire/verify all native mappings and locks, execute
these exact prepared effects under the existing root/child inbox owner, and
append/verify evidence in that transaction before enabling it. Pile custody
adapters and flat-file compound reservations remain separate incomplete work.
The adapter test runs both SQL and client-free compilation modes with ASan/UBSan;
its arithmetic parity does not prove backend storage parity.
