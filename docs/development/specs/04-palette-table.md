# 04 - The palette table, generated from the CPG

| | |
|---|---|
| Status | draft: design proposals, to be revised once steps 0 to 3 are implemented |
| Type | data, tooling |
| Size | S-M |
| Depends on | 00 (the submodule), 03 (the full vocabulary) |
| Blocks | 05 |

## 1. Purpose

Cadenza needs the CPG's palettes as data compiled into the plugin: 184 catalogue entries in 22 genres, each a palette of
16 chords with a starting order, plus a palette per Cadenza scale (study, avenues B and K; decisions 7, 10 and 18). The
CPG's own code computes them, offline; the plugin carries only the result. No palette logic is ported (decision 18).

## 2. Decisions (proposed)

| # | Decision | Rationale |
|---|---|---|
| D1 | **A Node generator,** `tools/cpg/generate.mjs` (Node 20 or later), importing the CPG's `musicTheory.js` and `generation.js` from the submodule. It runs during development and in CI, never in the plugin's build. | The CPG's modules run under Node already (its pack builder and audit do). The build stays free of Node. |
| D2 | **Its output is committed:** `src/cpg_palettes.c` (constant tables) and `src/cpg_palettes.h` (lookups only). CI regenerates it and fails on any difference. | The CPG's catalogue snapshot works the same way; a submodule bump shows its effect as a reviewable diff. |
| D3 | **One palette per entry, from the Classic variant.** | Cadenza re-voices every chord and adds sevenths with its COLOUR knob, so the variants' voicings and the Jazz variant's seventh upgrades are redundant; Classic carries the functional fourth row (secondary dominants, augmented sixths, the Neapolitan). The Jazz variant's tritone substitute is the one loss (open point). |
| D4 | **Chords are stored independently of the project scale:** root (semitones above the tonic), Cadenza quality (first or second chromatic range) and colour; with spice (0 to 3), category (the CPG's row-4 categories, or diatonic, borrowed, palette) and a flag for chords of the starting order. | The project's key and scale are chosen in Cadenza; mapping to a degree, a borrowed source or a chromatic one happens when a palette is loaded (step 5). |
| D5 | **Stable identifiers:** a 32-bit FNV-1a hash of `genre \| template value`; the generator fails on a collision. | An entry's identity in the CPG is that pair (its spec 01); indices would shift as the catalogue grows or is re-sorted. |
| D6 | **Starting orders laid out by decision 4:** the loop is the smallest of 1, 2, 4, 8 or 16 bars holding one bar per chord; the spare bars go to the tonic, at the end when the order ends on it, at the start when it only starts on it, else to the last chord. Stored as pairs of palette index and length in quarter bars. | The arranger's job, done once, here, and reviewable in the diff. |
| D7 | **Seven scale palettes** from the CPG's Scale Mode, one per Cadenza scale (Major, Minor, Dorian, Mixolydian, Phrygian, Lydian, Harmonic minor), 16 distinct chords each. | Decision 18's palette for parts without a catalogue palette. |
| D8 | **Nicknames in English** from the CPG's `locales/en.json`, UTF-8 as the CPG writes them. | The CPG's exports use canonical English (its export specification, D2). |
| D9 | **Strict mapping.** Every chord maps to exactly one Cadenza representation; the 11ths and 13ths without their fifth match by decision 12's explicit rule; any other mismatch stops the generation, naming the entry and the chord. | Decision 5, and the CPG's own strict parser as precedent. |

## 3. Specification

### 3.1 Data, per catalogue entry

| Field | Content |
|---|---|
| identifier | D5 |
| genre | index into the 22 genre names |
| nickname | D8 |
| tonality | major or minor, from the CPG's `templateTonality`: the browser's family filter (decision 6) |
| palette | 16 chords as D4 |
| starting order | up to 16 pairs of palette index and quarter bars, D6 |

Plus the 22 genre names, and the seven scale palettes (16 chords each, as D4, no starting order).

### 3.2 Interface, `src/cpg_palettes.h`

`cpg_count()`, `cpg_entry(i)`, `cpg_find(id)`, `cpg_genre_name(g)`, `cpg_scale_palette(scale)`: constant data, no
allocation, no state.

### 3.3 Expected starting orders, as D6 lays them out

| Template length | Loop | Example |
|---|---|---|
| 1, 2, 4, 8 | the same in bars | I-V-vi-IV: four bars, one each |
| 3, ending on the tonic | 4 bars | ii-V-I: ii, V, then I for two bars |
| 3, starting on it | 4 bars | i-♭VI-♭VII: i for two bars, then ♭VI, ♭VII |
| 5 to 7 | 8 bars | I-V-vi-iii-IV: I for four bars, then one bar each (open point) |
| 12 | 16 bars | the 12-bar blues: I7 for eight bars, then the usual eight |

## 4. Measurements

Binary size added (estimate 12 to 15 KB, against 2.5 KB of headroom at 0.28.1: step 0's measurement; room must be made
first); 184 entries, 22 genres, 191 palettes; the nickname
lengths against the tiles' 19 characters (ten of the 184 exceed it, the longest 24).

## 5. Verification

1. Two generations give identical bytes.
2. CI's regeneration matches the committed table.
3. For every stored chord, the pitch classes Cadenza voices (`theory_chord_notes` on the mapped representation) equal the
   CPG pad's, the fifth rule of D9 aside: the table is exact by construction.
4. Spot checks against the CPG's [catalogue snapshot](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/catalogue-snapshot.txt):
   Pop/Rock I-V-vi-IV in C, Film/Cinematic i-i♯5-i6-i♯5 in A.
5. The binary budget holds.

## 6. Out of scope

Template descriptions (too long for the screen); the other five styles; any logic beyond lookups (steps 5 onwards).

## 7. Open points, to settle while implementing

- The Jazz variant's tritone substitute in the fourth row (D3).
- Short forms for the ten nicknames over 19 characters, or truncation; whether MPC's font draws the é of "Ascending Line
  Cliché" (it draws the degree sign, so probably).
- The lopsided layouts of five-chord orders (I held four bars): keep, or let a later decision spread the spare time.

## 8. Implementation notes

Empty until implementation.
