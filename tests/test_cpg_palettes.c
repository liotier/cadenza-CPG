/* Offline test of the palette table (fork, docs/development/specs/04-palette-table.md, §5): every catalogue chord voices the CPG pad's
 * pitch classes (decision 12's fifth rule aside, S04-D6) as Cadenza's theory builds it, in every key; the starting orders as the
 * generator laid them out (S04-D9); the scale palettes' chords their scale's own; the lookups; and spot checks against the CPG's
 * catalogue snapshot. The expectations come from tools/cpg/generate.mjs (tests/cpg_expected.h). Run by tests/run_tests.sh under ASan and
 * UBSan. Exit 1 on failure. */
#include <stdio.h>
#include <string.h>
#include "cpg_palettes.h"
#include "theory.h"
#include "cpg_expected.h"

static int fails;
#define CHECK(c, ...) do { int ok_ = (c) != 0; printf("%s ", ok_ ? "ok  " : "FAIL"); printf(__VA_ARGS__); printf("\n"); if (!ok_) fails++; } while (0)

/* The pitch classes of a chord in key `key`, relative to the key (bit p: p semitones above it) */
static unsigned mask_of(int key, int slot_scale, int deg, int colour) {
    int n[THEORY_CHORD_MAX], k = theory_chord_notes(key, slot_scale, deg, colour, n);
    unsigned m = 0;
    while (k--) m |= 1u << ((n[k] - key + 120) % 12);
    return m;
}

static uint32_t fnv1a(const char *s) {
    uint32_t h = 0x811c9dc5u;
    for (; *s; s++) h = (h ^ (unsigned char)*s) * 0x01000193u;
    return h;
}

static void test_lookups(void) {
    int ok = cpg_count() == CPG_EXPECTED_ENTRIES && cpg_count() == 184 && cpg_genre_count() == 22 && !cpg_entry(-1) && !cpg_entry(cpg_count()) &&
             !cpg_genre_name(-1) && !cpg_genre_name(cpg_genre_count()) && !strcmp(cpg_genre_name(0), "Pop/Rock") && !cpg_find(0);
    for (int i = 0; i < cpg_count(); i++) {
        const cpg_entry_t *e = cpg_entry(i);
        ok &= cpg_find(e->id) == e && e->genre < cpg_genre_count() && e->minor <= 1 && cpg_nickname(e)[0];
        for (int j = 0; j < i; j++) ok &= cpg_entry(j)->id != e->id;
    }
    CHECK(ok, "184 entries in 22 genres, each found by its identifier, identifiers unique");
    const cpg_entry_t *e = cpg_find(fnv1a("Pop/Rock|I\xE2\x80\x94V\xE2\x80\x94vi\xE2\x80\x94IV"));
    CHECK(e && !strcmp(cpg_nickname(e), "Axis of Awesome") && !e->minor, "the identifier is FNV-1a of \"genre|template\" (S04-D8): Axis of Awesome");
    e = cpg_find(fnv1a("Film/Cinematic|i\xE2\x80\x94i\xE2\x99\xAF" "5\xE2\x80\x94i6\xE2\x80\x94i\xE2\x99\xAF" "5"));
    CHECK(e && !strcmp(cpg_nickname(e), "Ascending Line Clich\xC3\xA9") && e->minor, "and the Ascending Line Clich\xC3\xA9, its nickname in UTF-8, minor");
}

/* verification 2: every stored chord voices the CPG pad's pitch classes, in every key */
static void test_chords(void) {
    int bad = 0, fifthless = 0, spice = 1, sources = 1;
    for (int i = 0; i < cpg_count(); i++)
        for (int k = 0; k < CPG_PADS; k++) {
            cpg_chord_t c = cpg_chord(cpg_entry(i), k);
            int src = cpg_chord_src(c), rule = CPG_EXPECTED_FIFTHLESS[i] >> k & 1;
            unsigned want = CPG_EXPECTED_PCS[i][k], fifth = 1u << ((c.root + 7) % 12);
            sources &= theory_src_ok(src) && src > THEORY_NBORROW && c.root >= 0 && c.root < 12 && c.quality >= 0 &&
                       c.quality < CQ_COUNT + CQ2_COUNT && c.colour >= 0 && c.colour < COL_ALL && c.category == 0;
            spice &= c.spice == k / 4;
            fifthless += rule;
            for (int key = 0; key < 12; key++) {
                unsigned got = mask_of(key, theory_slot_scale(SCALE_MAJOR, src), 0, c.colour);
                if (got != (rule ? want & ~fifth : want)) {
                    if (!bad) printf("     entry %d chord %d key %d: %03x, want %03x%s\n", i, k, key, got, want, rule ? " less the fifth" : "");
                    bad++;
                }
            }
        }
    CHECK(sources, "every chord a chromatic source of either range, a colour below COL_ALL, no category yet");
    CHECK(!bad, "every chord of every palette voices the CPG pad's pitch classes in all 12 keys (%d wrong)", bad);
    CHECK(fifthless == 11, "%d chords take decision 12's fifth rule (a dom13 or minor11 Cadenza voices without its fifth)", fifthless);
    CHECK(spice, "a chord's spice is its row in the CPG's grid less one");
}

