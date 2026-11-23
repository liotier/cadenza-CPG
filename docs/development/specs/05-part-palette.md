# 05 - The part's palette

| | |
|---|---|
| Status | §2.1 answered by the owner, 2026-11-23; approved once spec 04 is, which it rests on |
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

Both answered by the owner, 2026-11-23, as proposed (study, decisions 22 and 23).

| # | Question | Answer | Alternatives considered | Rests on |
|---|---|---|---|---|
| S05-D2 | Does a project keep the palette it was made with, or follow the catalogue? | **It keeps it** (owner: the principle of least surprise, respect for the composer, and a cleaner boundary between the CPG and Cadenza): saved by value in a new field `<P>.pl=` (hex), only for a part that holds one, about 95 bytes a part. A later CPG revision never changes a saved project, as Cadenza's rule is for everything else. 0.28.1 ignores the unknown field: the slots play as written, the palette is dropped there. | Save only the catalogue identifier: a few bytes, and the project's palette follows CPG updates, which may then reshape a song's harmonic space without the user doing anything | S05-D1 |
| S05-D9 | Do the six genre presets get palettes? | **No** (owner: Cadenza's genres are about time, the CPG's are palettes; orthogonal dimensions): their parts hold none, and show their scale's palette (S05-D3). Presets are baked states (`presets_data.h`) whose building tool is not in the release export; they keep sounding the same. | Give each preset's parts the nearest catalogue palette: re-baking the presets, without the author's tool | none |

### 2.2 For the owner: after a further step

| # | Question | Waits for | Until then |
|---|---|---|---|
| S05-D4 | When a palette loads, does the loop keep its length, or take the starting order's? | step 6's device run, where loading is first felt | **Keep the loop's length**, as the progression presets do: `seq_palette_load` sets the part's palette and writes the starting order into the slots in one undo step, keeping locked slots, the order repeating over a longer loop as a preset's phrase does |

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
  implementation, not for the owner; S05-D4 is in §2.2.

## 8. Implementation notes

Empty until implementation.
