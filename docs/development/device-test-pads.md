# Device test: can the MPC One's pads reach Cadenza ?

Status: protocol, not yet run. It answers question 2 of the integration study
([§11](cpg-integration-study.md#11-open-questions)): whether the hardware pads can play avenue L's live palette pads.
Run on the way, it also tells whether notes written in Cadenza's own clip reach Cadenza, which bears on the parked question
of parts and clips (end of [§7](cpg-integration-study.md#7-integration-avenues)).

Device: an MPC One under MPC OS 3.10.1, external plugin support enabled, Cadenza 0.28.1 installed in `VST2 Plugins`.

## 1. What the code already settles

Every MIDI event MPC hands to Cadenza's track reaches Cadenza's engine, which discards it:

| Step | Where | What happens |
|---|---|---|
| MPC sends the plugin its track's MIDI through VST2 `effProcessEvents` | MPC | the unknown this test explores |
| The wrapper declares it receives MIDI | [vst2_wrap.c](https://github.com/Shrekage2024/mpc-vst-plugins/blob/8fbeffc/wrapper/vst2_wrap.c#L977), `effCanDo` | "receiveVstEvents" and "receiveVstMidiEvent" answered yes |
| The wrapper passes each MIDI event to the engine | [vst2_wrap.c](https://github.com/Shrekage2024/mpc-vst-plugins/blob/8fbeffc/wrapper/vst2_wrap.c#L956), `effProcessEvents` then `eng_midi` | only CC and NRPN maps may claim an event, and Cadenza enables neither ([vst/vst.json](../../vst/vst.json): `"cc": false`, `"nrpn": false`) |
| Cadenza's engine receives it | [engine.c:362](../../src/engine.c#L362), `midi()` | a deliberate no-op: "v1 takes no MIDI input" |

Two more facts from the code:

- MPC routes Cadenza's own output port back into Cadenza's track, and the engine ignores that echo ([engine.c](../../src/engine.c),
  header comment). Pad notes, if they arrive, arrive mixed with it.
- The toolchain provides `plugin_dir.h`, "so an engine can find presets, banks, ROMs and other files shipped next to it"
  ([plugin_dir.h](https://github.com/Shrekage2024/mpc-vst-plugins/blob/8fbeffc/wrapper/plugin_dir.h)): a diagnostic build
  can write its log into its own folder on the card.

The questions are therefore all on MPC's side:

| # | Question |
|---|---|
| Q1 | Does MPC send pad notes to the plugin on Cadenza's track ? |
| Q2 | Which notes, channel and velocities, per pad and bank; any pressure events ? |
| Q3 | Can pad notes be told apart from the echo of Cadenza's own port, by a track setting or by channel ? |
| Q4 | Do notes written in Cadenza's own clip reach Cadenza when the clip plays ? |
| Q5 | Does a Pad Perform chord pad arrive as several notes ? |

## 2. Stage 1: without code

A stock Cadenza 0.28.1 discards what it receives, so stage 1 observes MPC's routing from the outside: through a synth
plugin built with the same toolchain, and through what MPC records on Cadenza's track.

Before starting: save any open project, then make a new one at 120 BPM. Keep a pencil, or the phone's camera, for the
results table of §4.

| Step | Action | Record |
|---|---|---|
| 1.1 | Add a plugin track with a synth built on the same toolchain (sd88me's OB-Xd or Dexed). Select it and play pads A1 to A16. | Does it sound ? If yes, MPC hands pad notes to plugins built on this wrapper (Q1, for the toolchain). |
| 1.2 | Record one hit on each pad into a clip on that track, banks A to D. Open the clip's event list. | Note number and velocity per pad, per bank; the track's MIDI channel setting (Q2). |
| 1.3 | Switch the pads to Pad Perform, Chords mode, press one pad, record, inspect. Repeat in Progressions mode. | How many notes per press (Q5). |
| 1.4 | Set Cadenza up as usual: its track, the three receiving tracks, its clip. Open the Cadenza track's settings. | Its INPUT PORT and INPUT CHANNEL as they are by default. |
| 1.5 | Record-arm the Cadenza track and let its clip play two bars, touching nothing. Stop, inspect the Cadenza clip, then undo the recording. | Notes recorded ? If so, the echo of Cadenza's own port enters its track; their channels and notes if shown (Q3). |
| 1.6 | Change the Cadenza track's INPUT PORT to each available choice in turn, and repeat 1.5 with a few pad hits added. | The setting, if any, that records the pads but not the echo (Q3). |
| 1.7 | With the Cadenza track selected and Cadenza playing, play pads for a while. | Sanity: Cadenza's output unchanged, no stuck notes on the receiving tracks. |

Stage 1 delivers the pad map, the separating setting and Q5. Q1 and Q4 for Cadenza itself need stage 2.

## 3. Stage 2: a diagnostic build

Stage 2 needs code: a diagnostic-only build, like the existing `TAP_LOG` one ([engine.c](../../src/engine.c), `tap_log`),
built with `CFLAGS_EXTRA=-DMIDI_IN_LOG vst/build.sh` and never released. It is outside `docs/development`, so it waits for
the owner's go-ahead.

### 3.1 What the diagnostic build does

- **Records.** `midi()` stores each event (status byte, two data bytes, and the song position of the block it arrived in)
  in a fixed ring of 256 entries, and counts note-ons per channel. It classifies each note-on as *echo* when Cadenza itself
  sent that note on that channel within the last two blocks, *foreign* otherwise. No allocation and no I/O in the callback,
  which runs on the audio thread under the engine lock.
- **Shows.** PLAY's title line, an existing text cell, displays the last event and the counters, for instance
  `IN ch10 n36 v87 FOREIGN · f12 e340`. The skin and the parameter list are unchanged, so the build differs from 0.28.1 by
  `cadenza.so` alone.
- **Logs.** A tap on PLAY's UNDO, which runs on the screen thread, appends the ring to `cadenza-midi-in.log` in the plugin's
  own folder (found with `plugin_dir.h`), one line per event, readable on a computer.

### 3.2 Installation

Copy the diagnostic `cadenza.so` over the installed one in `VST2 Plugins/Cadenza - VST - Cadenza`, after keeping a copy of
the original, and restart the MPC. The plugin identity is unchanged (decision 11 of the study), so existing projects open
in it. Restore the original file after the test.

### 3.3 Steps

| Step | Action | Expected if pads work | Record |
|---|---|---|---|
| 2.1 | Stopped, Cadenza track selected: press each pad of bank A once. | FOREIGN note-ons and note-offs, the notes of step 1.2 | channel, notes, velocities (Q1, Q2) |
| 2.2 | Playing, pads untouched, for eight bars. | the echo count rises, the foreign count stays flat | both counts (Q3, baseline) |
| 2.3 | Playing, pads pressed. | the foreign count rises with each press | the pads' channel against the lanes' 1, 2, 3; if they collide, move the lanes on GLOBAL (MIDI CHANNELS) and repeat (Q3) |
| 2.4 | Repeat 2.2 and 2.3 with the INPUT PORT setting found in 1.6. | no echo at all | both counts (Q3) |
| 2.5 | Select another track, press pads. | nothing reaches Cadenza | counts unchanged |
| 2.6 | Write four notes in Cadenza's clip, on beat 1 of bars 1 to 4, and play. | four FOREIGN note-ons at those positions | positions shown (Q4) |
| 2.7 | Pad Perform chords: press one pad. | several note-ons at once | count per press (Q5) |
| 2.8 | Hold a pad and press harder. | pressure events, status `A0` (polyphonic) or `D0` (channel), if the pads send them | statuses seen (Q2) |
| 2.9 | Playing, tap a pad on a beat, ten times. | arrival within a block of the beat | positions from the log |
| 2.10 | Tap UNDO to write the log; copy `cadenza-midi-in.log` from the card. | one line per event | the file |

## 4. Results

| # | Question | Observation | Evidence (photo, log lines) |
|---|---|---|---|
| Q1 | Pad notes reach Cadenza | | |
| Q2 | Pad map, channel, velocity, pressure | | |
| Q3 | Separable from the echo, and how | | |
| Q4 | Clip notes reach Cadenza | | |
| Q5 | Pad Perform chords arrive as several notes | | |

## 5. What the results decide

- **Pads arrive and can be separated (Q1, Q3).** Avenue L's pads can be played on the hardware: bank A's sixteen pads as the
  part's sixteen palette chords, each landing at SWITCH AT, the on-screen pads remaining for the rest.
- **Clip notes arrive (Q4).** MPC's own clips could then carry Cadenza's harmony: chord changes recorded as notes in
  Cadenza's clip, edited in MPC's piano roll and arranged in the Clip Matrix, while Cadenza writes the three lanes around
  them. That would bring Cadenza a step closer to the MPC way, and belongs to the parked question of parts and clips.
- **The echo cannot be separated by a setting (Q3).** Separate by channel, the pads on a channel no lane uses, or by
  matching against what Cadenza itself sent, which the engine knows.
- **Chords arrive as several notes (Q5).** MPC's own Pad Perform chords could feed Cadenza too, identified as the CPG's
  Finder identifies chords from their notes.
- **Pads never arrive (Q1).** Avenue L keeps its on-screen pads only.
