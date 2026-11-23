/* Offline test of the fork's chromatic progression path (docs/development/specs/02-chromatic-progression-path.md): every writer of a
 * slot's or a change point's chord accepts the chromatic range (src 4..THEORY_SRC_MAX, degree 0), and what it writes plays, follows,
 * undoes, survives a chord-length change, a COPY TO and a save. Through the public seq_* API only, and a randomised sweep over the
 * whole range. Run by tests/run_tests.sh under ASan/UBSan. Exit 1 on failure. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "seq_core.h"

static int fails;
#define CHECK(c, ...) do { int ok_ = (c) != 0; printf("%s ", ok_ ? "ok  " : "FAIL"); printf(__VA_ARGS__); printf("\n"); if (!ok_) fails++; } while (0)

#define MAX_EV 16384
typedef struct { double beat; int status, note; } ev_t;
static ev_t evs[MAX_EV];
static int nev;
static double at_beat;
static void rec(void *ctx, int status, int note, int vel, int frame) {
    (void)ctx, (void)frame;
    if ((status & 0xF0) == 0x90 && vel && nev < MAX_EV) evs[nev++] = (ev_t){at_beat, status, note};
}

static void play(seq_t *s, double beats) {
    const int frames = 128;
    double step = frames * 120.0 / (60.0 * 44100.0);
    nev = 0;
    for (double ppq = 0; ppq < beats; ppq += step) { at_beat = ppq; seq_block(s, ppq, 120, 1, frames); }
    seq_block(s, -1, 120, 0, frames);
}

static void fresh(seq_t *s) { seq_init(s, rec, NULL); seq_init_preset(s); seq_set_rate(s, 44100); }

/* The pitch classes a lane (MIDI channel ch, 1..) plays from beat `from` to `to`, as a 12-bit mask */
static unsigned pcs_played(int ch, double from, double to) {
    unsigned m = 0;
    for (int i = 0; i < nev; i++)
        if ((evs[i].status & 15) == ch - 1 && evs[i].beat >= from && evs[i].beat < to) m |= 1u << (evs[i].note % 12);
    return m;
}

/* The three chords of the specification, in C major: Db7, Ab, F# diminished */
static const struct { const char *name; int root, quality, colour; unsigned pcs; } CH[3] = {
    {"Db7", 1, CQ_DOM, COL_7TH, (1u << 1) | (1u << 5) | (1u << 8) | (1u << 11)},
    {"Ab", 8, CQ_MAJOR, COL_TRIAD, (1u << 8) | (1u << 0) | (1u << 3)},
    {"F#dim", 6, CQ_DIM, COL_TRIAD, (1u << 6) | (1u << 9) | (1u << 0)},
};

static int src_of(int k) { return theory_chroma_src(CH[k].root, CH[k].quality); }

/* The slot holds chord k: its source, degree 0, colour, numeral, and the pitch classes it voices */
static int slot_is(const seq_t *s, int slot, int k) {
    int n[THEORY_CHORD_MAX], c = seq_slot_notes(s, slot, 0, n);
    char num[16], want[16];
    unsigned m = 0;
    for (int i = 0; i < c; i++) m |= 1u << (n[i] % 12);
    seq_slot_numeral(s, slot, num, sizeof num);
    theory_numeral(seq_key(s), seq_scale(s), theory_slot_scale(seq_scale(s), src_of(k)), 0, want, sizeof want);
    return seq_slot_src(s, slot) == src_of(k) && seq_slot_deg(s, slot) == 0 && seq_slot_colour(s, slot) == CH[k].colour && m == CH[k].pcs &&
           !strcmp(num, want);
}

static char saved[SEQ_STATE_MAX], again[SEQ_STATE_MAX], third[SEQ_STATE_MAX];

/* s saved, loaded into a new instance t: the same slots and change points, and t's save a fixed point (a save, loaded and saved
 * again, gives the same bytes). Not s's own bytes: in 0.28.1 a loop shortened after its drawn bars were filled saves the hidden
 * bars' rhythms once (tests/compat/writer.c) */
