# Typed bank admission and replay

Status: SQL admission was delivered in PR #604 on PR #603. The current flat-file
follow-up is based on [PR #607](https://github.com/Community-Duris/Duris/pull/607)
at `790665585`. These are partial #476/#478/#480 components; gameplay activation
and restart-safe publication remain open.

This increment reuses the bank-only transport portion of `a89fa8f18`. The
coordinator accepts an optional pure extension validator, shared by fresh submit,
publication-retaining submit and durable journal replay. Existing callers default
to no extension support. Failed initialization and shutdown clear registration.

The bank validator regenerates the typed frozen intent from its retained lineage,
epoch, wallet/bank lifetimes and canonical command, requiring exact byte equality.
It neither authorizes a new mutation nor consults current activation state. The
SQL owner still verifies retained receipts first and checks current authority only
for new operations. Frozen commands are not renormalized after binding.

SQL startup pairs this validator with the existing pooled root. The pool admits
only legacy commands and structurally valid bank envelopes, and retains existing
connection replacement and original-ID reconciliation after ambiguous commits.
The direct bank owner performs the transactional checks described in
[SQL_BANK.md](SQL_BANK.md). Unsupported nested SQL paths remain closed.

The flat-file follow-up on PR #607 now pairs the same bank-only validator with
`flatfile_accounting_apply_selected`. Schema-1 commands keep the selected legacy
repository path and context. Schema-2 bank commands keep their original envelope
and go to the typed native bank owner, using an explicit root or the configured
flat-file root. Unsupported schema-2 types return retryable ENOTSUP without legacy
fallback. The coordinator refuses unsupported durable envelopes during startup
without checkpointing them. Default callers without registration still refuse
accounting. No gameplay producer, baseline or activation is enabled.

## Verification boundary

- Pure transport tests in both compilation modes exercise explicit registration,
  malformed/mismatched bank intent, unsupported types, same-ID attachment/conflict,
  unresolved fences and durable replay. Owner doubles do not prove native storage.
- SQL pool tests use real disposable database connections for fresh apply, replay,
  lost commit reply and replacement-connection reconciliation. Retired authority
  replay must preserve the original receipt and exactly one financial posting.
- Existing coordinator and default-closed mixed-journal tests protect legacy
  behavior and refusal without forwarding/checkpointing.
- Both server builds qualify startup registration and linkage. A complete gameplay
  wallet/bank journey still requires baseline, selected-backend storage and final
  native publication/save acknowledgement.

Local results: all checks above passed, including MySQL 8.0.46 and MariaDB
10.11.14 pooled bank ASan/UBSan runs, both full server builds, timestamp-zero
admission and explicit coordinator publication acknowledgement. Hosted checks and
review remain pending. Native gameplay publication is not established by these tests.

## Flat-file publication boundary

Native bank effects and retained receipts reconcile exactly once through the
coordinator after restart. Accounting submissions now persist their publication
retention policy in journal version 2. Replay restores the policy before execution
and keeps the original fences and journal entry until explicit acknowledgement.
Older version-1 accounting records have no policy; replay conservatively retains
them. Legacy schema-1 coordinator submissions keep their existing replay behavior.

This establishes durable coordinator retention, not native gameplay publication
or save acknowledgement. Reconstructing domain publication obligations and proving
that the game-thread publisher acknowledges only after the required save remains
open before activation. This does not complete #474, #478, #479 or #480.

## Flat-file focused evidence

The dispatcher ASan/UBSan test passes in SQL and client-free compilation modes,
including configured/explicit roots, full result and failure-stage forwarding,
and refusal of unsupported families without legacy fallback. The native
coordinator/bank sanitizer journey proves assigned timestamp preservation,
exact-command retained replay after committed-but-unacknowledged shutdown,
unchanged balances/revisions, fresh acknowledgement retirement, and durable
unsupported-work refusal without execution or checkpoint. Both version-2 frames
and synthetic historical version-1 accounting frames retain replay fences until
explicit acknowledgement. Existing pure admission tests pass in both modes, and the
legacy flat-file gates still reject schema-2 calls without native file changes.
The source census and changed C++ formatting pass. Full build/boot evidence is
reported with the increment's PR.

## Game-thread bank publication owner

`economic_bank_publication` now restores schema-2 bank obligations from immutable
journal commands alongside the existing restitution replay observer. Its submit
entrypoint always requests publication retention. Each normal game-thread pulse
advances at most 32 entries, rotating past offline/unready players within the
existing 1,024-operation bound.

For a committed result, the owner validates player/account/racewar identity,
uses the existing currency balance publisher, and requests a status checkpoint
through `player_save_pipeline`. It holds the original operation until revision
tracking reports the requested save acknowledged with no unacknowledged status
component, then retries the coordinator acknowledgement if needed. Capture
failure, missing players, malformed/uncertain results, and save backpressure keep
the obligation. A terminal rejection has no new live/save effect and may retire.
Replay restores no historical actor callback and never reapplies balance deltas.

The focused sanitizer harness links the real owner, currency publisher and player
revision state in SQL and flatfile compilation modes. Save/coordinator endpoints
are controlled: it covers immutable replay registration, conflict refusal,
offline retention, failed capture, unrelated-component acknowledgement, exact
status-save acknowledgement, failed/retried journal acknowledgement, rejection
and uncertainty. It is not a native database/save integration or full boot test.

Gameplay producers are not switched by this increment. Offline completion without
login, account lifecycle changes, native end-to-end save qualification, restricted
startup drain and activation remain open before issue closure.
