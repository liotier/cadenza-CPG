# MPC memory probe (O10)

Status: procedure, 2026-11-25, corrected 2026-11-26 (both devices have 2 GB; what `base` holds); run 1 on 2026-11-26 (§4). It measures, on the owner's MPC One, what Cadenza's screens cost in
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

**Run 1, 2026-11-26**, by the owner: MPC One (2 065 140 kB of memory, that is 2 115 MB), MPC OS 3.10.1, the owner's project
with its instruments, Cadenza 0.28.1-cpg.1 test build `180f02c`. The log:
[measurements/mpc-memprobe-2026-11-26.csv](measurements/mpc-memprobe-2026-11-26.csv). Figures in MB (10^6 bytes, as the
gauge counts; the kernel's kB are 1 024 bytes), the last sample of each label:

| Label | MPC resident | Step | Against `base` | MemAvailable |
|---|---|---|---|---|
| `base` | 1 189.9 | | | 742.9 |
| `loaded` (window closed) | 1 207.6 | +17.6 | +17.6 | 731.1 |
| `play` | 1 211.3 | +3.8 | +21.4 | 727.3 |
| `chords` | 1 217.8 | +6.5 | +27.8 | 720.7 |
| `bass` | 1 223.0 | +5.2 | +33.1 | 714.9 |
| `melody` | 1 223.0 | 0.0 | +33.1 | 715.0 |
| `global` | 1 221.2 | -1.8 | +31.2 | 716.9 |
| eight other views (CHORDS, PHRASE and JAM of each lane) | 1 223.0 to 1 224.0 | -0.5 to +1.8 | +33.0 to +34.0 | 713.2 to 714.8 |
| `play-again` | 1 221.6 | -1.8 | +31.7 | 715.6 |
| `closed` | 1 219.2 | -2.4 | +29.3 | 718.8 |
| `reopen` | 1 221.6 | +2.4 | +31.7 | 715.2 |

The proportional set (`Pss`) follows the resident set 7 MB below it throughout: little of it is shared.

**What it shows.**

1. **MPC does not keep every page it has shown.** Over thirteen views, the resident set rose with the first three tabs,
   then stayed within 1 MB of its high point however many more were shown: 34.0 MB over `base` at most, for the plugin and
   its whole window. The skin gauge reads 79.1 MB for the same skin: it is no measure of what MPC holds. Upstream's finding
   on the Force (VST-9: the visible page only) holds on the MPC One.
2. **A shown page costs about 2.4 MB.** Closing the window gave back 2.4 MB, reopening it took exactly that again: PLAY's
   page, within upstream's "1 to 3 MB". The 10 MB still held after the tour (`play-again` against `play`) is either kept
   by MPC or kept by the allocator for reuse; the flat high point says it is reused, not accumulated.
3. **Loading Cadenza costs more than its window**: 17.6 MB with the window closed, against at most 16.4 MB more for the
   window and all its views. What makes up the 17.6 MB is not measured here; MPC's host objects for 2 037 parameters (about
   8.6 kB each, if they were all of it) are a likely part, besides the engine and the 662 KB plugin. Parameters, which new
   screens add, may cost device memory as much as images do.
4. **The headroom is large.** With Cadenza open and every view shown, 713 to 716 MB remained available, a third of the
   device; Cadenza's 29 to 34 MB at its peak is 4 to 5 % of it. On this device the project's own instruments and samples,
   not Cadenza, decide how close MPC comes to its limit.

**For O10** (strategy, §8): the device does not bind the skin budget. A raise of the gauge costs, on this evidence, the
size of the largest new page while shown, not the sum. A per-page limit would measure what MPC holds; a raised gauge with a
margin is simpler and keeps the build's existing check. Either way, the probe takes six minutes and can be run again after
each new view.
