# Specifications

One specification per step of the [implementation strategy](../implementation-strategy.md), in the format of the CPG's
[improvement specifications](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements):
a header, the problem, a decisions table, the specification, measurements, verification, what is out of scope, open
points, and implementation notes filled in as the step is built.

| # | Specification | Status | Depends on |
|---|---|---|---|
| 00 | [Infrastructure: CI, compatibility harness, the CPG submodule, test builds](00-infrastructure.md) | implemented; device run pending | |
| 01 | [Key-aware chord spelling](01-key-aware-spelling.md) | approved | 00 |
| 02 | [Chromatic chords through the progression path](02-chromatic-progression-path.md) | implemented; device run pending | 00 |
| 03 | [The eight missing sonorities](03-missing-sonorities.md) | draft: design proposals | 02 |
| 04 | [The palette table, generated from the CPG](04-palette-table.md) | draft: design proposals | 00, 03 |
| 05 | [The part's palette](05-part-palette.md) | draft: design proposals | 02, 04 |

## How far the drafting goes, and why

Specifications 00 to 02 are complete: the code determines them, and the owner approved them. Specifications 03 to 05
propose designs, which the implementation of the earlier steps will test and refine: the save format's fallbacks, the
generator's data layout, the palette's verbs. Their open points name what is expected to be learnt.

Steps 6 to 13 are not specified yet. They bring screens, whose costs depend on the skin system and the device, and they
follow milestone M2 (the engine complete), when a design note goes to Cadenza's author (study, §9). Specifying them now
would fix answers before the questions are understood.

## Status values

draft (for approval, or design proposals) → approved → implemented → verified on the device.

## Dependencies

```
00 infrastructure
 ├── 01 spelling
 └── 02 chromatic path
      └── 03 sonorities
           └── 04 palette table (also needs 00's submodule)
                └── 05 part's palette (also needs 02)
                     └── steps 6 to 10, not yet specified
```
