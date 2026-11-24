/* Offline test of the part's palette (fork, docs/development/specs/05-part-palette.md, §5): loading a catalogue palette writes its starting
 * order as Feel's presets write theirs, keeping locks, in one undo step with the palette; the palette is saved by value and survives a save
 * and load as a fixed point; a scale change keeps the loaded chords' sound; a first-used part copies the Verse's; editing never removes it;
 * the front end shows the scale's palette for a part without one and finds the nearest catalogue palette; six loaded parts fit the state.
 * Through the public API (seq_*, palette_*, cpg_*). Run by tests/run_tests.sh under ASan and UBSan. Exit 1 on failure. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "palette.h"

static int fails;
#define CHECK(c, ...) do { int ok_ = (c) != 0; printf("%s ", ok_ ? "ok  " : "FAIL"); printf(__VA_ARGS__); printf("\n"); if (!ok_) fails++; } while (0)

static void quiet(void *ctx, int status, int note, int vel, int frame) { (void)ctx, (void)status, (void)note, (void)vel, (void)frame; }
static void fresh(seq_t *s) { seq_init(s, quiet, NULL); seq_init_preset(s); seq_set_rate(s, 44100); }

static uint32_t fnv1a(const char *t) {
    uint32_t h = 0x811c9dc5u;
    for (; *t; t++) h = (h ^ (unsigned char)*t) * 0x01000193u;
    return h;
}

static int entry_of(const char *genre_template) {
    const cpg_entry_t *e = cpg_find(fnv1a(genre_template));
    for (int i = 0; e && i < cpg_count(); i++)
        if (cpg_entry(i) == e) return i;
    return -1;
}

/* the slots' names, one a bar (the loaded loops are a chord a bar), spelled as the slot labels are */
static void names(const seq_t *s, char *out, int len) {
    out[0] = 0;
    for (int i = 0; i < seq_slots(s); i++) {
        char nm[24];
        theory_chord_name_s(seq_key(s), seq_scale(s), theory_slot_scale(seq_scale(s), seq_slot_src(s, i)), seq_slot_deg(s, i), seq_slot_colour(s, i),
                            nm, sizeof nm);
        if (i) strncat(out, " ", (size_t)len - strlen(out) - 1);
        strncat(out, nm, (size_t)len - strlen(out) - 1);
    }
}

/* the pitch classes of slot i, relative to the key */
static unsigned slot_pcs(const seq_t *s, int i) {
    int n[THEORY_CHORD_MAX], k = theory_chord_notes(seq_key(s), theory_slot_scale(seq_scale(s), seq_slot_src(s, i)), seq_slot_deg(s, i), seq_slot_colour(s, i), n);
    unsigned m = 0;
    while (k--) m |= 1u << ((n[k] - seq_key(s) + 120) % 12);
    return m;
}

#define PR "Pop/Rock|I\xE2\x80\x94V\xE2\x80\x94vi\xE2\x80\x94IV"

