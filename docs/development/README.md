# Development notes

Studies and specifications for work on this fork of Cadenza. Nothing here is user documentation: the manual is
[MANUAL.md](../../MANUAL.md).

**Decisions waiting for the owner:** all of them, with their dependencies, in
[implementation strategy, §8](implementation-strategy.md#8-decisions-for-the-owner): those
[decidable now](implementation-strategy.md#81-decidable-now) and those
[waiting for a further step or event](implementation-strategy.md#82-depend-on-a-further-step-or-event). Decisions already taken are in the
study's [decision log](cpg-integration-study.md#10-decision-log).

| Document | Status | Subject |
|---|---|---|
| [cpg-integration-study.md](cpg-integration-study.md) | study, revision 9 | What the MPC Chord Progression Generator's palettes (and the Ouarpeggiator's palette walk) can bring to Cadenza, measured against Cadenza's engine, budgets and licence; the palette model; avenues ranked and staged; decision log; open questions |
| [implementation-strategy.md](implementation-strategy.md) | strategy, revision 2 | The order of work: where it stands, principles, what steps 0 to 2 taught, workflow and infrastructure, budgets, the main steps, milestones, next steps, risks |
| [specs/](specs/README.md) | 00 to 02 and 0a implemented, device runs pending; 03 to 05 for approval | One specification per step: infrastructure, spelling, the chromatic path, the missing sonorities, the palette table, the part's palette |
| [measurements/](measurements/coverage.txt) | generated, checked by CI | The CPG coverage measurement (`tools/cpg/coverage.sh`) |
| [device-test-pads.md](device-test-pads.md) | closed after three runs; findings kept | Whether the MPC One's hardware pads (and notes in Cadenza's own clip) reach Cadenza: a stage without code, a stage with a diagnostic build, a results table |

Conventions:

- Progression templates are written with hyphens (I-V-vi-IV). In the Chord Progression Generator's source the separator is
  U+2014, which is data there, not punctuation.
- Numerals follow the Generator's parallel-major convention (♭VI is the chord eight semitones above the tonic, whatever the
  mode) unless a passage says it quotes Cadenza, whose numerals count from the project's own scale.
- A study states measurements with their method; a later specification should follow the format of the Generator's
  [docs/improvements](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements):
  provenance, decisions table, measurements, verification.
