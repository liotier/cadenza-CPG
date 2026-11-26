# Cadenza and the Chord Progression Generator: an integration study

Status: study, revision 15 (the [revision history](#revision-history) closes the document). No code has been written; the
measurements come from throwaway scripts described in [Appendix A](#appendix-a-measurement-method).

| Project | Version studied | Licence |
|---|---|---|
| Cadenza: this repository, a copy of [Shrekage2024/cadenza-force](https://github.com/Shrekage2024/cadenza-force) | 0.28.1, commit `56e95a0` | [PolyForm Shield 1.0.0](https://polyformproject.org/licenses/shield/1.0.0) |
| [MPC Chord Progression Generator](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator), here "the CPG" | `liotier/AkaiMPC` commit `c84a306` | [Unlicense](https://unlicense.org) |
| [Ouarpeggiator](https://github.com/liotier/Ouarpeggiator) | commit `56485c0` | Unlicense |

Notation: templates are written with hyphens (I-V-vi-IV); numerals follow the CPG's parallel-major convention unless a
passage quotes Cadenza. Links to Cadenza files are relative to this repository.

## Contents

1. [Summary](#1-summary)
2. [The three projects, in the terms that matter here](#2-the-three-projects-in-the-terms-that-matter-here)
3. [Complementarity, measured](#3-complementarity-measured)
4. [Palettes, not progressions](#4-palettes-not-progressions)
5. [Constraints any integration must respect](#5-constraints-any-integration-must-respect)
6. [Mapping the CPG onto Cadenza](#6-mapping-the-cpg-onto-cadenza)
7. [Integration avenues](#7-integration-avenues)
8. [Recommended staging](#8-recommended-staging)
9. [Licence, governance and branding](#9-licence-governance-and-branding)
10. [Decision log](#10-decision-log)
11. [Open questions](#11-open-questions)
12. [Appendix A: measurement method](#appendix-a-measurement-method) · [Appendix B: source map](#appendix-b-source-map) ·
    [Revision history](#revision-history)

---

## 1. Summary

Cadenza and the CPG are complementary along a single axis: **time against harmony**.

- **Cadenza** writes and performs in time: three lanes that listen to each other, sixteen steps a bar, rhythm libraries,
  JAM blocks, phrase forms, call and answer, locks, song parts, placement from MPC's song position. Its harmony, as the
  Force's screens expose it, is small: 16 progressions of diatonic degrees, the scale's seven chords plus three borrowed
  ones in the bar editor, 7 scales, and chord names spelled with sharps in every key (E♭ major reads
  `D# Fm Gm G# A# Cm Ddim`).
- **The CPG** is, despite its name, a **palette generator**: for each of 184 genre templates it builds 16 distinct chords,
  from foundation to spice, whose fourth row is chosen by harmonic-function analysis (secondary dominants, Neapolitan,
  augmented sixths, tritone substitutes, mediants, mode flips), with six voicing treatments, key-aware spelling and a CI
  theory audit. Its templates are starting orders, not the point: the palette's chords are meant to be played in whatever
  order the musician likes, as Pad Perform invites. It has no notion of time, and should keep none: time is the
  arranger's business.
- **The Ouarpeggiator** tried to give the CPG a time axis. Cadenza's is already far richer; the economical move is the
  converse one, bringing the CPG's palettes into Cadenza.

Four findings shape the rest of this study.

1. **Palettes are not a blocker.** Cadenza's engine plays an ordered, timed list of chords, as any music in time must,
   but nothing in it requires that order to be fixed or authored in advance: slots change from setters at any moment,
   and JAM already lands changes at quantised switch points. The CPG's palette therefore fits as *the part's harmonic
   space*, and the order becomes the work of an orderer: the template's starting order, the user editing, the user
   playing pads live, a seeded palette walk, phrase-end turnarounds (§4). What would have been a blocker is an
   integration that imported templates as fixed progressions; this study no longer proposes one.
2. **Cadenza's chord model is nearly there.** Since 0.28 its engine stores chords on any root, chords of unequal length and
   hand-placed voicings: the Feel layer, built first for the iOS app's Feel page, which the Force does not expose yet.
   Measured over the whole catalogue, 85.8 % of template chords are diatonic in a fitting Cadenza scale and all but a
   handful of palette pads are representable. Exactness, the owner's requirement, needs eight more sonorities
   ([avenue H](#h-completing-the-chord-vocabulary)): no approximation is necessary.
3. **The silent clamp is a leftover guard.** The Force's progression path turns any chord other than diatonic or one of
   three borrowed into a diatonic one, but the 0.28 save format already stores chromatic chords on any slot. Lifting the
   clamp means opening six checks in the editing paths and one in the loader; only the new sonorities of avenue H will need
   a wider change-point encoding (§6.2).
4. **Voice leading is already sound in Cadenza.** Its Smooth mode finds the least total motion, which for chords of equal
   size is optimal, chains the loop cyclically and survives SPREAD. The CPG's proximity measures pay off more where chords
   are *chosen* (the palette walk, ranking palette chords in the bar editor, turnarounds) than in how they are voiced
   (§6.6).

Recommended path ([§8](#8-recommended-staging)): first the engine work that needs no new screen (key-aware spelling, a
palette table generated from the CPG's own modules, the clamp lifted, the missing sonorities, a palette per part), then
the palette browser and the palette in the bar editor, then live palette pads and turnarounds, then the palette walk and a
Pedal voicing mode. The fork is personal for now, a basis for a conversation with Cadenza's author and perhaps a later
pull request ([§9](#9-licence-governance-and-branding)).

---

## 2. The three projects, in the terms that matter here

### 2.1 Cadenza

**Origins.** Cadenza grew as a Force plugin: it was called Chord Seq until 0.25.0 and kept that name's plugin ID, `ChSq`,
and its budget history ([vst/budget.json](../../vst/budget.json)) records Force measurements from 0.7 onwards. The iOS app
and the browser version ([cadenzajam.com](https://cadenzajam.com)) came later and share its engine; only the most recent
engine features, the song order of 0.27 and the Feel layer of 0.28, were built for iOS first and have no Force screen
yet ([MANUAL, What's new](../../MANUAL.md#whats-new-in-028)). The plugin is not a port of a tablet app. Its genuine
workarounds lie on the MPC side: MPC ignores a plugin's MIDI output, so Cadenza opens its own ALSA port
([engine.c](../../src/engine.c), header comment), and MPC OS 3.10.1's plugin support is official but experimental, with no
support from Akai for third-party plugins.

**Architecture.** `src/` is a host-free C engine: [seq_core.c](../../src/seq_core.c) (10 271 lines) with theory, gen,
rhythm, bassline, melody2, voicing, jamlib and feel. [engine.c](../../src/engine.c) binds it to the
[mpc-vst-plugins](https://github.com/open-mpc/mpc-vst-plugins) VST2 wrapper; [vst/make_skin.py](../../vst/make_skin.py)
renders the skin. The plugin takes no MIDI input ("v1 takes no MIDI input", [engine.c](../../src/engine.c)).

**Harmony model.** A part (song section) is a loop of 1, 2, 4, 8 or 16 bars ([seq_core.h:194](../../src/seq_core.h#L194))
of 16 steps a bar. Its chords are *slots* of `chord_len` (two bars down to a quarter bar), at most 64
([seq_core.h:23](../../src/seq_core.h#L23)), each holding:

- a degree 0-6 of the project scale;
- a source, `src`: 0 the project scale, 1-3 one of the scale's three borrowed chords (a fixed table per scale,
  [theory.c:122](../../src/theory.c#L122)), 4-51 a chromatic chord, 12 roots by four qualities, major, minor, dominant,
  diminished ([theory.h:50-55](../../src/theory.h#L50));
- a colour: Triad, 7th, sus2, sus4, add9, Power, 9th, 11th, 13th, 6th ([theory.h:39](../../src/theory.h#L39)), its tones
  taken from the chord's own scale.

On top of the slots: up to four change points a bar (a chord change inside a bar), and since 0.28 the Feel layer per slot
(a mute, the chord as first written, hand-placed notes, a voicing recipe).

**The property that makes integration cheap.** Steps are chord-relative: a step stores a tone above the root of the chord
under it (TONE_R to TONE_7) and an octave, and the bass and melody models read the chord under each step
([seq_core.h:81](../../src/seq_core.h#L81), [melody2.h](../../src/melody2.h)). Change the slots and the three lanes
follow, borrowed, chromatic and suspended chords included. Cadenza treats harmony as an *input* of a generative arranger;
the CHORDS page's "Progression" and "Voicing", which prompted this study, are the visible tip of a design in which every
lane is derived from the slot list.

**Progressions.** 16 presets of at most 8 diatonic degrees ([theory.c:12](../../src/theory.c#L12)), named by nickname
(Epic, Pop, Sad Pop, 50s ... Canon (8)), shown in a browser of four rows, POP, MOODY, CLASSIC, LOOPS. A seeded generator
exists ([gen.c:1142](../../src/gen.c#L1142)): it starts on the tonic, weights degrees by tiers of a HARMONY value, ends on
a chord that leads home as the loop wraps, and adds at most one borrowed chord above HARMONY 60. Its HARMONY knob and its
NEW PROGRESSION button appear on no 0.28 Force screen ([vst/layout.conf](../../vst/layout.conf) has neither).

**Voicing.** Four connection modes (Close, Smooth, Top line, Rootless), the COLOUR, INVERSION, SPREAD (Cluster to Wide),
STRUM and OCTAVE knobs, 52 named styles in 13 genres ([voicing.c](../../src/voicing.c)). How Smooth works is examined in
§6.6. The CHORDS PHRASE page's CADENCE knob shapes how a phrase's last chord sounds, its top note set to the 5th, the 3rd
or the root ([seq_core.c:2452](../../src/seq_core.c#L2452)), not which chord it is.

**Names.** Every pitch is spelled with a sharp ([theory.c:6](../../src/theory.c#L6)). Measured: F major's IV reads `A#`,
E♭ major is shown as `D#` with chords `D# Fm Gm G# A# Cm Ddim`, and a chromatic ♭II7 in C is named `C#7` while its
numeral reads `bII7`. Numerals count from the project's own scale: A minor reads `i ii° III iv v VI VII`, where the CPG
writes ♭III, ♭VI and ♭VII.

**Invariants.** Nothing is random: outputs derive from seeds and hashes, and checksums pin them
([tests/core_parity.sha256](../../tests/core_parity.sha256); the comment on `theory_voice_next` in
[theory.h](../../src/theory.h) forbids changing how saved projects sound). Enumerations are append-only because they are
saved as digits; new behaviour gets a new identifier. Key, scale, MIDI channels and the PLAY knobs are shared by all six
song parts. The project state is text, at most 8 KB per instance ([seq_core.h:34](../../src/seq_core.h#L34)).

**Budgets** ([vst/budget.json](../../vst/budget.json)), every raise approved by the owner after measurement: skin images
79.1 MB decoded against 80, and binary 686 KB (701 956 bytes) against 688, as the fork's CI measured them on the 0.28.1
sources (the budget file's own notes, 78.8 MB and 666 KB, date from earlier versions); audio p99 24.2 % of a
block in the latest bench ([vst/bench.txt](../../vst/bench.txt)) against 49; 2 037 parameters. Harmony changes run in
setters, off the audio thread: a richer harmony costs no audio time, but it costs binary, parameters and skin.

**The public repository is a release export.** Each release arrives as one commit titled "public release export"; the
development history stays private. Referenced but absent: `tools/make_presets.*` (which generates
[presets_data.h](../../src/presets_data.h)), `tools/textfit.py`, `tests/ref/*.js`, the web and iOS parity tests. The
offline engine is nevertheless reproducible: the core parity trace, built in this container with gcc 13 on x86-64,
matches the pinned hash (`934d3f5c...`), so engine work can be developed and tested here, the device being needed only to
check screens and sound. The plugin test needs the toolchain branch
[`chordseq/open-mpc`](https://github.com/Shrekage2024/mpc-vst-plugins/tree/chordseq/open-mpc).

### 2.2 The Chord Progression Generator

**Data.** 184 template entries in 22 genres; 152 distinct values, 19 of which appear under several genres with
priorities of their own. A strict grammar ([musicTheory.js](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/modules/musicTheory.js),
`parseChordToken`) reads accidental, stem case and suffix (`♭II7`, `V7/ii`, `i(maj7)`, `i♯5`) into a root in semitones
above the tonic and a chord type from a vocabulary of 50. 121 entries carry palette priorities (preferred, allowed and
avoided chord types), 6 a hard palette filter, 22 a row-4 hint (chromatic mediants, mode flips). Nicknames and
descriptions exist in six languages. Scale Mode offers 34 scales.

**Engine** ([generation.js](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/modules/generation.js),
`generateVariant`): template, then chords, then a 16-pad palette.

- Rows 1-3: the template's chords, then extensions per degree (7, M7, 9 depending on the style), the colours the genre
  declares, complements (ii and vi in major; iiø7 and v in minor), the other mode's colour chords (♭VII, ♭VI, ♭III, iv),
  sorted by priority then by "spice", from foundation to spicy. No two pads share a sound.
- Row 4: candidates scored by the harmonic functions the grid lacks, root motion by semitone or fifth, common usage,
  variety and the template's preferences, each tagged with a category (borrowed, dominant, subdominant, secondary,
  augmented sixth, chromatic, substitution, modal, mediant, mode flip). The pool: borrowed chords, V7, ii7, iv, VI,
  secondary dominants (V7/V, V7/ii, V7/vi, V7/IV; in minor V7/iv, V7/♭III, V7/♭VI, V7/♭VII), the leading-tone seventh,
  augmented sixths, the Neapolitan, the tritone substitute, the Lydian II, chromatic mediants and the hexatonic pole, mode
  flips.

Six voicing treatments: Smooth (a scored search rewarding common tones, stepwise and contrary motion, penalising leaps
and range), Classic (least total motion by Hungarian assignment over the inversions and a dropped top voice), Jazz
(close, sevenths added from pad 5), Modal (drop-2), Experimental (spread), Pedal (the first chord's bass held under every
chord that contains it,
[spec 11](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/improvements/11-pedal-voicing-style.md)).
Spelling follows key and degree: F♯m in A major, never G♭m
([spec 13](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/improvements/13-spelling.md)).
Scale Mode lays out 16 distinct chords of any scale; the Chord Matcher infers keys from chords.

**Apparatus.** `generation.js` and `musicTheory.js` are free of the DOM and run under Node. CI runs a theory audit and
diffs a committed [catalogue snapshot](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/catalogue-snapshot.txt).
A table for Cadenza can therefore be *generated by the CPG's own code* instead of being transcribed.

**Output.** `.progression` JSON for Pad Perform (`name`, `rootNote`, `scale`, `recordingOctave`, and 16 chords each with
`name`, `role` and MIDI `notes`; [export specification](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/mpc-export-specification.md)),
and Standard MIDI Files. No durations. The CPG mixes two concepts in one grid: its first pads hold the template's order,
the rest the palette. Its users accept that; harmony purists have objected, rightly, since a palette is unordered by
nature.

### 2.3 The Ouarpeggiator

A browser arpeggiator over whole progressions, built on a copy of the CPG's theory module: Euclidean rhythms (Bjorklund's
algorithm, after [Toussaint 2005](http://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf): hits, steps, rotation, and a
free-running polymeter mode), note orders (up, down, up-down, converge, diverge, random...), octave spread, swing,
humanisation, velocity and gate shaped by curves, and a stab mode whose chord changes follow their own Euclidean pattern.

Its harmonic idea is the **palette walk**: the chord order either follows the template or walks the palette, each next pad
chosen by a score (40 % common tones, 40 % voice-leading distance, 20 % root motion by a fifth), sharpened by an
"adherence" exponent and optionally steered toward the smallest or the largest motion
([sequencerCore.js](https://github.com/liotier/Ouarpeggiator/blob/main/js/sequencerCore.js),
`calculateHarmonicScore` and `selectNextChordHarmonically` in
[musicTheory.js](https://github.com/liotier/Ouarpeggiator/blob/main/js/modules/musicTheory.js)). It is, in effect, the
first orderer of a palette, and the reason §4 can treat palettes and orders separately.

Two observations:

- It draws on `Math.random`, so a pattern one liked cannot be recalled. Cadenza's founding rule is the opposite.
- Its copy of the theory module, annotated "shared verbatim with the AkaiMPC generator - do not fork", has drifted: 173
  templates and five styles against the CPG's 184 and six. A third copy, in C and maintained by hand, would drift the same
  way; [avenue B](#b-a-palette-table-generated-from-the-cpg) prevents it. The Ouarpeggiator's own drift is out of scope
  here.

What the Ouarpeggiator has and Cadenza lacks: Euclidean patterns, polymeter, curve-shaped dynamics, the palette walk. Only
the last is harmonic, and it is the piece worth carrying over.

### 2.4 A neighbour: Midicake's ARP

Midicake's ARP is a hardware "4-track parametric arpeggiator & sequencer" that "does not make any sounds"
([manual](https://manuals.midicake.com/arp/), [introduction](https://manuals.midicake.com/arp/introduction-to-arp/)). Its
resemblance to Cadenza is close, and reads as convergent evolution rather than lineage: nothing in Cadenza's repository
mentions it, and both answer the same problem (several MIDI parts kept in key by one changing harmony, played live and
reproducibly) with the same devices.

| | Midicake ARP | Cadenza |
|---|---|---|
| Lanes | 4 polyphonic arpeggiators, each on its own MIDI channel, all drawing "the same notes from a shared Chord or Scale" | 3 lanes (chords, bass, melody), each on its own channel, all following the chord under each step |
| Harmony over time | [chord chains](https://manuals.midicake.com/arp/chord-chain-sequencing/): up to 32 chords, each with a type, a root, an inversion and a length from a clock tick to 256 bars; 12 stored | the part's slots: up to 64, of two bars to a quarter bar, plus change points and Feel's unequal lengths |
| Rhythm | 16-step rhythm patterns (velocity, gate, ties, mutes) and groove patterns (timing offsets) | 16-step drawn rhythms (ACCENT, HIT, GHOST, HOLD, REST), SWING, HUMANIZE, PUSH |
| Bass from chords | a pitch filter keeping a chord's first notes, for "more stable basslines" | the BASS lane's MOVEMENT (roots only, then fifths...) and its styles |
| Live changes | a chain change lands by the MIDI clock; pressing a NOTE button, which sets a chord live, deactivates the chain | JAM blocks land at SWITCH AT, song parts at the bar line |
| Randomness | FX, "a reproducible randomizer" with a seed; an appendix titled "Why is there no such thing as Random?" | "Nothing is random": seeds and hashes, pinned by checksums |

The differences mark each design's centre. The ARP is an arpeggiator first: directions, bounce, ratchet, chop, binary
patterns, and two LFOs per arpeggiator that can modulate "almost all parameters", so that patterns evolve over hours
([behaviour parameters](https://manuals.midicake.com/arp/arp-behaviour-parameters/)). Cadenza is an arranger first: each
lane is a musical role with a model of its own (voicing and voice leading, bass styles and approach notes, melody phrase
forms with call and answer and cadence), genre blocks, song parts. The ARP's chords are shapes, eight sets of eight
defined on C ([chords and scales](https://manuals.midicake.com/arp/chords-and-scales/)), given a root by its NOTE buttons;
Cadenza's are degrees of a key, and with the CPG's palettes they become chords chosen for their function in the key.

Two of the ARP's devices are precedents for this study: live chord entry overriding the programmed chain, as avenue L's
pads would, and External Note Sync, by which an external keyboard sets the chord, as the MPC's pads might set Cadenza's
([device-test-pads.md](device-test-pads.md)). One is an idea Cadenza lacks, outside this study's scope: slow,
deterministic modulation of its own knobs.

---

## 3. Complementarity, measured

| | Cadenza 0.28.1, as the Force's screens expose it | CPG |
|---|---|---|
| Harmonic material | 16 progressions, at most 8 diatonic degrees | 184 palettes (152 distinct templates) in 22 genres, each with a starting order of 1 to 12 chords |
| Chords offered while editing | the scale's 7 and 3 borrowed; types sus2, sus4, add9, 6 | 16 pads per palette, weighted by genre, with a functional fourth row |
| Chromatic chords | in the engine since 0.28, no Force screen | secondary dominants, Neapolitan, augmented sixths, tritone substitute, mediants, mode flips |
| Scales | 7 | 34, of which 23 heptatonic |
| Voicing | 4 modes, 5 knobs, 52 named styles | 6 treatments |
| Spelling | sharps only | by key and degree |
| Time | rhythm, phrase, three lanes, song parts | none, by design |
| Reproducibility | seeded; checksums pin the output | pure functions; the Ouarpeggiator uses `Math.random` |

**Coverage.** Every chord Cadenza's theory layer can voice was enumerated, and every CPG template and palette was rendered
by the CPG's own modules, assigned the Cadenza scale holding most of its template chords diatonically, and each chord
classified (method in Appendix A).

Template chords, repeats included (745):

| Class | Chords | Share |
|---|---|---|
| diatonic in the template's best Cadenza scale | 639 | 85.8 % |
| one of that scale's three borrowed chords (`src` 1-3) | 49 | 6.6 % |
| chromatic chord of the Feel layer (`src` 4-51) | 54 | 7.2 % |
| missing sonority (minor ♯5 twice, minor-major seventh once) | 3 | 0.4 % |

| Templates (184) | Count |
|---|---|
| entirely diatonic in their best Cadenza scale | 109 |
| reachable through today's progression path (diatonic and borrowed only) | 144 |
| reachable once the clamp is lifted, without new sonorities | 182 |

Best Cadenza scale per template: Major 85, Minor 63, Mixolydian 13, Dorian 11, Lydian 5, Harmonic minor 5, Phrygian 2.

Palette pads, all six styles (184 × 6 × 16 = 17 664): 52.9 % diatonic, 12.7 % borrowed, 31.9 % chromatic, and 2.5 %
needing attention, of two kinds:

- **Missing sonorities** (genuine gaps): the quartal triad (186), the Italian sixth (124, which Cadenza could only play as
  a full ♭VI7, adding a note), the minor-major seventh (17), 7♭5 (12), the half-diminished seventh where the scale has
  none (12), the minor ♯5 (12), 7♭9 (6), the minor sixth where the scale has none (6). Eight sonorities in all.
- **Fifth omitted** (66 pads: 13th and minor 11th chords): Cadenza's 11th and 13th colours leave out the fifth, as jazz
  voicing usually does. The chord symbol is the same; which tones sound is a voicing policy, of the same kind as Cadenza's
  Rootless mode, which drops the root. Under the principle of §6.5 (import chord identity, not voicing) these are exact.

Two remarks. The template chords use only nine of the CPG's 50 types (major, minor, dom7, major7, minor7, diminished,
minMaj7, minor6, minAug): its harmonic wealth lies in its roots and its palettes, not in exotic chord types. And no
missing sonority is unreachable: each can be built from a seven-note scale or a new colour (avenue H).

**Lengths.** 145 of the 184 starting orders have 1, 2, 4 or 8 chords; 39 do not (3: 23, 5: 8, 6: 3, 7: 4, 12: 1). Since
the order is only a starting point (§4), this matters only for laying out the default order (§6.4).

---

## 4. Palettes, not progressions

### 4.1 The distinction

A **progression** is an ordered succession of chords; its order is its identity. A **palette** is a set of chords that
belong together in a key and an idiom; it has no order. Pad Perform embodies the second: sixteen pads, and the performer
orders them in real time. The CPG computes palettes and seeds each with a starting order, its template, which is why its
name says "progression". Cadenza, in its current form, knows only progressions.

### 4.2 What Cadenza actually requires

At playback every lane renders from an ordered, timed slot list: music in time is a succession, whatever it was made from.
That is a requirement on *playback*, not on *identity*. Nothing in the engine requires the order to be fixed, authored in
advance or taken from a template:

- every harmony setter revoices and regenerates the lanes that read harmony, off the audio thread
  ([seq_core.h](../../src/seq_core.h), comment above `seq_set_slot`);
- JAM already queues changes to land at quantised switch points (SWITCH AT: BEAT, BAR, 2 BARS, PHRASE), rendering them on
  a scratch copy of the part and swapping at the boundary;
- the browsers already run as transactions whose CANCEL restores the part as it was at opening.

### 4.3 The proposed model

- **The palette is a property of the part**: up to 16 chords, each with its spice level and its CPG category, plus the
  identity of the catalogue entry it came from. Saved *by value* (about 70 characters a part, some 420 bytes for six
  parts, 5 % of the 8 KB state), so a project keeps the palette it was made with even if a later CPG revision changes the
  catalogue, which is Cadenza's rule for everything else; the entry's identity is kept for display and for an explicit
  "reload from the catalogue".
- **The slot list is the current order**, one of many. What sounds is saved explicitly, as today.
- **Orderers write the order**, all through one primitive, "this palette chord at this time":

| Orderer | Who orders | Avenue |
|---|---|---|
| Starting order | the CPG's template, laid out by the default of §6.4 | C |
| Editing | the user, offline, in the bar editor with the palette as a source | D |
| Playing | the user, live, on 16 palette pads; each tap lands at SWITCH AT, the lanes follow, and the session ends with KEEP or CANCEL like a browser | L |
| Walking | a seeded palette walk | E |
| Turning around | phrase-end substitutions toward the next phrase's first chord | D |

### 4.4 Consequences

- **The catalogue browser loads a palette**, with its starting order as a default, not "a progression". Its tiles and title
  should say so.
- **The live page is Pad Perform's gesture inside Cadenza's arranger**: the player orders the palette, and bass and melody
  follow each chord. Ending the session with KEEP turns a jam into the part's progression, which also serves, within
  Cadenza, the owner's wish that jams become static building blocks.
- **The purists' objection is answered by construction**: the order is never the palette's identity, and the palette
  survives every reordering.

### 4.5 Is it a blocker ?

No. The engine needs an order at every instant and gets one from whichever orderer is active. The cost of the palette
model over a progression catalogue is a per-part palette (data and a save field) and the live page; everything else was
already planned.

---

## 5. Constraints any integration must respect

1. **Determinism and saved sound.** A saved project must sound the same after an upgrade. New behaviour takes a new
   identifier (a new voicing mode, a new colour, a new generator version); existing outputs are pinned by checksums. The
   CPG is pure, so what is generated from it is reproducible; the palette walk must be seeded, not random.
2. **One engine, three products.** `seq_core` serves the Force, the iOS app and the browser version. Upstream, any engine
   change crosses the author's commercial products and parity tests this export does not contain.
3. **The Force's budgets.** At most 1.2 MB of skin headroom: a new full-panel view has cost 1.6 MB or more by the budget's
   own history, so screens must reuse existing views. At most 2.5 KB of binary headroom (2 556 bytes). Each new tile is a parameter, and
   the bench's random sweep grows with them. Upstream the owner approves every raise; in a fork the numbers still
   describe a real device.
4. **The time grid.** Loops of 1, 2, 4, 8 or 16 bars ("128 divides by every loop length",
   [MANUAL §3](../../MANUAL.md#3-set-up-a-project)); 16 steps a bar; Feel chords resize by half or quarter bars; at most
   four change points a bar and 64 slots a loop.
5. **The save format.** `.ch` stores `src` 0-3 only ([seq_core.c:3886](../../src/seq_core.c#L3886)); chromatic chords live
   in the 0.28 `.cx` field, their `src` written as two hex digits ([seq_core.c:3935](../../src/seq_core.c#L3935)). The Feel
   "base" code, `1 + deg + 7 × (src + 52 × colour)` ([seq_core.c:530](../../src/seq_core.c#L530)), bakes
   `THEORY_SRC_MAX + 1 = 52` into saved projects: enlarging `CQ_COUNT` would renumber every saved chromatic chord and
   misread every saved base. New qualities need an appended second range and a versioned code.
6. **Global key and scale.** All six song parts share key and scale; a palette cannot bring its own mode into one part.
   The lines take their non-chord tones from the project scale (in [melody2.h](../../src/melody2.h) only the chord-tone
   slots read the chord; in [bassline.h](../../src/bassline.h) a passing note is a scale step, only the approach is
   chromatic), so a minor palette in a major project gets correct chords and clashing passing notes (E naturals over Cm
   in C major).
7. **No MIDI input, by design.** The toolchain's wrapper passes every MIDI event MPC sends to Cadenza's engine, whose
   handler discards it ([engine.c:362](../../src/engine.c#L362)): Cadenza wants harmony sequenced inside Cadenza, not
   swayed by live input. The device test ([device-test-pads.md](device-test-pads.md)) found that the pads and the notes of
   Cadenza's own clip could reach it; decision 20 keeps the design as it is.
8. **One source of truth for harmony: the CPG.** Cadenza consumes generated data, never a hand-kept translation (the
   Ouarpeggiator's drift, §2.3).
9. **The licence** ([§9](#9-licence-governance-and-branding)).

---

## 6. Mapping the CPG onto Cadenza

### 6.1 Chords

Each CPG chord is a root `r` (semitones above the tonic) and a type `T`. Against the project scale, in order of
preference:

1. **Diatonic:** a degree whose root is `r` and whose chord in some colour sounds exactly `T` → (degree, `src` 0, colour).
2. **Borrowed:** one of the scale's three borrowed chords → (degree, `src` 1-3, colour).
3. **Chromatic:** `src = 4 + 4r + quality`, colour → the Feel layer's representation.
4. **New sonority:** one of the eight additions of avenue H.

No approximation is used (decision 5 of §10).

| CPG type | Cadenza (quality, colour) | Note |
|---|---|---|
| major, minor | (major or minor, Triad) | |
| dom7, major7, minor7 | (dominant, 7th), (major, 7th), (minor, 7th) | |
| diminished, dim7 | (diminished, Triad), (diminished, 7th) | |
| sus2, sus4, add9, minAdd9 | colours sus2, sus4, add9 on the right quality | |
| major6 | (major, 6th) | |
| dom9, major9, minor9 | 9th on the right quality | |
| 11th, 13th | 11th, 13th | fifth omitted by voicing policy (§3) |
| Ger+6 | (dominant, 7th) on ♭VI | exact |
| m7♭5, minMaj7, minor6 | a fitting diatonic degree, else a new chromatic quality | avenue H |
| It+6, quartal, 7♭5, 7♭9, minor ♯5 | new colour or quality | avenue H |
| augmented, augMaj7 | Harmonic minor's III+ only | not produced by any palette; a quality if ever needed |

### 6.2 Lifting the borrowed-only clamp

The question: can the progression path accept any chord ? Yes, and 0.28 did most of the work.

- **What clamps.** Six checks in the editing paths accept only `src` 0-3 (a seventh, in the loader, is discussed
  below): [`fill_phrase`](../../src/seq_core.c#L339) and
  [`set_prog`](../../src/seq_core.c#L6268) (presets, the PROGRESSION arrows and browser, `seq_set_progression`), which
  silently turn anything else into the diatonic degree; [`seq_set_slot`](../../src/seq_core.c#L5802) (a pick on a slot),
  [`cp_set`](../../src/seq_core.c#L2019) (a change point), the bar editor's chord classification
  ([seq_core.c:7481](../../src/seq_core.c#L7481)) and [`seq_tweak_point_ok`](../../src/seq_core.c#L8671), which refuse
  it. They date from before 0.28, when no slot could hold anything else.
- **What already works.** The save path is generic: `out_feel` writes a `.cx` entry for *any* slot whose `src` exceeds 3,
  whichever path put it there ([seq_core.c:3935](../../src/seq_core.c#L3935)), and `load_cx` restores it after the slots
  ([seq_core.c:4734](../../src/seq_core.c#L4734)). The Force's 0.28 build already plays chromatic chords made on iOS
  "as it was made", names and numerals included (`theory_numeral` has a chromatic branch). Older builds read `.ch` and
  hear the diatonic degree there, the degradation 0.28 already accepts.
- **What may need a format change.** Change points save their `src` as one character, `'0' + src`
  ([`out_cp`](../../src/seq_core.c#L3900)), and [`load_cp`](../../src/seq_core.c#L4690) accepts only 0-3. The state
  parser splits fields on `|` and entries on `,`, so the existing chromatic range (up to 51, characters `:` to `c`) fits
  that encoding: chromatic change points, needed for palette chords picked in the middle of a bar, only need the checks
  widened, and older builds skip such entries. Avenue H's second range would reach `|` from `src` 76 and needs a wider
  encoding, in a new field that older builds ignore.
- **What must be checked.** Every reader that may assume a borrow (the COLOUR knob's ranking, drift, JAM, LOCK WITH, Try
  and keep, copy and chord-length remapping), undo snapshots, save and load round trips, and the core parity hash, which
  must not move: no existing path produces chromatic sources through these writers.

Size: S to M. It belongs in the first stage, since every later avenue writes palette chords into slots.

### 6.3 Which scale

Decision 6 of §10: filter. The palette browser shows by default the palettes whose tonality matches the project scale's
family (major tonality for Major, Lydian and Mixolydian; minor for Minor, Dorian, Phrygian and Harmonic minor), keeping
lines free of the clashes of §5.6. Cadenza aims at easy jamming; the musician who mixes a minor palette into a major song
on purpose is not the one this browser is for. An "all" view can follow later if asked for.

### 6.4 The default order in time

The CPG gives no durations and keeps none (decision 3): laying a starting order out in time is the arranger's job,
Cadenza's. For the 145 orders of 1, 2, 4 or 8 chords, one chord a bar, or the part's CHORD LENGTH, is the obvious default.
For the 38 others (the 12-bar blues aside), the tonic decides: 15 end on it (ii-V-I, ♭VI-♭VII-I, iv-♭VII-I,
I-IV-I-IV-I-V-I...), 21 start on it without ending on it (i-♭VI-♭VII, i-iv-v, I-V-vi-iii-IV...), 2 do neither. The
default, adopted (decision 4): **the spare time goes to the tonic**, held at the end when the order ends on it, at the
start when it only starts on it, and to the last chord otherwise. A three-chord ii-V-I thus becomes ii, V, I held two bars,
as jazz writes it; i-♭VI-♭VII becomes i held two bars, then ♭VI and ♭VII. Cadenza already writes ii-V-I so: its own
progression preset *2-5-1-1 Jazz* is `{1, 4, 0, 0}`. The rule hands out whole bars only, so every laid-out order is one
chord a bar in a loop of 1, 2, 4, 8 or 16 bars, as Cadenza's presets are (spec 04, S04-D9).

The 12-bar blues starts on I7 and ends on its V7 turnaround, so the rule gives its four spare bars to the opening tonic:
eight bars of I7, then the usual eight. That is one of the layouts of the
[sixteen-bar blues](https://en.wikipedia.org/wiki/Sixteen-bar_blues), an extended opening tonic section; another doubles
the closing V, IV and I. No exception is needed, and it is only a starting order anyway.

### 6.5 Voicings, names, numerals

- **Import chord identity, not CPG voicings.** Cadenza re-voices from chord tones with its own modes and knobs, which are
  more expressive than the CPG's treatments. (A `.progression` import could have kept exact voicings through Feel's hand
  notes; [avenue J](#j-progression-import-and-export) is dropped, decision 16.)
- **Names on tiles, not numerals.** The CPG reads numerals against the parallel major, Cadenza against the project scale
  (§2.1). The existing browser already shows names in the key ("Epic · Am F C G"); keeping names sidesteps the conflict.
  Wherever numerals appear, they follow Cadenza's convention, consistent with the rest of its screens.

### 6.6 Voice leading: what Cadenza already does, what the CPG would add

The question: do the CPG's chord proximity calculations improve Cadenza's transitions, or does Cadenza already do that ?

**What Cadenza does** ([theory.c](../../src/theory.c) `voice_search`, [seq_core.c](../../src/seq_core.c) `voice_slots`,
`spread_notes`):

- Smooth tries every inversion of the chord, each shifted by an octave either way, keeps those whose lowest note lies in
  MIDI 43-62, and chooses the one whose notes, matched lowest to lowest, move least in total. For two chords with the same
  number of notes this is optimal: on a line, for any notes a < b and c < d, |a - c| + |b - d| ≤ |a - d| + |b - c|, so any
  crossed matching can be uncrossed without increasing the motion, and the sorted matching is minimal (the crossing-free
  property underlying [Tymoczko's geometry of voice leading](https://doi.org/10.1126/science.1126287)).
- The chain is cyclic: the first chord is voiced, the loop chained, then the first chord voiced again against the last and
  the chain recomputed, so the loop's wrap is as smooth as its inside.
- INVERSION and SPREAD apply afterwards as positional transforms of the voiced chord (the second-lowest note up an octave,
  the lowest down an octave, a cluster note added), the same transform on every chord, so the smoothness survives them.
- Top line weights the top voice's motion a hundredfold; Rootless drops the root of chords of four notes or more.

**Where it is weaker:**

- Chords of different sizes, which palettes bring together (a triad beside a ninth chord): the extra notes are measured
  against the previous chord's top note, a heuristic. The CPG pads the smaller chord and solves an assignment (the
  Hungarian algorithm), also a convention, but a more even-handed one.
- No musical preferences beyond total motion. The CPG's Smooth rewards common tones held (-5 each), stepwise motion (-2),
  contrary motion between voices (-3 a pair), and penalises leaps beyond a major third and notes outside C3-C6.
- A positional SPREAD across a change of chord size can move a voice by more than the close voicing did.

**Verdict.** Transitions are already handled well; nothing is broken. The CPG's measures bring a refinement, best offered as
a new voicing mode (call it Smooth+: assignment for unequal sizes plus the CPG's preferences), measured against Smooth on
the whole catalogue before adoption, Smooth itself staying pinned. Their larger payoff is in *choosing* chords: ranking
the palette in the bar editor by how smoothly each candidate connects the chords before and after, scoring the palette
walk, ranking turnaround candidates. There, proximity decides what is heard, not merely how it is spaced.

---

## 7. Integration avenues

Sizes follow the CPG's improvement specifications: **S** under a day of a session's work with review, **M** one to two,
**L** a multi-step job with a checkpoint review.

### A. Key-aware chord spelling

- **What.** Port the CPG's spelling rules (spec 13): a root takes the letter of its degree, a two-named tonic takes the
  spelling with fewer accidentals, chord tones take letters from their degrees. ASCII as today (`Bb`; the Force's font
  has no ♭).
- **Value.** Fixes a visible defect on every screen in every flat key: F major's IV becomes `Bb`, E♭ major stops
  reading `D#`, the chromatic ♭II7 becomes `Db7` next to its `bII7`.
- **Engine.** Display only: names are cached labels, never saved. Touches [theory.c](../../src/theory.c)
  (`theory_chord_name`, `theory_chord_name_c`, `theory_chord_card`, the key names) and the keyboard's note labels. Width
  checks: `Dbm7b5` is as long as `C#m7b5`.
- **Fit.** Self-contained, useful to the iOS and browser versions as well: the best first contribution upstream.
- **Size.** S to M.

### B. A palette table generated from the CPG

- **What.** A Node tool, kept in this repository (decision 7), importing the CPG's modules through a git submodule pinned
  to a commit of `liotier/AkaiMPC` (decision 10), and emitting a C header: per catalogue entry a stable identifier, its
  genre, English nickname, tonality, best Cadenza scale, starting order with default durations (§6.4), and its 16 palette
  chords as Cadenza representations with spice level and category. A CI check regenerates the header and fails on a
  difference, as the CPG's catalogue snapshot does.
- **How the CPG's improvements arrive.** By bumping the submodule's commit and regenerating; the header's diff is the
  review, as the catalogue snapshot is for the CPG's own changes. Nothing is ported. The plugin build reads only the
  committed header, so building the plugin needs neither Node nor the submodule; only the generator and its CI check do.
  The AkaiMPC repository also holds the Finder and the MIDI programs: a shallow or sparse submodule keeps the checkout
  small.
- **Strictness.** The generator fails, naming the chord and the entry, whenever the CPG emits a chord Cadenza cannot
  represent exactly. A CPG change that needs Cadenza work then announces itself instead of degrading silently (decision 5),
  in the spirit of the CPG's own strict template parser
  ([spec 06](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/improvements/06-template-parser.md)).
- **Why generate rather than port.** Computing palettes on the device would cost little: setters run off the audio
  thread. The reasons are elsewhere: the CPG's JavaScript stays the reference, with no C port of its thousand lines to
  drift from it, and the binary carries data instead of logic. The trade-off: palettes exist for catalogue entries only,
  which the palette model of §4 accepts, since a part keeps the palette it loaded however its order is edited.
- **Budget.** 184 entries × 16 pads × about 3 bytes, plus starting orders and nicknames: roughly 12 to 15 KB of binary,
  well beyond the 2.5 KB of headroom 0.28.1 leaves: room must be made first (the
  [implementation strategy](implementation-strategy.md)'s risks).
- **Identifiers.** Array indices would break as the CPG's catalogue grows and is re-sorted; an identifier derived from
  the entry's genre and template value survives both.
- **Size.** S to M.

### C. The palette browser

- **What.** The existing PROGRESSIONS browser ([layout.conf:133-141](../../vst/layout.conf#L133)) has 16 tiles, DONE,
  CANCEL and a title readout; its row labels POP, MOODY, CLASSIC, LOOPS are static text. Replace them with a genre
  selector (a readout with arrows, or the VOICING browser's genre list, whose 15 tiles cannot hold 22 genres without
  paging), filtered by the project scale's family (§6.3), and page the palettes 16 at a time (Jazz/Functional has 22,
  Film/Cinematic 18). A pick loads the palette into the part and lays out its starting order. PROGRESSION ◀ ▶ on the CHORDS
  page steps through the genre. Tiles keep today's format, "nickname · chords in the key", cut at 19 characters
  (`BPR_CHARS`, [engine.c:596](../../src/engine.c#L596)): long orders show their nickname alone.
- **Engine.** A "load palette" verb writing the palette and the starting order through Feel's store (which keeps the loop's
  length and handles unequal durations), keeping slot locks as `seq_prog_try` does. Requires §6.2.
- **Budget.** The 16 tile parameters exist; a few for the genre selector. If the view's image is unchanged, no new skin
  image: to be confirmed with the skin gauge, since `tools/textfit.py` is not in the export.
- **Value.** The highest: the CPG's material on the Force, every lane following.
- **Size.** M to L.

### D. The palette in the bar editor, and turnarounds

**The palette as a source.** The CHORDS bar editor offers the scale's seven chords and three borrowed ones. A source toggle
on the BORROW row (SCALE · BORROW · PALETTE) adds the part's palette, coloured by spice, which Cadenza's drift legend
(green close to red dissonant) already speaks; candidates can be ranked by how smoothly they connect the chords before and
after (§6.6).

**Finding the turnaround chords.** A term first: a *pivot* chord belongs to two keys and carries a modulation; what a
turnaround needs is an *approach* chord, one that resolves into the next phrase's first chord. With a global key there is
no modulation between Cadenza's parts, so approach chords are what matter. They are found by **target-relative function**:
each palette chord is related to the target, the first chord of the next phrase (the loop's own first chord at a loop
end, or the next part's), by its root interval and its quality:

| Relation to the target | Root | Sound | Example into C |
|---|---|---|---|
| dominant | a fifth above | major triad or dominant seventh | G7 |
| tritone substitute | a semitone above | dominant seventh | D♭7 |
| leading-tone | a semitone below | diminished | B° |
| backdoor | a whole tone below | dominant seventh | B♭7 |
| plagal | a fourth above | major or minor | F, Fm |
| modal | a major third or a whole tone below | major | A♭, B♭ |

Measured over the 184 palettes (Classic variant), relative to each loop's first chord: **151 palettes contain a
dominant-type approach** (dominant, tritone substitute, leading-tone or backdoor), the **33 others only plagal or modal
ones**, and **none is without**. The 33 cluster where they should: 9 in Film/Cinematic, whose templates avoid dominants by
design, the rest scattered (Jazz/Functional 4, Funk/Groove 4, R&B/Neo-Soul 3, Pop/Rock, Gospel/Worship, Hip-Hop/Trap,
Folk/Singer-Songwriter and Italo-Disco/House 2 each, three genres 1 each). Availability by relation: ♭VI 168, ♭VII 152,
IV 146, iv 122, V 81, V7 65, ♭VII7 63, vii° 54, tritone substitute 8 (the Jazz variant adds more). Because the palette
already embodies the genre's priorities, a folk palette offers no dominant seventh to its turnarounds: the idiom is
inherited, not re-specified.

**The turnaround verb.** At bar 4, or bars 2 and 4, of each phrase, the last chord (or the last half bar, for two-chord
turnarounds such as ii-V of the target or ♭VI-♭VII) is replaced by the best approach chord of a chosen family: plagal,
dominant, or chromatic (tritone substitute, backdoor). Ranking: the relation's strength, then the voice-leading distance
from the preceding chord, then a seed. Deterministic, recallable, and complementary to CADENCE, which sets the last
chord's top note: TURNAROUND would choose the chord, CADENCE its soprano.

**Palettes for progressions built from scratch.** The generated table (B) holds palettes for the 184 catalogue entries,
computed offline by the CPG's own code. A progression the user builds from scratch, chord by chord in the bar editor, has
no catalogue entry, hence no palette, unless something computes one on the device. Only C runs there, hence the question
of a C port.

- **What a port would give the user.** A palette whose spicy fourth row is analysed from the user's own chords: the
  secondary dominants of *those* chords, their substitutions, the borrowed chords around *their* key centre.
- **What it would cost.** A second implementation of the CPG's palette logic, roughly a thousand lines, to keep in step
  with every CPG improvement: precisely what decision 10 set out to avoid.
- **What gives most of the value without a port**, all generated offline from the CPG through the submodule:
  1. *The part keeps its palette.* A palette loaded from the catalogue stays with the part through every edit of its
     order; editing never takes it away.
  2. *A palette for every scale.* A part that never loaded one gets its scale's palette: the CPG's Scale Mode lays out 16
     distinct chords of a scale (its triads, then its sevenths), so seven precomputed palettes cover Cadenza's seven
     scales.
  3. *The nearest catalogue palette.* For a progression built from scratch, the catalogue palette that contains the most
     of its chords, found on the device by comparing roots and qualities (184 palettes of 16 chords: a negligible search),
     the table storing chords independently of the project's scale. Every chord stays exact; only the suggestions around
     them come from the nearest template instead of from an analysis of the user's own.

Decided (decision 18): no port; the three measures above.
- **Size.** M.

### E. The palette walk, deterministic

- **What.** The Ouarpeggiator's selection rule (common tones, voice-leading distance, fifth motion, adherence) as a new
  seeded generator version beside `gen_progression_v2`, constrained like it: start on the tonic, end on a chord that
  leads home as the loop wraps, keep locked slots. It walks the part's palette.
- **Value.** Endless orders in a palette's idiom, each recallable, the same seed giving the same walk: the Ouarpeggiator's
  ambition inside a sequencer that knows what to do with the result.
- **Save.** Nothing new: orders are saved as slots, and the generator's golden is "not save-critical" by its own header
  ([gen.h](../../src/gen.h)).
- **Screen.** A NEW tile in the palette browser or on the live page, and the adherence value somewhere visible; the eight
  CHORDS knobs are taken.
- **Size.** M.

### F. A Pedal voicing mode, and Smooth+

- **What.** The CPG's Pedal treatment as a fifth voicing mode: the part's first bass note held at the bottom of every chord
  that contains it, the other chords voiced above it. Distinct from the BASS page's PEDAL, which acts on the bass lane. New
  voicing styles can use it (the CPG's example: Em, C, A over a held E). Smooth+ (§6.6) as a sixth mode, under the same
  rule: append, never alter Smooth.
- **Size.** S to M each.

### G. A genre crosswalk, and presets

*Dropped by decision 24: the user chooses voicing and bass styles freely, coherent or not.*

- **What.** Map the CPG's 22 genres onto Cadenza's genre vocabularies: 6 JAM columns, 13 voicing genres, 12 bass genres.
  Italo-Disco/House → HOUSE and DISCO, voicing House, bass House; Trance/Psytrance/Goa → TRANCE; Jungle/Drum'n'Bass →
  voicing and bass Drum & Bass, no JAM column; Film/Cinematic → Film, Film / Ambient. Loading a palette could then offer,
  without imposing, a matching voicing style and bass style.
- **Presets.** Cadenza's preset index is capped at 16 ([seq_core.h:588](../../src/seq_core.h#L588)), each preset fills all
  six song parts, and the tool that bakes them is absent from the export: presets are a vehicle for curation, not for a
  catalogue.
- **Size.** M.

### H. Completing the chord vocabulary

- **What.** The eight missing sonorities of §3, so that every palette chord is exact:

| Sonority | Pads | Construction |
|---|---|---|
| quartal triad (1, 4, ♭7) | 186 | a new colour on the dominant quality |
| Italian sixth (♭6, 1, ♯4: a dominant seventh without fifth) | 124 | a new colour, "7 without 5", on the dominant quality |
| minor-major seventh | 17 | a chromatic quality on the melodic minor |
| 7♭5 | 12 | a chromatic quality on a scale whose fifth degree is ♭5 |
| half-diminished seventh | 12 | a chromatic quality on the Locrian |
| minor ♯5 (the line cliché) | 12 | a chromatic quality on a scale whose fifth degree is ♯5 |
| 7♭9 | 6 | a chromatic quality on the Phrygian dominant, whose 9th colour gives ♭9 |
| minor sixth | 6 | the melodic-minor quality's 6th colour |

- **How.** New chromatic qualities as a second range appended after `src` 51, each a seven-note scale like the existing
  chromatic rows, so every existing call reads them as any other chord; new colours appended to the colour list. The Feel
  base code (§5.5) needs a versioned form for the new range. Names and numerals learn the new qualities.
- **Saving new colours.** They cannot be written into `.ch` as it stands: an older build ignores the whole field for a
  single character outside its alphabet ([`load_slots`](../../src/seq_core.c#L4332)) and falls back to the legacy phrase,
  losing every slot. New colours take an extra field, `.ch` keeping a fallback colour per slot, as 0.28 did for chromatic
  sources with `old_src`.
- **Fit.** Shared engine and save format: upstream-sensitive, to be discussed with the author before anything else in the
  engine.
- **Size.** M.

**Do new colours burden the user ?** No, if they stay out of the controls. A colour reaches the screen in two places only:
the COLOUR knob's steps (POWER, TRIAD, 7TH, 9TH, 11TH, 13TH) and the bar editor's TYPE (as knobs, sus2, sus4, add9, 6).
Colours written by the palette loader need neither: the user meets them as palette chords with names. The same holds for
every sonority in the table above. The cost falls on development: each colour touches the colour tables, the names, the
drift scores, the save fields and the tests.

**The fifth in 11ths and 13ths: decided, to revisit.** Decision 12 keeps Cadenza's fifthless 11th and 13th colours and
adds none, for simplicity. The trade-off, recorded for a future review:

*For keeping them:*

- The chord's identity is unchanged: same symbol, same function, same characteristic tones.
- The fifth is the tone jazz voicing leaves out first; Cadenza's colours were designed so, within a six-note limit
  (`THEORY_CHORD_MAX`) that SPREAD's Cluster already fills with one added note.
- Two audibly different "13th" chords in one product, one from the knob and one from a palette, would be an inconsistency
  nobody asked for.
- A 13th with its fifth has six notes, and Cluster's addition would need a seventh place: that limit sizes arrays across the
  engine and the strum queue, so the change is wide for 66 pads of 17 664 (0.4 %).

*What is given up:*

- Note-for-note agreement with the CPG for those 66 pads: their sounding pitch classes differ from the CPG's by the fifth,
  so a test comparing Cadenza's palettes with the CPG's must allow that one difference.
- A fuller sound where the voicing has room for the fifth, in wide spreads and pads.

*Revisit when* a listener misses the fifth, a voicing style wants it, `THEORY_CHORD_MAX` is raised for another reason (the
main cost then disappears), or the CPG's own 11ths and 13ths change. *Ways forward:* first a voicing style that sounds the
fifth where SPREAD leaves room, which is how arrangers decide it; only then new colours, saved as described above.

### I. More heptatonic scales

- **What.** Of the CPG's 34 scales, 23 have seven notes and fit Cadenza's seven-degree model; 12 of them are new to Cadenza
  once duplicates are removed (Kafi is Dorian, Yaman Lydian, Bhairavi Phrygian, Bhairav Double harmonic): Locrian, Melodic
  minor, Double harmonic, Hungarian minor, Neapolitan major and minor, Phrygian dominant, Persian, Todi, Lydian dominant,
  Altered, Locrian ♯2. Each needs its three borrowed chords chosen. The eleven others (the major and minor pentatonics,
  blues, whole tone, the two octatonics, the augmented scale, Hirajoshi, Insen, Kumoi and the Egyptian pentatonic) break
  the seven-degree assumption that runs through the engine: out of scope.
- **Caveat inherited.** The CPG presents its ragas and exotic scales as twelve-tone approximations, to be treated "as
  flavour, not as authority"; Cadenza should say the same.
- **Screen.** GLOBAL's SCALE selector grows from 7 entries.
- **Size.** M.

### J. `.progression` import and export

**Dropped** by decision 16: the integration goes the Cadenza way, through the CPG's code and a generated table, not through Pad
Perform's files. Kept for the record:

- **What it was.** Reading Pad Perform files into a part's palette (each pad's chord identified from its notes, as the
  [Finder](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionFinder) does, exact voicings kept as Feel
  hand notes), and writing a part's palette back as a `.progression`.
- **File access.** Taken as available (decision 13). Presets do not show it: Cadenza's are compiled into its binary
  ([presets_data.h](../../src/presets_data.h)), and preset files are read by MPC, which hands the plugin a state chunk.
  What shows it is that a plugin is native code running inside MPC's process: Cadenza already loads the ALSA library
  from the system with `dlopen` ([engine.c](../../src/engine.c), header comment); the toolchain ships `plugin_dir.h` "so
  an engine can find presets, banks, ROMs and other files shipped next to it"
  ([plugin_dir.h](https://github.com/Shrekage2024/mpc-vst-plugins/blob/8fbeffc/wrapper/plugin_dir.h)); and other plugins
  do ship data files in their folders (sd88me's Dexed port, its DX7 cartridges). The fact still serves the parked
  question of parts and clips, should Cadenza write MIDI files.
- **Where the files are,** from a filesystem listing of an MPC One under MPC OS 3.10.1: the factory Pad Perform
  progressions, 82 files (Pop 28, Dance 20, Jazz 13, Gospel 12, Ballad 9), in `/usr/share/Akai/SME0/Progressions` on the
  system partition, which is mounted read-only; the user's folder, `/storage/MPC Documents/Progressions`, on the internal
  drive; external drives under `/media/<volume>`, where Akai documents a `Progressions` folder at the drive's root
  ([Akai support](https://support.akaipro.com/en/support/solutions/articles/69000824380-mpc-x-live-and-one-using-your-progressions-in-standalone)).
  By the CPG's own export notes, MPC reads progressions at startup.

### K. Words

- **What.** CPG nicknames on tiles ("Axis of Awesome", "Andalusian", "Hotel California"); the CPG's plain-language chord
  phrase ("Dominant seventh chord on the flattened second degree") on the message line when a chord is picked. The
  descriptions are too long for a line that fades after four seconds. English only: Cadenza has no localisation, and the
  CPG's exports are canonical English anyway (the CPG's export specification, D2).
- **Size.** S, once B exists.

### L. Live palette pads

- **What.** A PALETTE page of 16 pads, laid out like the MPC's (the browser's 4 × 4 tile grid has the geometry). A tap
  queues the pad's chord to land at the part's SWITCH AT point; bass and melody follow it. The session runs as a
  transaction: KEEP keeps what was played as the part's order, CANCEL restores the part as it was. Turning parts into MPC
  clips is a separate question, parked at the end of this section.
- **Cadenza's current behaviour.** Cadenza has no chord pads, so nothing defines their behaviour yet; but it has two
  precedents.
  - *Changing chords while playing* happens today in the PROGRESSIONS browser: a tile tap replaces the part's unlocked
    chords at once and auditions the first one it changed ([`seq_prog_try`](../../src/seq_core.c#L7039)); the new chords
    play from the next hit, mid-bar if need be, with no quantisation; they stay until the next tap; DONE keeps the last
    one tried, CANCEL puts back what was there when the browser opened. PROGRESSION ◀ ▶ does the same outside the
    browser, as one UNDO step ([`seq_prog_step`](../../src/seq_core.c#L7052)).
  - *Performing* happens on the JAM pages, and every live control there behaves alike: a tap changes the state at a
    quantised point counted from the song position, and the change stays until the next tap. A JAM block "takes over
    every bar that isn't locked, at the next switch point"; a song part switches "at the next bar line"; a knob turn lands
    "by the next beat"; tapping the queued item again cancels it; UNDO steps back
    ([MANUAL §11](../../MANUAL.md#11-jam-pages), [§10](../../MANUAL.md#10-song-parts-and-mutes),
    [§20](../../MANUAL.md#20-timing-how-cadenza-follows-mpc)).

  Nothing in Cadenza is momentary, and nothing records a performance onto the timeline. The PROGRESSIONS browser already
  is "hold until the next tap, then DONE or CANCEL", for whole progressions.
- **Decided (decision 17): the browser's behaviour, applied to single chords, with two changes.** A tap's chord holds until the
  next tap and becomes the harmony of the whole loop, as on a Pad Perform pad; the session ends with KEEP or CANCEL, as the
  browser's does. A looper's overdub would be foreign to Cadenza. The two changes:
  1. *Timing.* A tap lands at the part's SWITCH AT point, a second tap on the queued pad cancelling it, as on JAM, instead
     of at once: auditioning a progression tolerates an entrance mid-bar, performing a chord does not.
  2. *What KEEP keeps.* The browser's DONE keeps the last tap, which for pads would be one chord for the whole loop. KEEP
     instead keeps the performance, by retrospective capture: the engine remembers each tap and where it landed, and KEEP
     writes the last loop's worth into the part's slots, the chords that sounded at the places they sounded, much as
     [Ableton's Capture MIDI](https://help.ableton.com/hc/en-us/articles/360000776450-Capture-MIDI) retrieves what was
     just played. Capture changes nothing in how playing feels.

  Midicake's ARP, a hardware sequencer, behaves alike on the same point: a chord picked live takes over from its programmed
  chord chain (§2.4).
- **Engine.** Write-through on a scratch part with a swap at the boundary, as JAM's queue already does, rather than a
  harmony overlay that every lane would have to read: the lanes render whole loops in advance. One difference: a JAM block
  swaps one lane, a chord change all three (the Chords' voices and labels, the Bass, the Melody), which must swap together
  at the same boundary.
- **On screen only (decision 20).** Hardware pads were considered, by analogy with Pad Perform, and the device test
  showed them feasible; but driving Cadenza's chords from MIDI input would give the part a second source of harmony beside
  its own slots, against Cadenza's design of sequencing harmony inside Cadenza.
- **Size.** M to L.

### Ranking

| Avenue | Value | Cost | Risk | Upstream fit | Stage |
|---|---|---|---|---|---|
| A. Spelling | medium, visible everywhere | S-M | low | excellent | 1 |
| B. Generated palette table | enabling | S-M | low | good | 1 |
| §6.2 Clamp lifted | enabling | S-M | low | good | 1 |
| H. Missing sonorities | enabling, for exactness | M | medium (save format) | to discuss | 1 |
| C. Palette browser | very high | M-L | medium (screen) | to discuss | 2 |
| D. Palette in the bar editor | high | M | medium (screen) | to discuss | 2 |
| L. Live palette pads | very high | M-L | medium (screen, semantics) | to discuss | 3 |
| D. Turnarounds | high | M | low | good | 3 |
| E. Palette walk | high | M | low | good | 4 |
| F. Pedal voicing, Smooth+ | medium | S-M | low | good | 4 |
| G. Genre crosswalk | medium | M | low | good | 4 |
| K. Words | medium | S | low | good | 1-2 |
| I. Heptatonic scales | low to medium | M | medium | sensitive | 5 |
| J. `.progression` I/O | dropped (decision 16) | | | | |

### Not recommended now

- **The Euclidean engine and polymeter.** Rhythm features, not harmony; the second breaks the sixteen-steps-a-bar
  invariant. Cadenza's rhythm library already holds the most common Euclidean idiom (its Tresillo is E(3,8)) and its
  Latin relatives (Son Clave, Bossa). A future JAM column perhaps, competing for skin budget.
- **The Chord Matcher on the device.** Little use on a 7-inch screen where the key is one tap away on GLOBAL.
- **The CPG's six voicing treatments wholesale.** Cadenza's voicing is already more expressive; Pedal and the Smooth+
  refinement are what remains.
- **Anything flowing from Cadenza into the CPG** ([§9](#9-licence-governance-and-branding)).

### Parked: from parts to clips

Cadenza lives beside MPC's sequencer rather than inside it: its parts are not clips, and the Clip Matrix does not see them.
The owner would like jams to become static building blocks of a project; the study of that is deferred. Avenue L's KEEP
is a different matter: it keeps a palette performance as the part's order, inside Cadenza.

The owner's routes, noted 2026-11-22, as the starting point when the topic resumes:

| Route | For | Against |
|---|---|---|
| Record MIDI: to MPC, Cadenza is a MIDI controller, so record-arm the three receiving tracks and record | works today, nothing to build | one real-time pass per part, each into its own clip: six passes for six parts |
| Record the whole song into the arrangement, then copy the parts into clips | one pass | manual slicing. The engine's 0.27 song order (rows of parts by passes, no Force screen yet) would play every part in a known order and length, which makes the slicing mechanical |
| Cadenza writes MIDI files | no real-time pass at all | importing the files on the MPC, one by one |

On the third route: the engine is deterministic and host-free, and its parity harness ([tests/core_parity.c](../../tests/core_parity.c))
already drives it offline with a synthetic song position and prints every note it would play. Rendering each part over its
loop into a three-track Standard MIDI File, one track per lane, is a small step from there; loops of one or two bars need
four bars rendered, since their fills play every fourth bar. File access is taken as available (decision 13); on an MPC
One, the user's folders are under `/storage/MPC Documents` on the internal drive (`Sequences` among them), and external
drives under `/media/<volume>` (avenue J's notes).

All three routes run in one direction, from Cadenza to MPC's clips: the parts are made in Cadenza, then frozen as clips.
The reverse direction, MPC clips carrying chord changes into Cadenza through MIDI input, was raised by the device test and
is set aside by decision 20.

---

## 8. Recommended staging

| Stage | Content | Screen work | Exit criterion |
|---|---|---|---|
| 1 | A spelling; B palette table (with K's nicknames); §6.2 clamp lifted; H missing sonorities; the per-part palette and its save field | none | theory tests extended; save and load round trips; core parity unchanged; table check in CI |
| 2 | C palette browser; D palette in the bar editor | reuse of existing views | device test on the MPC One |
| 3 | L live palette pads; D turnarounds | a pad view, a knob step | device test |
| 4 | E palette walk; F Pedal and Smooth+; G crosswalk | a tile, two modes | device test, Smooth+ measured against Smooth |
| 5 | I if wanted | as needed | per specification |

The [implementation strategy](implementation-strategy.md) details these stages as numbered steps, with their workflow,
tests and milestones. Stage 1 is engine work that can be built and tested in this repository without the device (§2.1). Each stage first
becomes a specification in this directory, in the format of the CPG's
[docs/improvements](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements):
provenance, decisions, measurements, verification.

---

## 9. Licence, governance and branding

**What the licence grants.** [PolyForm Shield 1.0.0](https://polyformproject.org/licenses/shield/1.0.0) grants a copyright
licence, a distribution licence and a licence "to make changes and new works based on the software for any permitted
purpose", plus a patent licence. Every grant carries the same qualifier, and the Noncompete clause defines it: "Any
purpose is a permitted purpose, except for providing any product that competes with the software or any product the
licensor or any of its affiliates provides using the software", competition being possible "even when provided free of
charge". [NOTICE](../../NOTICE) adds "Copyright (c) 2026 Shrey Sharma" and two lines of business, the Cadenza iOS app and
the Cadenza browser version, which stay protected even if discontinued (the "Discontinued Products" clause).

**What that means here** (a reading, not legal advice). A personal fork, developed in the open as the basis of a
conversation with the author and perhaps of a pull request, is a permitted purpose: it provides no product. The line would
be crossed by offering the fork to others as an alternative to Cadenza. Any copy passed on must carry the licence and the
"Required Notice" line, as this repository does.

**Branding.** The licence grants no trademark rights: "These terms do not imply any other licenses" (No Other Rights).
The name "Cadenza" and its marks stay the author's; a repository name that describes the fork, as `cadenza-CPG` does, is
ordinary practice for a personal fork, but a build shared with anyone should not present itself as Cadenza. There is also
a practical side: the fork builds with Cadenza's own identity ([vst/vst.json](../../vst/vst.json): name "Cadenza", ID
`ChSq`, plugin folder `Cadenza - VST - Cadenza`, MIDI port "Cadenza MIDI Out"). Installed beside the official plugin on
the same MPC, the two would collide, and projects saved with one would open the other. Decision 11: test builds keep
Cadenza's identity for now, since the fork aims at a pull request. A test build therefore replaces the official plugin on
the MPC rather than sitting beside it; the official release's folder is kept to restore it. Projects travel both ways: one
saved by the fork opens in the official build with fork-only features degraded, never refused, since Cadenza skips
unknown fields and bad entries, provided new features are saved as avenue H prescribes. An identity of its own (name,
folder, port, ID) can come later for side-by-side tests, at the price of projects that only the fork opens.

**The CPG and the Ouarpeggiator.** The Unlicense: nothing prevents porting their data and algorithms into Cadenza;
contributing them upstream would donate public-domain work to a source-available product, the CPG itself staying public
domain. The reverse direction, Cadenza's code into the CPG or the Ouarpeggiator, is what the Noncompete clause targets.

**Upstream practicalities.** The public repository receives one commit per release, the history staying private; budgets
are raised only by the owner after measurement; the engine serves three products, two of them commercial; the iOS Feel
page already picks chords on any root, so the author may prefer to bring palettes to iOS first. A pull request would
probably be re-implemented rather than merged; proposals carry better than patches: a design note, a measured table, a
small fix. Decisions 29 and 30 set how the fork works with upstream, as a feature branch offering its work:
[upstream-relations.md](upstream-relations.md). Stage 1's spelling fix and generated table are the natural openers, with a field report: the README lists the
MPC One and MPC OS 3.10.1 as untested, and this fork's owner runs that combination daily, which an entry in
[tested.json](../../tested.json) would record.

---

## 10. Decision log

Decisions of the fork's owner, 2026-11-22: 1 to 9 in answer to revision 1, 10 and 11 to revision 2, 12 to 15 to
revision 3, 16 to revision 4, 17 to 19 to revision 5; 2026-11-23: 20, after the device test's runs, then 21 to 24; 2026-11-24:
25 and 26, after step 4; 2026-11-25: 27 and 28, after the device run; 2026-11-26: 29 to 31.

| # | Decision | Consequence in this revision |
|---|---|---|
| 1 | The fork is personal for now: a basis for a conversation with Shrey Sharma, perhaps a pull request later. Work happens in the open, in `docs/development`. | §9 |
| 2 | The CPG is a palette generator; its templates are starting points, and a palette's chords may be played in any order. | §4; avenues C, D, E, J and L reframed or added |
| 3 | The CPG gains no notion of time: durations are the arranger's job. | §6.4: the default order is laid out by Cadenza |
| 4 | Odd-length starting orders give their spare time to the tonic. | §6.4 |
| 5 | No approximation. | avenue H moves to stage 1; §6.1 |
| 6 | The palette browser filters by the project scale's family. | §6.3 |
| 7 | The generator tool lives in this fork, not in the CPG. | avenue B |
| 8 | Yes to the palette browser by genre, the palette in the bar editor with turnarounds, the palette walk and the Pedal voicing. | §7, §8 |
| 9 | From parts to clips: wanted, but not studied now. | parked, end of §7 |
| 10 | The CPG is pinned as a git submodule, so that its improvements reach Cadenza without being ported. | avenue B |
| 11 | Test builds keep Cadenza's identity for now, the fork aiming at a pull request; an identity of their own may come later, for side-by-side tests. | §9 |
| 12 | 11ths and 13ths keep Cadenza's fifthless colours, to keep it simple; the trade-off is recorded for review. | avenue H |
| 13 | A plugin can access files: avenue J needs no probe for that, only the folder's path. | avenue J |
| 14 | No knob-placement constraint: Q-Link banks are how every plugin spans the MPCs, from the XL's 16 Q-Links down to the One's 4. | §11 |
| 15 | The probe of whether the MPC One's physical pads reach the plugin is deferred. | §5.7, §11 |
| 16 | No `.progression` import or export: the integration goes the Cadenza way. | avenue J dropped |
| 17 | Live pads take the PROGRESSIONS browser's behaviour for single chords: hold until the next tap, landing at SWITCH AT, the performance kept on KEEP by retrospective capture, CANCEL restoring. | avenue L |
| 18 | No C port of the palette builder: the part keeps its palette, a part without one gets its scale's palette, a progression built from scratch can take the nearest catalogue palette. | avenue D |
| 19 | The device test of the physical pads goes ahead, by the protocol of [device-test-pads.md](device-test-pads.md). | §11 |
| 20 | No MIDI input: Cadenza wants harmony sequenced in Cadenza. Hardware pads and harmony carried by MPC clips are set aside; avenue L's pads stay on screen; the device test closes, its findings kept. | avenue L, §5.7, [device-test-pads.md](device-test-pads.md) |
| 21 | Room in the binary, both ways: the ARM build drops the functions the plugin never calls (section garbage collection, about 46 KB), and the fork's budget goes from 688 to 720 KB. The budget is upstream's own discipline, with no device limit cited for it. | §5, [spec 01](specs/01-key-aware-spelling.md) |
| 22 | A project keeps the palette it was made with, saved by value: the principle of least surprise, respect for the composer, and a cleaner boundary between the CPG and Cadenza (a CPG revision never reaches into a saved song). | [spec 05](specs/05-part-palette.md), S05-D2 |
| 23 | Cadenza's genres and the CPG's are orthogonal: Cadenza's (its genre presets, its JAM columns) are about time, rhythm and arrangement; the CPG's are palettes, harmony. The genre presets get no palettes, and neither kind of genre implies the other. | [spec 05](specs/05-part-palette.md), S05-D9; avenue G |
| 24 | No genre crosswalk: loading a palette suggests no voicing or bass style. The user is not constrained: they may choose coherent options, or be free to get weird. Avenue G and the strategy's step 12 are dropped. | avenue G; strategy, O14 |
| 25 | The CPG's palettes must not depend on the key, and the defect that makes them do so (row 4 scored against the voicing's bass notes, in the wrong frame) is fixed in the CPG, by a specification written for its repository; the fork then moves its submodule. | [upstream/cpg-18-row4-key-invariance.md](upstream/cpg-18-row4-key-invariance.md), implemented in the CPG as its spec 18; [spec 04](specs/04-palette-table.md), §8.4; strategy, O15 |
| 26 | Chords are not categorised, only palettes (by genre), and Cadenza's rhythms, independently (decision 23). A palette chord carries no category. | [spec 04](specs/04-palette-table.md), S04-D4; [spec 05](specs/05-part-palette.md), §3.3; strategy, O16 |
| 27 | The author's silence slows nothing: the owner's message of 2026-11-23 is unanswered, and no step waits for an answer. The patches offered (O3) wait for one; the design note is written anyway. | strategy, §8.3 (O3) |
| 28 | The web simulation is not pursued for now, its potential kept: with root SSH on the MPC One, testing on the device is fast, and the device is where this project's difficulties have been. | strategy, §4.7 and §8.3 (O8b) |
| 29 | Relations with upstream pursue two goals: offering the work to the author, and keeping the fork able to go on whatever the answer, none having come to the owner's issue or message. Upstream's public record shows releases, not work (one export per release; the gap before 0.28.1 was as long as the one since). | [upstream-relations.md](upstream-relations.md); strategy, O17 to O19 |
| 30 | The form: "for now we endeavour a gentle feature branch". The documents speak of offers and of keeping the builds sound, not of leaving. The toolchain comes from its origin, open-mpc's repository, forked into the owner's account with the author's six wrapper commits on one branch, "to align on the actual origin and benefit from its improvements", rather than from the author's branch. These documents use neutral pronouns. | [upstream-relations.md](upstream-relations.md), §4; strategy, O18 and O20 |
| 31 | MPC OS 3.9.1 and earlier: "Our branch does not need to cater for MPC OS 3.9.1 and earlier. Let's not remove support for that yet (we remain upstream-friendly), but let's not go out of our way to support it either." The fork's device runs are on MPC OS 3.10.1; the 3.9.1 installer route stays built, as upstream builds it, and untested by the fork. | strategy, §4.4 and §8.3 |

**To revisit.** Decision 12, the fifth of 11ths and 13ths: when a listener misses the fifth, when a voicing style wants it,
when `THEORY_CHORD_MAX` is raised for another reason, or when the CPG's 11ths and 13ths change. The trade-off and the ways
forward are in [avenue H](#h-completing-the-chord-vocabulary).

---

## 11. Open questions

The study's own questions are all answered in §10, the last one (the physical pads) by decision 20. The work proceeds by
the [implementation strategy](implementation-strategy.md), which keeps the one list of decisions waiting for the owner,
with their dependencies: [its §8](implementation-strategy.md#8-decisions-for-the-owner). Once taken, a decision that
concerns the integration as a whole is recorded in §10; one that concerns a step, in that step's specification.

---

## Appendix A: measurement method

All scripts were written in the session scratchpad and are not committed.

1. **Cadenza's chord space.** A C program linked against [src/theory.c](../../src/theory.c) enumerated, at key C, for each
   of the seven scales, every `src` from 0 to 51, the degrees each allows (any for `src` 0, the borrow's degree for 1-3,
   degree 0 for the chromatic chords) and the ten colours: 4 060 chords, each as root pitch class, pitch-class set and
   notes, from `theory_chord_notes`.
2. **The CPG's catalogue.** A Node script imported the CPG's `musicTheory.js` and `generation.js` at commit `c84a306`.
   Template chords came from `generateProgressionChords` (repeats kept, 745 chords); pads from `generateVariant` in C for
   each of the six styles (184 × 6 × 16). Roots: the parser's `scaleDegree` for template chords, the spelled chord name
   for pads.
3. **Classification.** For each template, the Cadenza scale maximising, in order, diatonic, borrowed, then chromatic
   chords was retained; each chord was matched on (root, pitch-class set). "Fifth omitted" and "a note added": the same
   root and sets differing only by the perfect fifth.
4. **Turnarounds.** For each Classic palette, every pad other than the first was related to the first pad's root by root
   interval and quality, as in the table of avenue D; "dominant-type" means a major triad or dominant seventh a fifth
   above, a dominant seventh a semitone above or a whole tone below, or a diminished chord a semitone below.
5. **Starting orders.** Lengths, and whether the first and last chords are the tonic (degree I or i without accidental),
   were read from `parseProgression`.
6. **Device facts.** The paths of avenue J's notes come from a filesystem listing of an MPC One under MPC OS 3.10.1,
   supplied by the fork's owner and not committed; only generic system paths are cited.
7. **Other figures.** Genres, priorities, hints and chord types were counted from the CPG's `progressions` and
   `CHORD_INTERVALS`; Cadenza's spellings were printed by `theory_chord_name`, `theory_chord_name_c`, `theory_degree_c` and
   `theory_numeral`; the core parity trace was built from [tests/core_parity.c](../../tests/core_parity.c) and hashed.

## Appendix B: source map

| Topic | Cadenza | CPG / Ouarpeggiator |
|---|---|---|
| Scales, chords, names, numerals, voice leading | [src/theory.h](../../src/theory.h), [src/theory.c](../../src/theory.c) | [musicTheory.js](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/modules/musicTheory.js) |
| Progression presets | [theory.c:12](../../src/theory.c#L12) | `progressions` in musicTheory.js; nicknames in [locales/en.json](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/locales/en.json) |
| Progression generator | [gen.c:1142](../../src/gen.c#L1142) | palette walk: [Ouarpeggiator musicTheory.js](https://github.com/liotier/Ouarpeggiator/blob/main/js/modules/musicTheory.js) |
| Palette, row 4, categories | none | [generation.js](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/modules/generation.js) |
| Slots, Feel layer, save format, clamps | [src/seq_core.h](../../src/seq_core.h), [src/seq_core.c](../../src/seq_core.c) | `.progression`: [export specification](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/mpc-export-specification.md) |
| Voicing styles, CADENCE, SPREAD | [src/voicing.c](../../src/voicing.c), [seq_core.c:2259](../../src/seq_core.c#L2259) | [spec 11, Pedal](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/improvements/11-pedal-voicing-style.md) |
| Spelling | [theory.c:6](../../src/theory.c#L6) (sharps only) | [spec 13](https://github.com/liotier/AkaiMPC/blob/main/AkaiMPCChordProgressionGenerator/docs/improvements/13-spelling.md) |
| Genre presets | [src/presets.c](../../src/presets.c) | genres: keys of `progressions` |
| Browsers and screens | [vst/layout.conf](../../vst/layout.conf), [src/engine.c](../../src/engine.c) | |
| Plugin identity | [vst/vst.json](../../vst/vst.json) | |
| Budgets | [vst/budget.json](../../vst/budget.json), [vst/bench.txt](../../vst/bench.txt) | |
| Quality apparatus | [tests/run_tests.sh](../../tests/run_tests.sh), [tests/core_parity.sha256](../../tests/core_parity.sha256) | [docs/improvements](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements), catalogue snapshot, theory audit |

## Revision history

- Revision 1 surveyed the three projects and proposed avenues.
- Revision 2 recorded the owner's first decisions ([§10](#10-decision-log)), rebuilt the integration around palettes
  rather than progressions ([§4](#4-palettes-not-progressions)), and answered three questions: whether the progression
  path can accept any chord ([§6.2](#62-lifting-the-borrowed-only-clamp)), how turnaround chords can be found in a palette
  ([avenue D](#d-the-palette-in-the-bar-editor-and-turnarounds)), and whether Cadenza's voice leading needs the CPG's
  ([§6.6](#66-voice-leading-what-cadenza-already-does-what-the-cpg-would-add)).
- Revision 3 recorded how the CPG is pinned and which identity test builds carry (decisions 10 and 11), and recommended
  answers on the fifth of 11ths and 13ths ([avenue H](#h-completing-the-chord-vocabulary)) and on how live pads behave
  ([avenue L](#l-live-palette-pads)).
- Revision 4 recorded decisions 12 to 15 (the fifth of 11ths and 13ths, with its trade-off kept for review; file access;
  Q-Link banks; the deferred probe of physical pads), corrected avenue J's risk, and noted the owner's routes from parts
  to clips in the parked topic at the end of [§7](#7-integration-avenues).
- Revision 5 dropped avenue J (decision 16), grounded the live pads' recommendation in Cadenza's current behaviour
  ([avenue L](#l-live-palette-pads)), answered what a C port of the palette builder would give the user and recommended
  against it ([avenue D](#d-the-palette-in-the-bar-editor-and-turnarounds)), and recorded where an MPC One keeps its
  progression files.
- Revision 6 recorded decisions 17 to 19 (live pads, no port, the pads' device test), compared Cadenza with Midicake's
  ARP ([§2.4](#24-a-neighbour-midicakes-arp)), and added the device test's protocol,
  [device-test-pads.md](device-test-pads.md).
- Revision 7 recorded decision 20 after the device test's runs: no MIDI input, harmony being sequenced in Cadenza; avenue
  L's pads stay on screen, and no question remains open ([§11](#11-open-questions)). It also pointed the work to the
  [implementation strategy](implementation-strategy.md).
- Revision 8 corrected the binary and skin figures of [§2.1](#21-cadenza) and the headroom of
  [§5](#5-constraints-any-integration-must-respect) and [avenue B](#b-a-palette-table-generated-from-the-cpg) with the
  first CI measurement (step 0): the binary has 2.5 KB of headroom, not 22 KB, the earlier figure coming from a budget
  note written for 0.24.
- Revision 9 recorded decision 21: section garbage collection in the ARM build and a budget of 720 KB, after step 1
  measured over the old one. Amended: §11 points to the strategy's single list of the owner's pending decisions.
- Revision 10 recorded decisions 22 and 23: a project keeps its palette by value; Cadenza's genres and the CPG's are
  orthogonal, so genre presets get no palettes. Decision 23 puts avenue G, the genre crosswalk, in question: a question
  for the owner (strategy, §8.1, O14).
- Revision 11 recorded decision 24: avenue G, the genre crosswalk, dropped.
- Revision 12 noted in §6.4 that Cadenza's own *2-5-1-1 Jazz* preset already lays ii-V-I out by decision 4's rule, which
  the owner confirmed for the palettes' starting orders (spec 04, S04-D9).
- Revision 13 records decisions 25 and 26, the owner's answers to the two questions step 4 found in the CPG: its palettes'
  key dependence is fixed upstream; chords carry no category.
- Revision 14 records decisions 27 and 28: the author's silence slows nothing; the web simulation is not pursued for now.
- Revision 15 records decisions 29 to 31: working with upstream as a feature branch, planned in
  [upstream-relations.md](upstream-relations.md); MPC OS 3.9.1 kept but not catered for.
