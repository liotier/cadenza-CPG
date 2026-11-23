/* The compatibility harness's loader (docs/development/specs/00-infrastructure.md, 3.2), built by tests/compat/run.sh
 * against the official engine's sources: loads a state saved by this fork as a new plugin instance would, and saves it
 * again.
 *   loader IN OUT    exit status 1 when the official engine refuses the state (seq_state_load returns 0) */
#include <stdio.h>
#include <stdlib.h>
#include "seq_core.h"

static void quiet(void *ctx, int status, int note, int vel, int frame) { (void)ctx; (void)status; (void)note; (void)vel; (void)frame; }

static char in[SEQ_STATE_MAX + 2], out[SEQ_STATE_MAX];

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "usage: loader IN OUT\n"); return 2; }
    FILE *f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "loader: cannot read %s\n", argv[1]); return 2; }
    size_t n = fread(in, 1, sizeof in - 1, f);
    fclose(f);
    if (n > SEQ_STATE_MAX - 1) { fprintf(stderr, "loader: %s is over the official state size\n", argv[1]); return 1; }
    in[n] = 0;
    seq_t *s = calloc(1, sizeof *s);
    if (!s) return 2;
    seq_init(s, quiet, NULL);
    seq_init_preset(s);
    if (seq_state_load(s, in) != 1) { fprintf(stderr, "loader: the official engine refuses %s\n", argv[1]); return 1; }
    seq_state_save(s, out, sizeof out);
    f = fopen(argv[2], "wb");
    if (!f || fputs(out, f) == EOF || fclose(f)) { fprintf(stderr, "loader: cannot write %s\n", argv[2]); return 2; }
    free(s);
    return 0;
}
