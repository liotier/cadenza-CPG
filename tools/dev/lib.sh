# Shared by tools/dev/native_tests.sh and tools/dev/armsize.sh (docs/development/specs/00-infrastructure.md, addendum A).
# Sourced, not run.

# dev_headers TREE MPC_VST: TREE/vst/build/keyflags.h and params.h, as vst/build.sh and tests/run_tests.sh would make them,
# generated in a temporary copy of TREE/vst: vst/make_skin.py also rewrites vst/params.json, vst/layout.conf and vst/images/,
# which are committed, so run in place it would dirty the tree (A2). Needs Python 3 with Pillow.
dev_headers() {
  local tree="$1" mv="$2" w
  w="$(mktemp -d)"
  cp -R "$tree/vst" "$w/vst"
  rm -rf "$w/vst/build"
  (cd "$w/vst" && SKIN_FONTS="$mv/tools/html_art/fonts" python3 make_skin.py >/dev/null) || { rm -rf "$w"; return 1; }
  python3 "$mv/tools/gen_vst.py" "$w/vst/vst.json" --params-h >/dev/null || { rm -rf "$w"; return 1; }
  mkdir -p "$tree/vst/build"
  cp "$w/vst/build/keyflags.h" "$w/vst/build/params.h" "$tree/vst/build/"
  rm -rf "$w"
}

# dev_toolchain REPO: the mpc-vst-plugins checkout, as the author's scripts find it (MPC_VST, else next to the repo)
dev_toolchain() {
  local mv="${MPC_VST:-$1/../mpc-vst-plugins}"
  [ -f "$mv/tools/gen_vst.py" ] || { echo "no mpc-vst-plugins at $mv: set MPC_VST (branch chordseq/open-mpc, commit in .github/toolchain)" >&2; return 1; }
  (cd "$mv" && pwd)
}
