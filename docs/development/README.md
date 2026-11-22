# Development notes

Studies and specifications for work on this fork of Cadenza. Nothing here is user documentation: the manual is
[MANUAL.md](../../MANUAL.md).

| Document | Status | Subject |
|---|---|---|
| [cpg-integration-study.md](cpg-integration-study.md) | study, revision 3, no code | What the MPC Chord Progression Generator's palettes (and the Ouarpeggiator's palette walk) can bring to Cadenza, measured against Cadenza's engine, budgets and licence; the palette model; avenues ranked and staged; decision log; open questions |

Conventions:

- Progression templates are written with hyphens (I-V-vi-IV). In the Chord Progression Generator's source the separator is
  U+2014, which is data there, not punctuation.
- Numerals follow the Generator's parallel-major convention (♭VI is the chord eight semitones above the tonic, whatever the
  mode) unless a passage says it quotes Cadenza, whose numerals count from the project's own scale.
- A study states measurements with their method; a later specification should follow the format of the Generator's
  [docs/improvements](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements):
  provenance, decisions table, measurements, verification.