/* verification 4: the starting orders, laid out by decision 4's rule */
static void test_orders(void) {
    int ok = 1;
    for (int i = 0; i < cpg_count(); i++) {
        const cpg_entry_t *e = cpg_entry(i);
        unsigned char o[CPG_MAX_BARS];
        int n = cpg_order(e, o), good = n == CPG_EXPECTED_ORDER[i][0] && (n == 1 || n == 2 || n == 4 || n == 8 || n == 16);
        for (int b = 0; good && b < n; b++) good = o[b] == CPG_EXPECTED_ORDER[i][1 + b] && cpg_chord(e, o[b]).start;
        if (!good) printf("     entry %d: %d bars\n", i, n);
        ok &= good;
    }
    CHECK(ok, "every starting order is whole bars in a loop of 1, 2, 4, 8 or 16, each bar a chord of the starting order");
    static const struct { const char *id, *names; } T[] = {
        {"Pop/Rock|I\xE2\x80\x94V\xE2\x80\x94vi\xE2\x80\x94IV", "C G Am F"},
        {"Jazz/Functional|ii\xE2\x80\x94V\xE2\x80\x94I", "Dm G C C"},
        {"Blues/Soul|12-bar-blues", "C7 C7 C7 C7 C7 C7 C7 C7 F7 F7 C7 C7 G7 F7 C7 G7"},
        {"Pop/Rock|I\xE2\x80\x94V\xE2\x80\x94vi\xE2\x80\x94iii\xE2\x80\x94IV", "C C C C G Am Em F"},
    };
    ok = 1;
    for (size_t t = 0; t < sizeof T / sizeof T[0]; t++) {
        const cpg_entry_t *e = cpg_find(fnv1a(T[t].id));
        unsigned char o[CPG_MAX_BARS];
        char got[160] = "", nm[24];
        for (int b = 0, n = e ? cpg_order(e, o) : 0; b < n; b++) {
            cpg_chord_t c = cpg_chord(e, o[b]);
            theory_chord_name_s(0, SCALE_MAJOR, theory_slot_scale(SCALE_MAJOR, cpg_chord_src(c)), 0, c.colour, nm, sizeof nm);
            if (b) strcat(got, " ");
            strcat(got, nm);
        }
        if (strcmp(got, T[t].names)) printf("     %s: %s, want %s\n", T[t].id, got, T[t].names), ok = 0;
    }
    CHECK(ok, "the layouts of spec 04, §3.3: I-V-vi-IV a bar each, ii-V-I with I held, I-V-vi-iii-IV with I four bars, the blues I7 eight bars");
}

/* verification 3: spot checks against the CPG's catalogue snapshot, named by Cadenza in the snapshot's key */
static void test_snapshot(void) {
    static const struct { const char *id; int key, scale; const char *names; } T[] = {
        {"Pop/Rock|I\xE2\x80\x94V\xE2\x80\x94vi\xE2\x80\x94IV", 0, SCALE_MAJOR, "C G Am F Dm Cmaj7 Gmaj7 Fmaj7 Bb Fm Ab Eb G7 A D7 Db"},
        {"Film/Cinematic|i\xE2\x80\x94i\xE2\x99\xAF" "5\xE2\x80\x94i6\xE2\x80\x94i\xE2\x99\xAF" "5", 9, SCALE_MINOR,
         "Am Am#5 Am6 Am#5 Em Dm Am7 Asus4 Em7 G C AmMaj7 E7 Fm D A"},
    };
    for (size_t t = 0; t < sizeof T / sizeof T[0]; t++) {
        const cpg_entry_t *e = cpg_find(fnv1a(T[t].id));
        char got[256] = "", nm[24];
        for (int k = 0; e && k < CPG_PADS; k++) {
            cpg_chord_t c = cpg_chord(e, k);
            theory_chord_name_s(T[t].key, T[t].scale, theory_slot_scale(T[t].scale, cpg_chord_src(c)), 0, c.colour, nm, sizeof nm);
            if (k) strcat(got, " ");
            strcat(got, nm);
        }
        CHECK(!strcmp(got, T[t].names), "the catalogue snapshot's Classic palette in %s: %s (got %s)", T[t].key ? "A minor" : "C major", T[t].names, got);
    }
}

/* the scale palettes: each chord its own scale's, voicing Scale Mode's pitch classes */
static void test_scale_palettes(void) {
    int ok = !cpg_scale_palette(-1) && !cpg_scale_palette(SCALE_COUNT), sevensus = 0;
    for (int s = 0; s < SCALE_COUNT; s++) {
        const cpg_scale_chord_t *p = cpg_scale_palette(s);
        for (int k = 0; p && k < CPG_PADS; k++) {
            ok &= p[k].src == 0 && p[k].deg >= 0 && p[k].deg < 7 && mask_of(0, s, p[k].deg, p[k].colour) == CPG_EXPECTED_SCALE_PCS[s][k];
            sevensus += p[k].colour == COL_7SUS4;
        }
    }
    CHECK(ok, "the seven scale palettes: 16 chords each, every one a degree of its scale voicing Scale Mode's pitch classes");
    CHECK(sevensus == 4, "the tonic's 7sus4 in Minor, Dorian, Mixolydian and Phrygian (spec 03; %d)", sevensus);
}

int main(void) {
    test_lookups();
    test_chords();
    test_orders();
    test_snapshot();
    test_scale_palettes();
    printf(fails ? "\nFAILED: %d\n" : "\nPASSED\n", fails);
    return fails != 0;
}
