# 03 - The missing sonorities

| | |
|---|---|
| Status | for approval (revised after steps 0 to 2) |
| Type | capability, save format |
| Size | M |
| Depends on | 02 |
| Blocks | 04, 05 |

## 1. Purpose

Decision 5 of the study rules out approximation. The coverage tool of step 0
([coverage.txt](../measurements/coverage.txt)) counts 375 of the catalogue's 17 664 pads, across the six styles, that
Cadenza cannot voice exactly; 66 more only omit the fifth of an 11th or a 13th, which decision 12 accepts. Eight
sonorities make up the 375:

| Sonority (CPG type) | Pads | Intervals |
|---|---|---|
| quartal (`quartal`) | 186 | 0 5 10 |
| Italian sixth (`It6`), a dominant seventh without its fifth | 124 | 0 4 10 |
| minor-major seventh (`minMaj7`) | 17 | 0 3 7 11 |
| 7♭5 (`dom7b5`) | 12 | 0 4 6 10 |
| half-diminished seventh (`m7b5`), on a root whose scale has none | 12 | 0 3 6 10 |
| minor ♯5 (`minAug`, the line cliché) | 12 | 0 3 8 |
| 7♭9 (`dom7b9`) | 6 | 0 4 7 10 13 |
| minor sixth (`minor6`), on a root whose scale has none | 6 | 0 3 7 9 |

Two measurements made after steps 0 to 2 widen the purpose:

- 30 Classic pads (24 half-diminished sevenths, 3 minor-major sevenths, 3 minor sixths) exist in Cadenza today only as
  some scale's own chord, never as a chromatic one (strategy, §3, lesson 12). The palette table of step 4 stores
  catalogue chords independently of the project scale, as chromatic chords, so those 30 need the new qualities too.
- The scale palettes of decision 18, from the CPG's Scale Mode (`generateScaleExploration`), hold one chord Cadenza
  cannot voice: the tonic's dominant 7sus4 with its fifth (C F G B♭, CPG type `dom7sus4`), in Minor, Dorian, Mixolydian
  and Phrygian. Its tones are scale tones in all four: a ninth sonority, as a colour. Every other Scale Mode chord is a
  chord of its own scale.

## 2. Decisions

Approving this specification means answering §2.1; §2.2 waits for a later step, with a proposed answer meanwhile; §2.3
follows from decisions already taken or from the code, needs no answer of its own, and is there to be read and
questioned. *Rests on*: what a decision presupposes, here or elsewhere.

### 2.1 For the owner: decidable now

| # | Question | Proposed answer | Alternatives | Rests on |
|---|---|---|---|---|
| D3 | Do the new sonorities stay off every control, reachable only as palette chords? | **Yes.** The colours are appended outside the controls' set: `COL_QUARTAL` (10), `COL_7NO5` (11), `COL_7SUS4` (12) after `COL_COUNT`, which stays 10, with `COL_ALL` (13) closing the set. `COL_COUNT` validates the saved digits of `.ch` and is what a front end may iterate to offer colours (lesson 6). | Offer some on the bar editor's TYPE list: more choice on screen, a type and a screen change for each, and the burden the study's avenue H warned against | D1 |
| D6 | Which chord does an older Cadenza play in place of each new one? | **The fallbacks of §3.3, computed by one rule at save time:** a new colour on an old source keeps its source and degree and takes the old colour that scores best on that chord's own scale; a second-range source takes the first-range quality and old colour, on the same root, that score best. The score: pitch classes shared with the exact chord less those added; ties to the more shared, then to the fewer notes. | Amend any row of §3.3 by hand (for instance the quartal's 11th, which keeps the whole stack, against sus4, which keeps two of its three notes) | D5 |
| D7 | What are the new chords called? | **The CPG's names** (`CHORD_NAME_SUFFIX`), from the quality and colour, never from intervals (§3.2); the Italian sixth `Ab7`, as the CPG writes it and as Cadenza already names its fifthless 11ths and 13ths by the full chord (decision 12). | `Ab7(no5)` for the Italian sixth, which tells it from a full dominant seventh on screen at the cost of differing from the CPG | D1, D3; spec 01 |

### 2.2 For the owner: after a further step

| # | Question | Waits for | Until then |
|---|---|---|---|
| D6, heard | Do the fallbacks sound acceptable when a fork-saved project is opened in the official 0.28.1? | step 5's device run, the first that saves such chords from the screen | D6's table |

### 2.3 Following from decisions already taken

