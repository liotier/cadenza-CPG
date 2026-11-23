# 05 - The part's palette

| | |
|---|---|
| Status | draft: design proposals, to be revised once steps 0 to 4 are implemented |
| Type | engine, save format |
| Size | M |
| Depends on | 02, 04 |
| Blocks | 06 to 10 |

## 1. Purpose

The palette becomes a property of the part, and the part's slots its current order (study, §4.3). This step gives the
engine that property, saves it, loads a catalogue palette with its starting order, and answers the two questions decision
18 settled without a C port: which palette a part has when none was loaded, and which catalogue palette fits a progression
built from scratch. No screen yet: steps 6 to 10 build on these verbs.

## 2. Decisions (proposed)

| # | Decision | Rationale |
|---|---|---|
| D1 | **Each part holds a palette:** up to 16 chords as the table stores them (root, quality, colour, spice, category), and the identifier of the catalogue entry it came from. | The study's §4.3; the representation of step 4, independent of the project scale. |
| D2 | **Saved by value**, in a new field `<P>.pl=` (hex), only for a part whose palette was loaded. | A project keeps the palette it was made with, whatever later CPG revisions do to the catalogue: Cadenza's rule for everything else. |
| D3 | **A part without a loaded palette has its scale's palette, implied and never saved**, and following key and scale changes. | Decision 18's second measure, at no cost in the state. |
| D4 | **Loading a palette is one undo step:** it sets the part's palette and writes the starting order into the slots through Feel's store, keeping locked slots and the loop's length, the order repeating over a longer loop as a preset's phrase does. | Feel's store handles the unequal lengths of starting orders (step 4, D6); keeping the loop's length is what the progression presets do today. |
| D5 | **Each chord maps to the project scale when written:** a degree of the scale if one gives it, else one of the three borrowed chords, else a chromatic source (step 2's range or step 3's). | The study's §6.1 order of preference; numerals and the bar editor then read as they do for any chord. |
| D6 | **Editing never removes the palette** (decision 18). | The palette is the part's harmonic space; the order is only its current path. |
| D7 | **The nearest catalogue palette is a query:** the entry, within the project scale's family (decision 6), whose palette holds the most of the part's distinct chords; ties to catalogue order. Loading it is a separate act. | A suggestion for the screens to offer; never a silent change of the part. |
| D8 | **A part used for the first time copies the Verse's palette**, with everything else it copies. | Today's rule for parts. |
| D9 | **Genre presets are unchanged:** their parts carry the implied scale palette. | Presets are baked states (`presets_data.h`) whose tool is absent from the export; they keep sounding the same. |

## 3. Specification

### 3.1 Engine

| Verb or query | Behaviour |
|---|---|
| `seq_palette_load(s, id)` | D4, D5; one undo step; nothing for an unknown identifier |
| `seq_palette(s, out)` | the playing part's palette, loaded or implied, as 16 chords |
| `seq_palette_id(s)` | the loaded entry's identifier, 0 for the implied scale palette |
| `seq_palette_nearest(s)` | D7 |

### 3.2 Saved state

`<P>.pl=`: the identifier (8 hex digits), the count (1), then per chord its root (1), quality (2), colour (1), spice (1)
and category (1): about 105 characters a part, some 630 bytes for six parts, under 8 % of the 8 KB state. Older builds
ignore the unknown field: the slots play as written, the palette is lost there.

## 4. Measurements

The state's size for a project with six loaded palettes; binary size before and after.

## 5. Verification

1. Loading writes the expected slots for the starting orders of step 4's §3.3, in a major and a minor project, locks kept;
   undo restores slots and palette together.
2. Save and load round trips with loaded palettes; the implied palette never saved.
3. A scale change re-maps the slots' representations and leaves the loaded palette's chords as they sound.
4. The nearest query: deterministic, within the family, on a set of hand-made progressions with known answers.
5. A first-used part copies the Verse's palette.
6. The core parity hash unchanged for projects without palettes; the compatibility harness: 0.28.1 loads states with
   palettes, the slots identical, the palette dropped.

## 6. Out of scope

Every screen (steps 6 to 10); editing a palette's chords (adding or removing one), which nothing has asked for yet.

## 7. Open points, to settle while implementing

- Whether D4's repetition over a longer loop suits palettes as it suits presets, or whether loading should set the loop's
  length to the starting order's.
- How Feel's `base` and `pick` fields should record a palette load (as the chord first written, presumably).
- The field's size, if six palettes prove too much of the state.

## 8. Implementation notes

Empty until implementation.
