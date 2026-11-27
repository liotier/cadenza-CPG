# Upstream

Documents this project writes for other repositories, kept here so that they are reviewed in the open with the rest of
the work. Each is written in its destination's own format and conventions, and its relative links resolve there, not
here.

| Document | For | Where it goes | Status |
|---|---|---|---|
| [cpg-18-row4-key-invariance.md](cpg-18-row4-key-invariance.md) | the MPC Chord Progression Generator ([liotier/AkaiMPC](https://github.com/liotier/AkaiMPC)) | `AkaiMPCChordProgressionGenerator/docs/improvements/18-row4-key-invariance.md` | implemented in the CPG, 2026-11-24 (`a0a6bf6`, merged as `5f091e7`); the CPG's copy, with its implementation notes, is the reference; this one is the proposal as sent. The fork's submodule follows (spec 04, §8.4) |

| [cadenza-issue-plugin-path.md](cadenza-issue-plugin-path.md) | Cadenza ([Shrekage2024/cadenza-force](https://github.com/Shrekage2024/cadenza-force)) | a new issue, posted by the owner | drafted, 2026-11-26, then put on hold: the release's two packages name the plugin file differently, so saved states do not travel between them (lesson 16), but they are the README's two installation routes, which a user following it never mixes (O17) |
| [open-mpc-wrapper-options.md](open-mpc-wrapper-options.md) | the mpc-vst-plugins toolchain ([open-mpc/mpc-vst-plugins](https://github.com/open-mpc/mpc-vst-plugins)) | a pull request from [liotier/mpc-vst-plugins `wrapper-options`](https://github.com/liotier/mpc-vst-plugins/tree/wrapper-options) | submitted by the owner, 2026-11-26, as [open-mpc/mpc-vst-plugins#1223](https://github.com/open-mpc/mpc-vst-plugins/pull/1223): the six wrapper commits of Cadenza's toolchain branch, credited to their author, with the documentation open-mpc asks for (O20) |
| [open-mpc-wrapper-options-review-1.md](open-mpc-wrapper-options-review-1.md) | the same pull request | a comment on it, posted by the owner | drafted, 2026-11-27, for the owner to post: the answer to the first review (changes requested); its patches are on the pull request since `cadenza/toolchain` was fast-forwarded to `wrapper-options` (`42161aa`): the missing commits, a test port, the prefix as a string literal, the 44.1 kHz comment, the engine table's retry kept |

The design note to Cadenza's author (milestone M2, offer A3 of [upstream-relations.md](../upstream-relations.md)) will
join this list.
