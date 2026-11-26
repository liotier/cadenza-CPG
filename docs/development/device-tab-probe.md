# MPC tab-switch probe

Status: procedure, 2026-11-26; results to come (§4). The owner sees each tab draw after a switch, slowly enough to notice:
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

To come. How they will be read, and what each outcome would lead to:

- **Filmstrip files written at each switch**, in proportion to the meters (the "per component" column): MPC decodes and
  writes a strip per meter. Then fewer frames (the dots' strips have 36 to 73, the knobs' 128), fewer or smaller strips,
  and lit list tiles or pictures instead of meters where they can do the same, cut the switch's cost directly.
- **Main-thread time in proportion to the tab's lines**, hidden views included: MPC builds every view of a tab. Then the
  rarer views can move where they cost only when shown, and static elements can go into the background art.
- **Main-thread time in proportion to the shown view only**, and a view switch inside a tab as costly as a tab switch:
  the images and components of the view itself are the cost. Then the view's own density is the lever.
- **A second visit much cheaper than the first**: MPC caches, and only first visits matter.
