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
| [cpg-integration-study.md](cpg-integration-study.md) | study, revision 16 | What the MPC Chord Progression Generator's palettes (and the Ouarpeggiator's palette walk) can bring to Cadenza, measured against Cadenza's engine, budgets and licence; the palette model; avenues ranked and staged; decision log; open questions |
| [implementation-strategy.md](implementation-strategy.md) | strategy, revision 2 | The order of work: where it stands, principles, what steps 0 to 2 taught, workflow and infrastructure, budgets, the main steps, milestones, next steps, risks |
| [specs/](specs/README.md) | 00 to 05 and 0a implemented (milestone M2), device runs pending | One specification per step: infrastructure, spelling, the chromatic path, the missing sonorities, the palette table, the part's palette |
| [measurements/](measurements/coverage.txt) | generated, checked by CI | The CPG coverage measurement (`tools/cpg/coverage.sh`) and the palette table's report ([palettes.txt](measurements/palettes.txt), `tools/cpg/generate.sh`) |
| [device-memory-probe.md](device-memory-probe.md) | run 1, 2026-11-26: Cadenza at most 34 MB of MPC's memory; O10 decided (decision 32) | What Cadenza's screens cost in the MPC One's memory, read over root SSH by `tools/dev/mpc_memprobe.sh`: the figures for the skin budget (O10) |
| [device-tab-probe.md](device-tab-probe.md) | run 1, 2026-11-26: 0.1 s a switch on the light tabs, 0.3 to 0.4 s on the dense ones | What a tab switch costs MPC (main-thread CPU, the filmstrip cache), read over root SSH by `tools/dev/mpc_tabprobe.sh`: where to gain latency (principle 10) |
| [device-run-test-build.md](device-run-test-build.md) | run 1 passed, 2026-11-25 | The device run of the test build (O1): where to get it, how to swap it for the official plugin and back, the checks with their expected chord names |
| [upstream-relations.md](upstream-relations.md) | plan, 2026-11-26 | Working with upstream (decisions 29 and 30): what is offered to Cadenza's author, in what order and form; what keeps the fork's builds sound (the toolchain from open-mpc, its origin; the README's notice) |
| [upstream/](upstream/README.md) | the CPG's spec 18 implemented there; an issue for Cadenza drafted, on hold | Documents written for other repositories: the CPG's spec 18, row 4 the same in every key; an issue for Cadenza's repository, its two packages naming the plugin file differently |
| [device-test-pads.md](device-test-pads.md) | closed after three runs; findings kept | Whether the MPC One's hardware pads (and notes in Cadenza's own clip) reach Cadenza: a stage without code, a stage with a diagnostic build, a results table |

Conventions:

- Progression templates are written with hyphens (I-V-vi-IV). In the Chord Progression Generator's source the separator is
  U+2014, which is data there, not punctuation.
- Numerals follow the Generator's parallel-major convention (♭VI is the chord eight semitones above the tonic, whatever the
  mode) unless a passage says it quotes Cadenza, whose numerals count from the project's own scale.
- A study states measurements with their method; a later specification should follow the format of the Generator's
  [docs/improvements](https://github.com/liotier/AkaiMPC/tree/main/AkaiMPCChordProgressionGenerator/docs/improvements):
  provenance, decisions table, measurements, verification.