| # | Decision | Rationale | Rests on |
|---|---|---|---|
| D1 | **Five new chromatic qualities and three new colours** give the nine exactly (§3.1). | Cadenza builds a chord from a degree, a source and a colour, its tones taken from the chord's own seven-note scale. A quality is such a scale; a colour picks tones from it. | decision 5: no approximation |
| D2 | **The qualities form a second chromatic range of sources**, 52 to 111: `52 + 5 × root + quality`. `THEORY_SRC_MAX` (51) keeps its meaning, the first range; `THEORY_SRC2_MAX` (111) closes the second. | Enlarging the first range would renumber every saved chromatic chord. 111 stays within a change point's `signed char` source (lesson 8): one more range of twelve would still fit (112 to 123), beyond which the field widens. | D1; strategy, principle 3 |
| D4 | **The engine accepts the new values wherever it accepts chords:** `theory_src_ok` covers 0 to 111; the slot, phrase, change-point and Feel setters take colours up to `COL_ALL`; change points gain the types that name the new colours. No control produces them. | Step 5 writes palette chords through these paths, as step 2 opened them for the first range. | D2, D3; spec 02 |
| D5 | **Old fields hold only values old builds can use:** each new value has an exact form in a new field and a fallback in the old one; a Feel base code naming a new value is saved as its fallback's code in `.cx`, exact in the new field. | 0.28.1 ignores unknown fields, skips out-of-range sources in `.cx` and `.cp`, refuses a whole `.ch` for one character outside its alphabet, and decodes a Feel base code with no check of its colour against its tables. | D2, D3; decision 11; strategy, principle 4 |
| D8 | **The fifthless 11ths and 13ths stay as they are.** | Recorded with its trade-off in the study, avenue H. | decision 12 |

## 3. Specification

### 3.1 Constructions

| New value | Kind | Seven-note scale, or tones | Gives, with |
|---|---|---|---|
| melodic minor | quality 0 | 0 2 3 5 7 9 11 | colour 7th: minor-major seventh; colour 6th: minor sixth |
| Locrian | quality 1 | 0 1 3 5 6 8 10 | colour 7th: half-diminished seventh |
| dominant ♭5 | quality 2 | 0 2 4 5 6 9 10 | colour 7th: 7♭5 |
| Phrygian dominant | quality 3 | 0 1 4 5 7 8 10 | colour 9th: 7♭9 |
| minor ♯5 | quality 4 | 0 2 3 5 8 9 10 | colour Triad: minor ♯5 |
| quartal | colour 10 | tones 1, 4, 7 | on the first range's dominant quality: 0 5 10 |
| seventh without fifth | colour 11 | tones 1, 3, 7 | on the dominant quality: 0 4 10, the Italian sixth on ♭VI |
| 7sus4 | colour 12 | tones 1, 4, 5, 7 | on the tonic of Minor, Dorian, Mixolydian, Phrygian: 0 5 7 10 |

`theory_slot_scale` maps the second range after the first (`SCALE_COUNT + 48 + source - 52`), and `theory_degree_pc` reads
it from a second table of five scales; every function that takes a chord's own scale then reads the new chords as any
other. The bass and the melody read a chord's root, third and fifth from that scale, so the minor ♯5's raised fifth and
the 7♭5's lowered one reach them unchanged.

### 3.2 Names and numerals

| Chord | Name (C root) | Degree text (theory_degree_c) |
|---|---|---|
| melodic minor, 7th | `CmMaj7` | `imaj7`, as Cadenza writes harmonic minor's i with its seventh |
| melodic minor, 6th | `Cm6` | `i6` |
| Locrian, 7th | `Cm7b5` | `im7b5`, Cadenza's form (`iim7b5`) |
| dominant ♭5, 7th | `C7b5` | `I7b5`: upper case, no diminished sign |
| Phrygian dominant, 9th | `C7b9` | `I7b9` |
| minor ♯5, Triad | `Cm#5` | `i#5` |
| quartal, on dominant | `C7sus4(no5)` | `I 7sus4(no5)`: Cadenza writes a sus after a space, the numeral whole |
| seventh without fifth, on dominant | `C7` | `I7` |
| 7sus4, on the minor tonic | `C7sus4` | `i 7sus4` |

Roots are spelled by step 1's rules; chord tones by degree (D4 of spec 01): C quartal reads C F B♭. Other combinations of
a new quality and a colour, which no catalogue chord uses, take the generic names of `theory_chord_name_c` on the chord's
own scale; the tests check that every combination has a name of at most 15 characters, the slot labels' room.

### 3.3 Saved state

