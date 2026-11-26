# Relations with upstream: two tracks

Status: plan, 2026-11-26. The owner's instruction (study, decision 29): "pursue dual goals: appealing to upstream, and also
being prepared to pursue our own way if we do not get any answer". Track A is the appeal, track B the preparation. Most of
the work serves both; this document says where they part, what each needs, and which decisions are the owner's
([strategy, §8](implementation-strategy.md#8-decisions-for-the-owner): O3, O17 to O19).

## 1. The facts

**Upstream's public record** ([Shrekage2024/cadenza-force](https://github.com/Shrekage2024/cadenza-force), read
2026-11-26): ten commits, one export per release, the history kept private; exports of 0.25.0 (29 October), 0.26.0
(31 October), 0.27.1 (2 November) and 0.28.1 (14 November); nothing since. Its [tested.json](../../tested.json) lists 32
versions tested on the author's Force between 11 October and 13 November, about one a day.

**What that record shows, and what it does not.** It shows that nothing has been published for twelve days. It cannot
show whether the author works, since only releases are exported: the gap between 0.27.1 and 0.28.1 was twelve days
too. The engine also serves the Cadenza iOS app and the browser version, the author's two declared lines of business
([NOTICE](../../NOTICE)), and 0.27.0 added to the shared core the song order "for the iOS song page": work may continue
there unseen. "Development is entirely inexistent since release" is therefore not yet established; what is established
is that the author has answered neither contact. Both tracks are worth pursuing either way: the appeal costs little, and
the preparation protects the owner's own use of the fork whatever upstream does.

**Contacts so far.** The owner's issue [Shrekage2024/cadenza-force#4](https://github.com/Shrekage2024/cadenza-force/issues/4),
"Documentation: works on MPC OS 3.10.1 on MPC One" (22 November), the repository's only issue, unanswered; the owner's
message of 23 November introducing the fork, offering the patches of milestone M1 and asking for the browser version's
sources, unanswered (decision 27). The repository has no Discussions.

**What the fork takes from upstream's accounts.** The toolchain: both workflows check out
[Shrekage2024/mpc-vst-plugins](https://github.com/Shrekage2024/mpc-vst-plugins/tree/chordseq/open-mpc), branch
`chordseq/open-mpc`, at `8fbeffc` ([.github/toolchain](../../.github/toolchain)), the author's fork of the MIT-licensed
open-mpc toolchain with five wrapper options Cadenza needs. Nothing else: this repository holds Cadenza's sources in full
(`main` is upstream's `56e95a0`), and the owner keeps the official 0.28.1 folder for restoring.

## 2. What the licence allows on each track

[PolyForm Shield 1.0.0](../../LICENSE) (a reading, not legal advice; the study's [§9](cpg-integration-study.md#9-licence-governance-and-branding)
gives the rest):

- **Permitted, with or without an answer:** what the fork does now. Personal use on the owner's MPC One; development in
  the open; builds as private drafts. "Any purpose is a permitted purpose, except for providing any product that competes
  with the software or any product the licensor or any of its affiliates provides using the software" (Noncompete): a
  personal fork provides no product. Track B needs no answer to continue to milestone M5.
- **Not permitted without the author's permission:** offering the fork's builds to others. "Goods and services compete
  even when provided free of charge. If you market a product as a practical substitute for the software or another
  product, it definitely competes" (Competition). Silence grants nothing: "These terms do not imply any other licenses"
  (No Other Rights). The same clause lets the author grant more ("or prevent the licensor from granting licenses to
  anyone else"): a permission is possible, and only the author can give it.
- **If the MPC plugin were withdrawn.** "You may begin using the software to compete with a product or service that the
  licensor or any of its affiliates has stopped providing, unless the licensor includes a plain-text line beginning with
  `Licensor Line of Business:`" (Discontinued Products). NOTICE names the iOS app and the browser version, not the MPC
  plugin. But a plugin left online, even unmaintained, is still provided; and a fork offered to others could still be held
  to compete with the iOS app, since goods compete "for different technical platforms" (Competition). Not a route to
  count on.
- **Outside the licence altogether:** a plugin written without Cadenza's code. The CPG and its palettes are the owner's,
  in the public domain; Cadenza's code, and the fork's changes to it, are not. That would be a new project, not this
  fork; recorded so that the choice is known, not recommended.

So "our own way", absent an answer, means: the fork continues as the owner's instrument, complete and self-sufficient,
and stays personal. Sharing it needs either upstream (track A succeeds) or the author's permission (O19).

## 3. Track A: the appeal

Proposals carry better than patches: one export per release suggests the author re-implements rather than merges (study,
§9). Each offer below stands alone, is small and measured, and costs the author little to accept; they go in order of
that cost, lowest first.

| # | Offer | State | Decision |
|---|---|---|---|
| A1 | A bug report against the official release: its two packages name the plugin file differently (`cadenza.so`, `cadenza-arm32.so`), and MPC ties a saved state to that path, so states saved under one package are refused under the other ("This state file is not for Cadenza"); lesson 16. A small fix in the packaging (one file name for both), and a note for users who switch packages. | drafted: [upstream/cadenza-issue-plugin-path.md](upstream/cadenza-issue-plugin-path.md) | O17 |
| A2 | Milestone M1: section garbage collection in the ARM build (one line of `vst/vst.json`, about 46 KB of uncalled functions dropped, same behaviour, parity unchanged) and key-aware spelling (about 4 KB). | ready; offered in the message of 23 November | O3 |
| A3 | The design note (milestone M2): palettes from the CPG, chords on any root, the full vocabulary, what changes for the author's other front ends, how projects stay readable by 0.28.1. | to write; the specifications' "other front ends" sections hold its material | none: it is written anyway (decision 27) |
| A4 | An explicit question, once the fork has screens to show (milestone M3): would the author take the work upstream, or permit the fork to be shared under a name of its own, or neither. | at M3 | O19 |

A1 comes first because it helps Cadenza's users whatever the author thinks of the fork, and asks nothing. It is best made with
official builds alone (O17): installed from the SDK package, does the official 0.28.1 refuse a project saved with the
official armv7 package ? Ten minutes on the device turn the fork's observation into a report on the official release alone.

The channel is upstream's issue tracker: the one public channel it has, where a report also serves other users. The tone
stays that of a user's report and an offer: no deadline, no pressure, nothing the author must answer for the fork to go
on.

## 4. Track B: being ready to go on alone

| # | Preparation | Why | State |
|---|---|---|---|
| B1 | A copy of the toolchain branch in the owner's account: a GitHub fork of `Shrekage2024/mpc-vst-plugins` (MIT, history kept), and `.github/toolchain` pointed at it at the same commit. | The one thing the fork's builds take from the author's account. If that repository went away, CI would stop building. GitHub keeps an active public fork when the original is deleted ("An active public fork becomes the new upstream repository for the network", [GitHub Docs](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/what-happens-to-forks-when-a-repository-is-deleted-or-changes-visibility)), so a fork made now survives that. | O18: creating it is the owner's act; repointing the pin is one edit here |
| B2 | A fork notice at the top of the README: a personal development fork, no builds offered, Cadenza itself at upstream. | The repository's front page (`main`) is upstream's README unchanged: it presents itself as "Cadenza 0.28.1" with install instructions. The notice avoids passing for Cadenza (no trademark rights; study, §9) and keeps the fork from being taken as a substitute, which matters on both tracks. | on this branch; reaches `main` with the release branch (O2) |
| B3 | The constraints that serve upstream alone, marked (below), so that they can be dropped at once if track A closes. | Several of the fork's rules exist to keep the work offerable; on track B some still earn their keep, some do not. | below |
| B4 | Upstream watched for releases: GitHub's Watch, Custom, Releases on upstream's repository. | If the author publishes again, the release is merged promptly (strategy, §10), and track A has news to answer. | the owner's account; one click |

**B3: which constraints serve whom.**

| Constraint | Serves upstream | Serves the fork alone | On track B |
|---|---|---|---|
| Projects travel both ways (principle 4) | a pull request that breaks no saved project | the owner's projects, and the route back to the official build | kept |
| Core parity unchanged (principle 3) | Cadenza's users' saved sound | the owner's saved sound | kept |
| Cadenza's identity (decision 11) | a pull request | the owner's existing projects open in the fork | kept, for the second reason; a name of its own only if sharing is ever permitted |
| Small, separable steps (principle 6) | patches the author can pick | reviewable work, bisectable faults | kept |
| "Other front ends" in each specification (lesson 6) | the iOS app and browser version upstream maintains | nothing | reduced to one line |
| The design note, the patch series | everything | nothing | not written beyond A3 |
| The binary budget (720 KB) | upstream's discipline | none cited for the device (decision 21) | the owner may raise it freely |
| The skin budget (O10) | upstream's gauge | the device's memory (the probe) | set from the probe |

Nothing changes now: every constraint above still holds while track A is open. Track A closes when the author declines,
or when the owner decides it has; there is no deadline, since track B needs no answer to proceed.

**What survives either outcome.** Everything the fork learns about harmony that does not need Cadenza goes to the CPG,
which is the owner's and in the public domain: its spec 18 (decision 25) is already there. The fork's specifications and
measurements are the owner's writing. Only the code that runs inside Cadenza follows Cadenza's licence.

## 5. Order

1. B2, the README notice (this commit); O17, A1: the owner confirms with official builds and posts the report.
2. O18, B1: the toolchain copy, if the owner agrees; then the pin moves.
3. A3, the design note, alongside specifications 06 and 07.
4. A4 at milestone M3.
