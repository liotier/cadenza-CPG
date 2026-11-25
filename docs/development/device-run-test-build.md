# Device run of the test build (O1)

Status: procedure, 2026-11-24; run 1 recorded in §5: row 7 failed, its cause being narrowed down (§5.1). Decision O1 of the
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
3. Download `Cadenza-0.28.1-cpg.1-mpc-sdk.zip`, the zip for MPC OS 3.10.1 and later (the other, `-mpc-armv7.zip`, is for
   3.9.1 and earlier, with an installer over SSH).
4. Unzip it on the computer. It holds one folder, `Cadenza - VST - Cadenza`: the plugin (`.so`), its `plugin.json` and its
   skin. It is the 32-bit build, as the official release is, which already runs on this MPC One.

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
| 1 | Add Cadenza to a track, from where the official one was in MPC's plugin list. | It loads; its screen draws. | |
| 2 | Open GLOBAL. Then on its piano tap the black key D# (the key selector labels every black key with its sharp, in both builds: spec 01, S01-D7), SCALE Major, and look at PLAY's title. | On GLOBAL's title row, in magenta between the part's name and TEMPO: `CPG 0.28.1-cpg.1` and the commit of step 1.2; the official build has nothing there. PLAY's title: `Eb MAJOR` (the official build shows `D# MAJOR`). | |
| 3 | Load the INIT program and play. | Chords, Bass and Melody sound on their tracks, as with the official build. | |
| 4 | Load House 124 and play. | It plays as with the official build. | |
| 5 | In each key of the table below (on GLOBAL's piano: F, D#, G#, C# for the majors, C# and D# with SCALE Minor), look at CHORDS, PHRASE, PLAY and GLOBAL while a progression plays (a preset, or chords picked on PHRASE). | Every chord name is one of that key's row, never its other spelling: no A♯ in F major, no D♭ in C♯ minor, no D♯ in E♭ major. PLAY's title as in the first column. | |
| 6 | In E♭ major with House 124 loaded, change something you can recognise (a chord picked on PHRASE), save the MPC project, then open it again. | It reopens as saved: key, chords, sound. | |
| 7 | Swap back to the official build (§4), restart, open the project of row 6. | It opens and plays the same chords; PLAY's title now reads `D# MAJOR`, the official spelling of the same key. | |
| 8 | Anything else you notice. | Nothing differs from 0.28.1 but the spelling. | |

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
| 7 | **Failed.** The official 0.28.1 refuses the test build's saved state: "This state file is not for Cadenza". |
| 8 | Nothing reported. |

The first loads found one procedural flaw, now fixed: the procedure named E♭ on the key selector, whose black keys carry
sharps in both builds (S01-D7); and the owner asked for the build's mark on GLOBAL (strategy, §4.5).

### 5.1 Row 7: what refuses the state

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
