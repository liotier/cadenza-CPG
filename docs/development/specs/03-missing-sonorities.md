# 03 - The missing sonorities

| | |
|---|---|
| Status | implemented, 2026-11-24 (§8); heard on the device with step 5 (§2.2) |
| Type | capability, save format |
| Size | M |
| Depends on | 02 |
| Blocks | 04, 05 |

## 1. Purpose

**Scope against spec 04's style question** (measured 2026-11-23): every CPG style uses seven of the eight sonorities;
the Italian sixth appears only in Classic (98 pads) and Jazz (26). Whichever style the palette table takes (S04-D3),
the seven are needed; the seventh-without-fifth colour that gives the Italian sixth would go unused only if the table
took neither Classic nor Jazz. The step does not wait for that answer.

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

All three answered by the owner, 2026-11-23, as proposed. S03-D6's answer, "the fallbacks computed at save time:", ended on a
colon; it is read as accepting the proposed fallbacks, the owner to say if anything was cut off.

| # | Question | Answer | Alternatives considered | Rests on |
|---|---|---|---|---|
| S03-D3 | Do the new sonorities stay off every control, reachable only as palette chords? | **Yes** (owner). The colours are appended outside the controls' set: `COL_QUARTAL` (10), `COL_7NO5` (11), `COL_7SUS4` (12) after `COL_COUNT`, which stays 10, with `COL_ALL` (13) closing the set. `COL_COUNT` validates the saved digits of `.ch` and is what a front end may iterate to offer colours (lesson 6). | Offer some on the bar editor's TYPE list: more choice on screen, a type and a screen change for each, and the burden the study's avenue H warned against | S03-D1 |
| S03-D6 | Which chord does an older Cadenza play in place of each new one? | **The fallbacks of §3.3, computed by one rule at save time** (owner): a new colour on an old source keeps its source and degree and takes the old colour that scores best on that chord's own scale; a second-range source takes the first-range quality and old colour, on the same root, that score best. The score: pitch classes shared with the exact chord less those added; ties to the more shared, then to the fewer notes. | Amend any row of §3.3 by hand (for instance the quartal's 11th, which keeps the whole stack, against sus4, which keeps two of its three notes) | S03-D5 |
| S03-D7 | What are the new chords called? | **The CPG's names** (owner) (`CHORD_NAME_SUFFIX`), from the quality and colour, never from intervals (§3.2); the Italian sixth `Ab7`, as the CPG writes it and as Cadenza already names its fifthless 11ths and 13ths by the full chord (decision 12). | `Ab7(no5)` for the Italian sixth, which tells it from a full dominant seventh on screen at the cost of differing from the CPG | S03-D1, S03-D3; spec 01 |

### 2.2 For the owner: after a further step

| # | Question | Waits for | Until then |
|---|---|---|---|
| S03-D6, heard | Do the fallbacks sound acceptable when a fork-saved project is opened in the official 0.28.1? | step 5's device run, the first that saves such chords from the screen | S03-D6's table |

### 2.3 Following from decisions already taken

| # | Decision | Rationale | Rests on |
|---|---|---|---|
| S03-D1 | **Five new chromatic qualities and three new colours** give the nine exactly (§3.1). | Cadenza builds a chord from a degree, a source and a colour, its tones taken from the chord's own seven-note scale. A quality is such a scale; a colour picks tones from it. | decision 5: no approximation |
| S03-D2 | **The qualities form a second chromatic range of sources**, 52 to 111: `52 + 5 × root + quality`. `THEORY_SRC_MAX` (51) keeps its meaning, the first range; `THEORY_SRC2_MAX` (111) closes the second. | Enlarging the first range would renumber every saved chromatic chord. 111 stays within a change point's `signed char` source (lesson 8): one more range of twelve would still fit (112 to 123), beyond which the field widens. | S03-D1; strategy, principle 3 |
| S03-D4 | **The engine accepts the new values wherever it accepts chords:** `theory_src_ok` covers 0 to 111; the slot, phrase, change-point and Feel setters take colours up to `COL_ALL`; change points gain the types that name the new colours. No control produces them. | Step 5 writes palette chords through these paths, as step 2 opened them for the first range. | S03-D2, S03-D3; spec 02 |
| S03-D5 | **Old fields hold only values old builds can use:** each new value has an exact form in a new field and a fallback in the old one; a Feel base code naming a new value is saved as its fallback's code in `.cx`, exact in the new field. | 0.28.1 ignores unknown fields, skips out-of-range sources in `.cx` and `.cp`, refuses a whole `.ch` for one character outside its alphabet, and decodes a Feel base code with no check of its colour against its tables. | S03-D2, S03-D3; decision 11; strategy, principle 4 |
| S03-D8 | **The fifthless 11ths and 13ths stay as they are.** | Recorded with its trade-off in the study, avenue H. | decision 12 |

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

