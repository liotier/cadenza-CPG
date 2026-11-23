# 05 - The part's palette

| | |
|---|---|
| Status | for approval (revised after steps 0 to 2) |
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
- The table of step 4 stays out of the core (its D11). The core holds a part's palette by value; the catalogue, the scale
  palettes and the nearest-palette query belong to the plugin's front end.
- A six-part genre preset saves in about 1.6 KB of the 8 KB state.

## 2. Decisions

The last column names what each decision presupposes, here or in another specification; a decision resting on none can be judged alone, and changing one changes those that rest on it.

| # | Decision | Rationale | Rests on |
|---|---|---|---|
| D1 | **Each part may hold a palette:** up to 16 chords as step 4 represents them (`cpg_chord_t`) and the identifier of the catalogue entry it came from. | The study's §4.3. | spec 04, D4 |
| D2 | **Saved by value,** in a new field `<P>.pl=` (hex), only for a part that holds one. | A project keeps the palette it was made with, whatever later CPG revisions do to the catalogue: Cadenza's rule for everything else. 0.28.1 ignores unknown fields (its `find_fields` matches known names only), so there the slots play as written and the palette is dropped. | D1 |
| D3 | **A part without a palette holds none in the core;** the front end shows its scale's palette (`cpg_scale_palette`), following key and scale changes. | Decision 18's implied palette, at no cost in the state and with no catalogue in the core (step 4, D11). | spec 04, D11 (decision 18) |
| D4 | **Loading is one undo step:** `seq_palette_load` sets the part's palette and writes the starting order into the slots, keeping locked slots and the loop's length, the order repeating over a longer loop as a preset's phrase does. | Keeping the loop's length is what the progression presets do today (open point). | D1, D5 |
| D5 | **Each chord maps to the project scale when written:** a degree of the scale if one voices it exactly, else one of the three borrowed chords, else a chromatic source (first range, then second); the colour carried over. | The study's §6.1 order; numerals, names and the bar editor then read the chord as any other. A scale change afterwards keeps each chord's sound (its representation may change) and the palette unchanged. | specs 02 and 03 |
| D6 | **Editing never removes the palette** (decision 18); `seq_palette_clear` does, as its own undo step. | The palette is the part's harmonic space; the order is only its current path. | D1 (decision 18) |
| D7 | **The nearest catalogue palette is the front end's query,** over the part's distinct chords and the table, within the project scale's family (decision 6); ties to catalogue order. Loading it is a separate act. | A suggestion for the screens; never a silent change of the part. It needs the catalogue, which the core does not link. | spec 04, D11 (decision 6) |
| D8 | **A part used for the first time copies the Verse's palette,** with everything else it copies. | Today's rule for parts. | D1, D2 |
| D9 | **Genre presets are unchanged:** their parts hold no palette. | Presets are baked states (`presets_data.h`); they keep sounding the same. | none |

## 3. Specification

### 3.1 Engine (core)

| Verb or query | Behaviour |
|---|---|
| `seq_palette_load(s, id, chords, n, order, n_order)` | D4, D5; one undo step; nothing for bad arguments. The front end passes the table's row (`cpg_find(id)`) |
| `seq_palette_clear(s)` | D6 |
| `seq_palette(s, out)` | the playing part's palette, its count (0: none, D3) |
| `seq_palette_id(s)` | the loaded entry's identifier, 0 for none |

### 3.2 Front end (`src/palette.c`, linked with `src/cpg_palettes.c` by the plugin and its test)

`palette_shown(s, out)`: the part's palette, else its scale's (D3). `palette_nearest(s)`: D7, deterministic.

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

- Whether loading should keep the loop's length (D4) or take the starting order's: to decide with step 6's device run,
  where it is first felt.
- How Feel's `base` field should record a palette load (as the chord first written, presumably: 0).

## 8. Implementation notes

Empty until implementation.
