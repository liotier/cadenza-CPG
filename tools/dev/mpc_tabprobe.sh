#!/bin/sh
# The MPC tab-switch probe (docs/development/device-tab-probe.md), run ON the device as root over SSH. Read-only: it reads /proc and
# /var/tmp/filmstrips and writes nothing but its own log. For each step: type a label and Enter, do the step on the MPC (switch the
# tab), wait until the page has drawn, then Enter again. Between the two Enters it measures the CPU time MPC's threads used (in ms, the
# main thread apart, and the three busiest threads by name) and what MPC added to its filmstrip cache (files, kB), as CSV on the
# terminal and appended to /tmp/mpc-tabprobe.csv. The wall time between the Enters is yours, not MPC's: read the CPU times. Ctrl-D ends.
#   scp tools/dev/mpc_tabprobe.sh root@<mpc>:/tmp/ && ssh -t root@<mpc> sh /tmp/mpc_tabprobe.sh
# PID=<pid> probes another process (for trying the script anywhere: PID=$$ sh mpc_tabprobe.sh).
out=/tmp/mpc-tabprobe.csv
fs=${FS:-/var/tmp/filmstrips}
pid=${PID:-$(pidof MPC 2>/dev/null | cut -d' ' -f1)}
[ -n "$pid" ] && [ -r "/proc/$pid/stat" ] || { echo "mpc_tabprobe: no MPC process (pidof MPC); PID=<pid> to name one" >&2; exit 1; }
hz=$(getconf CLK_TCK 2>/dev/null || echo 100)   # BusyBox has no getconf: Linux reports these in 1/100 s (USER_HZ)
line() { echo "$1" | tee -a "$out"; }
# ticks FILE: utime + stime of a /proc stat file (the name in parentheses may hold spaces: fields counted after the last ')')
ticks() { awk '{ sub(/.*\) /, ""); print $12 + $13 }' "$1" 2>/dev/null; }
# threads: "tid ticks name" for every thread of the process
threads() {
  for t in /proc/$pid/task/*; do
    [ -r "$t/stat" ] || continue
    echo "${t##*/} $(ticks "$t/stat") $(tr ' ,' '__' < "$t/comm" 2>/dev/null)"
  done
}
# cache: "files kB" in the filmstrip cache
cache() { if [ -d "$fs" ]; then ls -l "$fs" 2>/dev/null | awk '/temp_/ { n++; b += $5 } END { printf "%d %d\n", n, b / 1024 }'; else echo "0 0"; fi; }
uptime_ms() { awk '{ printf "%d\n", $1 * 1000 }' /proc/uptime; }
line "# $(date -u +%Y-%m-%dT%H:%M:%SZ) pid $pid ($(cat /proc/$pid/comm)), CLK_TCK $hz, kernel $(uname -r)"
line "label,wall_ms,main_cpu_ms,mpc_cpu_ms,top_threads_ms,filmstrip_files_added,filmstrip_kb_added"
echo "mpc_tabprobe: type a label and Enter, switch the tab, wait for the page, Enter again. Start with 'idle' (touch nothing for ~10 s). Ctrl-D ends. Log: $out" >&2
while printf 'label> ' >&2 && IFS= read -r label; do
  [ -n "$label" ] || continue
  label=$(echo "$label" | tr , ";")
  t0=$(uptime_ms); m0=$(ticks /proc/$pid/task/$pid/stat); p0=$(ticks /proc/$pid/stat); set -- $(cache); c0=$1; k0=$2
  threads > /tmp/mpc-tabprobe.t0
  printf '  now do "%s" on the MPC, wait for the page, then Enter ' "$label" >&2
  IFS= read -r _ || break
  t1=$(uptime_ms); m1=$(ticks /proc/$pid/task/$pid/stat); p1=$(ticks /proc/$pid/stat); set -- $(cache); c1=$1; k1=$2
  threads > /tmp/mpc-tabprobe.t1
  top=$(awk -v hz="$hz" 'NR == FNR { a[$1] = $2; next } ($1 in a) { d = $2 - a[$1]; if (d > 0) printf "%d %s\n", d * 1000 / hz, $3 }' \
        /tmp/mpc-tabprobe.t0 /tmp/mpc-tabprobe.t1 | sort -rn | awk 'NR <= 3' | awk '{ printf "%s%s:%d", (NR > 1 ? " " : ""), $2, $1 }')
  line "$label,$((t1 - t0)),$(( (m1 - m0) * 1000 / hz )),$(( (p1 - p0) * 1000 / hz )),$top,$((c1 - c0)),$((k1 - k0))"
done
rm -f /tmp/mpc-tabprobe.t0 /tmp/mpc-tabprobe.t1
echo "mpc_tabprobe: done; $out holds the log" >&2
