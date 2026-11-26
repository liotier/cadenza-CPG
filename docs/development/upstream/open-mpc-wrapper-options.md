<!--
Draft of a pull request to open-mpc/mpc-vst-plugins (O20; docs/development/upstream-relations.md, B3), from
liotier/mpc-vst-plugins branch wrapper-options to open-mpc main. Written in open-mpc's template
(.github/pull_request_template.md). To be opened by the owner, or by Claude on the owner's go. The author is mentioned with
"@", so GitHub notifies them: the owner's choice, 2026-11-26.
-->

**Title:** Wrapper: five optional VST2 behaviours and a `pthread_once`, from Cadenza's toolchain branch

## What and why

Cadenza (a chord-driven MIDI sequencer plugin, [Shrekage2024/cadenza-force](https://github.com/Shrekage2024/cadenza-force))
ships built on its author's branch of this repository, `Shrekage2024/mpc-vst-plugins` `chordseq/open-mpc`: `main` at
`52c1fb0` (2026-11-13) plus six wrapper commits. This PR brings those six commits here, their authorship kept (Shrey
Sharma, @Shrekage2024), so that Cadenza, and anything built on it, can use this repository's `main` as it is.
I maintain a feature branch of Cadenza and moved its builds to `main` plus these commits; they apply to `34b964c` without
conflict.

All five options are off by default: a port that sets none builds and behaves as before. Each is a `"defines"` key in
`vst.json`, for the VST2 wrapper only.

| Option | What it does | Why Cadenza needs it |
|---|---|---|
| `ON_NO_UPDATE_PREFIX` | List tiles whose key starts with the prefix (e.g. `"ph_"`) report their lit state by automate alone, without `audioMasterUpdateDisplay`. | Each UpdateDisplay makes MPC re-list the programs, which restarts the PRESET drawer's slide-in; a playhead lit each beat kept the drawer from opening while playing. |
| `LIT_UPDATE_ON_TEXT_POLL` | The UpdateDisplay that a lit tile, a polled text or `display_rev` asks for waits 100 ms from the first such change, so cells that change together (a bar line) cost one, although the polls (#530) read them at different blocks. Not with `DISPLAY_REV_THREAD`. | The same drawer: a bar line changed several cells, each read at a different block. |
| `ECHO_GUARD_POLLED_ONLY` | The list-tile echo guard skips `"poll": false` tiles. | Buttons drawn as list tiles that never light: MPC reads them back as 0, the guard ignored the second and later touches until a poll reset it, and no poll comes. |
| `OPTION_ANSWER_INDEX` | A fully numeric option answer is taken as the index before any label match, as before #859. | An engine that answers every option with its index, with labels "1".."16" (MIDI channels): since #859 the answer "9" reads as the label "9", index 8. |
| `NO_ENGINE_CHANGED` | Leaves out the `mpc_engine_changed` hook. | Its weak undefined reference does not link on Apple platforms (Mach-O), for an engine also built for macOS and iOS. |

And one change that is not optional: `g_api` is set under `pthread_once`, so two instances created on two threads at once
share one engine table instead of racing on it.

**Beyond Cadenza.** Two of these may concern every port, which is why they might be better as defaults than as options;
I kept them opt-in so that this PR changes nothing for existing ports, and leave that to you:
- `ECHO_GUARD_POLLED_ONLY`: as I read `set_param_now`, an unpolled list tile keeps `last_on` at 0 ("unknown"), so the
  guard is active for it, and a tile that never lights is read back as 0 and sends 1 at every touch: its second touch is
  dropped. `poc/steptest` and `poc/songpos` have `"poll": false` params; the host test does not tap twice. Not tested on
  another port.
- `NO_ENGINE_CHANGED`: `tools/test_port.sh` has a macOS branch, so if the weak reference does not link there, every
  port's host test on a Mac meets it. I have no Mac to check; a default under `__APPLE__` would then serve better.

The two display options help any port whose screen changes while it plays (a playhead, a step or bar readout: the
PRESET drawer, as `DISPLAY_REV_NO_UPDATE` did for the popup). `OPTION_ANSWER_INDEX` is niche: NOTES (2026-11-13) found no
local `vst.json` with a number-only label that differs from its index.

Commits: the six picked from `chordseq/open-mpc` (each message names its original), with their messages rewritten
without the branch's local tags; one that drops the same tags from the code comments (comments only); one for the docs.

## Checks
- [x] Offline first: `tools/test_port.sh` PASSED with Cadenza's `vst.json` (all five options on; its engine opens an ALSA
      port, so LeakSanitizer needs `leak:alsa_open` suppressed, as in Cadenza's own tests) and with `poc/steptest`,
      `poc/polltest` and `poc/selfset` (all off). Cadenza's own offline suite (5 240 checks) passes on this branch.
- [x] Device-verified: Force, MPC OS 3.9.1 (Cadenza's test history,
      [tested.json](https://github.com/Shrekage2024/cadenza-force/blob/main/tested.json), releases 0.22.1 to 0.28.1) and
      MPC One, MPC OS 3.10.1, 2026-11-26 (Cadenza built on this branch: the PRESET drawer usable while playing, the
      bar-editor buttons acting at every touch, MIDI channels and GATE read back right, projects reopening).
      `NO_ENGINE_CHANGED`: offline only, no Apple build here. NOTES has the details (2026-11-26).
- [ ] Catalog-conformant: not a port or a release.
- [ ] Catalog entry or skin: none.
- [ ] New catalog entry: none.
- [ ] Catalog PR: none.
- [x] Docs synced: `docs/NOTES.md` (dated section, device and offline), `docs/PORTING.md` checklist, `tools/gen_vst.py`
      "defines" docstring, `wrapper/engine.h` (the hook), the skill's note on UpdateDisplay. README and
      `docs/COMPATIBILITY.md`: no docs needed (no limitation removed). `vst3_wrap.c`: unchanged, the options answer MPC's
      VST2 host.
- [x] Repo stays device-generic.

🤖 Generated with [Claude Code](https://claude.com/claude-code)

https://claude.ai/code/session_01SiR8t8wwquqFGyk5vnvMbe
