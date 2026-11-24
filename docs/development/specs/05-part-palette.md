# 05 - The part's palette

| | |
|---|---|
| Status | implemented, 2026-11-24 (§8) |
| Type | engine, save format |
| Size | M |
| Depends on | 02, 03, 04 |
| Blocks | 06 to 10 |

## 1. Purpose

The palette becomes a property of the part, and the part's slots its current order (study, §4.3). This step gives the
engine that property, saves it, loads a catalogue palette with its starting order, and answers the two questions decision
18 settled without a C port: which palette a part shows when none was loaded, and which catalogue palette fits a
progression built from scratch. No screen yet: steps 6 to 10 build on these verbs.

What steps 0 to 2 measured, and this revision uses:

- Step 2's path writes any chromatic chord through every writer, undo, chord-length change, COPY TO and save; step 3's
  extends it to the full vocabulary. The verbs of this step write through those paths.
- The table of step 4 stays out of the core (S04-D11). The core holds a part's palette by value; the catalogue, the scale
  palettes and the nearest-palette query belong to the plugin's front end.
- A six-part genre preset saves in about 1.6 KB of the 8 KB state.

## 2. Decisions

Approving this specification means answering §2.1; §2.2 waits for a later step, with a proposed answer meanwhile; §2.3
follows from decisions already taken or from the code, needs no answer of its own, and is there to be read and
questioned. *Rests on*: what a decision presupposes, here or elsewhere.

### 2.1 For the owner: decidable now

All three answered by the owner, 2026-11-23: S05-D2 and S05-D9 as proposed (study, decisions 22 and 23), S05-D4 with
S04-D9.

| # | Question | Answer | Alternatives considered | Rests on |
|---|---|---|---|---|
| S05-D2 | Does a project keep the palette it was made with, or follow the catalogue? | **It keeps it** (owner: the principle of least surprise, respect for the composer, and a cleaner boundary between the CPG and Cadenza): saved by value in a new field `<P>.pl=` (hex), only for a part that holds one, about 95 bytes a part. A later CPG revision never changes a saved project, as Cadenza's rule is for everything else. 0.28.1 ignores the unknown field: the slots play as written, the palette is dropped there. | Save only the catalogue identifier: a few bytes, and the project's palette follows CPG updates, which may then reshape a song's harmonic space without the user doing anything | S05-D1 |
| S05-D9 | Do the six genre presets get palettes? | **No** (owner: Cadenza's genres are about time, the CPG's are palettes; orthogonal dimensions): their parts hold none, and show their scale's palette (S05-D3). Presets are baked states (`presets_data.h`) whose building tool is not in the release export; they keep sounding the same. | Give each preset's parts the nearest catalogue palette: re-baking the presets, without the author's tool | none |
| S05-D4 | When a palette loads, does the loop keep its length, or grow until the starting order fits? | **Answered with S04-D9 (owner, 2026-11-23): Feel's preset rule.** The loop doubles until the laid-out order fits, never shrinks; a longer loop repeats the order, as a preset's phrase repeats. `seq_palette_load` sets the part's palette and writes the order, one chord a bar, through the whole-loop progression path in one undo step, keeping locked slots | PHRASE's preset rule: the loop keeps its length, the order repeated or cut to fit it | S04-D9 |

### 2.2 For the owner: after a further step

None. S05-D4, first placed here to wait for step 6's device run, turned out to be the same choice as S04-D9, decidable
now (§2.1).

### 2.3 Following from decisions already taken

