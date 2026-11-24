# Specifications

One specification per step of the [implementation strategy](../implementation-strategy.md), in the format of the CPG's
[improvement specifications](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements):
a header, the problem, a decisions table, the specification, measurements, verification, what is out of scope, open
points, and implementation notes filled in as the step is built. Since revision 2 of the strategy, a decisions table has a
last column, *Rests on*, naming what each decision presupposes (here or in another specification): numbered rows are not
independent choices, and the column shows which can be judged alone and which change with another. Decision identifiers
are global: `SNN-Dk` is specification NN's decision k (`S04-D3`), `S00-Ak` one of spec 00's addendum A (strategy,
§4.1). The decisions are
split in three: §2.1, the owner's questions decidable now, each with a proposed answer and its alternatives; §2.2, the
owner's questions that wait for a later step, with what holds meanwhile; §2.3, decisions that follow from ones already
taken or from the code, to be read and questioned, needing no answer of their own. Approving a specification is answering
its §2.1. Specifications 00 to 02, implemented before these rules, keep their tables as they were; addendum A of spec 00
marks each decision's kind.

No approval is pending: specifications 03 to 05 were approved on 2026-11-23 ([strategy, §8.3](../implementation-strategy.md#83-taken-since-the-list-was-made)).

| # | Specification | Status | Depends on |
|---|---|---|---|
| 00 | [Infrastructure: CI, compatibility harness, the CPG submodule, test builds](00-infrastructure.md) | implemented; device run pending | |
| 0a | [Local tools: native runner, ARM size estimate](00-infrastructure.md#addendum-a-local-tools) (spec 00, addendum A) | implemented | 00 |
| 01 | [Key-aware chord spelling](01-key-aware-spelling.md), with addendum A (chromatic roots) | implemented; device run pending | 00 |
| 02 | [Chromatic chords through the progression path](02-chromatic-progression-path.md) | implemented; device run pending | 00 |
| 03 | [The missing sonorities](03-missing-sonorities.md) | implemented; heard on the device with step 5 | 02 |
| 04 | [The palette table, generated from the CPG](04-palette-table.md) | implemented; two questions for the owner (§8.4) | 00, 03 |
| 05 | [The part's palette](05-part-palette.md) | approved | 02, 03, 04 |

## How far the drafting goes, and why

Specifications 00 to 02 were approved and implemented; their implementation notes record what building them taught.
Specifications 03 to 05 were drafted before that and revised after it, with measurements the drafts lacked: the Classic
pads with no chromatic form, the 7sus4 of the CPG's Scale Mode, the spelling differences, the budgets as CI measures them,
what an older build trusts in a saved state. They are now for approval; their open points are the questions only
implementation or the device can answer. The lessons themselves are in the
[implementation strategy](../implementation-strategy.md#3-what-steps-0-to-2-taught).

Steps 6 to 13 are not specified yet. They bring screens, whose costs depend on the skin budget (0.9 MB of headroom,
the binding one now) and the device, and they follow milestone M2 (the engine complete), when a design note goes to
Cadenza's author (study, §9) and the owner decides the fork's skin budget (strategy, §8).

## Status values

draft (for approval, or design proposals) → approved → implemented → verified on the device.

## Dependencies

```
00 infrastructure
 ├── 0a local tools
 ├── 01 spelling
 └── 02 chromatic path
      └── 03 sonorities
           └── 04 palette table (also needs 00's submodule)
                └── 05 part's palette (also needs 02 and 03)
                     └── steps 6 to 10, not yet specified
```
