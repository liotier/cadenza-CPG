# MPC memory probe (O10)

Status: procedure, 2026-11-25, corrected 2026-11-26 (both devices have 2 GB; what `base` holds); results to come (§4). It measures, on the owner's MPC One, what Cadenza's screens cost in
memory, so that the fork's skin budget (strategy, §8.2, O10) rests on this device's figures before any new screen
(steps 6 to 10) is specified.

## 1. What is known

- `vst/budget.json`'s `skin_mb` (80 MB, 79.1 used) is a **gauge**: every image of every tab, at 4 bytes a pixel, added
  up. It is not what MPC holds in memory.
- Upstream's own probe (VST-9, on the author's Force) found that **MPC decodes the visible page only**: showing a tab
  cost about 1 to 3 MB of MPC's resident memory, a page with several views about 0.8 MB more while shown. Upstream raised
  the gauge eight times on that basis, each time with its owner's approval.
- The MPC One and the Force both have 2 GB of memory ([MPC One](https://www.musicradar.com/reviews/akai-mpc-one-review),
  [Force](https://www.musicradar.com/reviews/akai-professional-force)), and both run MPC OS on the same family of
  processor, so upstream's finding probably carries over. What is not known: whether it does on this device, and how much
  memory the MPC One has to spare with the owner's projects open. Those two figures decide how far the gauge may grow, or
  whether a per-page limit should replace it. (The second-generation MPCs have more memory, the MPC One's successor 4 GB
  ([Engadget](https://engadget.com/2196725/akai-refreshes-the-mpc-one-and-key-37-workstations)); the 2 GB devices
  set the limit.)

## 2. The probe

`tools/dev/mpc_memprobe.sh`, run on the device as root. It only reads `/proc` and writes nothing but its own log,
`/tmp/mpc-memprobe.csv`. After each label typed and Enter it takes three samples a second apart: the device's
`MemAvailable`, and the MPC process's resident set (`VmRSS`) and proportional set (`Pss`, which counts shared pages
fairly).

From the computer, in the repository's folder (the MPC's address as for SSH):

```bash
scp tools/dev/mpc_memprobe.sh root@<mpc-ip>:/tmp/
```

```bash
ssh -t root@<mpc-ip> sh /tmp/mpc_memprobe.sh
```

## 3. Steps

Prepare a project as you would use Cadenza in it: the instrument tracks Cadenza will play (chords, bass, melody, with
their instruments loaded) and the empty track where Cadenza will load. A project of your own is better than a small one:
`base` should measure what the instruments already hold, so that what follows measures Cadenza alone. Then type each label
below, after doing what it says and waiting a few seconds; an empty Enter samples again under the same label.

| Label | On the MPC |
|---|---|
| `base` | that project open, with its instruments, Cadenza not yet added to the empty track |
| `loaded` | Cadenza added to a track, its window not open |
| `play` | Cadenza's window open on PLAY |
| `chords`, `bass`, `melody`, `global` | each tab in turn |
| `chords-<view>`, `bass-<view>`, `melody-<view>` | each tab's other views you can reach (JAM, PHRASE, the browsers, a bar editor), named as you like |
| `play-again` | back to PLAY |
| `closed` | Cadenza's window closed |
| `reopen` | opened again |

Ctrl-D ends it. Send the terminal's output, or the file:

```bash
scp root@<mpc-ip>:/tmp/mpc-memprobe.csv .
```

The probe can be run with the official build or the test build: their skins differ by the build's mark alone.

## 4. Results

To come. How they will be read: the memory each page adds while shown, whether it falls back when another is shown
(only the visible page decoded, as on the Force), and `MemAvailable` with Cadenza open, the device's real headroom. From
these, a proposal for O10: a raised gauge with a stated margin, or a per-page limit, which measures what MPC holds.
