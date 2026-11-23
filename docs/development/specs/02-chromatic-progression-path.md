# 02 - Chromatic chords through the progression path

| | |
|---|---|
| Status | implemented; the device run is shared with step 1's |
| Type | capability |
| Size | S-M |
| Depends on | 00 |
| Blocks | 03, 05 |

## 1. Purpose

Since 0.28 Cadenza's engine stores a chord on any root, the chromatic sources 4 to 51 (12 roots by four qualities,
[theory.h:50-55](../../../src/theory.h#L50)), and its save format keeps them. The Feel layer writes them; the Force's
progression path does not: seven checks accept only the scale's chords and its three borrowed ones, and two of them turn
anything else silently into a diatonic chord (study, §6.2). Every later step writes palette chords into slots, so the
path must accept the chromatic range first.

## 2. Decisions

| # | Decision | Rationale |
|---|---|---|
| S02-D1 | **Open exactly the seven checks of §3.1 to the existing chromatic range**, `src` 0 to `THEORY_SRC_MAX` (51). Nothing beyond it. | The second range belongs to step 3, with its own save design. |
| S02-D2 | **A chromatic chord's degree is 0.** Every writer forces it, as `load_cx` already does. | The chromatic pseudo-scale's degree 0 is the chord's root (theory.h); any other degree would be a different chord. |
| S02-D3 | **No new saved field.** The 0.28 field `.cx` saves a chromatic source for any slot, whichever path wrote it ([seq_core.c:3935](../../../src/seq_core.c#L3935)), and `.ch` and `.pq` keep their fallback (`old_src`). | Already designed and released in 0.28. |
| S02-D4 | **Change points keep their one-character encoding**, `'0' + src`, which reaches `c` at 51 and never a separator (`\|` or `,`). | No format change for the existing range. |
| S02-D5 | **No screen change.** The Force's pickers still offer the scale's chords and the three borrowed ones. | Opening the engine's path is what steps 5 to 8 need; offering chromatic chords on screen is theirs. |
| S02-D6 | **Saved sound unchanged.** No existing path writes chromatic sources through these checks, so the parity hash cannot move. | Principle 3 of the strategy. |

## 3. Specification

### 3.1 The seven checks

| Site | Today | After |
|---|---|---|
| `fill_phrase`, [seq_core.c:339](../../../src/seq_core.c#L339) (presets, PROGRESSION ◀ ▶, a phrase through `seq_set_progression`) | `src > THEORY_NBORROW` written as 0, silently | `src` kept up to `THEORY_SRC_MAX`; degree 0 when chromatic |
| `set_prog`, [seq_core.c:6268](../../../src/seq_core.c#L6268) (the whole loop through `seq_set_progression`) | the same | the same as above |
| `seq_set_slot`, [seq_core.c:5802](../../../src/seq_core.c#L5802) (a pick on a slot) | refused | accepted up to `THEORY_SRC_MAX`; `theory_borrow_degree` already returns 0 for a chromatic source |
| `cp_set`, [seq_core.c:2019](../../../src/seq_core.c#L2019) (a change point) | refused | accepted, as above |
| `seq_rbar_chord`, [seq_core.c:7481](../../../src/seq_core.c#L7481) (the bar editor's chord pick) | refused | accepted, as above |
| `seq_tweak_point_ok`, [seq_core.c:8671](../../../src/seq_core.c#L8671) | refused | accepted, as above |
| `load_cp`, [seq_core.c:4697](../../../src/seq_core.c#L4697) (loading change points) | entries above 3 skipped | accepted up to `THEORY_SRC_MAX` |

A helper, `theory_src_ok(src)` in [theory.h](../../../src/theory.h), states the range once; step 3 extends it.

### 3.2 The readers' audit

Every reader of a slot's or a change point's source is listed in the implementation notes with whether it handles a
chromatic source, found by searching `THEORY_NBORROW`, `->src`, `.src[` and `src[` in [seq_core.c](../../../src/seq_core.c),
[feel.c](../../../src/feel.c) and [engine.c](../../../src/engine.c). Most go through `theory_slot_scale`, which handles
the chromatic range; those that do not are fixed in this step. Known candidates: `borrow_degrees` (at every revoice),
`lock_colour`, the COLOUR knob's ranking, `theory_drift`, `cp_sync`, the Try and keep logic of [feel.c](../../../src/feel.c),
JAM's queued renders, LOCK WITH, and the engine's chord rows and labels.

## 4. Measurements

Binary size before and after; no other budget moves.

## 5. Verification

1. **New tests**, `tests/test_chroma_path.c`, through the public API only. For each writer of §3.1, chromatic chords (D♭7
   in C, `src = 4 + 4 × 1 + CQ_DOM` with colour 7th; A♭ major; F♯ diminished) are written, then checked: the slot's name
   and numeral, its notes, the bass and melody following its chord tones; undo restoring the previous chords; a save and
   load round trip giving the same state bytes; a chord-length change and a COPY TO keeping the sources; change points
   with chromatic chords surviving a round trip.
2. A randomised sweep: progressions drawing on the whole chromatic range, played through the plugin test under ASan and
   UBSan, to catch a reader the audit missed.
3. The core parity hash unchanged (S02-D6).
4. The compatibility harness, two new scenarios: a state with chromatic slots loads in 0.28.1 and saves back identically
   (0.28.1 reads `.cx`); a state with chromatic change points loads in 0.28.1 with those points dropped, the slot's chord
   playing there instead, as `tests/compat/expected.txt` records.

## 6. Out of scope

Chromatic chords offered on the Force's screens (steps 6 and 7); the second chromatic range and new colours (step 3).

## 7. Open points, to settle while implementing

- Whether some reader assumes a borrowed source in a way the audit and the sweep miss; any such finding is added to the
  audit table.
- Whether the iOS-written chromatic slots and these path-written ones interact (Feel's `base` and `pick` fields); the
  tests cover a slot written by each route in turn.

## 8. Implementation notes

**The seven checks** (§3.1) now call `theory_src_ok(src)` (theory.h, theory.c: `0 <= src <= THEORY_SRC_MAX`). `fill_phrase`
and `set_prog` write degree 0 for a chromatic source and read any source outside the range as the scale's (0.28.1 did so
from 4 on); `seq_set_slot`, `cp_set` and `seq_rbar_chord` already took the degree from `theory_borrow_degree`, which gives
0 past the borrowed range; `load_cp` loads a chromatic point with degree 0 whatever its file says. The header comments of
`seq_cp_t`, `seq_part_t.src`, `seq_set_slot` and `seq_set_progression` name the range. Nothing else changed.

**The readers' audit** (§3.2), every reader of a slot's or a point's source in `seq_core.c`, `feel.c` and `engine.c`:

| Reader | How it reads the source | Chromatic |
|---|---|---|
| `slot_scale`, `slot_notes`, `eff_colour`, `lock_colour`, `chord_word`, `chord_level` (the COLOUR ranking), `theory_drift`, `theory_avoid`, Feel's `root_pc`, `quality`, its offers | through `theory_slot_scale` | handled since 0.28 |
| `borrow_degrees` (every revoice) | `theory_borrow_degree` | degree 0, as S02-D2 requires |
| `copy_slot`, `repeats`, `part_view`, `cp_sync`'s mirror, the slot comparisons of the Feel runs, the change test of `cp_sync` and `seq_chord_change_at` | equality | indifferent |
| `feel_code_of` | `1 + deg + 7 x (src + 52 x colour)` | sized for 0..51 since 0.28 |
| the preset match (`seq_prog_preset`) | a non-zero source is no preset's | indifferent |
| the save (`.pq`, `.ch` with `old_src`, `.cx`) and the load (`.cx` over the fallbacks) | 0.28's design | unchanged (S02-D3) |
| `out_cp` and `load_cp` | `'0' + src`, `'4'` to `'c'` | the load opened here (S02-D4) |
| Feel's `base.src` tests (its sus and add9 suggestions) | a non-zero source is not the scale's | as for a borrowed chord |
| the engine's caches (`fix_src_t`, `slot_src_t`, `bass_src_t`) | `memcmp` of the arrays | indifferent |
| the engine's labels (`page_labels`, `slot_labels`, the bar texts) | through `theory_slot_scale` | handled; swept (below) |
| the generator (`gen_progression_v2`) | writes 0 to 3 only | unchanged |

No reader needed a fix. Two observations: `theory_drift` adds its "borrowed" 30 to any chord off the project scale, so a
chromatic chord scores as a borrowed one, as Feel's have since 0.28; and `seq_tweak_point_ok` has no caller in the Force
plugin: it serves another front end's Tweak page (its header says "a change point can hold this chord"), where a chromatic
point becomes possible. That is a change in a product this fork does not build, for the conversation with the author.

**Tests.** `tests/test_chroma_path.c`, in `tests/run_tests.sh` after the Feel tests: each writer of §3.1 with D♭7, A♭
and F♯°, their sound (the Chords lane's notes, the bass on the root), undo, a save and load, a chord-length change (each
chord keeps its place, two slots long), key and scale changes, COPY TO with + CHORDS, a hand-edited point's stray degree,
and a sweep of 60 random loops over the whole range; 27 checks, of which 23 fail on 0.28.1's engine. `tests/test_plugin.c`
gains `test_chroma_sweep`: 12 rounds of random chords and points over the whole range, in random keys and scales, read
through PHRASE, the sound pages and JAM while playing, under ASan and UBSan. Three of the author's checks pinned 0.28.1's
refusal of source 4 (`test_core.c`, twice; `test_tweak.c`); they now probe `THEORY_SRC_MAX + 1`, and the Tweak test
checks that a chromatic point is accepted.

**Compatibility.** Two scenarios in `tests/compat/writer.c`: `chroma-slots` comes back identical from 0.28.1, which reads
`.cx`; `chroma-points` comes back without its two chromatic points (0.28.1's `load_cp` skips sources above 3) and with
its diatonic one, pinned in `tests/compat/expected.txt`.

**Measurements.** The core parity hash is unchanged (S02-D6). The binary grows by about 210 bytes (an ARM cross-compiler's
figure; CI's exact one goes with the commit's run).
