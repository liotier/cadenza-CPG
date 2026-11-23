# Implementation strategy

Status: strategy, no code. It turns the decisions of the [integration study](cpg-integration-study.md) (§10, decisions 1 to
20) into an order of work: principles, infrastructure, main steps, milestones, risks. Each step gets its own specification
in `docs/development/specs/` before any code is written.

## Contents

1. [Principles](#1-principles)
2. [Workflow and infrastructure](#2-workflow-and-infrastructure)
3. [Main steps](#3-main-steps)
4. [Milestones](#4-milestones)
5. [Risks](#5-risks)
6. [Not scheduled](#6-not-scheduled)

---

## 1. Principles

1. **Specification before code.** Every step starts as a specification in the format the CPG's
   [improvement specifications](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements)
   established: provenance, decisions, measurements, verification, implementation notes, status. The owner approves it
   before code begins.
2. **Engine before screens.** Each feature's engine part is built and tested offline first; its screen comes after, and is
   checked on the MPC One. Engine work needs no device: the core builds and passes its pinned parity check in a plain
   container (study, §2.1).
3. **Saved sound never moves.** The core parity hash ([tests/core_parity.sha256](../../tests/core_parity.sha256)) stays
   unchanged unless a step intends otherwise and says so in its specification; enumerations are appended, never
   renumbered; new behaviour takes a new identifier.
4. **Projects travel both ways.** A project saved by the fork opens in the official Cadenza 0.28.1, its fork-only features
   degraded, never refused (study, decision 11); a test checks it at every step that touches the saved state.
5. **The CPG stays the source of truth.** Its code reaches Cadenza through a pinned git submodule and a generator, never by
   hand (decisions 7 and 10); the generator refuses any chord it cannot represent exactly (decision 5).
6. **Small, separable steps.** Each step is a self-contained change that could be offered upstream on its own, with focused
   commits that can be cherry-picked; the spelling fix goes first.
7. **Budgets measured, not assumed.** Each specification records binary size, skin memory and parameter count before and
   after (study, §5.3); steps with screens add the device's CPU bench.
8. **Cadenza's way.** Harmony is sequenced inside Cadenza; no MIDI input (decision 20); everything happens on Cadenza's
   own screens.

## 2. Workflow and infrastructure

### 2.1 Specifications

One file per step, `docs/development/specs/NN-short-title.md`, numbered in the order of §3. A specification names what it
excludes as clearly as what it does, which keeps parked topics parked. Its status line moves from draft to approved, to
implemented, to verified on the device.

### 2.2 Branches

Proposed, to be set up with the owner's agreement:

| Branch | Role |
|---|---|
| `main` | mirrors upstream releases ([Shrekage2024/cadenza-force](https://github.com/Shrekage2024/cadenza-force)), nothing else |
| an integration branch, for instance `cpg` | collects the merged steps and `docs/development`; takes each new upstream release by merge |
| one branch per step | carries one specification's implementation, merged into the integration branch by pull request after review |

Until then, the documents stay on the present working branch.

### 2.3 Tests

| Level | What | Where |
|---|---|---|
| Engine | the existing offline suite ([tests/run_tests.sh](../../tests/run_tests.sh): theory, generator, core, the feel and tweak tests, the core parity trace), plus each step's new tests | natively with gcc, or with Docker as the script does; the plugin test needs the toolchain checkout (branch [`chordseq/open-mpc`](https://github.com/Shrekage2024/mpc-vst-plugins/tree/chordseq/open-mpc)) |
| Compatibility | states saved by the fork, loaded by the 0.28.1 engine built from upstream commit `56e95a0`: valid, and degraded only as the specification says | natively: the core is host-free, so both engines build side by side |
| Generator | the palette table regenerated from the pinned CPG and compared with the committed one | Node, as the CPG's own catalogue check |
| Device | the screens, the sound, the timing, on the MPC One, recorded as runs in the manner of [device-test-pads.md](device-test-pads.md) | the owner |

### 2.4 Continuous integration and builds

The export carries no CI; the fork adds it:

- **On every push:** the engine and compatibility tests, the core parity check, the generator check.
- **Test builds:** through the toolchain's reusable release workflow
  ([vst-release.yml](https://github.com/Shrekage2024/mpc-vst-plugins/blob/8fbeffc/.github/workflows/vst-release.yml)),
  which builds on Ubuntu runners with QEMU for ARM and packages with `release.py`, including the MPC OS 3.10.1
  `VST2 Plugins` folder, then attaches the zip to a draft release. The owner installs that zip on the MPC One. This settles
  who builds: CI, pinned to one toolchain commit.

### 2.5 Versions and identity

Fork builds keep Cadenza's plugin identity (decision 11) and so replace the official plugin on the device; their version in
[versions.json](../../versions.json) gains a suffix (for instance `0.28.1-cpg.1`), which names the zips and release notes
and tells builds apart. The official release's folder is kept on the side for restoring.

## 3. Main steps

### 3.1 Overview

| Step | Content | Study | Depends on | Screens | Size |
|---|---|---|---|---|---|
| 0 | Infrastructure: CI, compatibility harness, CPG submodule, version suffix | §2.4 here | | no | M |
| 1 | Key-aware chord spelling | avenue A | 0 | labels only | S-M |
| 2 | Chromatic chords through the progression path | §6.2 | 0 | no | S-M |
| 3 | The eight missing sonorities | avenue H | 2 | no | M |
| 4 | The palette table, generated from the CPG | avenues B, K | 0, 3 | no | S-M |
| 5 | The part's palette in the engine | §4.3, avenue D | 2, 4 | no | M |
| 6 | The palette browser | avenue C | 5 | yes | M-L |
| 7 | The palette in the bar editor | avenue D | 5 | yes | M |
| 8 | Live palette pads | avenue L | 5 | yes | M-L |
| 9 | Turnarounds | avenue D | 5 | a control | M |
| 10 | The palette walk | avenue E | 5 | a tile | M |
| 11 | Pedal voicing, then Smooth+ | avenue F, §6.6 | 0 | a mode | S-M each |
| 12 | Genre crosswalk | avenue G | 6 | suggestions | M |
| 13 | Heptatonic scales (optional) | avenue I | 3 | GLOBAL | M |

Steps 1 to 5 form phase A, the engine, buildable and testable without the device; steps 6 to 12 bring screens and need the
device.

### 3.2 Phase A: the engine

**Step 0. Infrastructure.** CI on every push (§2.4); the compatibility harness building the 0.28.1 engine beside the
fork's; the CPG added as a git submodule, `liotier/AkaiMPC` pinned to one commit, shallow or sparse; the version suffix.
*Done when* CI passes on the unchanged code, and a CI-built zip installs on the MPC One and behaves exactly as 0.28.1.

**Step 1. Key-aware chord spelling.** The CPG's spelling rules in [theory.c](../../src/theory.c): roots by degree, the
two-named tonic by fewer accidentals, chord tones by their degrees, ASCII as today. Display only: names are never saved.
*Done when* every key, scale and chord spells as the rules say (a test over all of them), the labels fit their cells (the
plugin test), the parity hash is unchanged, and the screens read right on the device. *Upstream:* the first offer, with a
field report for [tested.json](../../tested.json).

**Step 2. Chromatic chords through the progression path.** The six borrowed-only checks of the editing paths and the one
of the change-point loader opened to the existing chromatic range (study, §6.2). *Done when* chromatic chords survive
every writer, undo, copy, chord-length change and a save and load round trip; the parity hash is unchanged, no existing
path producing such chords; a fork-saved state loads in 0.28.1 as 0.28 already loads iOS-made chords.

**Step 3. The eight missing sonorities.** New chromatic qualities as an appended second range, new colours, an extra save
field with a fallback colour in `.ch`, a versioned Feel base code, names, numerals, drift scores (study, avenue H). The
fifthless 11ths and 13ths stay as they are (decision 12). *Done when* every pad of every catalogue palette, in all six
styles, is representable exactly, which step 4's strict generator then confirms on each regeneration; and the
compatibility test shows 0.28.1 loading such states degraded, never refused.

**Step 4. The palette table.** A Node generator in `tools/`, reading the CPG through the submodule and writing a header in
`src/`: per catalogue entry a stable identifier (from genre and template value), genre, English nickname, tonality, its
sixteen palette chords as root, quality and colour with spice and category, and its starting order laid out by the
spare-time rule (decision 4); plus one palette per Cadenza scale from the CPG's Scale Mode (decision 18). *Done when* the
generator is strict without exceptions, CI regenerates and compares, and the binary stays within budget.

**Step 5. The part's palette.** A palette per part, saved by value with its provenance (study, §4.3); a verb that loads a
palette and its starting order through Feel's store, keeping locks; the scale palette as the default of a part without
one; the nearest catalogue palette for a progression built from scratch (decision 18). *Done when* the verbs and their undo
are tested, the parity hash is unchanged for projects without palettes, and the compatibility test passes. *Upstream:* the
end of phase A is the moment for a design note to the author, before any screen (study, §9).

### 3.3 Phase B: palettes on screen

**Step 6. The palette browser.** The PROGRESSIONS browser reused: a genre selector in place of its static row labels, the
palettes filtered by the project scale's family (decision 6), sixteen tiles a page, DONE and CANCEL as today (study,
avenue C). *Done when* every genre and palette is reachable, the skin needs no new full-panel image (the gauge says so),
and the device run passes.

**Step 7. The palette in the bar editor.** A source toggle on the BORROW row, SCALE · BORROW · PALETTE, chords coloured by
spice and ranked by how smoothly they connect their neighbours (study, avenue D and §6.6). *Done when* palette chords can
be picked on any hit, mid-bar included, and the device run passes. Phase B ends with the first fork build worth playing
with.

### 3.4 Phase C: performance

**Step 8. Live palette pads.** A PALETTE view of sixteen pads with the PROGRESSIONS browser's behaviour for single chords:
each tap lands at SWITCH AT and holds until the next, KEEP captures the last loop's worth of the performance, CANCEL
restores (decision 17). The engine's new piece is a harmony queue that swaps all three lanes at one boundary. *Done when*
taps land in time at every SWITCH AT setting and tempo, capture writes exactly what sounded, and the device run passes.

**Step 9. Turnarounds.** Palette chords tagged by their function relative to the next phrase's first chord, ranked by the
relation's strength, then voice-leading distance, then a seed; a control choosing the family, plagal, dominant or
chromatic (study, avenue D). *Done when* the choice is deterministic, the genre's idiom is inherited from the palette, and
the device run passes.

### 3.5 Phase D: generation and colour

**Step 10. The palette walk.** A seeded generator version beside `gen_progression_v2`, walking the part's palette by
common tones, voice-leading distance and root motion, constrained to start on the tonic and lead home (study, avenue E).
*Done when* the same seed gives the same walk, locks are kept, and a tile starts it.

**Step 11. Pedal voicing, then Smooth+.** The CPG's Pedal treatment as an appended voicing mode; then Smooth+, assignment
for chords of unequal size with the CPG's preferences, adopted only if it measures better than Smooth over the whole
catalogue (study, avenue F, §6.6). Independent of the palettes: it can move earlier if wanted.

**Step 12. Genre crosswalk.** Loading a palette offers, without imposing, a matching voicing style and bass style (study,
avenue G).

**Step 13 (optional). Heptatonic scales.** The twelve seven-note scales the CPG adds, each with its three borrowed chords,
and the GLOBAL selector to match (study, avenue I).

## 4. Milestones

| Milestone | After step | What it delivers |
|---|---|---|
| M1 | 1 | the first upstream offer: spelling, with a field report from the MPC One |
| M2 | 5 | the engine complete: palettes, chords on any root, the full vocabulary, all tested offline; a design note to the author |
| M3 | 7 | palettes usable on the MPC One: browser and bar editor, the first fork build worth playing with |
| M4 | 9 | performance: live palette pads and turnarounds |
| M5 | 12 | generation and colour: the palette walk, the new voicings, the crosswalk |

## 5. Risks

| Risk | Mitigation |
|---|---|
| Upstream releases reshape `seq_core` | changes kept small and local; each upstream release merged promptly into the integration branch; generated data apart from logic |
| A save-format change breaks projects | the compatibility harness against 0.28.1 at every step touching the state; the never-refused rule |
| The skin budget runs out | existing views reused; the gauge run in every CI build |
| Faults visible only on the device | a device run per step with screens or timing; CI zips that install as they are |
| A CPG change breaks generation | the pinned submodule, bumped deliberately; the strict generator fails loudly and names the chord |
| The licence | builds for testing only, the fork being personal; a conversation with the author before screens (study, §9) |
| Scope creep | each specification lists what it excludes; parked topics stay parked |

## 6. Not scheduled

Set aside by decision, or parked: parts to clips (parked, study §7); MIDI input and hardware pads (decision 20);
`.progression` import and export (decision 16); a C port of the palette builder (decision 18); Euclidean rhythms and
polymeter, the Chord Matcher (study, §7).