static int round_trip(seq_t *s, seq_t *t) {
    static seq_t u;
    seq_cp_t a, b;
    int same = 1;
    seq_state_save(s, saved, sizeof saved);
    fresh(t);
    if (seq_state_load(t, saved) != 1) return 0;
    for (int i = 0; i < seq_slots(s); i++)
        same &= seq_slot_src(s, i) == seq_slot_src(t, i) && seq_slot_deg(s, i) == seq_slot_deg(t, i) && seq_slot_colour(s, i) == seq_slot_colour(t, i);
    for (int bar = 0; bar < SEQ_MAX_BARS; bar++) {
        same &= seq_cp_count(s, bar) == seq_cp_count(t, bar);
        for (int k = 0; k < seq_cp_count(s, bar); k++)
            same &= seq_cp_get(s, bar, k, &a) && seq_cp_get(t, bar, k, &b) && !memcmp(&a, &b, sizeof a);
    }
    seq_state_save(t, again, sizeof again);
    fresh(&u);
    if (seq_state_load(&u, again) != 1) return 0;
    seq_state_save(&u, third, sizeof third);
    return same && !strcmp(again, third);
}

/* 1. seq_set_progression, a phrase (fill_phrase) and the whole loop (set_prog) */
static void test_progression(seq_t *s, seq_t *t) {
    int deg[4] = {0, 3, 5, 6};   /* the degrees passed are ignored for the chromatic chords */
    unsigned char src[4] = {0, (unsigned char)src_of(0), (unsigned char)src_of(1), (unsigned char)src_of(2)};
    unsigned char col[4] = {COL_TRIAD, COL_7TH, COL_TRIAD, COL_TRIAD};
    fresh(s);
    seq_set_progression(s, 2, deg, src, col, 1);   /* a phrase of 2 over 4 slots */
    CHECK(seq_slot_src(s, 0) == 0 && slot_is(s, 1, 0) && seq_slot_src(s, 2) == 0 && slot_is(s, 3, 0), "a phrase keeps Db7 (src %d, degree 0)", src_of(0));
    fresh(s);
    seq_set_progression(s, seq_slots(s), deg, src, col, 1);   /* the whole loop */
    CHECK(seq_slots(s) == 4 && slot_is(s, 1, 0) && slot_is(s, 2, 1) && slot_is(s, 3, 2), "the whole loop keeps Db7, Ab and F#dim");
    play(s, 16);
    CHECK((pcs_played(1, 4, 8) & CH[0].pcs) == CH[0].pcs && !(pcs_played(1, 4, 8) & ~CH[0].pcs), "bar 2 sounds Db7's notes and no other");
    CHECK(pcs_played(2, 4, 8) & (1u << 1), "the bass plays Db under Db7");
    CHECK(pcs_played(2, 8, 12) & (1u << 8), "the bass plays Ab under Ab");
    seq_undo(s);
    CHECK(seq_slot_src(s, 1) == 0 && seq_slot_src(s, 2) == 0, "undo restores the chords before");
    seq_set_progression(s, seq_slots(s), deg, src, col, 1);
    CHECK(round_trip(s, t) && !strcmp(saved, again) && slot_is(t, 1, 0) && slot_is(t, 2, 1) && slot_is(t, 3, 2), "a save and load gives the same bytes and chords");
    seq_set_chord_len(s, 8);   /* half bars: each chord in its own place, now two slots long */
    CHECK(seq_slots(s) == 8 && slot_is(s, 2, 0) && slot_is(s, 3, 0) && slot_is(s, 4, 1) && slot_is(s, 5, 1) && slot_is(s, 6, 2) && slot_is(s, 7, 2),
          "a chord-length change keeps the sources in place");
    seq_set_key(s, 2);   /* D: every chromatic chord moves with the key */
    CHECK(seq_slot_src(s, 2) == src_of(0) && seq_slot_deg(s, 2) == 0, "a key change keeps the source, degree 0");
    seq_set_scale(s, SCALE_MINOR);
    CHECK(seq_slot_src(s, 2) == src_of(0) && seq_slot_deg(s, 2) == 0, "a scale change keeps the source, degree 0 (no borrow degree written)");
    deg[1] = 9, src[1] = THEORY_SRC_MAX + 1;   /* past the range: the scale's degree */
    fresh(s);
    seq_set_progression(s, seq_slots(s), deg, src, col, 1);
    CHECK(seq_slot_src(s, 1) == 0 && seq_slot_deg(s, 1) == 2, "a source past the range reads as the scale's (degree 9 wraps to 2)");
}

