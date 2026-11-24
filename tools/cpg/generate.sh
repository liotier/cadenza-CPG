#!/usr/bin/env bash
# Regenerates the palette table (docs/development/specs/04-palette-table.md): builds tools/cpg/enumerate.c against src/theory.c,
# lists Cadenza's chord space and its root names, and runs tools/cpg/generate.mjs over the pinned CPG. Writes src/cpg_palettes.c,
# tests/cpg_expected.h and docs/development/measurements/palettes.txt. With --check, compares instead of writing, and fails on any
# difference (CI). Needs gcc, Node 20 or later and the submodule (git submodule update --init third_party/AkaiMPC).
set -euo pipefail
cd "$(dirname "$0")/../.."
CPG=third_party/AkaiMPC/AkaiMPCChordProgressionGenerator
[ "${1:-}" = --check ] && GEN_CHECK=1
[ -f "$CPG/modules/musicTheory.js" ] || { echo "generate: no CPG in $CPG (git submodule update --init third_party/AkaiMPC)"; exit 1; }
W="$(mktemp -d)"
trap 'rm -rf "$W"' EXIT
"${CC:-gcc}" -std=gnu11 -Wall -Wextra -Werror -Isrc -o "$W/enumerate" tools/cpg/enumerate.c src/theory.c
"$W/enumerate" > "$W/chords.txt"
"$W/enumerate" --names > "$W/names.txt"
mkdir "$W/out"
CPG_SHA="$(git -C third_party/AkaiMPC rev-parse --short=7 HEAD)" node tools/cpg/generate.mjs "$CPG" "$W/chords.txt" "$W/names.txt" "$W/out"
fail=0
for pair in "cpg_palettes.c src/cpg_palettes.c" "cpg_expected.h tests/cpg_expected.h" "palettes.txt docs/development/measurements/palettes.txt"; do
  set -- $pair
  if [ "${GEN_CHECK:-}" = 1 ]; then
    diff -u "$2" "$W/out/$1" > "$W/diff" || { head -40 "$W/diff"; echo "generate: FAIL: $2 is out of date (tools/cpg/generate.sh rewrites it)"; fail=1; }
  else
    cp "$W/out/$1" "$2"
  fi
done
[ "$fail" = 0 ] || exit 1
[ "${GEN_CHECK:-}" = 1 ] && echo "generate: src/cpg_palettes.c, tests/cpg_expected.h and palettes.txt as committed" || echo "generate: wrote src/cpg_palettes.c, tests/cpg_expected.h, docs/development/measurements/palettes.txt"
