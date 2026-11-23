# 03 - The eight missing sonorities

| | |
|---|---|
| Status | draft: design proposals, to be revised once steps 0 to 2 are implemented |
| Type | capability, save format |
| Size | M |
| Depends on | 02 |
| Blocks | 04 |

## 1. Purpose

Decision 5 of the study rules out approximation. Across the six styles of the CPG's catalogue, 375 of 17 664 palette pads
are chords Cadenza cannot voice exactly (study, §3); the other 66 pads that differ only omit the fifth of an 11th or a
13th, which decision 12 accepts as Cadenza's voicing policy. Eight sonorities make up the gap:

| Sonority | Pads | Intervals |
|---|---|---|
| quartal triad | 186 | 0 5 10 |
| Italian sixth (a dominant seventh without its fifth) | 124 | 0 4 10 |
| minor-major seventh | 17 | 0 3 7 11 |
| 7♭5 | 12 | 0 4 6 10 |
| half-diminished seventh, on a root whose scale has none | 12 | 0 3 6 10 |
| minor ♯5 (the line cliché) | 12 | 0 3 8 |
| 7♭9 | 6 | 0 4 7 10 13 |
| minor sixth, on a root whose scale has none | 6 | 0 3 7 9 |

## 2. Decisions (proposed)

| # | Decision | Rationale |
|---|---|---|
| D1 | **Five new chromatic qualities and two new colours** give the eight, exactly (checked: the constructions of §3.1 give these intervals). | Cadenza builds a chord from a degree, a source and a colour, its tones taken from the chord's own seven-note scale (theory.h). A quality is such a scale; a colour picks tones from it. |
| D2 | **The new qualities form a second chromatic range**, appended after source 51: source `52 + 5 × root + quality`, up to 111. `THEORY_SRC_MAX` stays 51. | Enlarging the first range would renumber every saved chromatic chord (study, §5.5); a second range leaves them alone. |
| D3 | **The new colours are appended** to the colour list (10 and 11), never on the COLOUR knob, never in the bar editor's TYPE list. | They reach the user as palette chords with names; the controls stay as they are (study, avenue H). |
| D4 | **Every new value is saved twice: an exact value in a new field, and the nearest old value where older builds read.** | An older build ignores a whole `.ch` for one character outside its alphabet ([load_slots](../../../src/seq_core.c#L4332)) and skips a `.cx` entry beyond source 51; with fallbacks it plays a near chord on the right root instead of a wrong one. Degraded, never refused (decision 11). |
| D5 | **Names and numerals know the new values explicitly**, never inferring them from intervals. | `theory_triad_quality` would call the minor ♯5 minor and the 7♭5 diminished; the names must come from the quality itself. |
| D6 | **The fifthless 11ths and 13ths stay as they are** (decision 12). | Recorded with its trade-off in the study, avenue H. |

## 3. Specification

### 3.1 Constructions

| New value | Kind | Seven-note scale or tones | Gives, with |
|---|---|---|---|
| melodic minor | quality | 0 2 3 5 7 9 11 | colour 7th: minor-major seventh; colour 6th: minor sixth |
| Locrian | quality | 0 1 3 5 6 8 10 | colour 7th: half-diminished seventh |
| dominant ♭5 | quality | 0 2 4 5 6 9 10 | colour 7th: 7♭5 |
| Phrygian dominant | quality | 0 1 4 5 7 8 10 | colour 9th: 7♭9 |
| minor ♯5 | quality | 0 2 3 5 8 9 10 | colour Triad: minor ♯5 |
| quartal | colour | tones 1, 4, 7 | on the dominant quality: 0 5 10 |
| seventh without fifth | colour | tones 1, 3, 7 | on the dominant quality, root ♭6: the Italian sixth |

The bass and the melody read a chord's root, third and fifth from its own scale (`mjam_chord_of`), so the minor ♯5's raised
fifth and the 7♭5's lowered one reach them without further change.

### 3.2 Names and numerals

Following the CPG's suffixes (`CHORD_NAME_SUFFIX`, `CHORD_TYPE_DISPLAY`), in Cadenza's ASCII:

| Sonority | Name | Numeral |
|---|---|---|
| minor-major seventh | `CmMaj7` | `i(maj7)` |
| minor sixth | `Cm6` | `i6` |
| half-diminished | `Cm7b5` | `iim7b5`, as Cadenza writes it today |
| 7♭5 | `C7b5` | `V7b5` |
| 7♭9 | `C7b9` | `V7b9` |
| minor ♯5 | `Cm#5` | `i#5` |
| quartal | `C7sus4(no5)` | `I7sus4(no5)` |
| Italian sixth | `Ab7(no5)` | `bVI7(no5)` |

The CPG names the Italian sixth `Ab7`, like the German; `Ab7(no5)` keeps two different sonorities apart in Cadenza, where
names are all the screen shows (open point).

### 3.3 Saved state

| Value | Exact, in a new field | Fallback, where older builds read |
|---|---|---|
| second-range source of a slot | `<P>.cy=`: slot and source, in hex | `.cx`: the same root in the nearest first-range quality (melodic minor and minor ♯5 → minor; Locrian → diminished; dominant ♭5 and Phrygian dominant → dominant) |
| new colour of a slot | `<P>.cz=`: slot and colour, in hex | `.ch` and `.pq`: quartal → sus4; seventh without fifth → 7th |
| change point with a new value | `<P>.cq=`: bar, step, source and type, in hex | `.cp`: the fallback source and type |
| Feel base code naming a new value | a second code form, flagged by the top bit of the 16, holding degree, source and colour | older builds misread the base, which only records the chord as first written: harmless |

New builds read the exact fields after the old ones and let them override. `.pq` writes colours with `%d`, so a colour of
10 or more would break its two-character entries: the fallback applies there as well.

### 3.4 Other readers

Drift scores (`theory_drift`): quartal as sus4 (12), seventh without fifth as 7th (10); the chromatic +30 applies to the
new qualities as to the old. Avoid notes (`theory_avoid`) checked for the new qualities. `theory_src_ok` (step 2)
extended to the second range.

## 4. Measurements

The coverage tool of step 0 reports, across the six styles, no missing sonority and 66 fifth-omitted pads accepted by
decision 12. Binary size before and after.

## 5. Verification

1. Each construction of §3.1 gives its intervals in every key (theory tests).
2. Names and numerals as in §3.2 (theory tests), spelled by step 1's rules.
3. Bass and melody follow each new chord's tones (core tests).
4. Save and load round trips for slots, change points and Feel bases holding new values.
5. The compatibility harness: 0.28.1 loads every such state, playing the fallback chords of §3.3, never refusing;
   `tests/compat/expected.txt` lists the differences.
6. The core parity hash unchanged: no existing state contains a new value.

## 6. Out of scope

Offering the new sonorities on the COLOUR knob or in the bar editor's TYPE list; any chord type the catalogue does not use
(7sus4, the altered dominants), which can follow the same method if ever needed.

## 7. Open points, to settle while implementing

- The Italian sixth's name: `Ab7(no5)` against the CPG's `Ab7`.
- The fallback mappings of §3.3, to be heard on 0.28.1 before they are fixed.
- The exact layout of the second Feel code form, and whether the iOS app's Feel page must learn it (upstream's call).
- Whether the second range should leave room for later qualities (a stride above 5).

## 8. Implementation notes

Empty until implementation.