/* 2. seq_set_slot */
static void test_slot(seq_t *s, seq_t *t) {
    fresh(s);
    seq_set_slot(s, 2, 5, src_of(1), COL_TRIAD);
    CHECK(slot_is(s, 2, 1) && seq_slot_locked(s, 2), "a pick sets Ab (degree 0 whatever was passed) and locks it");
    seq_set_slot(s, 3, 0, src_of(0), COL_7TH);
    CHECK(slot_is(s, 3, 0), "a pick sets Db7");
    seq_undo(s);
    CHECK(seq_slot_src(s, 3) == 0 && slot_is(s, 2, 1), "undo takes back the last pick only");
    seq_set_slot(s, 1, 0, THEORY_SRC_MAX + 1, COL_TRIAD);
    CHECK(seq_slot_src(s, 1) == 0, "a pick past the range is refused");
    CHECK(round_trip(s, t) && slot_is(t, 2, 1) && seq_slot_locked(t, 2), "a locked chromatic slot survives a save");
}

/* 3. change points: seq_cp_set, the bar editor's chord (seq_rbar_chord), the tweak check, the load, COPY TO */
static void test_points(seq_t *s, seq_t *t) {
    seq_cp_t c;
    fresh(s);
    CHECK(seq_cp_set(s, 0, 8, 4, src_of(2), 0) == 1, "a change point takes F#dim");
    CHECK(seq_cp_get(s, 0, 0, &c) && c.src == src_of(2) && c.deg == 0 && c.step == 8, "the point holds src %d, degree 0", src_of(2));
    int v[THEORY_CHORD_MAX], k = seq_part_step_notes(s, 0, LANE_CHORDS, 0, 8, v);
    unsigned m = 0;
    for (int i = 0; i < k; i++) m |= 1u << (v[i] % 12);
    CHECK(m == CH[2].pcs, "the Chords lane's step 9 holds F#dim");
    CHECK(round_trip(s, t) && seq_cp_count(t, 0) == 1 && seq_cp_get(t, 0, 0, &c) && c.src == src_of(2) && c.deg == 0,
          "the point survives a save and load ('%c' in .cp)", '0' + src_of(2));
    CHECK(seq_tweak_point_ok(src_of(0), COL_7TH) && seq_tweak_point_ok(THEORY_SRC_MAX, COL_TRIAD) && !seq_tweak_point_ok(THEORY_SRC_MAX + 1, COL_TRIAD),
          "THIS HIT ONLY accepts the chromatic range, not past it");
    fresh(s);
    seq_rbar_open(s, LANE_CHORDS, 1);
    seq_rbar_select(s, LANE_CHORDS, 4);
    int r = seq_rbar_chord(s, 0, src_of(0));
    seq_rbar_done(s, LANE_CHORDS);
    CHECK(r == RBC_POINT && seq_cp_count(s, 1) == 1 && seq_cp_get(s, 1, 0, &c) && c.src == src_of(0) && c.deg == 0,
          "the bar editor's chord pick makes a Db point (got %d)", r);
    seq_rbar_open(s, LANE_CHORDS, 2);
    seq_rbar_select(s, LANE_CHORDS, 0);
    r = seq_rbar_chord(s, 0, src_of(1));
    seq_rbar_done(s, LANE_CHORDS);
    CHECK(r == RBC_SLOT && slot_is(s, 2, 1), "picked on a slot's first step, it sets the slot to Ab (got %d)", r);
    seq_rbar_open(s, LANE_CHORDS, 1);
    seq_rcopy_set_what(s, LANE_CHORDS, 1);   /* + CHORDS */
    seq_rbar_copy(s, LANE_CHORDS, 1u << 3);
    seq_rbar_done(s, LANE_CHORDS);
    CHECK(seq_cp_count(s, 3) == 1 && seq_cp_get(s, 3, 0, &c) && c.src == src_of(0) && c.deg == 0, "COPY TO + CHORDS copies the chromatic point to bar 4");
    /* a state written by hand, as an older or newer build might: a chromatic point with a stray degree loads with degree 0 */
    seq_state_save(s, saved, sizeof saved);
    char *cp = strstr(saved, "A.cp=");
    CHECK(cp != NULL, "the state has the Verse's change points");
    if (cp) {
        cp[5 + 2] = '3';   /* the first point's degree */
        fresh(t);
        CHECK(seq_state_load(t, saved) == 1 && seq_cp_get(t, 1, 0, &c) && c.src == src_of(0) && c.deg == 0, "a chromatic point's degree loads as 0");
    }
}

