# Device run of the test build (O1)

Status: run 1 **passed**, 2026-11-25 (§5; each row's outcome also in §3): row 7 failed at first because the two builds were installed from different
packages (§5.1), and passed once they came from the same one. Run 2, with the toolchain from open-mpc (§6): **passed**, 2026-11-26. Decision O1 of the
[implementation strategy](implementation-strategy.md#81-decidable-now): run the draft *Cadenza 0.28.1-cpg.1 (test build)*
on the MPC One and say whether it passes.

The draft holds steps 0 to 5. Only step 1, key-aware spelling, shows on screen; steps 2 to 5 (chromatic chords, the new
sonorities, the palette table, the part's palette) have no control yet, so nothing else should differ from the official
0.28.1. The run checks that the build loads and plays on the device, that it spells chords as specified, and that projects
travel both ways between it and the official build.

## 1. Get the build

1. Logged in to GitHub, open [the repository's releases](https://github.com/liotier/cadenza-CPG/releases). The draft
   *Cadenza 0.28.1-cpg.1 (test build)* is visible to you only.
2. Check its notes: "Built by GitHub Actions from liotier/cadenza-CPG@`<commit>`". The commit should be the branch's
   latest. The same commit appears on the device, on GLOBAL (§3, row 2).
3. Download the package **of the same kind as your official installation**. Look in your official
   `VST2 Plugins/Cadenza - VST - Cadenza` folder: MPC ties a project's or a preset's plugin state to the plugin's full file
   path (§5.1), and the release's two packages name the file differently.
   - It holds `cadenza.so`: download `Cadenza-0.28.1-cpg.1-mpc-armv7.zip` and use, after unzipping, the folder
     `Cadenza-0.28.1-cpg.1/portable/Cadenza - VST - Cadenza` (it holds `cadenza.so` and a `plugin.json` naming it; it works
     as it is under MPC OS 3.10.1). The owner's MPC One is in this case.
   - It holds `cadenza-arm32.so`: download `Cadenza-0.28.1-cpg.1-mpc-sdk.zip`; after unzipping, its one folder,
     `Cadenza - VST - Cadenza`.
4. Either is the 32-bit build, as the official release is, which already runs on this MPC One.

## 2. Swap the official plugin for the test build

The test build keeps Cadenza's identity (study, decision 11): the same folder name and plugin ID, so projects open in
either build, and MPC cannot hold both at once. It replaces the official folder, which you keep beside it.

1. Save your project and switch the MPC off. Take out the SD card or USB drive that holds `VST2 Plugins`, and open it on
   the computer.
2. Move `VST2 Plugins/Cadenza - VST - Cadenza` **out of** `VST2 Plugins`, for instance to a folder `Cadenza 0.28.1
   official` at the top of the drive. Do not leave it inside `VST2 Plugins` under another name: MPC would find two
   plugins with the same identity.
3. Copy the test build's `Cadenza - VST - Cadenza` folder into `VST2 Plugins`.
4. Put the drive back and start the MPC.

## 3. Checks

| # | Action | Expected | Result |
|---|---|---|---|
| 1 | Add Cadenza to a track, from where the official one was in MPC's plugin list. | It loads; its screen draws. || Run 1: passed (§5). |
| 2 | Open GLOBAL. Then on its piano tap the black key D# (the key selector labels every black key with its sharp, in both builds: spec 01, S01-D7), SCALE Major, and look at PLAY's title. | On GLOBAL's title row, in magenta between the part's name and TEMPO: `CPG 0.28.1-cpg.1` and the commit of step 1.2; the official build has nothing there. PLAY's title: `Eb MAJOR` (the official build shows `D# MAJOR`). || Run 1: passed (§5). |
| 3 | Load the INIT program and play. | Chords, Bass and Melody sound on their tracks, as with the official build. || Run 1: passed (§5). |
| 4 | Load House 124 and play. | It plays as with the official build. || Run 1: passed (§5). |
| 5 | In each key of the table below (on GLOBAL's piano: F, D#, G#, C# for the majors, C# and D# with SCALE Minor), look at CHORDS, PHRASE, PLAY and GLOBAL while a progression plays (a preset, or chords picked on PHRASE). | Every chord name is one of that key's row, never its other spelling: no A♯ in F major, no D♭ in C♯ minor, no D♯ in E♭ major. PLAY's title as in the first column. || Run 1: passed for the four majors (§5); run 2: C♯ and D♯ minor passed too (§6). |
| 6 | In E♭ major with House 124 loaded, change something you can recognise (a chord picked on PHRASE), save the MPC project, then open it again. | It reopens as saved: key, chords, sound. || Run 1: passed (§5). |
| 7 | Swap back to the official build (§4), restart, open the project of row 6. | It opens and plays the same chords; PLAY's title now reads `D# MAJOR`, the official spelling of the same key. || Run 1: passed both ways, same package (§5, §5.1). |
| 8 | Anything else you notice. | Nothing differs from 0.28.1 but the spelling. || Run 1: nothing reported (§5). |

The chords of each key as the test build names them, the scale's seven, then the three borrowed chords PHRASE offers
(computed from the build's own code):

| PLAY's title | The scale's chords | Borrowed |
|---|---|---|
| `F MAJOR` | F Gm Am Bb C Dm Edim | Bbm (iv), Db (♭VI), Eb (♭VII) |
| `Eb MAJOR` | Eb Fm Gm Ab Bb Cm Ddim | Abm (iv), B (♭VI), Db (♭VII) |
| `Ab MAJOR` | Ab Bbm Cm Db Eb Fm Gdim | Dbm (iv), E (♭VI), Gb (♭VII) |
| `Db MAJOR` | Db Ebm Fm Gb Ab Bbm Cdim | Gbm (iv), A (♭VI), B (♭VII) |
| `C# MINOR` | C#m D#dim E F#m G#m A B | F# (IV), G# (V), D (♭II) |
| `D# MINOR` | D#m E#dim F# G#m A#m B C# | G# (IV), A# (V), E (♭II) |

Three borrowed chords read as their plainer names, not the textbook ones: in E♭ major ♭VI is written B, not C♭; in A♭
major E, not F♭; in D♭ major A and B, not B𝄫 and C♭. This is spec 01's rule (S01-D3: a double accidental, or E♯, B♯, C♭ or
F♭ outside the scale, takes its plainest name) and the CPG's own spelling. Tell me if you would rather see the textbook
names.

## 4. Restore the official build

Switch the MPC off, delete the test build's `Cadenza - VST - Cadenza` from `VST2 Plugins`, move the official folder back
into `VST2 Plugins`, restart. Projects saved with the test build open in the official one (row 7 checks it, and the
compatibility harness checks it offline for every saved field).

## 5. Results

**Run 1, 2026-11-24**, by the owner: MPC One, the draft built from `ce39284`.

| # | Result |
|---|---|
| 1 | Passed: the plugin loads. |
| 2 | Passed: `CPG 0.28.1-cpg.1 ce39284` visible on GLOBAL; D# and Major give `Eb MAJOR` on PLAY. |
| 3, 4 | Passed: "all live functionality nominal". |
| 5 | Passed for F, A♭, D♭ and E♭ major (B♭, never A♯ in F); C♯ and D♯ minor not reported. |
| 6 | Passed: saving and loading worked in the test build. |
| 7 | **Passed both ways**, 2026-11-25, with the test build installed from the armv7 zip's `portable` folder, like the official installation: the official 0.28.1 opens a state saved by `ce39284`, and the test build opens a state and a project saved by the official 0.28.1. First attempt, with the SDK zip's folder: refused, "This state file is not for Cadenza" (§5.1). |
| 8 | Nothing reported. |

The first loads found one procedural flaw, now fixed: the procedure named E♭ on the key selector, whose black keys carry
sharps in both builds (S01-D7); and the owner asked for the build's mark on GLOBAL (strategy, §4.5).

### 5.1 Row 7: what refuses the state

**Found, 2026-11-25.** The owner sent two plugin state files (`.xpi`), one saved with each build. Their plugin descriptions
differ in one field only, the plugin file: `…/VST2 Plugins/Cadenza - VST - Cadenza/cadenza-arm32.so` for the test build,
`…/cadenza.so` for the official one; `version="1.0.0.0"` in both, which MPC reads from the plugin itself (the AEffect
version, `vst.json`'s 1000), not from `plugin.json` (the experiment below changed nothing, as this predicts). The official
release ships two packages that name the file differently: `Cadenza-0.28.1-mpc-armv7.zip`'s `portable/` folder holds
`cadenza.so`, `Cadenza-0.28.1-mpc-sdk.zip` holds `cadenza-arm32.so` (both read 2026-11-25). The owner's official
installation is the first, the test build was installed from the second, as §1 then said. The states themselves are sound:
decoded (JUCE's fxb chunk set, uid `43685371`, plugin version 1000), the test build's loads in the official 0.28.1 engine
built from `56e95a0`, under ASan and UBSan. So MPC refuses on the path, before Cadenza sees anything; between two
installations of the same package, row 7 should pass. §1 now picks the package by the official installation's file name;
the state saved with the SDK package's test build stays refused, as it would between the two official packages.

Why the release has two packages: they are the README's two installation routes. For MPC OS 3.9.1 and earlier, the armv7
zip's installer copies `portable/Cadenza - VST - Cadenza` to `/sdcard/Synths` over SSH and adds it to MPC's plugin list;
for 3.10.1 and later, the SDK zip's folder is copied into `VST2 Plugins`. The SDK zip follows Akai's plugin-folder format,
one folder for every MPC, whose `plugin.json` names a binary for each architecture (`<name>-arm32.so`, `<name>-arm64.so`)
so that both can sit side by side; the toolchain's `tools/sdk_zip.py` makes it from the per-build zip and renames the
binary. A user who follows the README never mixes them. The owner's official installation used the armv7 zip's
`portable/` folder in `VST2 Plugins` under 3.10.1, which works but is neither route; §1 keeps to it, so as to compare like
with like. What this means beyond the fork: a user who changes routes, for instance on moving from 3.9.1 to 3.10.1, should
expect saved states to be refused, since the folder changes as well as the file's name (not tested; upstream-relations.md,
A1).

The investigation, as it went:

The message is neither Cadenza's nor the toolchain wrapper's (neither holds the text; the wrapper hands the saved text to
the engine untouched): it is MPC OS's. Its plugin host keeps, with a plugin's saved state, the plugin's description, and
checks it on loading; a user on MPC Forums met the same refusal after reinstalling a plugin
([thread](https://www.mpc-forums.com/viewtopic.php?p=1838042), unanswered). The compatibility harness tests the engine's
state text, which 0.28.1 reads; it cannot see this layer.

What MPC can tell apart between the two builds: their folder name, `.so` name, uid (`0x43685371`), `version.xml`
(`1.0.0.0`) and AEffect version are the same; their `plugin.json` differs in three fields, compared with the official
release's own (`Cadenza-0.28.1-mpc-sdk.zip`, read 2026-11-25):

| Field | Official 0.28.1 | Test build |
|---|---|---|
| `version` | `0.28.1` | `0.28.1-cpg.1` |
| `description` | "Chord-driven generative MIDI sequencer: [...]" | "Test build of a personal fork of Cadenza ([...]), not an official release." |
| `url` | `https://github.com/Shrekage2024/cadenza-force` | `https://github.com/liotier/cadenza-CPG` |

The likeliest cause is the version. The experiment that tells, with no new build, the official folder installed:

1. In `VST2 Plugins/Cadenza - VST - Cadenza/plugin.json` (a text file), change `"version": "0.28.1"` to
   `"version": "0.28.1-cpg.1"`; restart; open the project of row 6. If it opens, the version is what MPC checks.
2. Else, also replace the `description` and `url` lines with the test build's; restart; open it again. If it opens, one of
   those is; if not, `plugin.json` is not the cause.
3. Either way, put the official `plugin.json` back afterwards.

Also worth knowing: whether a project saved with the official 0.28.1 opens in the test build (the other direction). If it
does not either, MPC asks for the same description both ways; if it does, it refuses only one way.

## 6. Run 2: the toolchain from open-mpc

Decision O18 (strategy, §8): the fork now builds with open-mpc's toolchain carrying the author's six wrapper commits
([spec 00, addendum B](specs/00-infrastructure.md#addendum-b-the-toolchain-from-open-mpc)). Cadenza's code is unchanged;
the wrapper around it is open-mpc's current one. This run checks that nothing moved on the MPC One, and what each of the
six commits does, which is also the device evidence for offering them to open-mpc (O20).

Get and install the new draft as in §1 and §2: **the armv7 zip, its `portable/Cadenza - VST - Cadenza` folder**, not the
SDK zip. Your official installation is that folder (`cadenza.so`); the SDK zip names the file `cadenza-arm32.so`, and
MPC would then refuse, in either build, the states saved by the other (row 5 below, lesson 16). On GLOBAL, the mark's commit is
the branch's latest (CI's release notes name it).

| # | Action | Expected | Checks | Result |
|---|---|---|---|---|
| 1 | Add Cadenza to a new track; open GLOBAL. | It loads; the mark shows the new commit. GLOBAL's MIDI channels read 1, 2, 3 and GATE 100 %. | `OPTION_ANSWER_INDEX` || Passed, 2026-11-26. |
| 2 | Load House 124 and play, PLAY open, for a minute. Then open MPC's PRESET drawer while it plays. | It plays as before. The drawer opens and stays open; its slide-in restarts at most once a bar, never on every beat. | `ON_NO_UPDATE_PREFIX`, `LIT_UPDATE_ON_TEXT_POLL` || Passed, 2026-11-26. |
| 3 | On CHORDS PHRASE, open a bar's editor; tap LOCK, then UNLOCK, three times in a row; then DONE. Again with CANCEL. | Every tap acts, not only the first: the label alternates LOCK and UNLOCK, DONE and CANCEL close the editor each time. | `ECHO_GUARD_POLLED_ONLY` || Passed, 2026-11-26. |
| 4 | Rows 3, 4 and 6 of §3: INIT plays; House 124 plays; a project saves and reopens. | As in run 1. | the wrapper as a whole || Passed, 2026-11-26. |
| 5 | Row 7 of §3, both ways, with the official 0.28.1 from the same kind of package. | A state saved by this build opens in the official one, and the other way. | the saved state's format || Passed, 2026-11-26. |
| 6 | Anything else. | Nothing differs from run 1. | || Passed, 2026-11-26. |

**Run 2, 2026-11-26**, by the owner: MPC One, MPC OS 3.10.1, the draft built from `180f02c` with
`liotier/mpc-vst-plugins@cdf25d6`, installed from the armv7 zip's `portable` folder. **All rows passed**, and with them
§3's row 5 in C♯ and D♯ minor, left unchecked by run 1.

The other two commits have no visible effect on the device: one sets the engine table under `pthread_once` (two instances
created at the same moment), the other only lets the wrapper link on Apple platforms.