/* verification 1: the starting orders of spec 04 §3.3, in a major and a minor project; locks kept; undo */
static void test_load(seq_t *s) {
    static const struct { const char *entry; int key, scale, bars; const char *names; } T[] = {
        {PR, 0, SCALE_MAJOR, 4, "C G Am F"},
        {PR, 7, SCALE_MAJOR, 4, "G D Em C"},
        {"Jazz/Functional|ii\xE2\x80\x94V\xE2\x80\x94I", 2, SCALE_MAJOR, 4, "Em A D D"},
        {"Pop/Rock|I\xE2\x80\x94V\xE2\x80\x94vi\xE2\x80\x94iii\xE2\x80\x94IV", 0, SCALE_MAJOR, 8, "C C C C G Am Em F"},
        {"Blues/Soul|12-bar-blues", 5, SCALE_MAJOR, 16, "F7 F7 F7 F7 F7 F7 F7 F7 Bb7 Bb7 F7 F7 C7 Bb7 F7 C7"},
        {"Film/Cinematic|i\xE2\x80\x94i\xE2\x99\xAF" "5\xE2\x80\x94i6\xE2\x80\x94i\xE2\x99\xAF" "5", 9, SCALE_MINOR, 4, "Am Am#5 Am6 Am#5"},
    };
    char got[256];
    for (size_t t = 0; t < sizeof T / sizeof T[0]; t++) {
        int e = entry_of(T[t].entry);
        fresh(s);
        seq_set_key(s, T[t].key);
        seq_set_scale(s, T[t].scale);
        int ok = e >= 0 && palette_load_entry(s, e);
        names(s, got, sizeof got);
        CHECK(ok && seq_bars(s) == T[t].bars && seq_slots(s) == T[t].bars && !strcmp(got, T[t].names) && seq_palette_id(s) == cpg_entry(e)->id,
              "%s loads %d bars: %s (got %d bars: %s)", cpg_nickname(cpg_entry(e)), T[t].bars, T[t].names, seq_bars(s), got);
    }
    fresh(s);   /* a lock kept, an undo step; from 1-6-4-5 (a new instance's own progression is I-V-vi-IV, the order loaded) */
    seq_set_prog_preset(s, 3);
    seq_set_slot(s, 1, 5, 0, COL_7TH);   /* Am7, locked: C Am7 F G */
    int undo = s->undo_n;
    palette_load_entry(s, entry_of(PR));
    names(s, got, sizeof got);
    CHECK(!strcmp(got, "C Am7 Am F") && seq_slot_locked(s, 1) && s->undo_n == undo + 1, "the locked slot stays (C Am7 Am F, got %s); one undo step", got);
    seq_undo(s);
    names(s, got, sizeof got);
    CHECK(!seq_palette(s, NULL) && !strcmp(got, "C Am7 F G"), "undo takes back the slots and the palette together (C Am7 F G, got %s)", got);
    theory_palette_chord_t bad[1] = {{12, 0, 0, 0, 0, 0}};
    unsigned char order[3] = {0, 0, 0};
    CHECK(!seq_palette_load(s, 1, bad, 1, order, 1) && !seq_palette_load(s, 1, bad, 0, order, 1) && !palette_load_entry(s, -1) &&
          !seq_palette(s, NULL), "bad arguments: nothing");
    bad[0].root = 0;
    CHECK(!seq_palette_load(s, 1, bad, 1, order, 3), "an order of 3 bars: nothing (1, 2, 4, 8 or 16)");
}

/* verification 2: save and load as fixed points; no `.pl` for a part without a palette */
static void test_save(seq_t *s, seq_t *t) {
    static char a[SEQ_STATE_MAX], b[SEQ_STATE_MAX];
    theory_palette_chord_t x[THEORY_PALETTE_MAX], y[THEORY_PALETTE_MAX];
    fresh(s);
    seq_state_save(s, a, sizeof a);
    CHECK(!strstr(a, ".pl="), "a part without a palette saves none");
    palette_load_entry(s, entry_of("Film/Cinematic|i\xE2\x80\x94i\xE2\x99\xAF" "5\xE2\x80\x94i6\xE2\x80\x94i\xE2\x99\xAF" "5"));
    seq_state_save(s, a, sizeof a);
    fresh(t);
    int n = seq_palette(s, x), ok = seq_state_load(t, a) == 1 && seq_palette(t, y) == n && n == 16 && !memcmp(x, y, sizeof x[0] * (size_t)n) &&
                                    seq_palette_id(t) == seq_palette_id(s);
    seq_state_save(t, b, sizeof b);
    CHECK(ok && !strcmp(a, b) && strstr(a, "|A.pl="), "the palette survives a save and load, chord for chord, the save a fixed point");
    char *pl = strstr(a, "|A.pl=");
    if (pl) pl[6] = 'z';   /* a bad identifier digit: the field is ignored, the load not refused */
    fresh(t);
    CHECK(seq_state_load(t, a) == 1 && !seq_palette(t, NULL), "a bad `.pl` is ignored whole, the state still loads");
}