Roots are spelled by step 1's rules; chord tones by degree (S01-D4): C quartal reads C F B♭. Other combinations of
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

The fallbacks by S03-D6's rule, computed for the combinations the catalogue and Scale Mode use (the tests compute and check
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

### 8.1 What was built

| Layer | Change |
|---|---|
| `src/theory.h`, `src/theory.c` | `COL_QUARTAL`, `COL_7NO5`, `COL_7SUS4` and `COL_ALL` after `COL_COUNT`, which stays 10; their tones (`COLOUR_TONES`), names (`THEORY_COLOUR_NAME`), drift costs and avoid-note entries; `theory_colour_tone` gives the 4th for the 3rd of quartal and 7sus4 as of sus4. The second range: `CQ2_*`, `THEORY_SRC2` (52), `THEORY_SRC2_MAX` (111), `theory_chroma2_src`, a second table of scales (`CHROMA2`) read by `theory_degree_pc`; `theory_slot_scale`'s formula already placed it after the first. `theory_src_ok` up to 111. Names and degree texts (§8.3). `theory_fallback`, S03-D6's rule. |
| `src/seq_core.c` | The setters take colours up to `COL_ALL` (`seq_set_slot`, the progression's two paths); `feel_chord_ok` the second range and the new colours. Change-point types 10 to 12 for the new colours (`CP_TYPES` 13, `CP_OLD_TYPES` 10). The Feel code's second form (`FEEL_CODE2`), and `feel_decode`. The save's fallbacks and the new fields (§8.4). The lanes: `mel_chord_colour` reads the 7th without its 5th as its 7th chord, `chord_tone` counts the 7th of quartal and 7sus4. |
| `src/feel.c` | `has_seventh` includes the three new colours, so Feel's suggestions read a 7sus4 or an Italian sixth as a chord with a 7th. |
| `src/seq_core.h` | The contracts of the setters and of `seq_cp_t` name the new ranges. |
| `tools/cpg` | `enumerate.c` lists the second range and every colour; `coverage.mjs` counts the pads exact only with step 3's values, the Classic pads with no chromatic form and the Scale Mode palettes Cadenza cannot voice; the report regenerated. |
| Tests | `tests/test_theory.c` (`test_fork_sonorities`), `tests/test_chroma_path.c` (section 4 and the sweep), `tests/test_plugin.c` (the sweep), `tests/compat` (three scenarios, pinned). |

### 8.2 Readers' audit (§3.4)

Every array sized by `COL_COUNT` or indexed by a colour, every reader of a source or of a change point's type, and what
each does with the new values.

| Where | Reader | With the new values |
|---|---|---|
| `theory.c` | `COLOUR_TONES`, `THEORY_COLOUR_NAME`, `theory_avoid`'s `EXT`, `theory_drift`'s `COST` | grown to `COL_ALL`; their clamps (`theory_chord_notes`, `theory_chord_tones`, `theory_avoid`, `theory_drift`) to `COL_ALL` |
| `theory.c` | `EXT_NUM` | stays `COL_COUNT` long: read only through `is_ext` (9th to 13th) |
| `theory.c` | `theory_triad_quality` | a major 3rd under a diminished 5th, the dominant ♭5's, is major (§8.3, 1); no earlier scale has one |
| `theory.c` | `theory_numeral`, `chroma_degree`, `root_in`, `tone_of`, `spell_pc` | read the second range through `theory_degree_pc` and `theory_triad_quality`: unchanged. The half-diminished 7th on F♯ in C reads `#ivm7b5`, the leading-tone rule of spec 01's addendum A |
| `theory.c` | `theory_chord_card`, `name_card` (no caller in the plugin, lesson 6) | quartal and 7sus4 as a 7sus4 card, `(7s4)` and `C7sus4`, the 7th without its 5th as a 7th, as a card already shows add9 as a 9 |
| `seq_core.c` | `PICK_TYPE` readers: `cp_sync`, `slot_chord`, `feel_put`, `tw_type`, `load_cp`, `cp_set` | loops over `CP_TYPES`: a point may hold types 10 to 12, made by THIS HIT ONLY or Feel of a slot with a new colour. `seq_cp_set` and the bar editor's TYPE (`seq_rbar_type`) stay 0 to 4 |
| `seq_core.c` | `eff_colour`, `chord_level` (the COLOUR knob, COMPLEXITY) | act on Triad slots only; a slot with a new colour keeps it, as any coloured slot |
| `seq_core.c` | `mel_chord_colour`, `chord_tone`, `mel_chords` | §8.1; the bass and the melody play only notes of each new chord's own scale (tested) |
| `seq_core.c` | `chord_notes_inv` (SPREAD's low-interval rule), `mel_chords`' Power case | 9th to 13th and Power only: the new colours, of 3 or 4 notes, are not concerned |
| `seq_core.c` | `old_src`, `out_slot_chords`, `save_part_head`, `out_cp`, `out_feel` | §8.4 |
| `seq_core.c` | `load_quality` (`.pq`), `load_slots` (`.ch`), `load_cx`, `load_cp` | unchanged: the older fields keep their alphabets and ranges; `.cy` and `.cq` override after them |
| `feel.c` | `has_seventh`, `is_dominant`, `is_dim7` | §8.1; `is_dim7` stays the 7th colour's |
| `engine.c` | `pty_of` | maps the new colours to type 0: PHRASE's TYPE row lights nothing for them, as for 7th to 13th |
| `engine.c` | `fo_label`, `VCOL_NAMES` | colours below 5, and the knob's levels: not concerned |
| `engine.c`, `seq_core.c` | the label buffers (16 bytes) | every name fits 15 characters (tested); degree texts §8.3, 3 |

### 8.3 Choices made while building

1. **The dominant ♭5's triad quality.** `theory_triad_quality` called every diminished 5th diminished; C E G♭ then read
   `Cm7b5` and `ii°`. The rule now asks for a minor 3rd as well; a major 3rd under a diminished 5th is major, its 5th
   marked by the name. No scale before step 3 has that triad, so every earlier chord keeps its quality (the parity hash
   is unchanged).
2. **Names of combinations no catalogue chord uses.** The second range marks what it alters where the name or numeral
   voices it: `C(b5)` for the bare major ♭5 triad (`Cb5` would read as a chord on C♭), `C9b5`, `Cm7#5`, `Cm6#5`,
   `C11b9`, `C13b9`, `Caddb9`. The Phrygian dominant's 13th is ♭13, which `C13b9` does not say, as Cadenza names its
   Phrygian and Aeolian 13ths without their ♭13 (spec §3.2: "the generic names"). Quartal and 7sus4 on a chord whose 7th
   is major read `maj7sus4(no5)` and `maj7sus4`.
3. **Fifteen characters.** Every name of every combination with a value of step 3 fits 15 characters, in every key and
   scale (tested; the longest, `Bbmaj7sus4(no5)`, exactly 15). Degree texts fit for the forms step 5 writes: quartal and
   7sus4 on a degree, a borrow or the first range, whose 7th is minor (`bVII 7sus4(no5)`, 15). A quartal on a
   second-range chord with a diminished triad and an accidental reaches 16 (`#vii° 7sus4(no5)`) and would be cut; step
   5 never writes it, since the first range's dominant voices the same notes (S05-D5).
4. **The fallback view covers every real slot.** The first version computed the older fields' fallbacks for the loop's
   slots only; a phrase longer than the loop saves hidden slots in `.prog`, and 26 of the author's checks failed at once.
   `save_part_head` now builds the view for all 64 slots and decides whether `.ch` is written from the exact slots.
5. **A Feel base's fallback** restores a borrow's degree before scoring: the code leaves it out (0.28's rule), and
   scoring degree 0 of the borrowed scale would choose the colour for the wrong chord.
6. **`.cy` and `.cq`** are fixed-width hex without separators, as `.cx`. `.cq` adds a chromatic point that `.cp` lacks
   (a hand-edited state), and sets a found point's source and type, its degree 0 for a chromatic source.
7. **Feel's suggestions.** `has_seventh` was not in §3.4's list; without it Feel would have read a 7sus4 as a chord
   without a 7th.
8. **The melody over quartal and 7sus4** may play the scale's 3rd, as 0.28.1's melody does over its own sus4 chords
   (measured: B over G sus4). The test checks that every melody note belongs to the chord's own scale, which holds for
   the nine; making the generated melodies avoid a sus chord's 3rd would change 0.28.1's sound and is not this step's.

**The author's checks this step moved** (principle 9), each with a comment naming this specification: the three source
probes §5 foresaw (`tests/test_core.c`: `seq_set_slot` and `seq_rbar_chord`; `tests/test_tweak.c`: `seq_tweak_point_ok`),
now at `THEORY_SRC2_MAX + 1`; and three colour probes it did not, at `COL_COUNT`, which is now the quartal, moved to
`COL_ALL` (`tests/test_core.c`: `seq_set_slot`; `tests/test_tweak.c`: `seq_tweak_chord`, `seq_tweak_point_ok`). The
fork's own probes in `tests/test_chroma_path.c` moved likewise.

### 8.4 Saved state, as built

The writers see each slot twice: as it is, and as `old_chord` (`theory_fallback`) gives it to an older build. `.prog`,
`.pq`, `.ch` and `.cx` hold the second, `.cy` the first for every slot whose chord or Feel base differs; `.cp` holds each
point's fallback (an old colour's type), `.cq` the points that differ. `old_base` gives a Feel base code's fallback code.
In the compatibility harness, 0.28.1 re-saves each such state with every older field as the fork wrote it and the new
fields dropped (`tests/compat/expected.txt`); a fallback point on a first-range source is dropped by 0.28.1, as step 2's
chromatic points are (the slot's chord plays there).

### 8.5 Measurements

| Quantity | Before (step 2) | After |
|---|---|---|
| Pads needing a new sonority | 375 | **0** |
| Classic pads with no chromatic form | 164 | **0** |
| Scale Mode palettes with a chord Cadenza cannot voice | 4 of 7 (the 7sus4) | **0 of 7** |
| Pads exact only with step 3's values | | 375 (186 quartal, 124 It6, 17 minMaj7, 12 dom7b5, 12 m7b5, 12 minAug, 6 dom7b9, 6 minor6) |
| Chords in Cadenza's space | 4 060 | 10 738 |
| Pads near (the fifthless 11ths and 13ths, decision 12) | 66 | 66 |
| Binary, local estimate (`tools/dev/armsize.sh`) | | +3 300 bytes of code, +36 of data |
| Binary, CI | 650 KB | see the strategy, §5 |
| Core parity hash | `934d3f5c...` | unchanged |
| Compatibility harness | 17 scenarios | 20: 16 identical, 4 as pinned (step 2's and this step's three) |

The coverage tool's new lines were first run against step 2's enumeration, where they give 375, 164 and 4 of 7: the
figures after measure the step, not a changed definition.

### 8.6 Other front ends

For the design note to the author: a Feel base code may take the second form; the iOS app's Feel page decodes it only if
it shares this engine. The new fields `.cy` and `.cq`, which a front end that saves through `seq_state_save` writes
without knowing. `theory_fallback` is public, for any front end that writes states itself.
