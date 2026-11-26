<!--
Draft of a pull request to open-mpc/mpc-vst-plugins (O20; docs/development/upstream-relations.md, B3), from
liotier/mpc-vst-plugins branch wrapper-options to open-mpc main. Written in open-mpc's template
(.github/pull_request_template.md), for its maintainers: what the repository's ports gain first, Cadenza as the origin
and the device evidence. To be opened by the owner, or by Claude on the owner's go. The author is mentioned with "@", so
GitHub notifies them: the owner's choice, 2026-11-26.
-->

**Title:** Wrapper: engine table under `pthread_once`; opt-in fixes for live screens, unpolled list tiles, index answers and Apple builds

## What and why

Six wrapper changes, all in `wrapper/vst2_wrap.c`, each already run on MPC hardware. What ports gain:

- **Every port:** the engine table is set once, under `pthread_once`, even when two instances are created on two threads
  at the same moment.
- **Ports whose screen changes while they play** (a playhead, a step or bar readout, a status line): MPC's PRESET drawer
  stays usable while playing, and MPC rebuilds its parameter screen less often.
- **Ports with buttons drawn as list tiles that are not polled:** every touch acts, not only the first.
- **Ports also built for Apple platforms:** the wrapper links without the `mpc_engine_changed` hook.
- **Engines that answer option params with their index while some labels are numbers:** the value reads back right.

Only the first is unconditional. The other five are `"defines"` options in `vst.json`, off by default: a port that sets
none builds and behaves as before.

### Every port: `g_api` under `pthread_once`

`VSTPluginMain` set the engine table with `if (!g_api) g_api = mpc_engine();`; two instances created on two threads at
once could both call `mpc_engine()` and race on `g_api`. It now goes through `pthread_once`.

### Screens that change while playing: `ON_NO_UPDATE_PREFIX`, `LIT_UPDATE_ON_TEXT_POLL`

Every `audioMasterUpdateDisplay` makes MPC re-list the programs, which restarts the PRESET drawer's slide-in, and rebuild
its parameter screen (NOTES 2026-10-21). A screen that changes on every beat or bar therefore keeps the drawer from
opening while the plugin plays. Since #530 spreads the polls over the blocks, cells that change at the same moment (a bar
line) are read at different blocks, and each asks for its own UpdateDisplay. `DISPLAY_REV_NO_UPDATE` already answers this
for `display_rev`; these two answer it for lit tiles and polled texts:

- `ON_NO_UPDATE_PREFIX` (a key prefix, e.g. `"ph_"`): list tiles whose key starts with it report their lit state by
  automate alone, with no UpdateDisplay. For tiles that change often and need no text refresh, a playhead's.
- `LIT_UPDATE_ON_TEXT_POLL`: the UpdateDisplay that a lit tile, a polled text or `display_rev` asks for waits 100 ms from
  the first such change, so the cells that change together cost one. Lit states still go out at once by automate; texts
  refresh with the held UpdateDisplay, at most 100 ms later. Not with `DISPLAY_REV_THREAD`.

Who gains: any port with a playhead or live readouts. Of the catalog's 329 entries, 56 describe themselves as
sequencers, arpeggiators, groove boxes or drum machines, the likeliest candidates.

### Buttons drawn as unpolled list tiles: `ECHO_GUARD_POLLED_ONLY`

As I read `set_param_now`, the list-tile echo guard is active for a tile whose `last_on` is 0 ("unknown"), which an
unpolled tile keeps for good. A tile that never lights is read back as 0, so MPC sends 1 at every touch; the guard drops
every touch after the first, and no poll comes to reset it. With the option, the guard is kept to polled tiles.
`poc/steptest` and `poc/songpos` have `"poll": false` params, and the host test does not tap twice, which may be why it
has gone unnoticed. If you agree with the reading, this may be better as the default than as an option; I kept it opt-in
so that this PR changes nothing for existing ports.

### Apple builds: `NO_ENGINE_CHANGED`

The weak undefined reference to `mpc_engine_changed` does not link on Apple platforms (Mach-O), which matters to an engine
also built for macOS or iOS. The option leaves the hook out. `tools/test_port.sh` has a macOS branch: if the reference
fails there too, every port's host test on a Mac meets it, and a default under `__APPLE__` would serve better. I have no
Mac to check.

### Engines that answer by index: `OPTION_ANSWER_INDEX`

Since #859 an exact label match wins, so an engine that answers its option params with the index, and has labels that are
numbers ("1".."16" for MIDI channels), reads back wrong: the answer "9" matches the label "9", index 8. The option takes a
fully numeric answer as the index first, as before #859. Niche: NOTES (2026-11-13) found no local `vst.json` with a
number-only label that differs from its index. It serves engines brought from elsewhere that answer by index.

### Where these come from

[Cadenza](https://github.com/Shrekage2024/cadenza-force), a chord-driven MIDI sequencer plugin, is built on its
author's branch of this repository, `Shrekage2024/mpc-vst-plugins` `chordseq/open-mpc`: `main` at `52c1fb0` (2026-11-13)
plus these six commits, written by Shrey Sharma (@Shrekage2024) for Cadenza's screens: its PLAY page's playhead (the
drawer), its bar editor's buttons (the echo guard), its MIDI channel options (the index answers), its macOS and iOS
builds (the hook). Their authorship is kept. I maintain a feature branch of Cadenza, which has been built on this
repository's `main` plus these commits since 2026-11-26; once they are here, Cadenza and any plugin built like it can use
`main` as it is. They apply to `34b964c` without conflict.

Commits: the six picked from `chordseq/open-mpc`, each message naming its original, rewritten without that branch's local
tags; one that drops the same tags from the code comments (comments only); one for the docs.

## Checks
- [x] Offline first: `tools/test_port.sh` PASSED with `poc/steptest`, `poc/polltest` and `poc/selfset` (all options off)
      and with Cadenza's `vst.json` (all five on; its engine opens an ALSA port, so LeakSanitizer needs `leak:alsa_open`
      suppressed, as in Cadenza's own tests). Cadenza's own offline suite (5 240 checks) passes on this branch.
- [x] Device-verified, with Cadenza as the test plugin: Force, MPC OS 3.9.1
      ([its test history](https://github.com/Shrekage2024/cadenza-force/blob/main/tested.json), releases 0.22.1 to 0.28.1),
      and MPC One, MPC OS 3.10.1, 2026-11-26, built on this branch: the PRESET drawer usable while playing, list-tile
      buttons acting at every touch, MIDI channels and GATE read back right, projects reopening.
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
