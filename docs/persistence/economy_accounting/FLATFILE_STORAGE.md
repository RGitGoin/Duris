# Flatfile retained accounting storage

Status: storage foundation for #478; gameplay integration and activation are
unfinished. The SQL and flatfile APIs share canonical EAI1 intent and EAP1 plan
bytes. A structural record is never proof that a domain mutation was authorized
or applied. The critical coordinator remains the only admission/retry mechanism.

## Atomic publication and ownership

The evidence directory is `FLATFILE_ROOT/economic-evidence`, owner-only. Store
number 7 extends the existing v2 authority journal without changing earlier
store numbers or v1/v2 framing. Generic runtime commit rejects this reserved
store. Only private staging/commit methods, accessible to future typed bank and
lifecycle owners, may publish its after-images. The test-only friend is absent
from production builds. Recovery can replay reserved operations from its
checksummed journal. An older binary must refuse an unknown store and retain the
journal; it cannot safely run against newly activated accounting state.

A domain owner must acquire the shared authority lock, recover authority and
legacy domain journals, look up the admitted operation ID, prepare/verify actual
locked domain effects, stage the exact domain and accounting bundle, commit,
then publish the retained result. Storage calls recover the authority journal
before reading; they do not establish domain-write authorization or replace the
owner's initial recovery/revalidation. Missing/corrupt state is unresolved, not
permission to acknowledge or apply another operation ID.

A root without children adds two journal operations: its active/new segment and bucket index. Compound roots add each affected segment and bucket index once, including any segment rotated during the batch. All domain and evidence after-images must fit the existing 32-operation limit. Preflight includes the actual 256 MiB journal limit, 50-byte framing and
8+filename-length bytes per operation. Future typed adapters must also bound
their combined maximum domain after-images; a 256 MiB world catalog leaves no
room for accounting. The storage bridge does not silently split one root commit.

## Record version 2 and file framing

All integers are explicit little-endian. Each file/record has a 48-byte envelope:
8-byte magic, u32 version 1, u32 payload length, and SHA-256 of the entire payload.
Magic/version/length must exactly match the expected layout. Reserved fields are
zero. Decoders bound lengths/counts before allocating and retain caller outputs
on error.

- Record magic `DURECR2\0` (the earlier unshipped prototype is refused). Its payload starts with u32 command/plan/result byte
  lengths, u32 result code, u64 durable revision and u16 failure stage, followed by those byte
  strings. It retains the exact canonical schema-2 admitted command (including
  intent and admission timestamp), canonical plan and original result. Success
  requires a structurally valid plan whose complete metadata matches the frozen
  intent; rejection requires a nonzero result code and no realized plan.
  Failure stage must be a defined value and must be none for success. Child-bearing plans require atomic reservations for every derived child ID; retained root lookup verifies those reservations. Domain-specific result/actual-effect verification remains the typed owner's job.
- Child reservation magic `DURECC1\0`, using the same checksummed envelope and existing segments. Payload: child ID[16], root ID[16], SHA-256 of the complete canonical root record[32], u32 domain, u64 discriminator, u16 parent index and u16 relationship (128 bytes including the envelope). Reservations share the root-ID namespace and consume the same bucket capacity; they cannot be replayed as independent roots. A missing or changed reservation blocks parent replay. Older readers refuse these records and compound root plans.
- Index `bucket-XX.eai`, magic `DURECI1\0`. Payload: lineage[16], u32 bucket,
  u32 entry count, u64 total record bytes, then sorted 64-byte entries containing
  operation ID[16], record SHA-256[32], u32 segment/offset/size and u32 reserved.
  Operation IDs are unique and their first byte identifies the bucket.
- Segment `bucket-XX-N.eas`, magic `DURECS1\0`. Payload: lineage[16], u32 bucket,
  u32 segment, u32 record count, u32 reserved, then contiguous complete records.
  Index offsets are relative to the first record. Every indexed range must be
  contiguous, nonoverlapping and within the exact segment payload; each record
  digest must match. Segment numbers are dense and start at zero.

The active segment grows by retaining its entire old record prefix and appending
root and child records. It rotates before exceeding 8 MiB; sealed segments are never rewritten
or pruned. The corresponding index retains every prior entry. All affected bucket segments and indexes are published by one authority journal together with domain state and the exact root receipt. Same-bucket records are combined before publication. Existing 32-operation and 256 MiB journal bounds still apply; a bundle that exceeds them fails before modifying caller staging or disk. No new durable file class or eviction policy is introduced.

## Bounds and stale-state refusal

There are 256 buckets, at most 4,096 records and 256 MiB record bytes per bucket.
A record is bounded by 48+26+512 KiB+4 MiB+4 KiB = 4,722,762 bytes. The derived
format limit is 74 segments per bucket. The index maximum is 262,224 bytes.
Aggregate upper bounds are 1,048,576 records, 64 GiB record payload and 19,200
segment/index files; an individual bucket can fill sooner and framing adds disk
space. These are capacity limits, never eviction or a rolling retention window.
At capacity new operations fail while retained lookup remains available.

Every lookup, including an old ID or absent ID, verifies the active segment and
requires the next segment name to be absent. This detects a stale valid index
whose old active segment was subsequently sealed. A stale index within the same
segment fails exact coverage/digest checks. An older selected segment is verified
before returning its record. Missing indexes or indexed segments, unsupported
versions, wrong lineage/bucket/number, checksum mismatch and corrupt pending
journals refuse access. Per-lookup reads are bounded by one index and at most two
8 MiB segments; staging performs bounded copies, never a scan of world history.

