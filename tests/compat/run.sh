#!/usr/bin/env bash
# The compatibility harness (docs/development/specs/00-infrastructure.md, 3.2): does a state saved by this fork open in the
# official Cadenza ? Builds tests/compat/writer.c against this fork's src/ and tests/compat/loader.c against the official
# src/ at COMPAT_BASE (default 56e95a0, Cadenza 0.28.1, extracted with git archive), both natively under ASan/UBSan; the
# writer saves one state per scenario and this fork's own re-save of it (tests/compat/writer.c says why the re-save is the
# reference); the loader loads each state in the official engine and saves it again.
# The official re-save must be identical to the fork's, unless tests/compat/expected.txt pins the official engine's re-save for that scenario (the
# difference a later step's specification permits): then that re-save's SHA-256 must match the pinned one.
# Needs git, gcc, sha256sum (or shasum) and the base commit in the clone's history.
set -euo pipefail
cd "$(dirname "$0")/../.."
BASE="${COMPAT_BASE:-56e95a0}"
CC="${CC:-gcc}"
SAN="-fsanitize=address,undefined -fno-omit-frame-pointer -g -O1"
ENGINE="seq_core.c theory.c gen.c melody.c voicing.c rhythm.c jamlib.c bassline.c melody2.c"
W="$(mktemp -d)"
trap 'rm -rf "$W"' EXIT
sha() { (sha256sum "$1" 2>/dev/null || shasum -a 256 "$1") | cut -d' ' -f1; }

git rev-parse -q --verify "$BASE^{commit}" >/dev/null || { echo "compat: FAIL: $BASE is not in this clone's history (fetch it first)"; exit 1; }
mkdir "$W/official" "$W/out"
git archive "$BASE" src | tar -x -C "$W/official"
fork_src=(); official_src=()
for f in $ENGINE; do fork_src+=("src/$f"); official_src+=("$W/official/src/$f"); done
$CC -std=gnu11 -Wall -Wextra -Werror $SAN -Isrc -o "$W/writer" tests/compat/writer.c "${fork_src[@]}" src/presets.c -lm
$CC -std=gnu11 -Wall -Wextra -Werror $SAN -I"$W/official/src" -o "$W/loader" tests/compat/loader.c "${official_src[@]}" -lm

"$W/writer" "$W/out" > "$W/scenarios"
fail=0; same=0; pinned=0
while read -r n; do
  if ! "$W/loader" "$W/out/$n.txt" "$W/out/$n.official.txt"; then fail=1; continue; fi
  if cmp -s "$W/out/$n.fork.txt" "$W/out/$n.official.txt"; then same=$((same + 1)); continue; fi
  got="$(sha "$W/out/$n.official.txt")"
  want="$(awk -v n="$n" '$1 == n { print $2 }' tests/compat/expected.txt)"
  if [ -n "$want" ] && [ "$got" = "$want" ]; then pinned=$((pinned + 1)); continue; fi
  echo "compat: FAIL: $n: the official engine's re-save differs from the fork's (sha256 $got${want:+, pinned $want}); fields, fork (<) and official (>):"
  diff <(tr '|' '\n' < "$W/out/$n.fork.txt") <(tr '|' '\n' < "$W/out/$n.official.txt") || true
  fail=1
done < "$W/scenarios"
# a pin for a scenario that no longer exists is stale
while read -r n _; do
  case "$n" in ''|'#'*) continue ;; esac
  grep -qx "$n" "$W/scenarios" || { echo "compat: FAIL: tests/compat/expected.txt pins $n, which the writer no longer saves"; fail=1; }
done < tests/compat/expected.txt
total=$(wc -l < "$W/scenarios" | tr -d ' ')
[ "$fail" = 0 ] || { echo "compat: FAIL"; exit 1; }
echo "compat: $total scenarios open in the official engine ($BASE): $same identical, $pinned as pinned in tests/compat/expected.txt"