/* 4. A randomised sweep: progressions and points over the whole range, colours, keys and scales, played, saved and loaded */
static unsigned rng = 12345u;
static int rnd(int n) { rng = rng * 1103515245u + 12345u; return (int)((rng >> 16) % (unsigned)n); }

static void test_sweep(seq_t *s, seq_t *t) {
    int bad = 0, rounds = 0;
    for (int round = 0; round < 60; round++) {
        int deg[64], n;
        unsigned char src[64], col[64];
        fresh(s);
        seq_set_key(s, rnd(12));
        seq_set_scale(s, rnd(SCALE_COUNT));
        seq_set_bars(s, 1 << rnd(4));
        seq_set_chord_len(s, 4 << rnd(3));
        n = seq_slots(s);
        for (int i = 0; i < n; i++) deg[i] = rnd(7), src[i] = (unsigned char)rnd(THEORY_SRC_MAX + 1), col[i] = (unsigned char)rnd(COL_COUNT);
        seq_set_progression(s, n, deg, src, col, 1);
        for (int k = 0; k < 4; k++) seq_cp_set(s, rnd(4), 1 + rnd(15), rnd(7), rnd(THEORY_SRC_MAX + 1), rnd(5));
        seq_set_slot(s, rnd(n), rnd(7), rnd(THEORY_SRC_MAX + 1), rnd(COL_COUNT));
        play(s, 8);
        for (int i = 0; i < n; i++) {
            char name[32];
            int b = seq_slot_src(s, i) > THEORY_SRC_MAX || (seq_slot_src(s, i) > THEORY_NBORROW && seq_slot_deg(s, i) != 0) || seq_slot_name(s, i, name, sizeof name) <= 0;
            if (b && !bad) printf("     round %d slot %d: src %d deg %d name '%s'\n", round, i, seq_slot_src(s, i), seq_slot_deg(s, i), name);
            bad |= b;
        }
        if (!round_trip(s, t)) {
            if (!bad) printf("     round %d: the save does not round-trip\n%s\n%s\n", round, saved, again);
            bad = 1;
        }
        seq_set_scale(s, rnd(SCALE_COUNT));
        play(s, 4);
        rounds++;
    }
    CHECK(!bad, "%d random loops over src 0..%d: sources in range, chromatic degrees 0, every slot named, saves round-trip", rounds, THEORY_SRC_MAX);
}

int main(void) {
    static seq_t s, t;
    test_progression(&s, &t);
    test_slot(&s, &t);
    test_points(&s, &t);
    test_sweep(&s, &t);
    printf(fails ? "\nFAILED: %d\n" : "\nPASSED\n", fails);
    return fails != 0;
}
