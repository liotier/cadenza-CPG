# 04 - The palette table, generated from the CPG

| | |
|---|---|
| Status | for approval (revised after steps 0 to 2) |
| Type | data, tooling |
| Size | M |
| Depends on | 00 (the submodule), 03 (the full vocabulary) |
| Blocks | 05 |

## 1. Purpose

Cadenza needs the CPG's palettes as data compiled into the plugin: 184 catalogue entries in 22 genres, each a palette of
16 chords with a starting order, plus a palette per Cadenza scale (study, avenues B and K; decisions 7, 10 and 18). The
CPG's own code computes them, offline; the plugin carries only the result. No palette logic is ported (decision 18).

What steps 0 to 2 measured, and this revision uses:

- With step 3, every Classic pad has a chromatic form (164 have none before it), so catalogue chords can be stored
  independently of the project scale. Scale Mode's chords are all chords of their own scale, the 7sus4 included once
  step 3 gives its colour (spec 03, §1).
- Cadenza spells a palette chord by its numeral (spec 01, D1); in C, 13 of the 2 804 Classic pads Cadenza can voice today
  are spelled differently from the CPG, all by the numeral's preference for flats (strategy, §3, lesson 7).
- The binary has about 70 KB of headroom (strategy, §5); `tests/run_tests.sh` links the core in fifteen command lines.

## 2. Decisions

The last column names what each decision presupposes, here or in another specification; a decision resting on none can be judged alone, and changing one changes those that rest on it.