/* verification 3: a scale change keeps every loaded chord's pitch classes and the palette */
static void test_scale(seq_t *s) {
    theory_palette_chord_t before[THEORY_PALETTE_MAX], after[THEORY_PALETTE_MAX];
    unsigned pcs[SEQ_MAX_PROG];
    int ok = 1, n;
    for (int scale = 0; scale < SCALE_COUNT; scale++) {
        fresh(s);
        palette_load_entry(s, entry_of("Blues/Soul|12-bar-blues"));
        seq_cp_t c;
        n = seq_palette(s, before);
        for (int i = 0; i < seq_slots(s); i++) pcs[i] = slot_pcs(s, i);
        seq_set_scale(s, scale);
        for (int i = 0; i < seq_slots(s); i++) ok &= slot_pcs(s, i) == pcs[i];
        ok &= seq_palette(s, after) == n && !memcmp(before, after, sizeof before[0] * (size_t)n) && !seq_cp_get(s, 0, 0, &c);
    }
    CHECK(ok, "a scale change, to every scale, keeps the blues' chords as they sound and the palette as it was");
    fresh(s);
    seq_set_slot(s, 0, 1, 0, COL_TRIAD);   /* ii, not from a palette: it follows the scale as ever */
    unsigned was = slot_pcs(s, 0);
    seq_set_scale(s, SCALE_MINOR);
    CHECK(slot_pcs(s, 0) != was, "a part without a palette: its degrees follow the scale, as before");
}

/* verification 5, S05-D6: a first-used part copies the Verse's palette; editing keeps it; clear is its own undo step */
static void test_parts(seq_t *s) {
    fresh(s);
    palette_load_entry(s, entry_of(PR));
    uint32_t id = seq_palette_id(s);
    seq_select_part(s, 1);
    CHECK(seq_part(s) == 1 && seq_palette_id(s) == id, "a part used for the first time copies the Verse's palette");
    seq_set_slot(s, 0, 3, 0, COL_7TH);
    seq_set_chord_len(s, 8);
    CHECK(seq_palette_id(s) == id, "editing the slots keeps the palette");
    int undo = s->undo_n;
    seq_palette_clear(s);
    CHECK(!seq_palette(s, NULL) && !seq_palette_id(s) && s->undo_n == undo + 1, "clear removes it, one undo step");
    seq_palette_clear(s);
    CHECK(s->undo_n == undo + 1, "clearing no palette is no step");
    seq_undo(s);
    CHECK(seq_palette_id(s) == id, "undo brings it back");
}

