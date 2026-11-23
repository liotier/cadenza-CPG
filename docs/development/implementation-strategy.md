# Implementation strategy

Status: strategy, revision 2, after steps 0 to 2 (the [revision history](#revision-history) closes the document). It turns
the decisions of the [integration study](cpg-integration-study.md) (§10, decisions 1 to 21) into an order of work:
principles, infrastructure, budgets, main steps, milestones, the owner's pending decisions, next steps, risks. Each step
gets its own specification in `docs/development/specs/` before any code is written.

**The decisions waiting for the owner are listed in one place: [§8](#8-decisions-for-the-owner).**

## Contents

1. [Where the work stands](#1-where-the-work-stands)
2. [Principles](#2-principles)
3. [What steps 0 to 2 taught](#3-what-steps-0-to-2-taught)
4. [Workflow and infrastructure](#4-workflow-and-infrastructure)
5. [Budgets](#5-budgets)
6. [Main steps](#6-main-steps)
7. [Milestones](#7-milestones)
8. [Decisions for the owner](#8-decisions-for-the-owner)
9. [Next steps](#9-next-steps)
10. [Risks](#10-risks)
11. [Not scheduled](#11-not-scheduled)

---

## 1. Where the work stands

| Step | State | Evidence |
|---|---|---|
| 0. Infrastructure | implemented; device run pending | [spec 00](specs/00-infrastructure.md); CI run 1 green; the CI build byte-identical to the official 0.28.1 |
| 1. Key-aware spelling | implemented; device run pending | [spec 01](specs/01-key-aware-spelling.md); over the old binary budget (CI run 4), then decision 21 |
| 2. Chromatic chords through the progression path | implemented; device run pending | [spec 02](specs/02-chromatic-progression-path.md); CI run 3 green |
| Decision 21 | room in the binary: dead code dropped, budget 720 KB | test build 4 (commit `d9b619d`): 650 KB |
| 3 to 5 | specified, for approval | [specs 03 to 05](specs/README.md), revised with what steps 0 to 2 measured |

The draft release *Cadenza 0.28.1-cpg.1 (test build)* holds steps 0 to 2 and the collected binary: one device run
checks all four at once (§8, O1).

## 2. Principles

1. **Specification before code.** Every step starts as a specification in the format the CPG's
   [improvement specifications](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements)
   established: provenance, decisions, measurements, verification, implementation notes, status. The owner approves it
   before code begins; deviations found while building are recorded in its implementation notes.
2. **Engine before screens.** Each feature's engine part is built and tested offline first; its screen comes after, and is
   checked on the MPC One.
3. **Saved sound never moves.** The core parity hash ([tests/core_parity.sha256](../../tests/core_parity.sha256)) stays
   unchanged unless a step intends otherwise and says so; enumerations are appended, never renumbered; new behaviour takes
   a new identifier.
4. **Projects travel both ways.** A project saved by the fork opens in the official 0.28.1, its fork-only features
   degraded, never refused (decision 11). The fields an older build reads hold only values it can *use*, not merely
   parse: an old build checks some fields' ranges and trusts others (§3, lesson 8). The compatibility harness checks every
   step that touches the saved state.
5. **The CPG stays the source of truth.** Its code reaches Cadenza through the pinned submodule and a generator, never by
   hand (decisions 7 and 10); the generator refuses any chord it cannot represent exactly (decision 5).
6. **Small, separable steps.** Each step could be offered upstream on its own, as focused commits that can be
   cherry-picked.
7. **Budgets measured, not assumed.** Each specification starts from the last CI measurement, not from recorded notes;
   it estimates its own cost locally (the ARM cross-compiler, §4.6) before pushing, and CI's figure is the authority.
8. **Cadenza's way.** Harmony is sequenced inside Cadenza; no MIDI input (decision 20); everything happens on Cadenza's
   own screens.
9. **The author's tests are kept, not bypassed.** When a step changes behaviour that an existing check pins, the check is
   changed with a comment naming the specification, never deleted; the specification lists those checks.

## 3. What steps 0 to 2 taught

| # | Lesson | Consequence, now practice |
|---|---|---|
| 1 | Recorded figures go stale: `vst/budget.json`'s notes said 666 KB and 78.8 MB; 0.28.1 measures 686 KB and 79.1 MB. The study's "22 KB of headroom" was 2.5 KB. | Principle 7: every specification starts from CI's last figures. |
| 2 | The binary budget bound first. Spelling alone cost about 4 KB on the device's build (690 KB against 688, CI run 4). Dropping the functions the plugin never calls freed about 40 KB (decision 21). | 70 KB of headroom now (§5); every step estimates its size before pushing. |
| 3 | Builds are reproducible: at the pinned toolchain, CI's binary was byte-identical to the official release. | An upstream merge can be verified the same way; since decision 21 the fork's own binary differs, but the same source still gives the same bytes. |
| 4 | 0.28.1's save, loaded and saved again, is not always the same bytes: a loop shortened after its drawn bars were filled saves the hidden bars once. | Round-trip tests compare chords and points and require the re-save to be a fixed point; the harness compares the official re-save with the fork's own. |
| 5 | The author's tests pin behaviour: steps 1 and 2 each moved three checks. | Principle 9. |
| 6 | One engine serves several front ends: some functions have no caller in the Force plugin (`seq_tweak_point_ok`, `theory_chord_card`) and serve the iOS app or the browser version. | Each specification lists its effects on other front ends, for the conversation with the author. With dead code dropped, an API's completeness no longer costs the plugin anything. |
| 7 | Names follow Cadenza's numerals, and `theory_numeral` prefers the flat of the degree above: D minor's major chord on F♯ reads `Gb7`. Of the 2 804 Classic pads Cadenza can voice, 13 are spelled differently from the CPG in C (F♯→G♭ 5, D♯→E♭ 5, C♯→D♭ 3). | An open point for the author (numerals are upstream behaviour); step 4 measures the disagreement for the whole table. |
| 8 | Old builds trust some fields: 0.28.1 decodes a Feel base code without checking its colour against the colour tables, and a change point's source is a `signed char` (127 at most). | Principle 4's "use, not parse"; step 3's save design follows from it. |
| 9 | This cloud session has no Docker daemon: the toolchain's scripts run only in CI. A native test runner and an ARM cross-compiler did the local work. | Step 0a commits both as tools (§4.6). |
| 10 | GitHub offers manual runs only for workflows on the default branch, and `main` holds upstream's tree. | Test builds start from a commit message marker (§4.4). |
| 11 | The CPU bench needs root SSH on the device (`tools/bench.sh`). I assumed the owner's MPC One had none; it has (owner, 2026-11-23). | The device bench and an MPC memory probe are possible: the CPU and skin budgets can be measured on the device, not only by offline proxies (§5). Ask before assuming what the owner's setup allows. |
| 12 | 30 Classic pads exist in Cadenza only as some scale's own chord (24 half-diminished sevenths, 3 minor-major sevenths, 3 minor sixths), not as chromatic chords. | Step 3's new qualities serve twice: the 134 Classic pads Cadenza cannot voice at all, and the palette table's storage independent of the scale (step 4). |
| 13 | The scale palettes of decision 18 (the CPG's Scale Mode) hold a chord the catalogue does not: the tonic's dominant 7sus4 with its fifth, in Minor, Dorian, Mixolydian and Phrygian. | A ninth sonority in step 3, as a colour; measure every source of data a step will use before specifying it. |

## 4. Workflow and infrastructure

### 4.1 Specifications

One file per step, `docs/development/specs/NN-short-title.md`, listed in [specs/README.md](specs/README.md). Status:
draft → approved → implemented → verified on the device. Steps 3 to 5 are specified for approval; steps 6 onwards will be
specified after milestone M2 (§6.3).

Decisions put to the owner come with their dependencies: which decision another presupposes, which one's answer
informs another, and which are independent, so that they can be taken in a sensible order and none is taken on a
premise a pending one may overturn. A list of decisions given without that structure (as at the end of revision 2's
first presentation) reads as independent when it is not.

### 4.2 Branches

| Branch | Role |
|---|---|
| `main` | takes pull requests from tested release branches only |
| one Claude branch, at present `claude/gifted-carson-j102ab` | the development line: specifications, code, tests, CI |
| a release branch per tested build, `release/0.28.1-cpg.N` | cut from the Claude branch once a build has passed its device run, with the owner's go; merged into `main` by pull request |

Upstream releases are fetched from [Shrekage2024/cadenza-force](https://github.com/Shrekage2024/cadenza-force) and merged
into the Claude branch. A new Claude session is told to continue on the existing Claude branch.

### 4.3 Tests

| Level | What | Where |
|---|---|---|
| Engine and plugin | [tests/run_tests.sh](../../tests/run_tests.sh), with each step's tests added to it | CI (Docker); locally with the native runner (§4.6) |
| Compatibility | [tests/compat/run.sh](../../tests/compat/run.sh): each scenario saved by the fork, loaded and saved by 0.28.1 (`56e95a0`), identical or as [expected.txt](../../tests/compat/expected.txt) pins | CI and locally |
| Generator | [tools/cpg/coverage.sh](../../tools/cpg/coverage.sh) now; the palette table's regeneration from step 4 | CI and locally (Node) |
| Device | screens, sound, timing on the MPC One, recorded as runs | the owner |

### 4.4 Continuous integration and builds

- [ci.yml](../../.github/workflows/ci.yml), on every push to the Claude branch and to `release/*`, and on pull requests
  to `main`: the toolchain's build with its budget check, the offline suite, the compatibility harness, the coverage
  check. About 13 minutes.
- [build.yml](../../.github/workflows/build.yml): a test build, as a **draft release only** (no run artifacts: anyone
  signed in can download those from a public repository). It runs on a push to the Claude branch whose head commit
  message holds `[test build]`, and on every push of a `release/*` branch.
- A draft is named by the version: test builds of one version replace each other's draft until a device run passes;
  then the release branch freezes that version and `versions.json` moves to the next (§4.5).

### 4.5 Versions and identity

Fork builds keep Cadenza's plugin identity (decision 11) and replace the official plugin on the device; `versions.json`
holds `0.28.1-cpg.N`, which names the zips, the drafts and the release branches. `N` moves to `N + 1` when
`release/0.28.1-cpg.N` is cut. The official release's zip is kept for restoring.

### 4.6 Local tools (step 0a)

The toolchain's scripts need Docker, which a cloud session lacks. Two tools, used for steps 1 and 2 and to be committed
under `tools/dev/` ([spec 00, addendum A](specs/00-infrastructure.md#addendum-a-local-tools)): a native runner that
executes `tests/run_tests.sh`'s steps with the host's gcc, and an ARM size estimate that builds the plugin with a
cross-compiler and the toolchain's flags, before and after a change. Their figures guide; CI's decide.

### 4.7 A web simulation (proposed)

Upstream's browser version (cadenzajam.com/try, built from upstream commit `2b2862a` with the toolchain at `8fbeffc`)
runs the release's own engine and wrapper as a standalone WebAssembly module (`chordseq.wasm`: two imports, MIDI out and
a clock; fifteen `wh_*` exports) behind a small C host, `web/web_host.c`, built by `web/build.sh`; the page draws the
built skin (`TUI.json` and its images) with a renderer its comment calls not specific to Cadenza, and plays the lanes
with SoundFonts in an AudioWorklet. None of `web/`, `tests/web_*` or `tools/release_web.sh` is in the release export;
the page's JavaScript is readable on the site but carries no licence granting its use; `web_host.c` and `build.sh` are
not published.

**Can `web_host.c` be inferred from the calls to it?** Not entirely. Its interface is fully visible: the published page
names every function, its arguments and what it returns. Part of its behaviour can be inferred: the toolchain's wrapper
is open, the author's own test hosts (`tests/test_plugin.c`, the toolchain's `host_test.c`) imitate MPC's touches and
Q-Links, and the page shows when each call is made. One essential mechanism is not visible: in this export the engine
sends its MIDI only to an ALSA port it opens with `dlopen` (`engine.c`, `alsa_open`), and nothing in it reaches the
module's `host_midi` import, so the web build routes MIDI by a means the export does not show. A rewrite would design its
own and could match the published module only by testing against it, which means running code no licence grants us.
Route chosen by the owner, 2026-11-23 (§8, O8): ask the author for `web/`; a rewrite only if he declines.

**Is the compiler a risk?** Measured, 2026-11-23: the published module was built with Emscripten in standalone mode (its
exports name it; its compiler records are stripped). With an unrelated C-to-wasm toolchain, Zig 0.17.1's clang and WASI
libc, the core parity trace compiled to wasm and run in Node gives the pinned hash, byte for byte; and the whole plugin,
engine and toolchain wrapper with `vst.json`'s defines, links needing one platform import, `clock_time_get`, the one the
published module also imports beside its own MIDI callback. The engine is written to compute the same on every target,
and the parity test proves it there. What a rewrite could get wrong is not the compiler but the host's imitation of MPC
in `web_host.c` (when texts are read, what a touch and a Q-Link send, which parameters are polled), and the published page
already encodes that behaviour on its side of the interface.

## 5. Budgets

| Budget | Limit | Now | Headroom | Checked by |
|---|---|---|---|---|
| Binary | 720 KB (decision 21) | 650 KB (test build 4, `d9b619d`) | about 70 KB | CI, every build |
| Skin images, decoded | 80 MB | 79.1 MB | **0.9 MB** | CI, every build |
| Parameters | none set | 2 037 | n/a | each costs about 110 bytes of binary (0.22: 152 parameters, about 17 KB) and, when polled, audio-thread time |
| Audio p99, worst block | 49 %, 80 % | 24.2 %, 65.5 % (the last device bench, [vst/bench.txt](../../vst/bench.txt), the author's Force) | | `tools/bench.sh` over the owner's root SSH, on the MPC One |
| Screen updates | 80 a second | 71 a second at 160 BPM with FOLLOW on (the plugin test) | | offline |
| Saved state | 8 KB | about 1.6 KB for a six-part genre preset | | offline |

**Planned spending.** Step 3 about 2 to 4 KB of binary; step 4's table 12 to 15 KB; step 5 about 2 to 4 KB: some 25 KB
of the 70. The screens of phase B cost parameters and images. The skin is now the binding budget: 0.9 MB, where a new
full-panel view has cost 1.6 MB or more. Phase B's screens therefore reuse existing views, and any new view needs a
decision on the fork's skin budget first (§8, O10). The skin budget rests on a device probe ("MPC decodes the visible page
only", VST-9); with root SSH, MPC's memory can be measured on the owner's MPC One before and after a new view, so that
decision can rest on the fork's own figures.

## 6. Main steps

### 6.1 Overview

| Step | Content | Study | Depends on | Screens | Size | State |
|---|---|---|---|---|---|---|
| 0 | Infrastructure | | | no | M | implemented |
| 0a | Local tools: native runner, ARM size estimate | §4.6 | 0 | no | S | for approval |
| 1 | Key-aware chord spelling | avenue A | 0 | labels | S-M | implemented |
| 2 | Chromatic chords through the progression path | §6.2 | 0 | no | S-M | implemented |
| 3 | The missing sonorities: the catalogue's eight and Scale Mode's 7sus4 | avenue H | 2 | no | M | for approval |
| 4 | The palette table, generated from the CPG | avenues B, K | 0, 3 | no | M | for approval |
| 5 | The part's palette in the engine | §4.3, avenue D | 2, 4 | no | M | for approval |
| 6 | The palette browser | avenue C | 5 | yes | M-L | after M2 |
| 7 | The palette in the bar editor | avenue D | 5 | yes | M | after M2 |
| 8 | Live palette pads | avenue L | 5 | yes | M-L | after M2 |
| 9 | Turnarounds | avenue D | 5 | a control | M | after M2 |
| 10 | The palette walk | avenue E | 5 | a tile | M | after M2 |
| 11 | Pedal voicing, then Smooth+ | avenue F, §6.6 | 0 | a mode | S-M each | after M2 |
| 12 | Genre crosswalk | avenue G | 6 | suggestions | M | after M2 |
| 13 | Heptatonic scales (optional) | avenue I | 3 | GLOBAL | M | after M2 |

### 6.2 Phase A: the engine

**Step 0a. Local tools.** The native runner and the ARM size estimate as committed tools; CI's summary gains the exact
binary size in bytes and the headroom. *Done when* both reproduce, locally, what CI reports for the current commit.

**Step 3. The missing sonorities** ([spec 03](specs/03-missing-sonorities.md)). Five chromatic qualities in a second
range of sources (52 to 111) and three colours outside the controls' set (quartal, seventh without fifth, 7sus4), each
saved exactly in new fields with a fallback older builds can use, chosen by one rule. *Done when* the coverage tool
reports no pad needing a new sonority, every Classic pad with a chromatic form (lesson 12) and every Scale Mode chord
voiced (lesson 13); the parity hash is unchanged; and 0.28.1 loads every such state with its fallbacks.

**Step 4. The palette table** ([spec 04](specs/04-palette-table.md)). A Node generator writing `src/cpg_palettes.c`,
linked by the plugin's front end only: per catalogue entry an identifier, genre, nickname, tonality, sixteen chords stored
independently of the scale, a starting order; a palette per Cadenza scale, relative to it; and a report comparing
Cadenza's spellings with the CPG's. *Done when* the generator is strict without exceptions, CI regenerates and compares,
every stored chord voices the CPG's pitch classes, and the binary stays within budget.

**Step 5. The part's palette** ([spec 05](specs/05-part-palette.md)). A palette per part, held by value in the core and
saved; loading a palette and its starting order in one undo step; in the front end, the scale palette shown for a part
without one and the nearest catalogue palette as a query, so the core never links the table. *Done when* the verbs, their
undo, the save and the compatibility harness pass, and the parity hash is unchanged for projects without palettes. The end of phase A, milestone M2: a design note to the author before any screen (study, §9).

### 6.3 Phases B to D: screens, performance, generation

Specified after M2, each from the budgets of §5 at that moment.

**Step 6. The palette browser.** The PROGRESSIONS browser reused: a genre selector in place of its static row labels, the
palettes filtered by the project scale's family (decision 6), sixteen tiles a page, DONE and CANCEL as today (study,
avenue C). Its view and tiles exist: no new image, few new parameters. *Done when* every genre and palette is reachable,
the skin gauge shows no new full-panel image, and the device run passes.

**Step 7. The palette in the bar editor.** A source toggle on the BORROW row, SCALE · BORROW · PALETTE, chords coloured by
spice and ranked by how smoothly they connect their neighbours (study, avenue D and §6.6). *Done when* palette chords can
be picked on any hit, mid-bar included, and the device run passes. Phase B ends with the first fork build worth playing
with.

**Step 8. Live palette pads.** A PALETTE view of sixteen pads with the PROGRESSIONS browser's behaviour for single chords:
each tap lands at SWITCH AT and holds until the next, KEEP captures the last loop's worth, CANCEL restores (decision 17).
The engine's new piece is a harmony queue that swaps all three lanes at one boundary. Probably a view of its own: the
first screen to need the skin decision (§8, O10). *Done when* taps land in time at every SWITCH AT setting and tempo, capture
writes exactly what sounded, and the device run passes.

**Step 9. Turnarounds.** Palette chords tagged by their function relative to the next phrase's first chord, ranked by the
relation's strength, then voice-leading distance, then a seed; a control choosing the family, plagal, dominant or
chromatic (study, avenue D). *Done when* the choice is deterministic and the device run passes.

**Step 10. The palette walk.** A seeded generator version beside `gen_progression_v2`, walking the part's palette by
common tones, voice-leading distance and root motion, starting on the tonic and leading home (study, avenue E). *Done
when* the same seed gives the same walk, locks are kept, and a tile starts it.

**Step 11. Pedal voicing, then Smooth+.** The CPG's Pedal treatment as an appended voicing mode; then Smooth+, adopted only
if it measures better than Smooth over the whole catalogue (study, avenue F, §6.6). Independent of the palettes: it can
move earlier if wanted.

**Step 12. Genre crosswalk.** Loading a palette offers, without imposing, a matching voicing style and bass style (study,
avenue G).

**Step 13 (optional). Heptatonic scales.** The twelve seven-note scales the CPG adds, each with its three borrowed chords,
and the GLOBAL selector to match (study, avenue I).

## 7. Milestones

| Milestone | After step | What it delivers |
|---|---|---|
| M1 | 1, and its device run | the first upstream offer, as two patches: dropping dead code from the ARM build, which frees about 40 KB, and the spelling, which needs about 4 KB; with a field report from the MPC One |
| M2 | 5 | the engine complete: palettes, chords on any root, the full vocabulary, all tested offline; a design note to the author |
| M3 | 7 | palettes usable on the MPC One: browser and bar editor |
| M4 | 9 | performance: live palette pads and turnarounds |
| M5 | 12 | generation and colour: the palette walk, the new voicings, the crosswalk |

## 8. Decisions for the owner

Every decision only the owner can take, as of 2026-11-23, with what it rests on (a decision it presupposes) and what it
informs (a decision whose answer it changes). Nothing else is waiting for the owner. Taken decisions are recorded in the
study's [decision log](cpg-integration-study.md#10-decision-log).

| # | Decision | Options | Rests on | Informs | Needed before |
|---|---|---|---|---|---|
| O1 | Run the draft *Cadenza 0.28.1-cpg.1 (test build)* on the MPC One, and say whether it passes (checks in §9, item 1) | pass, or what failed | none | O2, O3 | the release branch; M1 |
| O2 | Cut `release/0.28.1-cpg.1` and open its pull request to `main` | go, or not yet | O1 passed | | the next version, `0.28.1-cpg.2` |
| O3 | Offer milestone M1 to the author: the dead-code patch (about 40 KB freed) and the spelling (about 4 KB), with a field report | now, with O9's message; later; not at all | O1 passed | | nothing in the fork |
| O4 | Approve step 0a, the local tools ([spec 00, addendum A](specs/00-infrastructure.md#addendum-a-local-tools)): its two choices, A1 (the native runner) and A4 (the ARM size estimate) | approve, amend, refuse | none | | step 0a |
| O5 | Approve step 3, the missing sonorities ([spec 03](specs/03-missing-sonorities.md)) | approve, amend | none | O6, O7 | step 3 |
| O6 | Approve step 4, the palette table ([spec 04](specs/04-palette-table.md)) | approve, amend | O5: its chord storage presupposes spec 03 | O7 | step 4 |
| O7 | Approve step 5, the part's palette ([spec 05](specs/05-part-palette.md)) | approve, amend | O5, O6 | | step 5 |
| O8 | The web simulation's route (§4.7) | **taken: ask the author for `web/`**; if he declines or does not answer, decide between a rewrite (B, with the licence question of testing against the published module) and a page of our own (C) | none | O10 | any screen work in a browser |
| O9 | Send the author the message drafted for O8 (introducing the fork, asking for `web/`) | send as drafted, edit, hold | none | O3, O11 | the author's answer to O8 |
| O10 | The fork's skin budget: may new views raise it, and by how much | to take at milestone M2, after an MPC memory probe over root SSH with and without a new view | the probe; specifications 06 to 08 | | the first screen needing a new view (likely step 8) |
| O11 | Whether to raise with the author the numerals that spell D minor's F♯ major as `Gb7` ([spec 01](specs/01-key-aware-spelling.md), open point) | raise it, leave it | none | | nothing in the fork; best in O9's message or the M2 design note |

Order that respects the dependencies: O1 and O4 to O7 can be taken now, O5 before O6 before O7; O9 now if O3 and O11
ride with it; O2 and O3 after O1; O10 at M2.

## 9. Next steps

In order:

1. **The device run of the draft** *Cadenza 0.28.1-cpg.1 (test build)* (O1). It holds steps 0 to 2 and the collected
   binary. Checks: the plugin loads from `VST2 Plugins` under MPC OS 3.10.1; INIT and House 124 play; a project saves and
   reopens, and opens again after the official 0.28.1 is restored; CHORDS, PHRASE, PLAY and GLOBAL read correctly in F,
   E♭, A♭ and D♭ major and C♯ and D♯ minor (PLAY's title: `Eb MAJOR`, `D# MINOR`). Step 2 shows nothing.
2. **If it passes:** the release branch and its pull request (O2); `versions.json` to `0.28.1-cpg.2`; M1's offer (O3).
3. **Step 0a**, once O4 is taken: small, independent of the device run.
4. **Step 3**, then **step 4**, then **step 5**, each once its approval (O5 to O7) is given, each pushed with CI green
   before the next starts. Step 4 can begin its generator while step 3 is built; it cannot pass its strictness check
   before step 3 lands.
5. **The web simulation**, once the author answers (O8, O9).
6. **At M2:** the MPC memory probe over root SSH, the skin budget (O10), the design note to the author, then
   specifications 06 and 07.

## 10. Risks

| Risk | Mitigation |
|---|---|
| The skin budget runs out: 0.9 MB of headroom, a full-panel view costing 1.6 MB or more | views reused; a decision on the fork's skin budget before any new view (§8, O10); the gauge in every CI build |
| The binary budget runs out again | 70 KB of headroom after decision 21; each step estimated locally, measured in CI; further raises are the owner's |
| CPU costs invisible offline | the device bench over root SSH at each step with screens; offline, few polled parameters and the plugin test's screen-update count |
| A save-format change breaks projects in older builds | principle 4 (old fields hold usable values only); the harness at every step touching the state |
| Upstream releases reshape `seq_core` | small, local changes; upstream releases merged promptly; generated data kept apart from logic |
| Changes reach front ends the fork does not build | each specification lists them (lesson 6); the design note to the author |
| Faults visible only on the device | a device run per step with screens or timing; CI zips that install as they are |
| A CPG change breaks generation | the pinned submodule, bumped deliberately; the strict generator fails loudly |
| The licence | builds for testing only, the fork being personal; a conversation with the author before screens (study, §9) |
| Scope creep | each specification lists what it excludes; parked topics stay parked |

## 11. Not scheduled

Set aside by decision, or parked: parts to clips (parked, study §7); MIDI input and hardware pads (decision 20);
`.progression` import and export (decision 16); a C port of the palette builder (decision 18); Euclidean rhythms and
polymeter, the Chord Matcher (study, §7).

---

## Revision history

- Revision 1 set the principles, the workflow and infrastructure, the fourteen main steps in four phases, the milestones
  and the risks, and adopted the owner's branch model.
- Revision 2, after steps 0 to 2: where the work stands (§1); thirteen lessons and the practice each led to (§3); CI, test
  builds and versions as built (§4.4, §4.5); local tools as step 0a (§4.6); a budgets table from CI's measurements, with
  the skin as the binding budget (§5); steps 3 to 5 restated from their revised specifications, and the budget each later
  screen is known to need (§6); M1 as two patches (§7); next steps in order (§8); risks for the skin, the CPU bench and
  other front ends (§9). Amended the same day: root SSH is available (lesson 11 corrected; the device bench and a memory
  probe become the measures of CPU and skin); decisions are put with their dependencies (§4.1); the web simulation the
  owner proposed, with what upstream's browser version is made of (§4.7, §8). Amended again: the owner's pending
  decisions gathered in one table with their dependencies (§8, O1 to O11), next steps renumbered (§9); whether
  `web_host.c` can be inferred from its calls (§4.7), and the route the owner chose.
