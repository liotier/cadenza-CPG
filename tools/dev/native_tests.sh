#!/usr/bin/env bash
# tests/run_tests.sh without Docker (docs/development/specs/00-infrastructure.md, addendum A): the author's offline suite,
# every step of it, compiled and run with the host's gcc instead of the gcc:12 image, for a machine with no Docker daemon
# (a cloud session). The steps are read from tests/run_tests.sh, never copied (S00-A1): three edits turn its Docker call into a
# plain bash one, and the runner stops if any no longer applies exactly once. The generated headers are made in a copy of
# vst/ (S00-A2); LeakSanitizer is told about libasound's cached configuration, as CI is (S00-A3). CI's run stays the reference.
#   tools/dev/native_tests.sh      MPC_VST: the toolchain checkout (default: next to this repo). Needs gcc, g++, Python 3
#                                  with Pillow. Exit status: run_tests.sh's.
set -euo pipefail
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
. "$REPO/tools/dev/lib.sh"
MPC_VST="$(dev_toolchain "$REPO")"
dev_headers "$REPO" "$MPC_VST"
export LSAN_OPTIONS="${LSAN_OPTIONS:-suppressions=$REPO/.github/lsan.supp}"
W="$(mktemp -d)"
trap 'rm -rf "$W"' EXIT
python3 - "$REPO/tests/run_tests.sh" "$W/run_tests.sh" <<'PY'
import sys
s = open(sys.argv[1]).read()
edits = [('cd "$(dirname "$0")/.."', 'cd "$REPO"'),
         ('docker run --rm -e SCREEN_BUDGET="$SCREEN_BUDGET" -v "$PWD":/w -v "$MPC_VST":/mv:ro -w /w gcc:12 bash -euc \'',
          'SCREEN_BUDGET="$SCREEN_BUDGET" MV="$MPC_VST" bash -euc \'')]
for old, new in edits:
    n = s.count(old)
    if n != 1:
        sys.exit("native_tests: tests/run_tests.sh changed: %d of %r, want 1; update tools/dev/native_tests.sh" % (n, old))
    s = s.replace(old, new)
if s.count("/mv/") < 1:
    sys.exit("native_tests: tests/run_tests.sh no longer names the toolchain as /mv/; update tools/dev/native_tests.sh")
s = s.replace("/mv/", "$MV/")   # inside the former container's script: the toolchain, expanded by that bash
open(sys.argv[2], "w").write(s)
PY
REPO="$REPO" MPC_VST="$MPC_VST" bash "$W/run_tests.sh"
