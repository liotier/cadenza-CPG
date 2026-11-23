#!/usr/bin/env bash
# The plugin's ARM size before it is pushed (docs/development/specs/00-infrastructure.md, addendum A, A4): builds cadenza.so
# for armhf with a cross-compiler, the sources and flags gen_vst.py --shell gives for each tree (vst/vst.json), once for the
# working tree and once for commit REF, and prints the code (text), data and stripped file size of each and the difference.
# The cross-compiler is not the toolchain's (arm32v7/gcc:11-bullseye): the differences guide, CI's figures decide.
#   tools/dev/armsize.sh [REF]     REF: a commit (default HEAD). MPC_VST: the toolchain checkout (default: next to this repo).
#                                  Needs arm-linux-gnueabihf-gcc and Python 3 with Pillow.
set -euo pipefail
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
. "$REPO/tools/dev/lib.sh"
REF="${1:-HEAD}"
CC=arm-linux-gnueabihf-gcc
command -v "$CC" >/dev/null || { echo "armsize: needs $CC (Debian, Ubuntu: apt-get install gcc-arm-linux-gnueabihf)" >&2; exit 2; }
MPC_VST="$(dev_toolchain "$REPO")"
SHA="$(git -C "$REPO" rev-parse --short "$REF^{commit}")"
W="$(mktemp -d)"
trap 'rm -rf "$W"' EXIT
mkdir "$W/ref"
git -C "$REPO" archive "$SHA" | tar -x -C "$W/ref"

build() {   # build TREE OUT: OUT the stripped plugin, as tools/build_port.sh builds it but with $CC
  local t="$1" out="$2" SOURCES CFLAGS CFLAGS_ARM LIBS
  dev_headers "$t" "$MPC_VST"
  eval "$(python3 "$MPC_VST/tools/gen_vst.py" "$t/vst/vst.json" --shell | grep -E '^(SOURCES|CFLAGS|CFLAGS_ARM|LIBS)=')"
  if ! (cd "$t" && $CC -O2 -Wall -Wextra -Wno-unused-parameter -fPIC -shared -fvisibility=hidden -std=gnu11 $CFLAGS $CFLAGS_ARM \
        -I vst/build -I "$MPC_VST/wrapper" $SOURCES "$MPC_VST/wrapper/vst2_wrap.c" $LIBS -lpthread -ldl -Wl,--no-undefined \
        -o "$out" 2>"$out.log"); then
    grep -E "error" "$out.log" >&2 || cat "$out.log" >&2
    exit 1
  fi
  arm-linux-gnueabihf-strip "$out"
}
sizes() { arm-linux-gnueabihf-size "$1" | awk 'NR == 2 {print $1, $2}'; }

build "$W/ref" "$W/ref.so"
build "$REPO" "$W/tree.so"
read -r rt rd <<<"$(sizes "$W/ref.so")"
read -r tt td <<<"$(sizes "$W/tree.so")"
rf=$(stat -c %s "$W/ref.so"); tf=$(stat -c %s "$W/tree.so")
printf 'armsize: %s, cadenza.so for armhf, bytes\n' "$($CC --version | head -1)"
printf '  %-14s text %7d  data %7d  file %7d\n' "$SHA" "$rt" "$rd" "$rf"
printf '  %-14s text %7d  data %7d  file %7d\n' "working tree" "$tt" "$td" "$tf"
printf '  %-14s text %+7d  data %+7d  file %+7d\n' "difference" $((tt - rt)) $((td - rd)) $((tf - rf))
echo "  (the file size moves in whole pages; text and data are the measure. CI's budget check, with the toolchain's compiler, decides.)"
