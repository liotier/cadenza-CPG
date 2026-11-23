# 02 - Chromatic chords through the progression path

| | |
|---|---|
| Status | draft, for the owner's approval |
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
| D1 | **Open exactly the seven checks of §3.1 to the existing chromatic range**, `src` 0 to `THEORY_SRC_MAX` (51). Nothing beyond it. | The second range belongs to step 3, with its own save design. |
| D2 | **A chromatic chord's degree is 0.** Every writer forces it, as `load_cx` already does. | The chromatic pseudo-scale's degree 0 is the chord's root (theory.h); any other degree would be a different chord. |
| D3 | **No new saved field.** The 0.28 field `.cx` saves a chromatic source for any slot, whichever path wrote it ([seq_core.c:3935](../../../src/seq_core.c#L3935)), and `.ch` and `.pq` keep their fallback (`old_src`). | Already designed and released in 0.28. |
| D4 | **Change points keep their one-character encoding**, `'0' + src`, which reaches `c` at 51 and never a separator (`\|` or `,`). | No format change for the existing range. |
| D5 | **No screen change.** The Force's pickers still offer the scale's chords and the three borrowed ones. | Opening the engine's path is what steps 5 to 8 need; offering chromatic chords on screen is theirs. |
| D6 | **Saved sound unchanged.** No existing path writes chromatic sources through these checks, so the parity hash cannot move. | Principle 3 of the strategy. |

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
3. The core parity hash unchanged (D6).
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

Empty until implementation.