| # | Decision | Rationale | Rests on |
|---|---|---|---|
| D1 | **A Node generator,** `tools/cpg/generate.mjs` (Node 20 or later), importing the CPG's `musicTheory.js` and `generation.js` from the submodule, and reading Cadenza's chord space from `tools/cpg/enumerate.c`'s listing. It runs during development and in CI, never in the plugin's build. | The CPG's modules run under Node already (the coverage tool proved it, step 0). Cadenza's side comes from Cadenza's own code, not from a JavaScript copy of it. | none (decisions 7, 10) |
| D2 | **Its output is committed:** `src/cpg_palettes.c` (constant tables) and `src/cpg_palettes.h` (lookups only). CI's `cpg` job regenerates it and fails on any difference. | A submodule bump shows its effect as a reviewable diff, as `coverage.txt` does. | D1 |
| D3 | **One palette per entry, from the Classic variant.** | Cadenza re-voices every chord and adds sevenths with its COLOUR knob; Classic carries the functional fourth row (secondary dominants, augmented sixths, the Neapolitan). The Jazz variant's tritone substitute is the one loss (open point). | none |
| D4 | **Catalogue chords are stored independently of the scale,** as chromatic chords: root (semitones above the tonic), quality (first or second range), colour; with spice (0 to 3), category (the CPG's row-4 categories, diatonic or borrowed) and a flag for chords of the starting order. | The project's key and scale are chosen in Cadenza; step 5 maps each chord to a degree, a borrowed chord or a chromatic one when it writes it. | spec 03, D1 and D2 |
| D5 | **Scale palettes are stored relative to their scale:** degree, source, colour, as Cadenza's own slots hold them. | Each belongs to one Cadenza scale and is used only there; all its chords are that scale's (measured). | spec 03, D3 (the 7sus4) |
| D6 | **A canonical form for each chord:** of the representations that voice the CPG pad's pitch classes exactly, the first-range quality before the second, then the lowest colour index. The fifth rule of decision 12 applies to the CPG's `dom13` and `minor11` only. | Several representations can voice the same pitch classes; one rule makes the table reproducible and reviewable. | D4, D5 |
| D7 | **Strict.** Any pad with no exact representation stops the generation, naming the entry and the chord. | Decision 5; the CPG's own strict parser as precedent. | spec 03 implemented; decision 5 |
| D8 | **Stable identifiers:** a 32-bit FNV-1a hash of `genre \| template value`; the generator fails on a collision. | An entry's identity in the CPG is that pair (its spec 01); indices would shift as the catalogue grows. | none |
| D9 | **Starting orders laid out by decision 4:** the loop is the smallest of 1, 2, 4, 8 or 16 bars holding one bar per chord; the spare bars go to the tonic, at the end when the order ends on it, at the start when it only starts on it, else to the last chord. Stored as pairs of palette index and length in quarter bars. | The arranger's job, done once, here, and reviewable in the diff. | none (decision 4) |
| D10 | **Nicknames in English, whole,** from the CPG's `locales/en.json`, UTF-8 as the CPG writes them. | The CPG's exports use canonical English; how a tile shortens the ten names over 19 characters is the browser's question (step 6). | none |
| D11 | **The table is linked by the plugin's front end, never by the core.** `src/cpg_palettes.c` joins `vst/vst.json`'s sources and the plugin test's command line; the core's verbs (step 5) take palette data as arguments. | The core's fifteen command lines in the author's test script and its other front ends stay as they are (lesson 6); the core needs no catalogue to hold a part's palette. | none; spec 05 rests on it |
| D12 | **The generator also writes a report,** `docs/development/measurements/palettes.txt`: counts, the table's bytes, nickname lengths, the starting orders' layouts by template length, and, for every chord, Cadenza's spelling of its root (from `enumerate.c`, which gains a names column) against the CPG's in the entry's key. CI compares it as it compares `coverage.txt`. | Lesson 7 measured in C only; the report measures the whole table, and keeps the numeral question in view for the author. | D1; spec 01 |

## 3. Specification

### 3.1 Data

Per catalogue entry:

| Field | Content |
|---|---|
| identifier | D8 |
| genre | index into the 22 genre names |
| nickname | D10 |
| tonality | major or minor, from the CPG's `templateTonality`: the browser's family filter (decision 6) |
| palette | 16 chords as D4, three bytes each: root (4 bits), quality (4: the four first-range and five second-range qualities), colour (4), spice (2), category (3), in the starting order (1) |
| starting order | up to 16 pairs of palette index (4 bits) and quarter bars (6 bits), D9 |

Plus the 22 genre names and seven scale palettes of 16 chords as D5.

### 3.2 Interface, `src/cpg_palettes.h`

`cpg_count()`, `cpg_entry(i)`, `cpg_find(id)`, `cpg_genre_name(g)`, `cpg_scale_palette(scale)`: constant data, no
allocation, no state. A chord's representation is `cpg_chord_t` (root, quality, colour, spice, category, flag), shared with
step 5's verbs.

### 3.3 Expected starting orders, as D9 lays them out

| Template length | Loop | Example |
|---|---|---|
| 1, 2, 4, 8 | the same in bars | I-V-vi-IV: four bars, one each |
| 3, ending on the tonic | 4 bars | ii-V-I: ii, V, then I for two bars |
| 3, starting on it | 4 bars | i-♭VI-♭VII: i for two bars, then ♭VI, ♭VII |
| 5 to 7 | 8 bars | I-V-vi-iii-IV: I for four bars, then one bar each (open point) |
| 12 | 16 bars | the 12-bar blues: I7 for eight bars, then the usual eight |

## 4. Measurements

| Quantity | Expected |
|---|---|
| Entries, genres, palettes | 184, 22, 191 (184 and the seven scale palettes) |
| Table size | 184 × 16 × 3 bytes of chords, about 3 KB of starting orders, about 3 KB of nicknames: 12 to 15 KB of binary |
| Binary | 650 KB before; within 720 (estimated locally, measured in CI) |
| Root spellings differing from the CPG's | reported (13 of 2 804 Classic pads in C, before step 3) |
| Nicknames over 19 characters | 10 of 184, the longest 24 |

## 5. Verification

1. Two generations give identical bytes; CI's regeneration matches the committed table and report.
2. Every stored chord voices the CPG pad's pitch classes, the fifth rule of D6 aside: a C test over the whole table, its
   expected pitch classes written by the generator into a header used by the test only.
3. Spot checks against the CPG's [catalogue snapshot](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/catalogue-snapshot.txt):
   Pop/Rock I-V-vi-IV in C, Film/Cinematic i-i♯5-i6-i♯5 in A.
4. The starting orders of §3.3, for the examples given.
5. The binary budget holds; the core parity hash is unchanged (the core does not change).

**The author's checks this step moves:** none expected. **Other front ends:** none; the table is the plugin's.

## 6. Out of scope

Template descriptions (too long for the screen); the other five styles; any logic beyond lookups (step 5 onwards); how
tiles shorten long nicknames (step 6).

## 7. Open points, to settle while implementing

- The Jazz variant's tritone substitute in the fourth row (D3).
- The lopsided layouts of five-chord orders (the tonic held four bars): keep, or spread the spare time.

## 8. Implementation notes

Empty until implementation.
