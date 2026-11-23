# 01 - Key-aware chord spelling

| | |
|---|---|
| Status | implemented; the device run pending |
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
| S01-D1 | **A root takes the letter of its degree.** A diatonic chord: its degree's letter in the project scale's spelling. A borrowed or chromatic chord: the letter of the degree its numeral names (`theory_numeral`'s choice), with whatever accidental the semitones require. | What a numeral means; the CPG's spec 13, D1. It spells B♭ as F major's IV and D♭7 as C's ♭II7, and keeps a chord's name and numeral in agreement. |
| S01-D2 | **The tonic of a two-named key** (C♯/D♭, D♯/E♭, F♯/G♭, G♯/A♭, A♯/B♭) is the name whose spelling of the project scale needs fewer accidentals, a double accidental counting two; on a tie, the sharp name, which the key selector lists. | The CPG's spec 13, D2, applied to the project scale itself, since Cadenza's scales are modes of their own: E♭ major, but D♯ Phrygian. |
| S01-D3 | **A root is respelled to its plainest name only when S01-D1 gives a double accidental, or E♯, B♯, C♭ or F♭ on a note outside the key's scale.** | The CPG's spec 13, D3: F♯ major keeps its E♯dim; D♭ major's chromatic ♭VI is written A, not B𝄫. |
| S01-D4 | **Chord tones take letters by degree.** A colour's tones are scale degrees of the chord's own scale (`COLOUR_TONES`, [theory.c](../../../src/theory.c)), so tone *k* takes the letter *k - 1* steps above the root's. | The CPG's spec 13, D4, which Cadenza's model makes simpler: a chord tone already knows its degree. Csus4 is C F G, not C E♯ G. |
| S01-D5 | **Single notes** (bass and melody steps, the pickers' keys): a note of the project scale takes the scale's letter; else a tone of the chord sounding there takes the chord's letter for it; else the plainest name on the key's side (flats when the tonic's spelling has flats, sharps otherwise). | A line reads in the key, a chord tone as the chord spells it, and a passing note does not invent a remote spelling. |
| S01-D6 | **ASCII, as today:** `b`, `#`, and `bb`, `##` for the rare double accidental S01-D3 leaves. | The Force's font has no ♭ (MANUAL, §B). |
| S01-D7 | **Texts that name the key follow S01-D2** (PLAY's title line, `Eb MAJOR`); **the key selector's piano keeps its labels.** | The title names the key in use; the selector names twelve keys before any is chosen, as the CPG's spec 13, D9 keeps its own selector. |
| S01-D8 | **Display only.** Names are cached labels, never saved; no MIDI changes. | The core parity hash and the saved state stay exactly as they are. |

## 3. Specification

### 3.1 Theory

New functions in [theory.h](../../../src/theory.h), the existing ones kept for other callers (the iOS app and the browser
version share the engine):

| Function | Returns |
|---|---|
| `theory_spell_tonic(key, scale)` | the tonic's letter (0-6, C to B) and accidental (-2 to +2), by S01-D2 |
| `theory_spell_degree(key, scale, deg)` | a degree's letter and accidental in the project scale's spelling |
| `theory_spell_root(key, scale, slot_scale, deg)` | a chord's root by S01-D1 and S01-D3 |
| `theory_chord_name_s(key, scale, slot_scale, deg, colour, buf, len)` | `theory_chord_name_c`'s text with the spelled root |
| `theory_chord_card_s(...)` | `theory_chord_card`'s names form, spelled |
| `theory_note_name_s(key, scale, ctx, note, buf, len)` | a single note by S01-D5; `ctx` the chord sounding there, if any |
| `theory_key_name_s(key, scale)` | the key's name by S01-D2 |

The project scale is a parameter throughout: today's chord-name functions receive only the chord's own scale (the
`slot_scale` that `name_slots` passes), which cannot tell a borrowed chord's spelling.

### 3.2 Callers

| Where | What changes |
|---|---|
| [seq_core.c](../../../src/seq_core.c) `name_slots` | the slot labels: `theory_chord_name_s` |
| [seq_core.c](../../../src/seq_core.c) `seq_cell_label` (line 2549) | step labels: `theory_note_name_s` with the chord under the step |
| [engine.c](../../../src/engine.c), some twenty sites | the PROGRESSIONS tiles (`prog_text`), the bar editor's chord row and note pianos, the chord-note texts, the PEDAL, pentatonic and root texts, PLAY's title line; each named in the implementation notes |
| `kw_label`, `kb_label` ([engine.c:508](../../../src/engine.c#L508)) | unchanged (S01-D7) |

## 4. Measurements

A sweep over the 12 keys, the 7 scales and the 16 progression presets, counting labels before and after: how many change,
and that no label grows beyond its cell (a double accidental adds a character; `BPR_CHARS` cuts tiles at 19).

## 5. Verification

1. **An independent table**, written by hand in `tests/test_theory.c`, not derived from the code under test (the CPG's
   method): for the 12 keys and the 7 scales, the tonic and the seven letters; with the CPG's §3.2 table as the reference
   for Major and Minor.
2. Chord names for every degree, colour, borrowed chord and a set of chromatic chords in F, E♭, A♭, D♭, B, F♯ major and
   C♯, D♯, G♯ minor, against expected strings, including F♯ major's `E#dim` (kept, S01-D3) and D♭ major's chromatic ♭VI (`A`,
   S01-D3).
3. The plugin test passes, and no label exceeds its cell (§4).
4. The core parity hash and the compatibility harness are unchanged (S01-D8).
5. A device run: CHORDS, PHRASE, PLAY and GLOBAL read correctly in F, E♭, A♭ and D♭ major, and C♯ and D♯ minor.

## 6. Out of scope

The key selector's labels (S01-D7); numerals, which are right already; localisation; the spelling of the new sonorities of
step 3, which that step adds.

## 7. Open points, to settle while implementing

- S01-D5's last clause (passing notes on the key's side) may read oddly in some lines; adjust after reading real ones on the
  device.
- Whether a double accidental ever survives S01-D3 in Cadenza's chord model; the sweep of §4 will say.

## 8. Implementation notes

**The binary budget.** Step 0 measured 2 556 bytes of headroom (spec 00, §4). An ARM cross-compiler (GCC 13.3, the
toolchain's flags; the toolchain itself uses GCC 11) put this step at +4 476 bytes of code, 3.4 KB of it the spelling
logic, after a rewrite for size (the project scale spelled once per call, one name table without pointers). The first
push of this step failed CI's budget check, as expected: 690 KB against 688 (CI run 4, the toolchain's compiler), so the
step costs about 4 KB on the device's build. The owner chose both remedies (study, decision 21): section
garbage collection in the ARM build (`-ffunction-sections -fdata-sections -Wl,--gc-sections` in `vst/vst.json`'s
`cflags_arm`), which drops about 46 KB of functions the plugin never calls (kept for the other front ends), and a budget
of 720 KB (`vst/budget.json`). The collected build measures 650 KB (test build 4, commit `d9b619d`): about 40 KB of
dead code gone, some 70 KB of headroom. The plugin still exports `VSTPluginMain` alone. Fork builds are no longer byte-identical to
official ones; the device run (verification 5) also confirms that the collected binary loads.

**Interface** ([theory.h](../../../src/theory.h)), differing from §3.1 for size: `theory_spelling_t` (letter, accidental),
`theory_spelling_text`, `theory_spell_tonic`, `theory_spell_degree`, `theory_spell_root`, `theory_spell_tone` (S01-D4),
`theory_chord_tones`, `theory_chord_name_s`, `theory_pc_text_s` (a constant string, in place of a `theory_pc_name_s`),
`theory_note_name_s`, `theory_key_name_s`; `theory_chord_card_s` is left out, the plugin calling no `theory_chord_card`.
In [seq_core.h](../../../src/seq_core.h): `seq_note_name` and `seq_pc_name`, a note named by the chord sounding at a
step of a part (S01-D5's context).

**Two rules made precise.** S01-D3 reaches single notes and chord tones too: no double accidental is ever shown, so G♯
harmonic minor's seventh degree, F𝄪 in the scale's spelling, shows as G, as its vii° is named Gdim. S01-D5's key side is
flats when the project scale's spelling holds a flat.

**Callers.** The core: `name_slots` and `voice_points` (every slot and change-point label), `chord_word` (the move
texts), `seq_cell_label` (the grid's Bass and Melody steps, with their chord). The engine: `prog_text` (the PROGRESSIONS
tiles and field), `fix_labels` (the picker's chord names, the bass keys' note names, Pentatonic's home notes), the PEDAL
texts (four sites), `page_labels` (the CHORDS strip's names, and the chord-note texts by tone, S01-D4), `slot_labels` (the
triad names of PLAY's cells, the fs_ texts), the melody bar roles, `add_note` (twelve sites, each with the chord at its
step), the BASS piano's keys, the bass "ends on" text's root, PLAY's title. The key selector (`kw_label`, `kb_label`)
is unchanged (S01-D7).

**Measurements** (§4). Over 12 keys, 7 scales and every source, degree and colour, 48 720 chord names: 13 220 change
(27.1 %); 460 grow by one character (E♯, B♯, C♭ or F♭ where 0.28.1 printed F, C, B or E), the longest stays 9 characters
(`Dbdimadd9`, as `C#dimadd9` was), so no label exceeds what 0.28.1 could already produce; no double accidental. The
progression presets' chords: 1 022 of 5 124 change (19.9 %). Pitch classes without a chord: 187 of 1 008.

**Tests.** `tests/test_theory.c`: the 84 keys and scales against a table written by hand from the key signatures
(verification 1), triads and borrowed chords in F, E♭, A♭, D♭, B, F♯ major and C♯, D♯, G♯ minor, twelve coloured and
chromatic names (verification 2), single notes, octaves and tones. Three of the author's checks pinned sharp spellings:
`tests/test_plugin.c` (C minor's `G# D# A#`, now `Ab Eb Bb`; the title `D# DORIAN`, now `Eb DORIAN`, with a new check of
`D# PHRYGIAN`) and `tests/test_feel.c` (below). The core parity hash and the compatibility harness are unchanged (S01-D8).

**Open point found, then settled.** S01-D1 follows `theory_numeral`, which named a chromatic root by the degree above it
when it is not a scale degree: in D minor the major chord on F♯ read `bIV`, and S01-D1 spelt it `Gb7`, where `F#7` (♯III)
is the usual reading. The owner asked for the musically most correct fix (strategy, O11): [addendum A](#addendum-a-chromatic-roots).

**Verification.** 1 to 4 hold; 5, the device run, waits.

---

## Addendum A: chromatic roots

| | |
|---|---|
| Status | implemented; the owner asked for "the musically most correct way" (strategy, §8.3, O11), the rule is Claude's, for review |
| Type | defect, display only |
| Size | S |

### A.1 Problem

A chromatic chord's root is not a scale degree, so it must be written as the degree above it, flattened, or the degree
below, sharpened. 0.28.1's `theory_numeral` took the flat whenever it could, whatever the chord: D minor's F♯ major read
`bIV`, and with S01-D1 the name followed, `Gb7`. In C major it read F♯ diminished, the leading-tone chord of V, as `bV°`,
`Gbdim`.

### A.2 Decisions

| # | Decision | Rationale | Rests on |
|---|---|---|---|
| S01-A1 | **A chromatic root takes the degree the harmonic chromatic scale names** (1 ♭2 2 ♭3 3 4 ♯4 5 ♭6 6 ♭7 7 above the tonic, whatever the mode), **except a chord whose triad is diminished, which takes the raised degree below its target** (the leading-tone chord: C♯°7 is vii°7 of ii in C). When the named degree is not in the project scale, the other one. | The harmonic chromatic scale is the conventional spelling of chromatic notes relative to a tonic, the same ascending and descending: the major and minor scales' notes plus the flattened second and the sharpened fourth. It gives the Neapolitan ♭II, the mixture chords ♭III, ♭VI, ♭VII, the major third ♯III in minor. A secondary leading-tone chord is built on the raised degree a half step below the chord it tonicises. | S01-D1; owner, O11 |
| S01-A2 | **The numeral and the name use the same degree:** `theory_numeral` and the spelling share one helper, `chroma_degree`. | S01-D1's agreement between name and numeral holds; `theory_numeral` is upstream's, so this changes what the author's numerals print for chromatic chords: display only, for the design note to the author. | S01-A1 |

Sources: the harmonic chromatic scale as "the same whether rising or falling", holding "all the notes in the major,
harmonic minor or melodic minor scales plus flattened second and sharpened fourth degrees"
([Wikipedia, Talk: Chromatic scale](https://en.wikipedia.org/wiki/Talk:Chromatic_scale); a talk page, so a secondary
source; the same definition in [these chromatic scale rules](https://www.scribd.com/document/458942220/Chromatic-scale-rules-pdf));
secondary leading-tone chords on the raised degree below their target
([Wikipedia, Secondary chord](https://en.wikipedia.org/wiki/Secondary_chord);
[Wikipedia, Leading tone](https://en.wikipedia.org/wiki/Leading_tone)).

### A.3 Measurements and verification

- 14 textbook cases written by hand in `tests/test_theory.c` (`test_fork_chromatic_degrees`): D minor's `F#7`, `#III`;
  C major's Neapolitan `Db`, `bII`; the leading-tone sevenths `C#dim7`, `D#dim7`, `F#dim7`, `G#dim7`; `bIII`, `bVI`,
  `bVII`; the tritone dominant `F#7`, `#IV`; A minor's Neapolitan `Bb` and `G#dim7`; C harmonic minor's diminished chord
  on B♭, whose raised degree below is not a degree (`Bbdim`, `bvii°`).
- `tests/test_feel.c` expects `F#7` again for D minor's chord, as 0.28.1's own test did, now through the numeral `#III`.
- Agreement with the CPG's spelling: of the 2 804 Classic pads Cadenza can voice, 13 were spelled differently in C
  (F♯→G♭ 5, D♯→E♭ 5, C♯→D♭ 3); none are now.
- The offline suite passes, the core parity hash and the compatibility harness are unchanged; the binary grows by about
  416 bytes (the ARM estimate).
