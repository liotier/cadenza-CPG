#!/bin/sh
# The MPC memory probe (docs/development/device-memory-probe.md), run ON the device as root over SSH. Read-only: it reads /proc and
# writes nothing but its own log. After each label typed and Enter, three samples a second apart of the device's MemAvailable and the
# MPC process's resident (VmRSS) and proportional (Pss) set sizes, as CSV, on the terminal and appended to /tmp/mpc-memprobe.csv.
# An empty line samples again under the last label; Ctrl-D ends.
#   scp tools/dev/mpc_memprobe.sh root@<mpc>:/tmp/ && ssh -t root@<mpc> sh /tmp/mpc_memprobe.sh
# PID=<pid> probes another process (for trying the script anywhere: PID=$$ sh mpc_memprobe.sh).
out=/tmp/mpc-memprobe.csv
pid=${PID:-$(pidof MPC 2>/dev/null | cut -d' ' -f1)}
[ -n "$pid" ] && [ -r "/proc/$pid/status" ] || { echo "mpc_memprobe: no MPC process (pidof MPC); PID=<pid> to name one" >&2; exit 1; }
kb() { awk -v k="$1:" '$1 == k { print $2; exit }' "$2"; }
line() { echo "$1" | tee -a "$out"; }
line "# $(date -u +%Y-%m-%dT%H:%M:%SZ) pid $pid ($(cat /proc/$pid/comm)), MemTotal $(kb MemTotal /proc/meminfo) kB, kernel $(uname -r)"
line "label,seconds,mem_available_kb,mpc_rss_kb,mpc_pss_kb"
start=$(date +%s)
last=start
echo "mpc_memprobe: type a label and Enter after each step (empty: sample again; Ctrl-D: end). Log: $out" >&2
while IFS= read -r label; do
  [ -n "$label" ] && last=$label
  for i in 1 2 3; do
    pss=""
    [ -r "/proc/$pid/smaps_rollup" ] && pss=$(kb Pss "/proc/$pid/smaps_rollup")
    line "$last,$(( $(date +%s) - start )),$(kb MemAvailable /proc/meminfo),$(kb VmRSS /proc/$pid/status),$pss"
    [ "$i" = 3 ] || sleep 1
  done
done
echo "mpc_memprobe: done; $out holds the log" >&2