| Value | Exact, in a new field | Fallback, in the old one |
|---|---|---|
| a slot's second-range source, new colour or Feel base naming either | `<P>.cy=`: per such slot, slot (2 hex), source (2), colour (1), base (4) | `.cx`: the fallback's first-range source and its base code; `.ch` and `.pq`: the fallback's colour |
| a change point's | `<P>.cq=`: per such point, bar (1 hex), step (1), source (2), type (1) | `.cp`: the fallback's source and type |
| a Feel base code naming a second-range source | a second code form: `0x8000` + (source - 52) × 16 + colour | in `.cx`, the fallback's first-form code |

New builds read the old fields first, then let the new ones override. A Feel `pick` code is recomputed on load and never
saved: it takes the second form in memory only. The first code form stays unique for the new colours on first-range
sources.

The fallbacks by D6's rule, computed for the combinations the catalogue and Scale Mode use (the tests compute and check
every combination):

| Exact | Fallback (first-range quality, colour) | Pitch classes | Shared, added |
|---|---|---|---|
| minor-major seventh | minor, Triad | 0 3 7 | 3, 0 |
| minor sixth | minor, Triad | 0 3 7 | 3, 0 |
| half-diminished seventh | diminished, Triad | 0 3 6 | 3, 0 |
| 7♭5 | dominant, 7th | 0 4 7 10 | 3, 1 |
| 7♭9 | dominant, 7th | 0 4 7 10 | 4, 0 |
| minor ♯5 | minor, 6th (the minor quality's 6th is ♭6) | 0 3 7 8 | 3, 1 |
| quartal, on the dominant quality | the same chord, 11th | 0 2 4 5 10 | 3, 2 |
| seventh without fifth, on the dominant quality | the same chord, 7th | 0 4 7 10 | 3, 1 |
| 7sus4, on a tonic | the same chord, sus4 | 0 5 7 | 3, 0 |

The quartal's fallback holds its whole stack (C F B♭ within C11, which Cadenza voices without its fifth), where sus4
would keep two of its three notes.

### 3.4 Other readers

An audit as step 2's, listed in the implementation notes: every array sized by `COL_COUNT` or indexed by a colour, and
every reader of a source, in `theory.c`, `seq_core.c`, `feel.c` and `engine.c`. Known: the colour tables of `theory.c`
(tones, names, extension numbers, drift costs, avoid notes) grow to `COL_ALL`; drift scores quartal as sus4 (12) and the
seventh without fifth as a 7th (10), the chromatic 30 as for the first range; the bar editor's TYPE shows no type for a
new colour, which no control sets.

## 4. Measurements

| Quantity | Before (step 2) | Expected after |
|---|---|---|
| Pads needing a new sonority (coverage tool) | 375 | 0 |
| Classic pads with no chromatic form (a new line of the coverage tool) | 164 (134 + 30) | 0 |
| Scale Mode palettes with a chord Cadenza cannot voice (a new line of the coverage tool) | 4 of 7 (the 7sus4) | 0 |
| Chords in Cadenza's space (coverage tool) | 4 060 | the enumeration extended to the second range and the new colours |
| Binary | 650 KB | about 2 to 4 KB more (estimated locally, measured in CI) |
| Core parity hash | `934d3f5c...` | unchanged |

## 5. Verification

1. Each construction of §3.1 gives its intervals in every key (theory tests).
2. Names and degree texts as in §3.2, in several keys, spelled; every combination of a new quality or colour named
   within 15 characters (theory tests).
3. The fallback rule's results for every combination, against §3.3's table for the catalogue's (theory tests).
4. The bass and the melody follow each new chord's tones (core tests).
5. Every writer of step 2 accepts the new values, and save and load round trips keep them exactly: slots, change points,
   Feel bases (core tests, extending `tests/test_chroma_path.c`).
6. The compatibility harness, three scenarios: new values in slots, at change points, in a Feel base. 0.28.1 loads each,
   under ASan and UBSan, playing the fallbacks; `tests/compat/expected.txt` pins its re-saves.
7. The coverage tool's report as §4 expects, committed.
8. The core parity hash unchanged: no existing state or path contains a new value.

**The author's checks this step moves** (principle 9): three probes that step 2 set at `THEORY_SRC_MAX + 1` (two in
`tests/test_core.c`, one in `tests/test_tweak.c`) and the same in `tests/test_chroma_path.c` become valid sources; they
move to `THEORY_SRC2_MAX + 1`.

**Other front ends** (lesson 6): a Feel base code may take the second form; the iOS app's Feel page decodes it only if
it shares this engine. To be named in the design note to the author.

## 6. Out of scope

Offering the new sonorities on the COLOUR knob, in the bar editor's TYPE list or on any control; chord types neither the
catalogue nor Scale Mode uses (the altered dominants, for instance), which the same method could add.

## 7. Open points, to settle while implementing

None beyond §2.2.

## 8. Implementation notes

Empty until implementation.
