# SQL initial lifetime enrollment

`economic_sql_enrollment_transaction` is private to the future accounting
lifecycle owner. It enrolls a current observed wallet or shared bank in one
metadata transaction without activating an epoch or changing native money.
It does not establish global quiescence, authorize an operator, classify all
sources, or prove that a lineage has never been activated. The caller must
independently retain the frozen native boundary through enrollment and openings.

A wallet locator is a positive native PID at most INT32_MAX, with context zero.
A bank locator is a positive native bank ID at most UINT32_MAX; its context is
the native nonnegative signed-tinyint racewar value. Names are captured evidence,
not lifetime keys. The allocated mapping ID identifies the new accounting
lifetime. No historical provenance or current-player eligibility is inferred:
the native capture does not include immutable player-generation evidence.

The owner requires an idle reconnect-disabled READ COMMITTED connection. It
reserves the exact original inbox operation, exclusively locks the retained
lineage, checks its expected revision and NULL active epoch, and verifies the
retained epoch's completed initialization receipt. It refuses *any* prior mapping
for `(lineage, SQL backend, locator kind, native ID)`, including retired records
and records under another account-kind/context alias. A different operation,
source hash, name, balance or epoch does not bypass retained history. Typed
retirement, generation transitions and existing-lifetime baseline binding remain
separate required lifecycle work.

The borrowed-transaction `economic_sql_verify_holding_source` locks the exact
registered wallet/bank projection and recomputes its capture ESD1/ESR1 digest.
It preserves binary cells and NULL/empty distinctions. The caller retains
transaction ownership and must roll back after failure. The owner also verifies
the native balance, revision and bank context against the command, then allocates
one mapping, advances the lineage revision, and completes its canonical receipt.
Those metadata changes commit together. No currency/custody ledger, outbox,
native save, UID allocation, Redis update or publication occurs.

The canonical ELC1 request is 200 bytes, little-endian:

| Offset | Bytes | Field |
| --- | --- | --- |
| 0 / 4 / 6 | 4 / 2 / 2 | Magic, version 1, size 200 |
| 8 / 24 / 40 | 16 each | Lineage, epoch, epoch-creation operation |
| 56 / 64 | 8 each | Operator and expected lineage revision |
| 72 / 74 | 2 / 6 | Account kind and reserved zero bytes |
| 80 / 88 / 96 | 8 each | Native ID, context and native revision |
| 104 | 32 | Four nonnegative signed denomination counts |
| 136 / 168 | 32 each | Frozen-boundary digest and captured source-row digest |

Command type 21 is appended without changing older durable type numbers. Its
schema-1 envelope carries metadata only; no artificial financial intent/plan is
attached. Existing legacy execution and coordinator admission reject it. The
fixed recovery fence serializes enrollment submissions only; it is not a global
native-write freeze.

The inbox retains a 224-byte ELR1 receipt: 8-byte magic/version/reserved header,
8-byte allocated mapping ID, 8-byte resulting lineage revision, then the full
200-byte canonical request. Existing schema 0031 and the 4096-byte inbox result
limit suffice; no migration or bootstrap bytes change. On original-ID replay,
the owner compares complete command/key hashes, canonical receipt bytes,
immutable mapping creation fields, retained epoch and lineage membership, and
absence of financial/publication rows under that operation. Later native changes
or mapping retirement do not rewrite the original result. Missing/corrupt evidence
refuses; an uncertain COMMIT requires reconciliation with the same original ID.
The lifecycle caller must discard connections after uncertain transaction errors;
a best-effort rollback does not establish that a damaged session is reusable.

The disposable native test joins real source capture and normalization to
wallet/shared-bank enrollment, then existing `economic_baseline_prepare` and SQL
baseline storage. It exercises sequential and concurrent retries, refusals and
fault boundaries. Actual results are reported with this increment after completion.
This is an enabling integration, not complete baseline/activation qualification.

History refusal uses the retained-locator index and stops at its first match.
Exact zero/one cardinality checks inspect at most one/two matching rows while
preserving duplicate detection. This bounds matching-row work, not wall-clock
latency under lock waits, MVCC history or storage faults.

This increment uses the version-1, 18-table source registry from PR #613.
Character names and immutable player generations are not captured. The stale-source
fixture changes captured `account_name`; it does not claim that an uncaptured
character-name change alone is detected. No broader inventory capture is imported.

## Local verification (2026-09-21, unpublished)

Against PR #613 head `084bdc3ce130377cc22b395c4155586cbcae0ba8` plus this
working-tree enrollment increment, MySQL 8.0.46 and MariaDB 10.11.14 ASan/UBSan
suites passed 23 query failures, 5 post-write/lost-ack failures and 224 allocation
failures per engine. Fifty rounds of same-ID and different-ID concurrency passed
on each engine with exact mapping/revision and stale-lineage refusal assertions;
no transient-retry relaxation was needed. Capture, normalization, enrollment and
baseline opening were exercised together. Client-free refusal/codec checks and
both pure admission modes passed. Both full server builds and focused formatting
passed; the accounting contract suite passed 26 tests with census 115 routes /
2,731 candidates and release readiness false. Disposable schemas and engines were
cleaned up. These results do not qualify a complete cutover or close #479.

No PR is published for this component. The issue-level acceptance audit and
review-batch gate in `ISSUE_CLOSURE_STATUS.md` takes precedence over the former
component-by-component publication sequence.