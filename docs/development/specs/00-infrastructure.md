# 00 - Infrastructure: CI, compatibility harness, the CPG submodule, test builds

| | |
|---|---|
| Status | implemented; the device run of verification 4 pending |
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
| S00-D1 | **No change to Cadenza's behaviour.** Step 0 adds files under `.github/`, `tests/compat/`, `tools/cpg/`, `third_party/` and `docs/development/measurements/`, and edits `versions.json`; nothing under `src/` or `vst/` changes. | Infrastructure must be proven on unchanged code, so that later differences belong to later steps. |
| S00-D2 | **CI runs the existing scripts unchanged:** `vst/build.sh`, then `tests/run_tests.sh`, on GitHub's Ubuntu runners with Docker and QEMU. | The scripts are the author's own definition of a passing build; running them as they are keeps the fork comparable with upstream. The plugin test needs `vst/build.sh` first: it includes `keyflags.h`, which the skin build generates. |
| S00-D3 | **The toolchain is pinned** to [Shrekage2024/mpc-vst-plugins](https://github.com/Shrekage2024/mpc-vst-plugins/tree/chordseq/open-mpc) commit `8fbeffc`, branch `chordseq/open-mpc`, named in one place. | The README requires that branch; `8fbeffc`'s own message names Cadenza 0.28.1. One place means one edit to move it. |
| S00-D4 | **The compatibility harness builds two engines:** the fork's, and the official one from upstream commit `56e95a0`, as two separate programs. | The engine is host-free, so both build natively in seconds; two programs avoid every symbol clash. |
| S00-D5 | **Its baseline is byte identity:** a state saved by the fork, loaded by the official engine and saved again, gives the same bytes. Later steps add scenarios whose permitted differences their specifications list. | On unchanged code, anything but identity is a fault of the harness. |
| S00-D6 | **The CPG is a git submodule:** `third_party/AkaiMPC`, from `https://github.com/liotier/AkaiMPC`, pinned to `c84a306`, the commit the study measured; `shallow = true`. The plugin build never needs it. | Decision 10. Pinning the studied commit makes the first measurement reproduce the study's figures. |
| S00-D7 | **The study's coverage measurement becomes a tool:** `tools/cpg/enumerate.c` lists every chord Cadenza's theory layer can voice; `tools/cpg/coverage.mjs` classifies the CPG's catalogue against it; the output is committed as `docs/development/measurements/coverage.txt` and compared by CI. | It makes the study's figures reproducible, and gives steps 3 and 4 their measure: the count of missing sonorities, which they drive to zero. |
| S00-D8 | **Test builds go through the toolchain's reusable workflow** ([vst-release.yml](https://github.com/Shrekage2024/mpc-vst-plugins/blob/8fbeffc/.github/workflows/vst-release.yml)) at the pinned commit, **as draft releases only.** | It builds and packages exactly as releases are made, the MPC OS 3.10.1 `VST2 Plugins` folder included. A draft is visible to the repository's collaborators only, whereas run artifacts of a public repository can be downloaded by any signed-in user; builds of a personal fork stay private (study, §9). |
| S00-D9 | **Versions carry a suffix:** the Claude branch holds the next release's version, `0.28.1-cpg.N`; a release branch `release/0.28.1-cpg.N` freezes it; the Claude branch then moves to `N + 1`. The series starts at `0.28.1-cpg.1`. | The plugin shows no version on screen, so the zips' and drafts' names are what tell builds apart. |

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

| Quantity | Source | Value at 0.28.1, CI run 1 |
|---|---|---|
| Binary size | `tests/budget.sh` | 686 KB (701 956 bytes), budget 688 KB (704 512 bytes): **2 556 bytes of headroom** |
| Skin images, decoded | `tests/budget.sh` | 79.1 MB, budget 80 |
| Parameters | `vst/params.json` | 2 037 |
| Core parity hash | `tests/core_parity.sha256` | `934d3f5c...`, 610 notes at 44.1 and 48 kHz |
| Coverage | `coverage.txt` | the study's §3 figures |

The binary and skin figures recorded in `vst/budget.json`'s notes (666 KB, 78.8 MB) date from earlier versions; the
study's "22 KB of headroom" was taken from them and is corrected in its revision 8.

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

Implemented in commit `8561054` and the commit that adds these notes. CI run 1,
[45130618321](https://github.com/liotier/cadenza-CPG/actions/runs/45130618321), is green on its three jobs in 12 minutes
(the build under QEMU about 4, the offline tests about 7).

**Files.** `.github/workflows/ci.yml`, `.github/workflows/build.yml`, `.github/toolchain`, `.github/lsan.supp`,
`.gitmodules` and `third_party/AkaiMPC`, `tests/compat/` (`run.sh`, `writer.c`, `loader.c`, `expected.txt`), `tools/cpg/`
(`enumerate.c`, `coverage.mjs`, `coverage.sh`), `docs/development/measurements/coverage.txt`, `versions.json`. Nothing
under `src/` or `vst/` (S00-D1).

**Deviations from the specification.**

1. *Test builds do not call the reusable workflow* (S00-D8, §3.5). At `8fbeffc`, `vst-release.yml` uploads the zips as run
   artifacts on every run (its upload step has no condition), which is what S00-D8 rules out for a public repository; it
   also checks out `open-mpc/mpc-vst-plugins` where the README names `Shrekage2024`'s branch. `build.yml` runs the same
   tools (`vst/build.sh`, the toolchain's `release.py` and `sdk_zip.py`) and creates the draft itself, with no
   artifacts. Inputs: `--license PolyForm-Shield-1.0.0`, an `--about` line calling the build a test build of a personal
   fork; the catalog checks are left out, the fork not being meant for the catalog.
2. *Test builds are started by a commit message.* GitHub offers manual runs only for workflows on the default branch,
   and `main` holds upstream's tree. `build.yml` therefore also runs on a push to the Claude branch whose head commit
   message holds `[test build]`; manual runs become available if the workflow ever reaches `main`.
3. *The harness compares with the fork's own re-save* (S00-D5, §3.2). Its first run on unchanged code found two scenarios
   whose save, loaded and saved again, differs from itself in 0.28.1: a loop shortened after its drawn bars were filled
   saves the hidden bars' rhythms (`.rb` up to `seq_part_t.filled`) once, and a reload saves the loop's bars only. So
   the writer also saves its own engine's re-save, checks that it is a fixed point, and the official re-save is
   compared with it: the official engine must read the state as the fork does. An upstream quirk, harmless (hidden bars
   only), worth a line to the author.
4. *Three files beyond the specification:* `.github/toolchain` holds S00-D3's pin, read by both workflows; `.github/lsan.supp`
   suppresses the leaks reported under `engine.c`'s `alsa_open` on GitHub's runners, which have libasound where the
   offline test container has none (`snd_seq_open` fails for want of `/dev/snd/seq`, and libasound's cached
   configuration is reported once the library is closed); `tools/cpg/coverage.sh` builds, runs and compares (`--check`
   in CI).
5. *The coverage report says more than §3.4 asked:* per-style tallies, the templates' counts (109 entirely diatonic, 144
   within `src` 0-3, 182 exact), the 66 pads exact by decision 12, and **375 pads needing a new sonority**: the figure
   step 3 drives to zero.

**Open points, settled.**

- `vst/build.sh` and `tests/run_tests.sh` run unchanged on GitHub's runners; the only adjustment is the leak
  suppression above, in the workflow's environment.
- `8fbeffc` is the exact toolchain: CI's `cadenza.so` is byte-identical to the official 0.28.1 release's (MD5
  `4e13dd200a0378c3f04e29dbc040818c`, 701 956 bytes). Builds are reproducible, and a fork build differs from the
  official one only by what the fork changes.
- The reusable workflow's inputs: moot (deviation 1).

**Found on the way.** The binary has 2 556 bytes of headroom, not the 22 KB the study assumed (§4). Step 1 alone needs
more (its implementation notes); the budget is the owner's decision.

**Verification.** 1 to 3 hold (CI run 1). 4, the device run, waits for the first draft release.

---

## Addendum A: local tools

| | |
|---|---|
| Status | approved by the owner, 2026-11-23 (strategy, §8.3, O4); implemented |
| Step | 0a |
| Size | S |
| Depends on | 00 |

### A.1 Problem

The toolchain's scripts run in Docker, and a cloud session has no Docker daemon: steps 1 and 2 were built and measured
with two scratch scripts that a later session would have to rewrite. Both proved their worth: the native runner ran the
whole offline suite in a few minutes, and the size estimate showed step 1 over the budget before CI did (strategy, §3,
lessons 2 and 9).

### A.2 Decisions

Not six independent decisions: two choices for the owner (S00-A1, S00-A4), one change to CI (S00-A5), two ways of building S00-A1 (S00-A2,
S00-A3) and a constraint on all the tools (S00-A6). The last column names what each presupposes; a decision resting on none can be
judged alone.

| # | Kind | Decision | Rationale | Rests on |
|---|---|---|---|---|
| S00-A1 | owner's choice | **`tools/dev/native_tests.sh`** runs `tests/run_tests.sh`'s compile-and-run steps with the host's gcc and the parity check, reading the steps from that script, never copying them. | One list of tests, the author's; the runner follows any change to it. | none |
| S00-A2 | how S00-A1 is built | **It generates `vst/build/keyflags.h` and `params.h` in a temporary copy of the tree** and copies the two headers back. | `vst/make_skin.py` also rewrites `vst/params.json`, `vst/layout.conf` and `vst/images/`, which are committed: run in place, it would dirty the tree. | S00-A1 |
| S00-A3 | how S00-A1 is built | **It sets the LeakSanitizer suppression of `.github/lsan.supp`.** | A host with libasound reports its cached configuration as leaks, as GitHub's runners do (spec 00, deviation 4). | S00-A1; spec 00, deviation 4 |
| S00-A4 | owner's choice | **`tools/dev/armsize.sh`** builds the plugin with an ARM cross-compiler (`arm-linux-gnueabihf-gcc`), the sources and flags `gen_vst.py --shell` gives, for the working tree and for a commit, and prints the code size of each and the difference. | A step's cost before it is pushed. The cross-compiler is not the toolchain's (GCC 13 against 11): its differences guide, CI's absolute figure decides. Step 1: estimated +4 476 bytes, measured about +4 KB. | S00-A2 (the engine includes the generated headers) |
| S00-A5 | CI change | **CI's summary states the binary in bytes and the headroom** against `vst/budget.json`. | The budget check prints whole kilobytes; comparing S00-A4's estimates with CI needs the exact figure. | none; its use comes from S00-A4 |
| S00-A6 | constraint | **The tools change nothing the plugin or CI depends on.** | Development aids, not part of the build. | applies to S00-A1 to S00-A4 |

### A.3 Verification

1. On the current commit, `native_tests.sh` passes as CI's offline tests do, and leaves `git status` clean.
2. `armsize.sh HEAD~1` on a commit that changes code reports a difference of the sign and order CI then measures.
3. CI's summary shows the bytes and the headroom.

### A.4 Out of scope

Installing the cross-compiler in a setup script: an environment matter, proposed separately if wanted.

### A.5 Implementation notes

`tools/dev/native_tests.sh`, `tools/dev/armsize.sh`, and `tools/dev/lib.sh` for what they share: the headers made in a
copy of `vst/` (S00-A2) and the toolchain's location (`MPC_VST`, else next to the repository, as the author's scripts look).

- **The runner** writes a temporary copy of `tests/run_tests.sh` with three edits, each required to apply: its `cd` to
  the repository, its `docker run ... gcc:12 bash -euc '` to a plain `bash -euc '` with the toolchain in `MV`, and `/mv/`
  to `$MV/` in the former container's script. The parity check and the toolchain's host test then run as the author
  wrote them. If `run_tests.sh` changes shape, the runner stops and names the edit that no longer applies.
- **The size estimate** builds both trees with each tree's own `vst/vst.json`, so a commit from before decision 21 is
  built without section garbage collection, as CI built it then.

**Verification.**

1. From an empty `vst/build/`: all 15 suites pass, the core parity trace as pinned, and `git status` shows nothing the
   runner wrote.
2. `armsize.sh 1baa448` (step 1, before decision 21; CI measured 690 KB) gives text −46 299 bytes and file −45 064
   bytes for the working tree, where CI measured −40 KB between the same pair (650 KB): the right sign and order, the
   cross-compiler's absolute figures larger than the toolchain's. `armsize.sh d9b619d`, whose code is the working tree's,
   gives 0.
3. CI's summary gains the binary's bytes and headroom: the step ran in CI run 10
   ([45261448699](https://github.com/liotier/cadenza-CPG/actions/runs/45261448699), commit `17bd792`), green on its
   three jobs; the figures are on that run's summary page.

## Addendum B: the toolchain from open-mpc

Decision O18 of the [strategy](../implementation-strategy.md#8-decisions-for-the-owner), 2026-11-26 (study, decision 30).
S00-D3 pinned the toolchain to the author's branch, `Shrekage2024/mpc-vst-plugins` `chordseq/open-mpc` at `8fbeffc`:
open-mpc's `main` at `52c1fb0` plus six wrapper commits. The pin now names the owner's fork of open-mpc,
[liotier/mpc-vst-plugins](https://github.com/liotier/mpc-vst-plugins), branch `cadenza/toolchain`: open-mpc's `main` at
`34b964c` plus the same six commits, their authorship kept and their messages cleaned for open-mpc (each names the
commit it was picked from); the change is the same patch (`git patch-id` equal). The branch only grows: a later move
adds commits or merges open-mpc's `main`, never rewrites, so every commit the pin has named stays fetchable. The pull
request to open-mpc (O20) goes from a topic branch of its own.

S00-D3's reasons hold (one place, one edit); its first reason changes: the README now names this branch. Lesson 3's byte
comparison with the official release, already lost to decision 21, does not return.

**Verification.** Before the pin moved: the six commits apply to `34b964c` without conflict; Cadenza builds for armhf
with the local cross-compiler (`tools/dev/armsize.sh`), code +152 bytes and data +8 148 bytes against `8fbeffc`, the data
being open-mpc's new per-parameter `automatable` field (4 bytes for each of 2 037 parameters); the offline suite passes
natively (`tools/dev/native_tests.sh`, 5 240 checks). After: CI run 34
([45788259722](https://github.com/liotier/cadenza-CPG/actions/runs/45788259722), commit `180f02c`) green on its three
jobs, the plugin 662 KB (654 before; budget 720), the skin 79.1 MB unchanged, the toolchain's own host test passing its
new checks (`effCanBeAutomated` follows "automatable" for all 2 037 parameters; an 8 193-byte chunk refused); the draft
rebuilt from `180f02c` (run [45788259709](https://github.com/liotier/cadenza-CPG/actions/runs/45788259709)). Then a device run, **passed** 2026-11-26:
([device-run-test-build.md, §6](../device-run-test-build.md#6-run-2-the-toolchain-from-open-mpc)), which also checks on
the MPC One what each of the six commits does.
