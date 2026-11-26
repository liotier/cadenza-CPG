# MPC tab-switch probe

Status: procedure, 2026-11-26; run 1 on 2026-11-26 (§4). The owner sees each tab draw after a switch, slowly enough to notice:
"not annoyingly so, but [...] lower latency could be useful". This probe measures, on the MPC One, what a switch costs
MPC and where, so that any change to the skin aims at the actual cost (strategy, principle 10: interaction first).

## 1. What may cost

What MPC does when a plugin page is shown is not documented; three costs are known or likely:

- **Filmstrips written out.** "Every time a plugin screen loads, MPC decodes its filmstrip images (knobs, sliders,
  `meter`s [...]) into `/var/tmp/filmstrips/temp_<hex>.img`, raw RGBA, and never deletes them while running"
  (open-mpc's [NOTES](https://github.com/open-mpc/mpc-vst-plugins/blob/main/docs/NOTES.md), "MPC's filmstrip cache fills
  internal storage over a session", MPC One, 2026-10-01). If that happens at every tab switch, and for every meter, MPC
  decodes and writes megabytes to its internal storage each time. Cadenza's test history records a "filmstrips" figure of
  about 239 MB in its soaks, steady while playing (its [tested.json](../../tested.json), 0.24.1 to 0.28.1), probably that
  cache's size (the soak script is not public).
- **Components built.** Each tab is a set of MPC components, those of its hidden views (`when=`) included, unless MPC
  builds only the shown view's. Cadenza's tabs are dense (from `vst/layout.conf`; meters are filmstrips, the dots of the
  strips and centres; knob strips have 128 frames):

  | Tab | Lines | Meters | Their strips, per component | Distinct strips | Knobs | Buttons | Lists | Readouts |
  |---|---|---|---|---|---|---|---|---|
  | PLAY | 29 | 0 | 0 | 0 | 6 | 0 | 8 | 3 |
  | CHORDS | 444 | 150 | 9.6 MB | 0.7 MB | 24 | 66 | 37 | 73 |
  | BASS | 577 | 226 | 35.2 MB | 1.1 MB | 24 | 90 | 36 | 99 |
  | MELODY | 371 | 160 | 20.6 MB | 0.3 MB | 16 | 49 | 24 | 35 |
  | GLOBAL | 27 | 0 | 0 | 0 | 0 | 0 | 8 | 2 |

  "Per component" counts each meter's strip at frames x width x height x 4 bytes, as if MPC decoded it once per meter;
  "distinct" counts each strip file once. Which of the two MPC does is one of the things the probe shows.
- **Images decoded.** Each view's background and mode images, about 1.6 to 2.4 MB decoded each (`vst/budget.json`'s
  history). The memory probe showed a shown page costing about 2.4 MB, decoded again at each showing
  ([device-memory-probe.md, §4](device-memory-probe.md#4-results)).

## 2. The probe

`tools/dev/mpc_tabprobe.sh`, run on the device as root. It reads `/proc` and the filmstrip cache's folder, and writes
only its own log, `/tmp/mpc-tabprobe.csv`. For each step you type a label and Enter, do the step on the MPC, wait until the
page has drawn, and press Enter again; between the two Enters it records the CPU time of MPC's main thread and of the
whole process, the three busiest threads by name, and the files and kilobytes MPC added to the filmstrip cache. The wall
time between your two Enters is yours: read the CPU times.

```bash
scp tools/dev/mpc_tabprobe.sh root@<mpc-ip>:/tmp/
```

```bash
ssh -t root@<mpc-ip> sh /tmp/mpc_tabprobe.sh
```

## 3. Steps

Cadenza loaded, its window open on PLAY, the transport **stopped** (playing adds the screen's own updates). Labels
without commas; any wording will do, the table's is a suggestion.

| Label | Between the two Enters |
|---|---|
| `idle` | touch nothing for about ten seconds: the baseline |
| `to-chords` | tap CHORDS |
| `to-bass` | tap BASS |
| `to-melody` | tap MELODY |
| `to-global` | tap GLOBAL |
| `to-play` | tap PLAY |
| `to-bass-again` | tap BASS again: a second visit, cached or not |
| `bass-phrase`, `bass-jam`, `bass-bass` | BASS's three views in turn: a view switch inside a tab |
| `to-play-2` | tap PLAY |
| `close`, `open` | close Cadenza's window; open it again |
| `idle-2` | touch nothing for about ten seconds |

Ctrl-D ends. Send the terminal's output, or the file:

```bash
scp root@<mpc-ip>:/tmp/mpc-tabprobe.csv .
```

The filmstrip cache grows by what the probe reports. A reboot clears it; between reboots
`rm -f /var/tmp/filmstrips/temp_*.img` frees it safely when no plugin screen is loading (open-mpc's NOTES).

## 4. Results

**Run 1, 2026-11-26**, by the owner: MPC One, MPC OS 3.10.1, the owner's project, test build `180f02c`, transport stopped.
The log: [measurements/mpc-tabprobe-2026-11-26.csv](measurements/mpc-tabprobe-2026-11-26.csv) (the first two runs stopped
on BusyBox's `head`, lesson 17; their idle lines are kept). MPC's main thread uses 24.9 ms a second at rest (the three idle
lines, 69.8 s in all); each step's cost is its main-thread time less that rate over the step's wall time. The steps took
9 to 13 s each between the Enters, so each figure is uncertain by about 50 to 80 ms (the idle rate varied from 19 to
25 ms a second between runs).

| Step | Main thread, net | Tab's lines (`vst/layout.conf`) |
|---|---|---|
| `to-play` (from GLOBAL), `to-play-2` (from BASS) | 79, 123 ms | 29 |
| `to-global` | 101 ms | 27 |
| `to-chords` | 283 ms | 444 |
| `to-melody` | 337 ms | 371 |
| `to-bass`, `to-bass-again` | 426, 384 ms | 577 |
| `bass-phrase`, `bass-jam`, `bass-bass` (views inside BASS) | 260, 347, 245 ms | |
| `close`, `open` (the window, on PLAY) | 196, 210 ms | |

**What it shows.**

1. **No filmstrip is written at a switch**: the cache did not grow at any step, opening the window included. That suspect
   is cleared, provided the cache is still `/var/tmp/filmstrips` under MPC OS 3.10.1 (the probe counts nothing if the
   folder is missing; to check).
2. **A switch costs the main thread 0.1 s on the light tabs and 0.3 to 0.4 s on the dense ones**, in rough proportion to
   the tab's lines, hidden views included: on the order of 0.6 ms a line above about 80 ms. This is the delay the owner sees.
3. **A view switch inside BASS costs nearly as much as a tab switch** (0.25 to 0.35 s). Each view switch is a parameter
   with `refresh_display` (as are 2 008 of the 2 037 parameters), so MPC is asked to refresh the screen; but a refresh alone
   was measured at about 1.2 ms on the Force (`vst/budget.json`, `_screen_updates_per_s`). The cost is therefore in what
   the view change makes MPC show, hide and draw, not in the refresh request.
4. **The process totals are MPC's audio threads** (`Audio_Processing`, `AudioWorker0` and `2`), busy at the same rate at
   rest: they say nothing about the switch.

**Open questions**, for a second run: which components cost most (the BASS tab's 226 dot meters, its 90 buttons, its
hidden views); whether a switch also costs MPC's Q-Link work (each dense tab binds 16 to 24 knobs, PLAY 6); and the
owner's observation that opening Cadenza from MPC's main screen through Program Edit feels quicker than switching tabs
inside it.

How they will be read, and what each outcome would lead to:

- **Filmstrip files written at each switch**, in proportion to the meters (the "per component" column): MPC decodes and
  writes a strip per meter. Then fewer frames (the dots' strips have 36 to 73, the knobs' 128), fewer or smaller strips,
  and lit list tiles or pictures instead of meters where they can do the same, cut the switch's cost directly.
- **Main-thread time in proportion to the tab's lines**, hidden views included: MPC builds every view of a tab. Then the
  rarer views can move where they cost only when shown, and static elements can go into the background art.
- **Main-thread time in proportion to the shown view only**, and a view switch inside a tab as costly as a tab switch:
  the images and components of the view itself are the cost. Then the view's own density is the lever.
- **A second visit much cheaper than the first**: MPC caches, and only first visits matter.