/* S05-D3 and verification 4: what the front end shows; the nearest catalogue palette */
static void test_front_end(seq_t *s) {
    palette_pad_t p[THEORY_PALETTE_MAX];
    fresh(s);
    seq_set_scale(s, SCALE_DORIAN);
    int n = palette_shown(s, p), ok = n == 16;
    const cpg_scale_chord_t *sp = cpg_scale_palette(SCALE_DORIAN);
    for (int k = 0; ok && k < n; k++) ok = p[k].deg == sp[k].deg && p[k].src == sp[k].src && p[k].colour == sp[k].colour;
    CHECK(ok, "a part without a palette shows its scale's (Dorian's 16, the tonic's 7sus4 among them)");
    palette_load_entry(s, entry_of(PR));
    n = palette_shown(s, p);
    CHECK(n == 16 && p[0].start && p[0].src > THEORY_NBORROW && p[1].src == 2 && p[1].deg == 4 && p[2].src > THEORY_NBORROW && p[3].src == 0 &&
          p[3].deg == 3, "with one, its chords as the project's scale places them: in C Dorian, C and Am chromatic, G its borrowed V, F its IV");
    /* the nearest: deterministic, within the family, and no entry of the family shares more of the part's chords */
    fresh(s);
    palette_load_entry(s, entry_of(PR));
    seq_palette_clear(s);   /* the chords stay: C G Am F */
    CHECK(palette_nearest(s) == entry_of(PR) && palette_nearest(s) == palette_nearest(s), "C G Am F in C major: the nearest is Axis of Awesome, the first that holds all four");
    int all = 1;
    unsigned r = 7u;
    for (int round = 0; round < 40; round++) {
        int deg[16], best, shared[2] = {0, 0};
        unsigned char src[16], col[16];
        fresh(s);
        seq_set_scale(s, (int)(round % SCALE_COUNT));
        for (int i = 0; i < seq_slots(s); i++)
            r = r * 1103515245u + 12345u, deg[i] = (int)(r >> 16) % 7, src[i] = (unsigned char)((r >> 20) % 4 == 0 ? 1 : 0), col[i] = (unsigned char)((r >> 24) % 3);
        seq_set_progression(s, seq_slots(s), deg, src, col, 1);
        best = palette_nearest(s);
        if (best < 0) continue;
        all &= cpg_entry(best)->minor == (theory_triad_quality(0, seq_scale(s), 0) == 1);
        for (int e = 0; e < cpg_count(); e++) {   /* no entry of the family holds more: load each and count */
            if (cpg_entry(e)->minor != cpg_entry(best)->minor) continue;
            int count = 0;
            for (int i = 0; i < seq_slots(s); i++) {
                int dup = 0, in = 0;
                for (int j = 0; j < i; j++) dup |= seq_slot_deg(s, j) == seq_slot_deg(s, i) && seq_slot_src(s, j) == seq_slot_src(s, i) && seq_slot_colour(s, j) == seq_slot_colour(s, i);
                if (dup) continue;
                int nt[THEORY_CHORD_MAX], k = theory_chord_notes(0, theory_slot_scale(seq_scale(s), seq_slot_src(s, i)), seq_slot_deg(s, i), seq_slot_colour(s, i), nt), m = 0;
                while (k--) m |= 1 << nt[k] % 12;
                for (int c = 0; c < CPG_PADS; c++) {
                    cpg_chord_t x = cpg_chord(cpg_entry(e), c);
                    int nx[THEORY_CHORD_MAX], kx = theory_chord_notes(0, theory_slot_scale(0, cpg_chord_src(x)), 0, x.colour, nx), mx = 0;
                    while (kx--) mx |= 1 << nx[kx] % 12;
                    in |= mx == m && x.root == theory_degree_pc(0, theory_slot_scale(seq_scale(s), seq_slot_src(s, i)), seq_slot_deg(s, i));
                }
                count += in;
            }
            if (e == best) shared[0] = count;
            if (count > shared[1] || (count == shared[1] && e < best)) shared[1] = count;
        }
        all &= shared[0] == shared[1];
    }
    CHECK(all, "40 random progressions: the nearest is of the scale's family, and no entry of it holds more of the part's chords");
}

/* §4: six parts with loaded palettes fit the state whole */
static void test_size(seq_t *s, seq_t *t) {
    static char a[SEQ_STATE_MAX];
    fresh(s);
    for (int part = 0; part < 6; part++) {
        seq_select_part(s, part);
        seq_set_bars(s, 16);
        palette_load_entry(s, entry_of("Blues/Soul|12-bar-blues") + part);
    }
    seq_select_part(s, 0);
    int len = seq_state_save(s, a, sizeof a), used = 0;
    fresh(t);
    for (int part = 0; part < 6; part++) used += seq_part_used(s, part);
    CHECK(len > 0 && seq_unsaved_parts(s) == 0 && seq_state_load(t, a) == 1 && used == 6,
          "six parts of 16 bars, each with a palette, save whole: %d bytes of %d", len, SEQ_STATE_MAX);
}

int main(void) {
    seq_t *s = calloc(1, sizeof *s), *t = calloc(1, sizeof *t);
    if (!s || !t) return 1;
    test_load(s);
    test_save(s, t);
    test_scale(s);
    test_parts(s);
    test_front_end(s);
    test_size(s, t);
    free(s), free(t);
    printf(fails ? "\nFAILED: %d\n" : "\nPASSED\n", fails);
    return fails != 0;
}