Private bucket initialization checks an existing private directory and refuses
any matching bucket files. It is not callable by gameplay. Before activation,
the lifecycle owner must additionally prove durably that the bucket was never
activated; absence alone cannot distinguish fresh state from lost history. All
required buckets and lifetime/epoch mappings must be initialized consistently.
No automatic missing-index reconstruction, empty reset or compaction is provided.

## Retention and remaining integration

Lifecycle entries protect indexes and segments through season reset and restore.
The existing managed backup recursively captures both classes under the shared
authority lock; its existing metadata/disk budgets still apply. Native restore
qualification must gain a full semantic store scan before activation; capture
coverage alone does not prove restored accounting consistency.

The existing 512-receipt player cap and other bounded domain receipts remain.
A typed flatfile adapter must atomically connect actual domain after-images,
retained accounting lookup and exact result, then safely adapt those hot receipts
without losing old replay fences. Source claims, retained lifetime/epoch metadata,
compound savepoints, reconciliation/baseline/export tooling and release writer
coverage remain unfinished.

## Storage qualification

The reused native ASan/UBSan harness exercises syscall fault/process-exit cases across
both initial commit and journal recovery: short and interrupted writes, zero
writes, ENOSPC, file data sync, rename, directory sync and journal removal.
Failures do not acknowledge success or overwrite the lookup result. A clean
retry recovers the complete domain/evidence bundle or proves the journal was
never published, then retries the original ID once. Repeated lookup preserves
exact result/plan bytes and does not append another event. These tests model
process exits and syscall failures; they do not simulate storage power loss.

Other cases cover canonical/corrupt/unsupported bytes, stale indexes, wrong
locks, operations older than 512 later receipts, segment rotation, full retained
indexes, and exact 32-operation/256 MiB journal limits. The byte-boundary test
stages a synthetic large image in memory without publishing it. These storage
tests do not qualify the future gameplay adapter or complete slice 04.

## Current extraction

Based on PR #604, this increment reuses `42cdf47c1` with selected durability
fixes from `6631c4e9b`, `411d8102` and `0d8e6bb3`. A pending journal must
block a second commit without recovering already-prepared after-images. Allocation
failures preserve lock reuse and return I/O failure with ENOMEM; authority files
with multiple hardlinks are refused. This does not import authority checkpoint v3,
root descriptors, legacy indexing or baseline activation. Compound reservations now use the existing bounded evidence segments and authority journal.
The new receipt preserves the current 4096-byte completion limit and failure stage;
legacy player-domain receipt limits remain independent.

The expanded ASan/UBSan suite passes the 85 original commit/recovery fault cases
plus pending-journal overwrite refusal, reusable allocation-failed locks, encoder
ENOMEM classification, hardlinked index/segment/lock refusal, uninitialized child-bucket
refusal, failure-stage roundtrip and full 4096-byte result retention. Existing
authority/player-domain/account, lifecycle, backup and provisioning tests pass.
Both full server builds (flatfile and MariaDB) pass. Hosted qualification and review remain pending.


## Compound reservation verification

The native store harness runs 173 additional commit/recovery fault cases with a
root, three child reservations across three buckets, and a domain after-image.
They cover process termination, short/interrupted/zero writes, ENOSPC, data sync,
rename, directory sync and journal removal. Recovery yields the prior bundle or
the complete bundle, including every reservation, before replay can succeed.

Focused cases cover same-bucket coalescing, cross-bucket reservations, full-width
64-bit discriminators, competing roots at reserved IDs, missing reservation
indexes, checksum-preserving parent-digest tampering, and the exact 32-operation
bundle boundary. Reservations consume the existing per-bucket entry/byte limits;
no new retention pruning or durable file class is introduced. Root lookup checks
at most 64 child links, one additional bucket/segment context at a time. Staging
holds at most 16 affected bucket contexts plus the bounded authority bundle.

The standalone typed wallet-to-wallet coin owner now stages native player/bank
images with the accounting root and child reservations under one authority lock
and journal. It checks retained ownership-catalog and both player-domain receipts
for root/child collisions; an absent catalog remains unresolved. Success and
business rejection retain exact results, and replay verifies the plan, failure
stage and result against the original intent and historical lifetime/epoch data.
Post-commit readback checks native images and balances, including a shared bank's
final revision. Existing bounded legacy receipts are preserved, not extended.

The native coin harness covers shared/separate banks, unchanged rejection states,
SQL-policy canonical plan/error parity, paused-epoch replay, legacy root/child
collisions, missing catalog refusal, forged retained result/stage/revision, and
recovery after journal publication and every after-image boundary. These checks
do not qualify a MySQL server or power-loss behavior.

The schema-2 flatfile dispatcher now routes coin commands to this owner. The
native coin harness exercises that route with both real typed owners linked;
the unrelated legacy dispatcher is a link-time failure sentinel. The coordinator
admission test continues to reject the closed coin family without checkpointing
its journal entry. Gameplay admission remains closed. Full
historical legacy fencing, remaining typed domains and activation/reconciliation
qualification are still required; this increment does not complete #478 or #474.
