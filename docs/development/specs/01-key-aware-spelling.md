# 01 - Key-aware chord spelling

| | |
|---|---|
| Status | draft, for the owner's approval |
| Type | defect |
| Size | S-M |
| Depends on | 00 |
| Blocks | nothing; offered upstream first (milestone M1) |

## 1. Problem

Cadenza spells every pitch with a sharp: one table, `NOTE[12]` ([theory.c:6](../../../src/theory.c#L6)), names every root,
note and key. Measured (study, §2.1):

| Key | Printed | Correct |
|---|---|---|
| F major, chords I to vii° | F Gm Am **A#** C Dm Edim | F Gm Am **Bb** C Dm Edim |
| E♭ major | **D#** Fm Gm **G# A#** Cm Ddim | **Eb** Fm Gm **Ab Bb** Cm Ddim |
| A♭ major | **G# A#m** Cm **C# D#** Fm Gdim | **Ab Bbm** Cm **Db Eb** Fm Gdim |
| C major, chromatic ♭II7 | **C#7** beside its numeral `bII7` | **Db7** |

The names reach the chord labels of every page, the PROGRESSIONS tiles, the bar editor's chord and note pickers, the
step labels on PLAY ("A#2"), the PEDAL and pentatonic texts, and PLAY's title line ("D# MAJOR"). Numerals are right
already: `theory_numeral` writes b and # against the project scale.

## 2. Decisions

The CPG solved the same defect ([spec 13](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/improvements/13-spelling.md));
its rules carry over, adapted to Cadenza's seven scales and its chord model.

| # | Decision | Rationale |
|---|---|---|
| D1 | **A root takes the letter of its degree.** A diatonic chord: its degree's letter in the project scale's spelling. A borrowed or chromatic chord: the letter of the degree its numeral names (`theory_numeral`'s choice), with whatever accidental the semitones require. | What a numeral means; the CPG's D1. It spells B♭ as F major's IV and D♭7 as C's ♭II7, and keeps a chord's name and numeral in agreement. |
| D2 | **The tonic of a two-named key** (C♯/D♭, D♯/E♭, F♯/G♭, G♯/A♭, A♯/B♭) is the name whose spelling of the project scale needs fewer accidentals, a double accidental counting two; on a tie, the sharp name, which the key selector lists. | The CPG's D2, applied to the project scale itself, since Cadenza's scales are modes of their own: E♭ major, but D♯ Phrygian. |
| D3 | **A root is respelled to its plainest name only when D1 gives a double accidental, or E♯, B♯, C♭ or F♭ on a note outside the key's scale.** | The CPG's D3: F♯ major keeps its E♯dim; D♭ major's chromatic ♭VI is written A, not B𝄫. |
| D4 | **Chord tones take letters by degree.** A colour's tones are scale degrees of the chord's own scale (`COLOUR_TONES`, [theory.c](../../../src/theory.c)), so tone *k* takes the letter *k - 1* steps above the root's. | The CPG's D4, which Cadenza's model makes simpler: a chord tone already knows its degree. Csus4 is C F G, not C E♯ G. |
| D5 | **Single notes** (bass and melody steps, the pickers' keys): a note of the project scale takes the scale's letter; else a tone of the chord sounding there takes the chord's letter for it; else the plainest name on the key's side (flats when the tonic's spelling has flats, sharps otherwise). | A line reads in the key, a chord tone as the chord spells it, and a passing note does not invent a remote spelling. |
| D6 | **ASCII, as today:** `b`, `#`, and `bb`, `##` for the rare double accidental D3 leaves. | The Force's font has no ♭ (MANUAL, §B). |
| D7 | **Texts that name the key follow D2** (PLAY's title line, `Eb MAJOR`); **the key selector's piano keeps its labels.** | The title names the key in use; the selector names twelve keys before any is chosen, as the CPG's D9 keeps its own selector. |
| D8 | **Display only.** Names are cached labels, never saved; no MIDI changes. | The core parity hash and the saved state stay exactly as they are. |

## 3. Specification

### 3.1 Theory

New functions in [theory.h](../../../src/theory.h), the existing ones kept for other callers (the iOS app and the browser
version share the engine):

| Function | Returns |
|---|---|
| `theory_spell_tonic(key, scale)` | the tonic's letter (0-6, C to B) and accidental (-2 to +2), by D2 |
| `theory_spell_degree(key, scale, deg)` | a degree's letter and accidental in the project scale's spelling |
| `theory_spell_root(key, scale, slot_scale, deg)` | a chord's root by D1 and D3 |
| `theory_chord_name_s(key, scale, slot_scale, deg, colour, buf, len)` | `theory_chord_name_c`'s text with the spelled root |
| `theory_chord_card_s(...)` | `theory_chord_card`'s names form, spelled |
| `theory_note_name_s(key, scale, ctx, note, buf, len)` | a single note by D5; `ctx` the chord sounding there, if any |
| `theory_key_name_s(key, scale)` | the key's name by D2 |

The project scale is a parameter throughout: today's chord-name functions receive only the chord's own scale (the
`slot_scale` that `name_slots` passes), which cannot tell a borrowed chord's spelling.

### 3.2 Callers

| Where | What changes |
|---|---|
| [seq_core.c](../../../src/seq_core.c) `name_slots` | the slot labels: `theory_chord_name_s` |
| [seq_core.c](../../../src/seq_core.c) `seq_cell_label` (line 2549) | step labels: `theory_note_name_s` with the chord under the step |
| [engine.c](../../../src/engine.c), some twenty sites | the PROGRESSIONS tiles (`prog_text`), the bar editor's chord row and note pianos, the chord-note texts, the PEDAL, pentatonic and root texts, PLAY's title line; each named in the implementation notes |
| `kw_label`, `kb_label` ([engine.c:508](../../../src/engine.c#L508)) | unchanged (D7) |

## 4. Measurements

A sweep over the 12 keys, the 7 scales and the 16 progression presets, counting labels before and after: how many change,
and that no label grows beyond its cell (a double accidental adds a character; `BPR_CHARS` cuts tiles at 19).

## 5. Verification

1. **An independent table**, written by hand in `tests/test_theory.c`, not derived from the code under test (the CPG's
   method): for the 12 keys and the 7 scales, the tonic and the seven letters; with the CPG's §3.2 table as the reference
   for Major and Minor.
2. Chord names for every degree, colour, borrowed chord and a set of chromatic chords in F, E♭, A♭, D♭, B, F♯ major and
   C♯, D♯, G♯ minor, against expected strings, including F♯ major's `E#dim` (kept, D3) and D♭ major's chromatic ♭VI (`A`,
   D3).
3. The plugin test passes, and no label exceeds its cell (§4).
4. The core parity hash and the compatibility harness are unchanged (D8).
5. A device run: CHORDS, PHRASE, PLAY and GLOBAL read correctly in F, E♭, A♭ and D♭ major, and C♯ and D♯ minor.

## 6. Out of scope

The key selector's labels (D7); numerals, which are right already; localisation; the spelling of the new sonorities of
step 3, which that step adds.

## 7. Open points, to settle while implementing

- D5's last clause (passing notes on the key's side) may read oddly in some lines; adjust after reading real ones on the
  device.
- Whether a double accidental ever survives D3 in Cadenza's chord model; the sweep of §4 will say.

## 8. Implementation notes

Empty until implementation.
