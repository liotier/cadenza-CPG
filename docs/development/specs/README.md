# Specifications

One specification per step of the [implementation strategy](../implementation-strategy.md), in the format of the CPG's
[improvement specifications](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements):
a header, the problem, a decisions table, the specification, measurements, verification, what is out of scope, open
points, and implementation notes filled in as the step is built.

| # | Specification | Status | Depends on |
|---|---|---|---|
| 00 | [Infrastructure: CI, compatibility harness, the CPG submodule, test builds](00-infrastructure.md) | implemented; device run pending | |
| 0a | [Local tools: native runner, ARM size estimate](00-infrastructure.md#addendum-a-local-tools) (spec 00, addendum A) | for approval | 00 |
| 01 | [Key-aware chord spelling](01-key-aware-spelling.md) | implemented; device run pending | 00 |
| 02 | [Chromatic chords through the progression path](02-chromatic-progression-path.md) | implemented; device run pending | 00 |
| 03 | [The missing sonorities](03-missing-sonorities.md) | for approval | 02 |
| 04 | [The palette table, generated from the CPG](04-palette-table.md) | for approval | 00, 03 |
| 05 | [The part's palette](05-part-palette.md) | for approval | 02, 03, 04 |

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