| # | Decision | Rationale | Rests on |
|---|---|---|---|
| S05-D1 | **Each part may hold a palette:** up to 16 chords as step 4 represents them (`cpg_chord_t`) and the identifier of the catalogue entry it came from. | The palette is the part's property, the slots its current order. | study §4.3 (decision 2); S04-D4 |
| S05-D3 | **A part without a palette holds none in the core;** the front end shows its scale's palette (`cpg_scale_palette`), following key and scale changes. | The implied palette, at no cost in the state and with no catalogue in the core. | decision 18; S04-D11 |
| S05-D5 | **Each chord maps to the project scale when written:** a degree of the scale if one voices it exactly, else one of the three borrowed chords, else a chromatic source (first range, then second); the colour carried over. | Numerals, names and the bar editor then read the chord as any other. A scale change afterwards keeps each chord's sound (its representation may change) and the palette unchanged. | study §6.1; specs 02 and 03 |
| S05-D6 | **Editing never removes the palette;** `seq_palette_clear` does, as its own undo step. | The palette is the part's harmonic space; the order is only its current path. | decision 18 |
| S05-D7 | **The nearest catalogue palette is the front end's query,** over the part's distinct chords and the table, within the project scale's family; ties to catalogue order. Loading it is a separate act. | A suggestion for the screens; never a silent change of the part. It needs the catalogue, which the core does not link. | decisions 6 and 18; S04-D11 |
| S05-D8 | **A part used for the first time copies the Verse's palette,** with everything else it copies. | Today's rule for parts. | S05-D1, S05-D2 |

## 3. Specification

### 3.1 Engine (core)

| Verb or query | Behaviour |
|---|---|
| `seq_palette_load(s, id, chords, n, order, n_order)` | S05-D4, S05-D5; one undo step; nothing for bad arguments. The front end passes the table's row (`cpg_find(id)`) |
| `seq_palette_clear(s)` | S05-D6 |
| `seq_palette(s, out)` | the playing part's palette, its count (0: none, S05-D3) |
| `seq_palette_id(s)` | the loaded entry's identifier, 0 for none |

### 3.2 Front end (`src/palette.c`, linked with `src/cpg_palettes.c` by the plugin and its test)

`palette_shown(s, out)`: the part's palette, else its scale's (S05-D3). `palette_nearest(s)`: S05-D7, deterministic.

### 3.3 Saved state

`<P>.pl=`: the identifier (8 hex digits), the count (1), then per chord root (1), quality (1), colour (1), spice (1) and
category (1), all hex: at most 89 characters and the field's name, about 95 bytes a part, 570 for six. Chords using step
3's values are written into the slots with step 3's fallbacks, so an older build plays the fallbacks there.

## 4. Measurements

| Quantity | How |
|---|---|
| The state's size | the largest state the tests build, with six loaded palettes, against 8 KB; a part that does not fit is left out whole (`seq_unsaved_parts`), which the tests must never see |
| Binary | before and after, locally and in CI |
| Core parity hash | unchanged for projects without palettes |

## 5. Verification

1. Loading writes the expected slots for the starting orders of step 4's §3.3, in a major and a minor project, locks
   kept; undo restores slots and palette together.
2. Save and load round trips with loaded palettes, as fixed points (strategy, §3, lesson 4); no palette is saved for a
   part without one.
3. A scale change keeps every loaded chord's pitch classes and the palette unchanged.
4. The nearest query: deterministic, within the family, on hand-made progressions with known answers (plugin-side test).
5. A first-used part copies the Verse's palette.
6. The compatibility harness: 0.28.1 loads states with palettes, their slots as written (step 3's fallbacks where they
   apply), the palette dropped, as `tests/compat/expected.txt` pins.
7. The core parity hash unchanged.

**The author's checks this step moves:** none expected. **Other front ends:** new core verbs and the `.pl` field, which
a front end that does not use them leaves alone; for the design note to the author (milestone M2).

## 6. Out of scope

Every screen (steps 6 to 10); editing a palette's chords, which nothing has asked for yet.

## 7. Open points, to settle while implementing

- How Feel's `base` field should record a palette load (as the chord first written, presumably: 0). A question of
  implementation, not for the owner; S05-D4 is in §2.2. *Settled (§8.2, 6): 0, as a Feel preset leaves it.*

## 8. Implementation notes

### 8.1 What was built

