# 04 - The palette table, generated from the CPG

| | |
|---|---|
| Status | approved: §2.1 answered by the owner, 2026-11-23 |
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
- Cadenza spells a palette chord by its numeral (S01-D1). In C, 13 of the 2 804 Classic pads Cadenza could voice were
  spelled differently from the CPG, all by the numeral's old preference for flats; since spec 01's addendum A none are
  (strategy, §3, lesson 7).
- The binary has about 70 KB of headroom (strategy, §5); `tests/run_tests.sh` links the core in fifteen command lines.

## 2. Decisions

Approving this specification means answering §2.1; nothing waits for a later step (§2.2); §2.3 follows from decisions
already taken or from the code, needs no answer of its own, and is there to be read and questioned. *Rests on*: what a
decision presupposes, here or elsewhere.

### 2.1 For the owner: decidable now

| # | Question | Proposed answer | Alternatives | Rests on |
|---|---|---|---|---|
| S04-D3 | Which of the CPG's six styles fills each catalogue palette, and with which colours? | **Answered (owner, 2026-11-23): Classic, its colours kept** (§2.4, option a): "a solid foundation"; other styles' colourings, the "jazzy frills", to be considered after a first version. COLOUR acts on Classic's triads, 53 % of the pads and 82 % of the starting orders' chords; the rest keep the palette's own colour, as a chord with its own type does in Cadenza today | b, c and d of §2.4; c (a second style, about 9 KB) stays possible later | none |
| S04-D9 | How does a starting order become bars? (The owner asked whether the progression plays in order, which it does as the palette's starting order, and whether Cadenza had a mechanism to reuse, which it has: §2.4.) | **Answered (owner, 2026-11-23): decision 4's rule**, "the established common practices of competent composers", unless it rocked the boat; it does not. The spare bars go to the tonic, held at the end when the order ends on it, at the start when it only starts on it, else to the last chord. The rule hands out whole bars only, so a laid-out order is one chord a bar, in a loop of 1, 2, 4, 8 or 16 bars: the shape of Cadenza's own presets, one of which, *2-5-1-1 Jazz* (`{1, 4, 0, 0}`), is this very layout of ii-V-I. Loading writes it as Feel's presets write theirs (S05-D4); Feel's store is not needed | Feel's preset layout (the order repeated into the spare bars); loading writes nothing | decision 4; S05-D4 |

### 2.2 For the owner: after a further step

None.

### 2.3 Following from decisions already taken

| # | Decision | Rationale | Rests on |
|---|---|---|---|
| S04-D1 | **A Node generator,** `tools/cpg/generate.mjs` (Node 20 or later), importing the CPG's `musicTheory.js` and `generation.js` from the submodule, and reading Cadenza's chord space from `tools/cpg/enumerate.c`'s listing. It runs during development and in CI, never in the plugin's build. | The CPG's modules run under Node already (the coverage tool proved it, step 0). Cadenza's side comes from Cadenza's own code, not from a JavaScript copy of it. | decisions 7 and 10 |
| S04-D2 | **Its output is committed:** `src/cpg_palettes.c` (constant tables) and `src/cpg_palettes.h` (lookups only). CI's `cpg` job regenerates it and fails on any difference. | A submodule bump shows its effect as a reviewable diff, as `coverage.txt` does. | S04-D1; strategy, principle 5 |
| S04-D4 | **Catalogue chords are stored independently of the scale,** as chromatic chords: root (semitones above the tonic), quality (first or second range), colour; with spice (0 to 3), category (the CPG's row-4 categories, diatonic or borrowed) and a flag for chords of the starting order. | The project's key and scale are chosen in Cadenza; step 5 maps each chord to a degree, a borrowed chord or a chromatic one when it writes it. | S03-D1 and S03-D2 |
| S04-D5 | **Scale palettes are stored relative to their scale:** degree, source, colour, as Cadenza's own slots hold them. | Each belongs to one Cadenza scale and is used only there; all its chords are that scale's (measured). | S03-D3 (the 7sus4); decision 18 |
| S04-D6 | **A canonical form for each chord:** of the representations that voice the CPG pad's pitch classes exactly, the first-range quality before the second, then the lowest colour index. The fifth rule of decision 12 applies to the CPG's `dom13` and `minor11` only. | Several representations can voice the same pitch classes; one rule makes the table reproducible and reviewable. | S04-D4, S04-D5; decision 12 |
| S04-D7 | **Strict.** Any pad with no exact representation stops the generation, naming the entry and the chord. | The CPG's own strict parser as precedent. | decision 5; spec 03 implemented |
| S04-D8 | **Stable identifiers:** a 32-bit FNV-1a hash of `genre \| template value`; the generator fails on a collision. | An entry's identity in the CPG is that pair (its spec 01); indices would shift as the catalogue grows. | the CPG's spec 01 |
| S04-D10 | **Nicknames in English, whole,** from the CPG's `locales/en.json`, UTF-8 as the CPG writes them. | Cadenza's screens are in English, and the CPG's exports use canonical English; how a tile shortens the ten names over 19 characters is the browser's question (step 6). | Cadenza's language |
| S04-D11 | **The table is linked by the plugin's front end, never by the core.** `src/cpg_palettes.c` joins `vst/vst.json`'s sources and the plugin test's command line; the core's verbs (step 5) take palette data as arguments. | The core's fifteen command lines in the author's test script and its other front ends stay as they are; the core needs no catalogue to hold a part's palette. Spec 05 rests on it. | strategy, lesson 6 |
| S04-D12 | **The generator also writes a report,** `docs/development/measurements/palettes.txt`: counts, the table's bytes, nickname lengths, the starting orders' layouts by template length, and, for every chord, Cadenza's spelling of its root (from `enumerate.c`, which gains a names column) against the CPG's in the entry's key. CI compares it as it compares `coverage.txt`. | Lesson 7 measured in C only; the report measures the whole table, and keeps the numeral question in view for the author. | S04-D1; spec 01 |

### 2.4 Background to S04-D3 and S04-D9: COLOUR, the CPG's styles, and starting orders

**How COLOUR works** ([seq_core.c](../../../src/seq_core.c), `eff_colour`, `chord_level`). A slot whose own colour is
Triad and which is not locked plays the COLOUR knob's level (Power, Triad, 7th, 9th, 11th, 13th), shaped slot by slot by
COMPLEXITY's colour layers (a fixed rank per slot, so turning up colours one more chord at a time). Every other slot, one
with its own colour (7th, sus4, add9, 6th...) or a locked one, plays its own colour and ignores the knob. The knob's
extensions come from the chord's own scale: on G in C major, 7th gives G7.

**What the CPG's styles change** (measured over the 184 palettes, each style in C): every style shares 93 to 96 % of
its chords' roots and triads with Classic; they differ mainly in colour, and in voicing, which Cadenza replaces with its
own. A palette's four rows are its template chords, then mostly seventh versions, borrowed chords, and functional chords.
Pop/Rock I-V-vi-IV in Classic: `C G Am F`, `Dm Cmaj7 Gmaj7 Fmaj7`, `Bb Fm Ab Eb`, `G7 A D7 Db`; in Jazz the third row
becomes `Bbmaj7 Fm7 Abmaj7 Ebmaj7`. The styles' sevenths are a taste, not the scale's: `Gmaj7` where COLOUR on G gives
`G7`, `Bbmaj7` where it gives `Bb7`. So COLOUR and a CPG style are two different colourings, and neither reproduces the
other.

**The colour each pad would be stored with,** were the CPG's colours kept:

| Style | Triad: COLOUR colours it | A fixed 7th to 13th | A fixed sus, add9 or 6th | A new sonority (step 3) |
|---|---|---|---|---|
| Classic | 53 % | 37 % | 5 % | 6 % |
| Smooth, Modal, Pedal | 55 to 56 % | 37 to 38 % | 4 to 5 % | 2 % |
| Experimental | 54 % | 39 % | 5 % | 2 % |
| Jazz | 43 % | 51 % | 4 % | 3 % |

In Classic, the first row, which the starting order uses, is 82 % triads (606 of 736 pads): a loaded palette's loop
follows COLOUR almost entirely; the colour variants come in when picked.

**The options for S04-D3:**

- **a. Classic, its colours kept** (proposed). Exact, as decision 5 requires. COLOUR acts as it always has: on chords
  without a colour of their own. A user who wants more colour turns COLOUR up and gets Cadenza's scale-consistent
  extensions on the triads; the palette's own coloured chords stay as the CPG wrote them.
- **b. Every chord stored as a triad,** COLOUR owning all colour. The palette's seventh and sus versions would collapse
  onto its triads (`C` and `Cmaj7` become one chord), losing up to 42 % of what the palette holds: against decision 5.
- **c. A second style, Jazz, chosen when a palette loads.** About 9 KB more (184 × 16 × 3 bytes), a style choice on the
  browser's screen (step 6). Adds the Jazz style's colouring and its tritone substitute.
- **d. All six styles.** About 44 KB more, most of the 70 KB of headroom, for differences that are mostly colour.

Option a now leaves c open: the generator can add a style later, at about 9 KB each.

**What Cadenza already does** (read in [seq_core.c](../../../src/seq_core.c), 2026-11-23). Two preset loaders and one
store, all in 0.28.1:

- **PHRASE's progression presets** (`seq_set_prog_preset`): the preset's chords written as a phrase over the loop's
  slots at the current chord length, repeated if the loop is longer (`fill_phrase`); the loop keeps its length.
- **Feel's progression presets** (`seq_feel_preset`): a bar a chord, the loop doubled until it holds the whole
  progression, the change points cleared, then the same phrase repetition into any spare bars.
- **Feel's store** (`feel_store`): writes runs of chords, each with its own start and length in quarter bars, into the
  slots; the Feel page's add, remove and resize verbs use it.

Cadenza's own presets all have 1, 2, 4 or 8 chords, so a spare bar never arises for them; but the repetition rule is
there, general. Two layouts follow, neither needing a new mechanism:

| | Layout 1: Feel's preset layout, as it stands | Layout 2: decision 4's rule, through Feel's store |
|---|---|---|
| The 145 orders of 1, 2, 4, 8 chords | a bar a chord | the same |
| Three chords ending on the tonic (ii-V-I) | ii V I ii: the first chord gets the spare bar | ii V I I: the tonic held two bars, as jazz writes it |
| Three chords starting on the tonic (i-♭VI-♭VII) | i ♭VI ♭VII i: around the loop, the same cycle as layout 2, begun a bar later | i i ♭VI ♭VII |
| Five to seven chords (I-V-vi-iii-IV in eight bars) | I V vi iii IV I V vi | I I I I V vi iii IV |
| The 12-bar blues in 16 bars | its first four bars again at the end | I7 for eight bars, then the usual eight |
| What the table stores | the order | the order and a length per chord |
| New code | none in the engine; the loader calls Feel's preset path with the order | the layout rule in the generator; the loader writes runs through Feel's store |
| Consistency | a palette loads exactly as a Feel preset does | a palette's starting order can differ from what the same chords would give as a Feel preset |

Of the 184 orders, 145 lay out the same either way; the 39 others (3 chords: 23; 5 to 7: 15; the blues: 1) differ, 15
of them ending on the tonic, 21 starting on it, 2 neither. Layout 1 is Cadenza's way and needs nothing new; layout 2 is
the musician's preference recorded as decision 4. Both are only starting orders.

**Chosen: layout 2** (S04-D9, owner, 2026-11-23), and it needs less than the table above says. Decision 4 hands out whole
bars only (checked over all 184 orders: every layout is whole bars in a loop of 1, 2, 4, 8 or 16), so the generator
stores each laid-out order as one palette index a bar, and the loader writes it as Feel's presets write theirs: no
durations, no Feel store. Cadenza's own *2-5-1-1 Jazz* preset already lays ii-V-I out this way.

The loop's length (S05-D4) has the same two existing answers: PHRASE's presets keep it, Feel's grow it until the order
fits, never shrinking it. Layout 1 goes with Feel's growth; the two questions are one choice of which existing preset a
palette load behaves like.

**Starting orders (S04-D9).** The study's model (§4.3) gives a palette two parts: its chords, the harmonic space, and a
starting order, the template it came from, so that a loaded palette plays at once. Decision 2 calls the templates starting
points, decision 4 gives their spare time to the tonic, and S05-D4 writes the order into the slots when a palette
loads, as the progression presets do today. The alternative: loading a palette writes nothing, the part's slots stay as
they are, and the palette is only the space the editing tools draw from (steps 7 to 10). Then a newly loaded palette is
silent until chords are picked from it, S04-D9 disappears, and S05-D4 becomes *loading sets the palette only*.

## 3. Specification

### 3.1 Data

Per catalogue entry:

| Field | Content |
|---|---|
| identifier | S04-D8 |
| genre | index into the 22 genre names |
| nickname | S04-D10 |
| tonality | major or minor, from the CPG's `templateTonality`: the browser's family filter (decision 6) |
| palette | 16 chords as S04-D4, three bytes each: root (4 bits), quality (4: the four first-range and five second-range qualities), colour (4), spice (2), category (3), in the starting order (1) |
| starting order | laid out by S04-D9: one palette index a bar of its loop (1, 2, 4, 8 or 16 bars), 4 bits each. The order is the CPG parser's (`parseProgression`), never the template text split: the catalogue writes the 12-bar blues as the single token `12-bar-blues`, which the parser expands to twelve chords |

Plus the 22 genre names and seven scale palettes of 16 chords as S04-D5.

### 3.2 Interface, `src/cpg_palettes.h`

`cpg_count()`, `cpg_entry(i)`, `cpg_find(id)`, `cpg_genre_name(g)`, `cpg_scale_palette(scale)`: constant data, no
allocation, no state. A chord's representation is `cpg_chord_t` (root, quality, colour, spice, category, flag), shared with
step 5's verbs.

### 3.3 Expected starting orders, as S04-D9 lays them out

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
| Table size | 184 × 16 × 3 bytes of chords (8.8 KB), 184 × 8 bytes of starting orders (1.5 KB), about 3 KB of nicknames: 12 to 14 KB of binary |
| Binary | 650 KB before; within 720 (estimated locally, measured in CI) |
| Root spellings differing from the CPG's | reported (none of the 2 804 Classic pads Cadenza can voice in C, since spec 01's addendum A; the whole table once step 3 lands) |
| Nicknames over 19 characters | 10 of 184, the longest 24 |

## 5. Verification

1. Two generations give identical bytes; CI's regeneration matches the committed table and report.
2. Every stored chord voices the CPG pad's pitch classes, the fifth rule of S04-D6 aside: a C test over the whole table, its
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

None: the two questions the draft left open are S04-D3 and S04-D9 of §2.1, both answered.

## 8. Implementation notes

Empty until implementation.
