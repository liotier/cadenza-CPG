/* The compatibility harness's writer (docs/development/specs/00-infrastructure.md, 3.2), built against this fork's
 * engine: plays a fixed script of setters through the public seq_* API, one scenario per state, and saves each state
 * as <out>/<scenario>.txt, and this fork's own re-save of it (loaded in a new instance, saved again) as
 * <out>/<scenario>.fork.txt. tests/compat/loader.c, built against the official engine, loads each <scenario>.txt and saves it
 * again; tests/compat/run.sh compares that with <scenario>.fork.txt: the official engine must read the state as this fork does.
 * The re-save, not the first save, is the reference, because a save-load-save cycle is not byte-stable in 0.28.1 itself: a loop
 * shortened after its drawn bars were filled saves the hidden bars' rhythms once (up to seq_part_t.filled), and a reload saves
 * only the loop's. The writer checks that this fork's re-save is a fixed point (a second cycle gives the same bytes).
 *   writer OUTDIR    prints one scenario name per line; exit status 1 on any failure
 * A later step adds its scenarios at the end of the table, never in the middle, and lists the differences its
 * specification permits in tests/compat/expected.txt. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "seq_core.h"
#include "presets.h"

static void quiet(void *ctx, int status, int note, int vel, int frame) { (void)ctx; (void)status; (void)note; (void)vel; (void)frame; }

static int fails;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "writer: FAIL %s: %s\n", name, #c); fails++; } } while (0)

typedef void (*scenario_fn)(seq_t *s, const char *name);

static void sc_init(seq_t *s, const char *name) { (void)s; (void)name; }

static void sc_key_scale(seq_t *s, const char *name) {
    (void)name;
    seq_set_key(s, 3);
    seq_set_scale(s, SCALE_HARM_MINOR);
    seq_set_bars(s, 2);
    seq_set_chord_len(s, 8);
}

static void sc_prog_preset(seq_t *s, const char *name) {
    (void)name;
    seq_set_key(s, 9);
    seq_set_scale(s, SCALE_MINOR);
    seq_set_prog_preset(s, 3);
}

static void sc_borrowed(seq_t *s, const char *name) {
    seq_set_slot(s, 1, 0, 1, COL_TRIAD);   /* src 1: the first borrowed chord (its own degree) */
    seq_set_slot(s, 2, 0, 3, COL_7TH);     /* src 3: the third, as a seventh */
    CHECK(seq_slot_src(s, 1) == 1 && seq_slot_src(s, 2) == 3);
}

static void sc_colour(seq_t *s, const char *name) {
    seq_set_slot(s, 0, 0, 0, COL_9TH);
    seq_set_slot(s, 3, 4, 0, COL_SUS4);
    CHECK(seq_slot_colour(s, 0) == COL_9TH && seq_slot_deg(s, 3) == 4);
}

static void sc_change_point(seq_t *s, const char *name) {
    CHECK(seq_cp_set(s, 0, 8, 4, 0, 0) == 1);   /* the V at beat 3 of bar 1 */
    CHECK(seq_cp_set(s, 1, 12, 0, 2, 1) == 1);  /* a borrowed chord at beat 4 of bar 2 */
    CHECK(seq_cp_count(s, 0) == 1 && seq_cp_count(s, 1) == 1);
}

static void sc_bar_edit(seq_t *s, const char *name) {
    seq_rbar_open(s, LANE_CHORDS, 1);
    seq_rbar_select(s, LANE_CHORDS, 4);
    CHECK(seq_rbar_chord(s, 3, 0) != RBC_NONE);
    seq_rbar_done(s, LANE_CHORDS);
}

static void sc_song_part(seq_t *s, const char *name) {
    seq_select_part(s, 1);
    seq_set_key(s, 7);
    seq_set_slot(s, 0, 5, 0, COL_7TH);
    seq_select_part(s, 2);
    seq_set_bars(s, 8);
    seq_select_part(s, 0);
    CHECK(seq_arr_set_form(s, 0) == 1);
    CHECK(seq_part_used(s, 1) && seq_part_used(s, 2));
}

static void sc_lock(seq_t *s, const char *name) {
    seq_toggle_slot_lock(s, 2);
    CHECK(seq_slot_locked(s, 2));
    seq_set_prog_preset(s, 5);   /* every lock cleared: lock again after */
    seq_toggle_slot_lock(s, 1);
    CHECK(seq_slot_locked(s, 1));
}

static const struct { const char *name; scenario_fn fn; } scenarios[] = {
    {"init", sc_init},
    {"key-scale", sc_key_scale},
    {"prog-preset", sc_prog_preset},
    {"borrowed", sc_borrowed},
    {"colour", sc_colour},
    {"change-point", sc_change_point},
    {"bar-edit", sc_bar_edit},
    {"song-part", sc_song_part},
    {"lock", sc_lock},
};

static char first[SEQ_STATE_MAX], resave[SEQ_STATE_MAX], again[SEQ_STATE_MAX];

static int save_file(const char *dir, const char *name, const char *suffix, const char *text) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s%s.txt", dir, name, suffix);
    FILE *f = fopen(path, "wb");
    if (!f || fputs(text, f) == EOF || fclose(f)) { fprintf(stderr, "writer: cannot write %s\n", path); return 0; }
    return 1;
}

/* A new instance of this fork, as the plugin makes one, loads text and saves it into out */
static void reload(seq_t *t, const char *name, const char *text, char *out, int len) {
    seq_init(t, quiet, NULL);
    seq_init_preset(t);
    CHECK(seq_state_load(t, text) == 1);
    CHECK(seq_state_save(t, out, len) > 0);
}

/* Saves s and its re-save; checks the re-save is a fixed point; writes both files */
static void emit_state(seq_t *s, seq_t *t, const char *dir, const char *name) {
    int n = seq_state_save(s, first, sizeof first);
    CHECK(n > 0 && n < (int)sizeof first);
    CHECK(seq_unsaved_parts(s) == 0);
    reload(t, name, first, resave, sizeof resave);
    reload(t, name, resave, again, sizeof again);
    CHECK(strcmp(resave, again) == 0);
    if (!save_file(dir, name, "", first) || !save_file(dir, name, ".fork", resave)) fails++;
    printf("%s\n", name);
}

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "usage: writer OUTDIR\n"); return 2; }
    seq_t *s = calloc(1, sizeof *s), *t = calloc(1, sizeof *t);
    if (!s || !t) return 1;
    for (size_t i = 0; i < sizeof scenarios / sizeof scenarios[0]; i++) {
        seq_init(s, quiet, NULL);
        seq_init_preset(s);
        scenarios[i].fn(s, scenarios[i].name);
        emit_state(s, t, argv[1], scenarios[i].name);
    }
    for (int i = 0; i < PRESETS_COUNT; i++) {   /* each genre preset, as the PRESET menu loads it */
        char name[32];
        snprintf(name, sizeof name, "genre-%d", i + 1);
        seq_init(s, quiet, NULL);
        seq_init_preset(s);
        CHECK(seq_song_load(s, preset_chunk(i)) == 1);
        emit_state(s, t, argv[1], name);
    }
    free(s);
    free(t);
    return fails ? 1 : 0;
}
