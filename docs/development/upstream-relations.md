# Working with upstream

Status: plan, 2026-11-26, revised the same day (decisions 30 and 31; O17 and O18 taken). The fork is a feature branch of Cadenza, developed in the
open with a view to offering its work to the author; this document says what is offered, in what order and in what form,
and what keeps the fork's builds sound meanwhile. The decisions are the owner's
([strategy, §8](implementation-strategy.md#8-decisions-for-the-owner): O3, O17 to O20).

## 1. The facts

**Upstream's public record** ([Shrekage2024/cadenza-force](https://github.com/Shrekage2024/cadenza-force), read
2026-11-26): ten commits, one export per release, the history kept private; exports of 0.25.0 (29 October), 0.26.0
(31 October), 0.27.1 (2 November) and 0.28.1 (14 November). Its [tested.json](../../tested.json) lists 32 versions tested
on the author's Force between 11 October and 13 November, about one a day. Since only releases are exported, the record
shows releases, not work: the gap between 0.27.1 and 0.28.1 was twelve days, as long as the one since. The engine also
serves the Cadenza iOS app and the browser version ([NOTICE](../../NOTICE)), and 0.27.0 added to the shared core the song
order "for the iOS song page".

**Contacts so far.** The owner's issue [Shrekage2024/cadenza-force#4](https://github.com/Shrekage2024/cadenza-force/issues/4),
"Documentation: works on MPC OS 3.10.1 on MPC One" (22 November); the owner's message of 23 November introducing the
fork, offering the patches of milestone M1 and asking for the browser version's sources. Neither is answered yet, and
nothing in the fork waits for an answer (decision 27).

## 2. What the licence means here

[PolyForm Shield 1.0.0](../../LICENSE) (a reading, not legal advice; the study's
[§9](cpg-integration-study.md#9-licence-governance-and-branding) gives the rest). A personal fork developed in the open
provides no product, and is a permitted purpose. Builds for anyone else are a different matter: "Goods and services
compete even when provided free of charge" (Competition), so the fork's builds stay the owner's until the author agrees
otherwise. The README says so at its top (§4).

## 3. Offers to the author

Proposals carry better than patches: one export per release suggests the author re-implements rather than merges (study,
§9). Each offer stands alone, is small and measured, and costs the author little to accept; they go in order of that
cost, lowest first.

| # | Offer | State | Decision |
|---|---|---|---|
| A1 | A note on the release's two packages: they name the plugin file differently (`cadenza.so`, `cadenza-arm32.so`), and MPC ties a saved state to the file's path, so a state saved under one is refused under the other (lesson 16). | a paragraph of the design note (O17, 2026-11-26), not an issue: narrower than first thought (below); the [issue draft](upstream/cadenza-issue-plugin-path.md) kept for the record | O17, taken |
| A2 | Milestone M1: section garbage collection in the ARM build (one line of `vst/vst.json`, about 46 KB of uncalled functions dropped, same behaviour, parity unchanged) and key-aware spelling (about 4 KB). | ready; offered in the message of 23 November | O3 |
| A3 | The design note (milestone M2): palettes from the CPG, chords on any root, the full vocabulary, what changes for the author's other front ends, how projects stay readable by 0.28.1. | to write; the specifications' "other front ends" sections hold its material | none: it is written anyway (decision 27) |
| A4 | Once the fork has screens to show (milestone M3), a question: how the author would like to receive the work, and whether others may try the fork's builds meanwhile. | at M3 | O19 |

**Why A1 is on hold.** The two packages are the two installation routes of the README: MPC OS 3.9.1 and earlier, the
armv7 zip's installer, which copies the plugin to `/sdcard/Synths` over SSH; MPC OS 3.10.1 and later, the SDK zip's
folder, copied into `VST2 Plugins`. The SDK zip follows Akai's plugin-folder format, one folder for every MPC, in which
`plugin.json` names a binary for each architecture, `<name>-arm32.so` and `<name>-arm64.so`, so that both can sit side by
side; the toolchain's `tools/sdk_zip.py` makes it from the per-build zip, renaming the binary. A user who follows the
README never mixes the two. The owner's official installation used the armv7 zip's `portable/` folder in
`VST2 Plugins`, outside both routes, and the test build was first installed from the SDK zip. What remains is a note for
users who change routes, for instance on moving from 3.9.1 to 3.10.1, where the folder changes as well as the name; that
concerns every plugin the open-mpc toolchain packages, not Cadenza alone, and is untested beyond the owner's case.

## 4. Keeping the fork's builds sound

| # | Measure | Why | State |
|---|---|---|---|
| B1 | The toolchain from its origin: a fork of [open-mpc/mpc-vst-plugins](https://github.com/open-mpc/mpc-vst-plugins) in the owner's account, with one branch: open-mpc's `main` at a pinned commit plus the author's six wrapper commits (below), their authorship kept; `.github/toolchain` points at it. | One step from the origin instead of two; open-mpc's improvements reach the fork by moving the pin, deliberately, as the CPG submodule moves. | O18, taken: [liotier/mpc-vst-plugins](https://github.com/liotier/mpc-vst-plugins/tree/cadenza/toolchain) `cadenza/toolchain`, pinned; run 2 on the device to come |
| B2 | A notice at the top of the README: a personal feature branch of Cadenza, developed with a view to offering its work upstream; no builds offered; Cadenza itself at upstream. | The repository's front page (`main`) is upstream's README unchanged, presenting itself as "Cadenza 0.28.1" with install instructions; the notice avoids passing for Cadenza (no trademark rights; study, §9). | on this branch; reaches `main` with the release branch (O2) |
| B3 | The author's six wrapper options offered to open-mpc as a pull request, credited to the author. | They are off by default and generic; once in open-mpc, the fork's toolchain is open-mpc's `main` as it is, and so could the author's be. | O20, after B1's device run |
| B4 | Upstream watched for releases: GitHub's Watch, Custom, Releases on upstream's repository. | A new release is merged promptly (strategy, §10). | the owner's account |

**B1, measured 2026-11-26.** Cadenza's toolchain branch, `Shrekage2024/mpc-vst-plugins` `chordseq/open-mpc` at
`8fbeffc`, is open-mpc's `main` at `52c1fb0` (13 November) plus six commits, all in `wrapper/vst2_wrap.c` (56 lines
added, 7 removed): the options `ON_NO_UPDATE_PREFIX`, `LIT_UPDATE_ON_TEXT_POLL`, `ECHO_GUARD_POLLED_ONLY`,
`OPTION_ANSWER_INDEX` and `NO_ENGINE_CHANGED`, which `vst/vst.json` turns on, and `g_api` set under `pthread_once`. None
is in open-mpc's `main`, which has 537 commits more since `52c1fb0` (GitHub's comparison with the author's fork may count
from another branch). The six apply to open-mpc's `main` at `34b964c` (26 November) without conflict, and Cadenza builds
on the result (the local ARM cross-compiler, `tools/dev/armsize.sh`): code +152 bytes, data +8 148 bytes, which is
open-mpc's new per-parameter `automatable` field, 4 bytes for each of Cadenza's 2 037 parameters. Within the binary budget
(about 66 KB of headroom; CI's compiler decides). CI then measured 662 KB, 8 KB above step 5's 654 KB (run 34).

What open-mpc changed in the files Cadenza uses, since `52c1fb0`: the state chunk's size made configurable (`STATE_MAX`,
default 8 192 bytes, the size it had); a per-parameter `automatable` flag (default true, so every parameter answers as
before); flush-to-zero around the audio callback (no effect on a MIDI generator); the Sample page and its knob mapping
(not used by Cadenza); install notes and line endings in the packaging. The plugin file's name, the folder and the
identity are unchanged, so states should travel as before; a device run checks it before the pin moves (rows 1 to 7 of
the [device run](device-run-test-build.md#3-checks)).

## 5. Order

1. B2, the README notice (on this branch).
2. O18, B1: done 2026-11-26 (the owner's fork, the branch, the pin); CI, then run 2 on the device
   ([device-run-test-build.md, §6](device-run-test-build.md#6-run-2-the-toolchain-from-open-mpc)).
3. A3, the design note, alongside specifications 06 and 07.
4. O20, B3, once B1's device run passes.
5. A4 at milestone M3.
