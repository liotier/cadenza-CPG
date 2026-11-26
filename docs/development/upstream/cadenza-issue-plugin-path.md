<!--
Draft of an issue for Shrekage2024/cadenza-force (offer A1: docs/development/upstream-relations.md, §3). ON HOLD since
2026-11-26: the two packages are the README's two installation routes (3.9.1: the armv7 zip's installer into
/sdcard/Synths; 3.10.1: the SDK zip's folder into VST2 Plugins, in Akai's format, which names the binary by architecture).
A user following the README never mixes them; the owner's installation used the armv7 zip's portable/ folder under
3.10.1, outside both. What remains is a note for users who change routes, which concerns every plugin the open-mpc
toolchain packages; O17 proposes a paragraph of the design note instead of an issue. Kept as written, for the record.
Nothing below names the owner's drive or files.
-->

**Title:** Saved states do not open across the release's two packages: they name the plugin file differently

**Summary.** MPC ties a saved plugin state to the plugin's file path. The 0.28.1 release's two MPC packages name the plugin
file differently, so a project or plugin state saved with Cadenza installed from one package is refused, with "This state
file is not for Cadenza", once Cadenza is installed from the other.

**Device.** MPC One, MPC OS 3.10.1, Cadenza 0.28.1, plugins on an external drive under `VST2 Plugins`.

**What differs.**

| Package | Plugin file in `Cadenza - VST - Cadenza/` |
|---|---|
| `Cadenza-0.28.1-mpc-armv7.zip`, folder `portable/` | `cadenza.so` |
| `Cadenza-0.28.1-mpc-sdk.zip` | `cadenza-arm32.so` |

**Why the state is refused.** MPC saves, with the plugin's state, a description of the plugin (a plugin state exported
from MPC as `.xpi` shows it):

```xml
<PLUGIN name="Cadenza" format="VST" category="Synth" manufacturer="Cadenza"
        version="1.0.0.0" file="/media/<volume>/VST2 Plugins/Cadenza - VST - Cadenza/cadenza.so"
        uid="43685371" isInstrument="1" ... />
```

Two states saved with the two packages differ in `file` alone. The state itself is sound: decoded, the one MPC refused
loads in the 0.28.1 engine. So MPC refuses on the path, before Cadenza sees anything. `fileTime` is not compared: two
different builds of `cadenza.so` installed at the same path exchange states both ways.

<!-- If the check below confirms it with official builds (expected): -->
**To confirm.** With the official 0.28.1 installed from the armv7 package, save a project; install the official 0.28.1
from the SDK package instead, in the same place; open the project: MPC refuses it. Putting the armv7 package's folder back
opens it again. [Result on the MPC One: …]

<!-- If the check was not run, replace the paragraph above with: -->
<!-- **How it was found.** With two builds of 0.28.1's source, one installed from each kind of package; their states differ
in the file path alone, and between two installations of the same kind both open in both directions. Not yet repeated
with the two official packages. -->

**Consequences for users.**

- Someone who installed from one package and later installs an update from the other can no longer load the Cadenza
  state saved in their projects: MPC refuses each with the message above.
- The same probably holds when the plugin folder moves to another drive, since the drive is part of the path (not tested).

**Suggestion.** One file name in both packages (whichever the SDK route requires), and, in the install notes, a line
telling users to keep to one package and one drive. If the name must change, a release note saying that states saved
before will not reload.

Found while testing a fork of Cadenza on the MPC One ([liotier/cadenza-CPG](https://github.com/liotier/cadenza-CPG), a
personal development fork, no builds offered); the investigation is written up in its
[device run](https://github.com/liotier/cadenza-CPG/blob/claude/gifted-carson-j102ab/docs/development/device-run-test-build.md#51-row-7-what-refuses-the-state).