| Layer | Change |
|---|---|
| `src/theory.h`, `src/theory.c` | The palette chord, `theory_palette_chord_t` (root, quality, colour, spice, category, start), shared by the core and the table; `theory_palette_ok`, `theory_palette_src`, and `theory_palette_place`, S05-D5's rule. |
| `src/cpg_palettes.h` | `cpg_chord_t` is now `theory_palette_chord_t`: the table and the core hold chords alike, and the core includes no header of the table (S04-D11). |
| `src/seq_core.h`, `src/seq_core.c` | `seq_palette_t` in `seq_part_t` (S05-D1) and in the undo snapshot (`push_snap_part`, `restore_prog`); `seq_palette_load`, `seq_palette_clear`, `seq_palette`, `seq_palette_id`; the `.pl` field (`out_palette`, `load_pl`); `palette_rescale` in `seq_set_scale` (S05-D5). A part's first use copies the whole Verse, its palette with it (S05-D8): no code needed. |
| `src/palette.h`, `src/palette.c` | The front end (§3.2): `palette_shown`, `palette_nearest`, and `palette_load_entry`, which passes a catalogue entry and its starting order to `seq_palette_load`. Joins `vst/vst.json`'s sources and the plugin test's command line. |
| Tests | `tests/test_palette.c` (a new line of `tests/run_tests.sh`): §5's items 1 to 5 and §4's state size; `tests/compat/writer.c`: the `palette` scenario, pinned. |

### 8.2 Choices made while building

1. **One chord type.** The specification said the core holds chords "as step 4 represents them (`cpg_chord_t`)". So that
   the core need not include the table's header, the type moved to `theory.h` and the table's name became an alias.
2. **`.pl`'s count and flags.** One hex digit cannot hold 16, so the field holds the count less one. The starting-order
   flag (S05-D1: the chords as step 4 represents them) shares a digit with the spice, `spice + 4 × start`, which keeps §3.3's
   five characters a chord: 89 at most, as specified.
3. **The loop's growth is not undone**, exactly as with Feel's presets (`seq_feel_preset`), which grow the loop through
   `seq_set_bars` before their undo step: undo restores the slots and the palette, and leaves the loop as long as the load
   made it. A step that wants the length undone too would change Feel's presets as well; left as 0.28.1 has it.
4. **S05-D5's placement** carries the colour over: a degree whose chord, in that colour, has the same root and pitch
   classes; else such a borrowed chord; else the chromatic source. In C Dorian, Axis of Awesome's C and Am are chromatic,
   G is the borrowed V, F the IV.
5. **A scale change** writes again every slot and change point whose root, colour and pitch classes are a palette chord's
   (`palette_rescale`), so it keeps its sound; any other chord of the part follows the scale as it always has. Tested over
   every scale with the blues, and for a part without a palette, whose degrees still follow.
6. **Feel's state of a written slot** (mute, base, hand notes) goes, as a Feel preset clears it; a locked slot keeps its
   own. A loaded chord's base is 0: §7's open point.
7. **The nearest palette** reads the part's slots, not its change points, and counts a slot chord as held when an entry
   has a chord of the same root and pitch classes. Tested on C G Am F (Axis of Awesome, the first entry holding all four)
   and, for 40 random progressions over every scale, against a count made independently in the test.
8. **Bad arguments** to `seq_palette_load` change nothing and return 0: a chord out of range, an empty palette, an order
   of a length other than 1, 2, 4, 8 or 16 bars, or an index past the chords.

### 8.3 Measurements

| Quantity | Expected | Measured |
|---|---|---|
| The state, six parts of 16 bars, each with a loaded palette | within 8 KB, no part left out | 2 358 bytes, every part saved |
| `.pl` | about 95 bytes a part | 95 for 16 chords, its name included |
| Core parity hash | unchanged | unchanged |
| Compatibility harness | `.pl` dropped by 0.28.1 | 21 scenarios: 16 identical, 5 as pinned; `palette`'s re-save drops `.pl` and the It6's `.cy`, every other field as written |
| Binary | about 2 to 4 KB | CI: 654 KB, from 650 after step 3 (CI run 20); the local estimate, +1 416 bytes of code, undercounted. The table joins the plugin when a screen calls the front end (phase B) |

### 8.4 Other front ends

New core verbs and the `.pl` field, which a front end that does not use them leaves alone; a front end saving through
`seq_state_save` writes it without knowing. For the design note to the author (milestone M2), with spec 03's.
