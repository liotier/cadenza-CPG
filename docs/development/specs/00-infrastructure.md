# 00 - Infrastructure: CI, compatibility harness, the CPG submodule, test builds

| | |
|---|---|
| Status | draft, for the owner's approval |
| Type | infrastructure |
| Size | M |
| Depends on | nothing |
| Blocks | every other step |

## 1. Purpose

The fork can neither test itself automatically, nor check that its projects still open in the official Cadenza, nor reach
the CPG, nor tell its builds apart. Step 0 adds the four, and changes nothing in how Cadenza behaves.

Today:

- **No CI.** The release export carries no `.github` folder. The offline suite exists ([tests/run_tests.sh](../../../tests/run_tests.sh))
  but runs only where someone runs it.
- **No compatibility check.** Nothing verifies that a state saved by a modified engine loads in the official 0.28.1, which
  decision 11 of the study requires ("degraded, never refused").
- **No CPG.** Steps 3 and 4 need the CPG's modules at a pinned commit (decisions 7 and 10).
- **No build identity.** A fork build installs under Cadenza's own identity (decision 11); only its version can tell it
  apart, and [versions.json](../../../versions.json) says `0.28.1`.

## 2. Decisions

| # | Decision | Rationale |
|---|---|---|
| D1 | **No change to Cadenza's behaviour.** Step 0 adds files under `.github/`, `tests/compat/`, `tools/cpg/`, `third_party/` and `docs/development/measurements/`, and edits `versions.json`; nothing under `src/` or `vst/` changes. | Infrastructure must be proven on unchanged code, so that later differences belong to later steps. |
| D2 | **CI runs the existing scripts unchanged:** `vst/build.sh`, then `tests/run_tests.sh`, on GitHub's Ubuntu runners with Docker and QEMU. | The scripts are the author's own definition of a passing build; running them as they are keeps the fork comparable with upstream. The plugin test needs `vst/build.sh` first: it includes `keyflags.h`, which the skin build generates. |
| D3 | **The toolchain is pinned** to [Shrekage2024/mpc-vst-plugins](https://github.com/Shrekage2024/mpc-vst-plugins/tree/chordseq/open-mpc) commit `8fbeffc`, branch `chordseq/open-mpc`, named in one place. | The README requires that branch; `8fbeffc`'s own message names Cadenza 0.28.1. One place means one edit to move it. |
| D4 | **The compatibility harness builds two engines:** the fork's, and the official one from upstream commit `56e95a0`, as two separate programs. | The engine is host-free, so both build natively in seconds; two programs avoid every symbol clash. |
| D5 | **Its baseline is byte identity:** a state saved by the fork, loaded by the official engine and saved again, gives the same bytes. Later steps add scenarios whose permitted differences their specifications list. | On unchanged code, anything but identity is a fault of the harness. |
| D6 | **The CPG is a git submodule:** `third_party/AkaiMPC`, from `https://github.com/liotier/AkaiMPC`, pinned to `c84a306`, the commit the study measured; `shallow = true`. The plugin build never needs it. | Decision 10. Pinning the studied commit makes the first measurement reproduce the study's figures. |
| D7 | **The study's coverage measurement becomes a tool:** `tools/cpg/enumerate.c` lists every chord Cadenza's theory layer can voice; `tools/cpg/coverage.mjs` classifies the CPG's catalogue against it; the output is committed as `docs/development/measurements/coverage.txt` and compared by CI. | It makes the study's figures reproducible, and gives steps 3 and 4 their measure: the count of missing sonorities, which they drive to zero. |
| D8 | **Test builds go through the toolchain's reusable workflow** ([vst-release.yml](https://github.com/Shrekage2024/mpc-vst-plugins/blob/8fbeffc/.github/workflows/vst-release.yml)) at the pinned commit, **as draft releases only.** | It builds and packages exactly as releases are made, the MPC OS 3.10.1 `VST2 Plugins` folder included. A draft is visible to the repository's collaborators only, whereas run artifacts of a public repository can be downloaded by any signed-in user; builds of a personal fork stay private (study, §9). |
| D9 | **Versions carry a suffix:** the Claude branch holds the next release's version, `0.28.1-cpg.N`; a release branch `release/0.28.1-cpg.N` freezes it; the Claude branch then moves to `N + 1`. The series starts at `0.28.1-cpg.1`. | The plugin shows no version on screen, so the zips' and drafts' names are what tell builds apart. |

## 3. Specification

### 3.1 CI: `.github/workflows/ci.yml`

Triggers: push to the Claude branch, pull requests to `main`.

| Job | Steps |
|---|---|
| `build-and-test` | check out the fork; check out the toolchain at `8fbeffc` beside it (`MPC_VST`); set up QEMU; `vst/build.sh`; `tests/run_tests.sh` (it must end with PASSED) |
| `compat` | check out with enough history to reach `56e95a0`; build and run the harness (§3.2) |
| `cpg` | check out with the submodule; build `tools/cpg/enumerate.c`; run `tools/cpg/coverage.mjs`; compare with `docs/development/measurements/coverage.txt` |

The budget check runs inside `vst/build.sh` (`tests/budget.sh --offline`), so the binary and skin budgets are checked on
every push.

### 3.2 Compatibility harness: `tests/compat/`

- `tests/compat/run.sh` extracts `src/` at `56e95a0` (`git archive`) into a temporary folder, builds there the official
  loader, and builds the fork's writer from the working tree.
- `tests/compat/writer.c`, built against the fork: plays a script of setters through the public `seq_*` API (INIT, each
  genre preset, a progression preset, a borrowed chord, a change point, a bar edit, a song part, a lock) and saves each
  resulting state to a file.
- `tests/compat/loader.c`, built against the official engine: loads each file with `seq_state_load` (it must return 1),
  saves it again, and writes the result.
- `run.sh` compares each pair. Step 0 requires identity for every scenario. A file `tests/compat/expected.txt` lists, per
  scenario added by later steps, the differences their specifications permit; anything else fails.

### 3.3 The CPG submodule

`.gitmodules` declares `third_party/AkaiMPC`, `shallow = true`. The submodule's pinned commit is changed only by a
deliberate bump, reviewed through what it changes in `coverage.txt` (and, from step 4, in the generated table).

### 3.4 The coverage tool: `tools/cpg/`

- `enumerate.c`, linked with [src/theory.c](../../../src/theory.c): for each of the seven scales, every `src` from 0 to
  `THEORY_SRC_MAX`, every degree it allows and every colour, prints root, pitch-class set and notes (the study's
  Appendix A, item 1).
- `coverage.mjs`, Node 20 or later, importing the CPG's `musicTheory.js` and `generation.js` from the submodule: the
  classification of the study's Appendix A, items 2 and 3, over the template chords and over the palettes of all six
  styles.
- The committed output reproduces the study's §3 figures: 745 template chords (639 diatonic, 49 borrowed, 54
  chromatic, 2 near, 1 absent); 17 664 pads (9 336 diatonic, 2 246 borrowed, 5 641 chromatic, 202 near, 239 absent).

### 3.5 Test builds: `.github/workflows/build.yml`

Calls the reusable workflow at `8fbeffc` with `vst_dir: vst`, `build: vst/build.sh`, the version read from
`versions.json`, `tools_ref: 8fbeffc`, and `dry_run: false`. Triggers: manual dispatch from the Claude branch, and push of
a `release/*` branch. Each run leaves a draft release holding the installable zips.

### 3.6 Versions

`versions.json` becomes `{"force": "0.28.1-cpg.1"}`. `tools/version.sh` reads it unchanged.

## 4. Measurements

Recorded from the first green CI run, as the reference every later specification compares with:

| Quantity | Source | Value at 0.28.1 |
|---|---|---|
| Binary size | `tests/budget.sh` | 666 KB at the last recorded measurement, budget 688 |
| Skin images, decoded | `tests/budget.sh` | 78.8 MB at the last recorded measurement, budget 80 |
| Parameters | `vst/params.json` | 2 037 |
| Core parity hash | `tests/core_parity.sha256` | `934d3f5c...` |
| Coverage | `coverage.txt` | the study's §3 figures |

## 5. Verification

1. CI is green on unchanged code: all three jobs.
2. The compatibility harness reports identity for every scenario.
3. `coverage.txt` reproduces the study's figures exactly.
4. A draft release built by `build.yml` installs on the MPC One under MPC OS 3.10.1 in place of the official folder, and
   behaves as 0.28.1: INIT and House 124 load and play, a project saves and reopens, the official build restores
   afterwards. Recorded as a device run.

## 6. Out of scope

Any change under `src/` or `vst/`; the palette generator (step 4); showing the version on the device.

## 7. Open points, to settle while implementing

- Whether `vst/build.sh` and `tests/run_tests.sh` run unchanged on GitHub's runners (Docker image builds, QEMU for the ARM
  build, run time). Adjustments, if any, go into the workflow, not the scripts.
- Whether the reusable workflow needs inputs beyond those listed (an `about` line, the licence identifier
  `PolyForm-Shield-1.0.0`), and whether its catalog steps must be skipped for a fork.
- Whether `8fbeffc` is the exact toolchain commit 0.28.1 was built with; a byte comparison of the CI-built `cadenza.so`
  with the official one would tell, timestamps aside.

## 8. Implementation notes

Empty until implementation.
