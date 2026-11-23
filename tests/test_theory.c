/* Offline test of src/theory.c's 0.9 chord functions: borrowed chords, colours, names, numerals, n-note voicing
 * (run by tests/run_tests.sh under ASan/UBSan). Exit 1 on failure. */
#include "../src/theory.h"
#include <stdio.h>
#include <string.h>

static int fails;

#define CHECK(c, ...) do { printf("%s ", (c) ? "ok  " : "FAIL"); printf(__VA_ARGS__); printf("\n"); if (!(c)) fails++; } while (0)

static int notes_are(const int *g, int n, int a, int b, int c, int d) {
    return g[0] == a && g[1] == b && g[2] == c && (n == 3 ? d < 0 : g[3] == d);
}

/* name and numeral of borrow b in key/scale, notes into g; returns the note count */
static int borrow(int key, int scale, int b, int *g, char *name, char *num) {
    int ss = theory_slot_scale(scale, b), deg = theory_borrow_degree(scale, b);
    theory_chord_name_c(key, ss, deg, COL_TRIAD, name, 16);
    theory_numeral(key, scale, ss, deg, num, 16);
    return theory_chord_notes(key, ss, deg, COL_TRIAD, g);
}

static void test_borrow(void) {
    char nm[16], nu[16];
    int g[THEORY_CHORD_MAX], n;
    CHECK(THEORY_NBORROW == 3 && theory_slot_scale(SCALE_MINOR, 0) == SCALE_MINOR, "src 0 is the project scale");
    n = borrow(4, SCALE_MINOR, 1, g, nm, nu);
    CHECK(theory_borrow_scale(SCALE_MINOR, 1) == SCALE_MAJOR && theory_borrow_degree(SCALE_MINOR, 1) == 3 && n == 3 &&
          notes_are(g, n, 57, 61, 64, -1) && !strcmp(nm, "A") && !strcmp(nu, "IV"),
          "E minor borrow 1: A3 C#4 E4, A, IV (got %d %d %d, %s, %s)", g[0], g[1], g[2], nm, nu);
    n = borrow(4, SCALE_MINOR, 3, g, nm, nu);
    CHECK(theory_borrow_scale(SCALE_MINOR, 3) == SCALE_PHRYGIAN && theory_borrow_degree(SCALE_MINOR, 3) == 1 &&
          notes_are(g, n, 53, 57, 60, -1) && !strcmp(nm, "F") && !strcmp(nu, "bII"),
          "E minor borrow 3: F3 A3 C4, F, bII (got %d %d %d, %s, %s)", g[0], g[1], g[2], nm, nu);
    n = borrow(0, SCALE_MAJOR, 2, g, nm, nu);
    CHECK(notes_are(g, n, 56, 60, 63, -1) && !strcmp(nm, "G#") && !strcmp(nu, "bVI"),
          "C major borrow 2: Ab3 C4 Eb4, G#, bVI (got %d %d %d, %s, %s)", g[0], g[1], g[2], nm, nu);
    n = borrow(0, SCALE_MAJOR, 1, g, nm, nu);
    CHECK(!strcmp(nm, "Fm") && !strcmp(nu, "iv"), "C major borrow 1: Fm, iv (got %s, %s)", nm, nu);
    n = borrow(0, SCALE_MAJOR, 3, g, nm, nu);
    CHECK(!strcmp(nm, "A#") && !strcmp(nu, "bVII"), "C major borrow 3: A#, bVII (got %s, %s)", nm, nu);
    /* the whole table (decision 2) */
    static const int T[SCALE_COUNT][3][2] = {
        {{SCALE_MINOR, 3}, {SCALE_MINOR, 5}, {SCALE_MINOR, 6}},          {{SCALE_MAJOR, 3}, {SCALE_MAJOR, 4}, {SCALE_PHRYGIAN, 1}},
        {{SCALE_MINOR, 3}, {SCALE_MAJOR, 4}, {SCALE_MINOR, 5}},          {{SCALE_MINOR, 3}, {SCALE_MINOR, 5}, {SCALE_MAJOR, 4}},
        {{SCALE_MAJOR, 3}, {SCALE_MAJOR, 4}, {SCALE_LYDIAN, 1}},         {{SCALE_MAJOR, 3}, {SCALE_MINOR, 3}, {SCALE_MINOR, 6}},
        {{SCALE_MAJOR, 3}, {SCALE_PHRYGIAN, 1}, {SCALE_MINOR, 6}}};
    int ok = 1;
    for (int s = 0; s < SCALE_COUNT; s++)
        for (int b = 1; b <= 3; b++)
            ok &= theory_borrow_scale(s, b) == T[s][b - 1][0] && theory_slot_scale(s, b) == T[s][b - 1][0] &&
                  theory_borrow_degree(s, b) == T[s][b - 1][1];
    CHECK(ok, "borrow table matches decision 2 for every scale");
}

static void test_colours(void) {
    char b[16];
    int g[THEORY_CHORD_MAX], n;
    CHECK(!strcmp(THEORY_COLOUR_NAME[COL_TRIAD], "Triad") && !strcmp(THEORY_COLOUR_NAME[COL_7TH], "7th") &&
          !strcmp(THEORY_COLOUR_NAME[COL_SUS2], "sus2") && !strcmp(THEORY_COLOUR_NAME[COL_SUS4], "sus4") &&
          !strcmp(THEORY_COLOUR_NAME[COL_ADD9], "add9"), "colour names");
    CHECK(theory_colour_tone(COL_SUS4, TONE_3) == TONE_4 && theory_colour_tone(COL_SUS2, TONE_3) == TONE_2 &&
          theory_colour_tone(COL_7TH, TONE_3) == TONE_3 && theory_colour_tone(COL_SUS4, TONE_5) == TONE_5,
          "sus turns tone 3 into 2/4, nothing else");
    n = theory_chord_notes(4, SCALE_MINOR, 0, COL_7TH, g);
    theory_chord_name_c(4, SCALE_MINOR, 0, COL_7TH, b, sizeof b);
    CHECK(n == 4 && notes_are(g, n, 52, 55, 59, 62) && !strcmp(b, "Em7"), "Em7: E G B D (got %d: %d %d %d %d, %s)", n, g[0], g[1], g[2], g[3], b);
    n = theory_chord_notes(4, SCALE_MINOR, 0, COL_SUS2, g);
    theory_chord_name_c(4, SCALE_MINOR, 0, COL_SUS2, b, sizeof b);
    CHECK(n == 3 && notes_are(g, n, 52, 54, 59, -1) && !strcmp(b, "Esus2"), "Esus2: E F# B (got %d %d %d, %s)", g[0], g[1], g[2], b);
    n = theory_chord_notes(4, SCALE_MINOR, 0, COL_SUS4, g);
    theory_chord_name_c(4, SCALE_MINOR, 0, COL_SUS4, b, sizeof b);
    CHECK(n == 3 && notes_are(g, n, 52, 57, 59, -1) && !strcmp(b, "Esus4"), "Esus4: E A B (got %d %d %d, %s)", g[0], g[1], g[2], b);
    n = theory_chord_notes(4, SCALE_MINOR, 0, COL_ADD9, g);
    theory_chord_name_c(4, SCALE_MINOR, 0, COL_ADD9, b, sizeof b);
    CHECK(n == 4 && notes_are(g, n, 52, 55, 59, 66) && !strcmp(b, "Emadd9"), "Emadd9: E G B F#4 (got %d %d %d %d, %s)", g[0], g[1], g[2], g[3], b);
    n = theory_chord_notes(4, SCALE_MINOR, 0, COL_TRIAD, g);
    CHECK(n == 3 && notes_are(g, n, 52, 55, 59, -1), "Triad is theory_triad");
    theory_chord_name_c(4, SCALE_MINOR, 1, COL_7TH, b, sizeof b);
    CHECK(!strcmp(b, "F#m7b5"), "E minor ii7 is F#m7b5 (got %s)", b);
    theory_chord_name_c(4, SCALE_MINOR, 1, COL_ADD9, b, sizeof b);
    CHECK(!strcmp(b, "F#dimadd9"), "E minor ii add9 is F#dimadd9 (got %s)", b);
    n = theory_chord_notes(4, SCALE_HARM_MINOR, 6, COL_7TH, g);
    theory_chord_name_c(4, SCALE_HARM_MINOR, 6, COL_7TH, b, sizeof b);
    CHECK(notes_are(g, n, 51, 54, 57, 60) && !strcmp(b, "D#dim7"), "E harm. minor vii7: D# F# A C, D#dim7 (got %s)", b);
    theory_chord_name_c(0, SCALE_MAJOR, 4, COL_7TH, b, sizeof b);
    CHECK(!strcmp(b, "G7"), "C major V7 is G7 (got %s)", b);
    theory_chord_name_c(0, SCALE_MAJOR, 0, COL_7TH, b, sizeof b);
    CHECK(!strcmp(b, "Cmaj7"), "C major I7 is Cmaj7 (got %s)", b);
    theory_chord_name_c(0, SCALE_MAJOR, 0, COL_ADD9, b, sizeof b);
    CHECK(!strcmp(b, "Cadd9"), "Cadd9 (got %s)", b);
    theory_chord_name_c(9, SCALE_HARM_MINOR, 0, COL_7TH, b, sizeof b);
    CHECK(!strcmp(b, "AmMaj7"), "A harm. minor i7 is AmMaj7 (got %s)", b);
    theory_chord_name_c(9, SCALE_HARM_MINOR, 2, COL_7TH, b, sizeof b);
    CHECK(!strcmp(b, "C+maj7"), "A harm. minor III7 is C+maj7 (got %s)", b);
    theory_chord_name_c(9, SCALE_HARM_MINOR, 2, COL_TRIAD, b, sizeof b);
    CHECK(!strcmp(b, "C+"), "A harm. minor III is C+ (got %s)", b);
    /* every 7th on every degree of every scale has one of the eight suffixes, and every name fits 16 bytes */
    static const char *const SUF[] = {"maj7", "7", "m7", "mMaj7", "m7b5", "dim7", "+maj7", "+7"};
    int ok = 1, k, s, d, c;
    for (k = 0; k < 12; k++)
        for (s = 0; s < SCALE_COUNT; s++)
            for (d = 0; d < 7; d++) {
                int known = 0, l = theory_chord_name_c(k, s, d, COL_7TH, b, sizeof b), r = (int)strlen(theory_key_name(theory_degree_pc(k, s, d)));
                for (unsigned i = 0; i < sizeof SUF / sizeof SUF[0]; i++) known |= !strcmp(b + r, SUF[i]);
                ok &= known && l < 16;
                for (c = 0; c < COL_COUNT; c++) ok &= theory_chord_name_c(k, s, d, c, b, sizeof b) < 16;
            }
    CHECK(ok, "every 7th name is root + one of the 8 suffixes; every name fits");
}

static void test_names_match_triads(void) {
    char a[16], b[16];
    int ok = 1, g[THEORY_CHORD_MAX], t[3];
    for (int k = 0; k < 12; k++)
        for (int s = 0; s < SCALE_COUNT; s++)
            for (int d = 0; d < 7; d++) {
                int la = theory_chord_name(k, s, d, a, sizeof a), lb = theory_chord_name_c(k, s, d, COL_TRIAD, b, sizeof b);
                theory_triad(k, s, d, t);
                ok &= la == lb && !strcmp(a, b) && theory_chord_notes(k, s, d, COL_TRIAD, g) == 3 && !memcmp(g, t, sizeof t);
            }
    CHECK(ok, "Triad names and notes equal theory_chord_name / theory_triad on every key, scale and degree");
}

#define DIM "\xC2\xB0"   /* THEORY_DIM */
static void test_numerals(void) {
    char b[16];
    static const char *const EM[7] = {"i", ("ii" DIM), "III", "iv", "v", "VI", "VII"};
    static const char *const CMAJ[7] = {"I", "ii", "iii", "IV", "V", "vi", ("vii" DIM)};
    int ok = 1;
    for (int d = 0; d < 7; d++) {
        theory_numeral(4, SCALE_MINOR, SCALE_MINOR, d, b, sizeof b);
        ok &= !strcmp(b, EM[d]);
        theory_numeral(0, SCALE_MAJOR, SCALE_MAJOR, d, b, sizeof b);
        ok &= !strcmp(b, CMAJ[d]);
    }
    CHECK(ok, "diatonic numerals in E minor and C major");
    theory_numeral(9, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 2, b, sizeof b);
    CHECK(!strcmp(b, "III+"), "A harm. minor III+ (got %s)", b);
    theory_numeral(4, SCALE_MINOR, SCALE_MAJOR, 4, b, sizeof b);
    CHECK(!strcmp(b, "V"), "E minor borrowed V (got %s)", b);
    theory_numeral(0, SCALE_LYDIAN, SCALE_MINOR, 6, b, sizeof b);
    CHECK(!strcmp(b, "bVII"), "C lydian borrowed bVII (got %s)", b);
    theory_numeral(4, SCALE_PHRYGIAN, SCALE_LYDIAN, 1, b, sizeof b);
    CHECK(!strcmp(b, "#II"), "E phrygian borrowed II from Lydian is a semitone up: #II (got %s)", b);
    CHECK(theory_numeral(4, SCALE_MINOR, SCALE_MINOR, 8, b, sizeof b) == 4 && !strcmp(b, "ii" DIM), "degrees wrap by octave");
}

/* degree labels: the numeral and a 7th as a suffix (the chord's own seventh, theory's quality); 0.13: the dim sign, and a sus or
 * add9 after a space */
static void test_degrees(void) {
    static const struct { int key, scale, slot_scale, deg, colour; const char *want; } T[] = {
        {4, SCALE_MINOR, SCALE_MINOR, 3, COL_7TH, "iv7"},          /* Am7 */
        {0, SCALE_MAJOR, SCALE_MINOR, 5, COL_TRIAD, "bVI"},        /* G#: borrowed */
        {0, SCALE_MAJOR, SCALE_MAJOR, 0, COL_SUS4, "I sus4"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 5, COL_ADD9, "vi add9"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 4, COL_7TH, "V7"},           /* G7 */
        {4, SCALE_MINOR, SCALE_MINOR, 1, COL_7TH, "iim7b5"},       /* F#m7b5 */
        {0, SCALE_MAJOR, SCALE_MAJOR, 0, COL_7TH, "Imaj7"},        /* Cmaj7 */
        {4, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 6, COL_7TH, "vii" DIM "7"},   /* D#dim7 */
        {9, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 0, COL_7TH, "imaj7"},   /* AmMaj7 */
        {9, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 2, COL_7TH, "III+maj7"},   /* C+maj7 */
        {9, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 2, COL_TRIAD, "III+"},
        {4, SCALE_MINOR, SCALE_MINOR, 1, COL_TRIAD, "ii" DIM},
        {4, SCALE_MINOR, SCALE_MINOR, 1, COL_SUS2, "ii" DIM " sus2"},   /* 0.13: the numeral whole */
        {4, SCALE_MINOR, SCALE_MINOR, 5, COL_SUS4, "VI sus4"},
        {4, SCALE_MINOR, SCALE_MINOR, 1, COL_ADD9, "ii" DIM " add9"},
        {4, SCALE_MINOR, SCALE_MAJOR, 3, COL_TRIAD, "IV"},         /* E minor's borrow 1 */
        {4, SCALE_MINOR, SCALE_PHRYGIAN, 1, COL_7TH, "bIImaj7"},   /* Fmaj7 */
    };
    char b[16];
    int ok = 1;
    for (unsigned i = 0; i < sizeof T / sizeof T[0]; i++) {
        int n = theory_degree_c(T[i].key, T[i].scale, T[i].slot_scale, T[i].deg, T[i].colour, b, sizeof b);
        int good = !strcmp(b, T[i].want) && n == (int)strlen(b);
        if (!good) printf("     degree %u: got %s, want %s\n", i, b, T[i].want);
        ok &= good;
    }
    CHECK(ok, "the degree table (iv7, bVI, I sus4, vi add9, V7, iim7b5, vii\xC2\xB0" "7, ...)");
    int longest = 0;   /* every key, scale, borrow and colour fits 15 characters */
    for (int k = 0; k < 12; k++)
        for (int s = 0; s < SCALE_COUNT; s++)
            for (int src = 0; src <= THEORY_NBORROW; src++)
                for (int d = 0; d < 7; d++)
                    for (int c = 0; c < COL_COUNT; c++) {
                        theory_degree_c(k, s, theory_slot_scale(s, src), d, c, b, sizeof b);
                        if (theory_text_width(b) > longest) longest = theory_text_width(b);
                    }
    CHECK(longest <= 11, "every degree label is at most 11 characters (longest %d)", longest);
    CHECK(theory_degree_c(0, SCALE_MAJOR, SCALE_MAJOR, 5, COL_ADD9, b, 4) == 7 && !strcmp(b, "vi "), "a short buffer cuts (got %s)", b);
}

/* cards (fd_, pn_). 0.13 degrees: the numeral whole, the colour in brackets, (7), (M7), (dim 7), (9), (s2), (s4). Names (0.12):
 * compact suffixes 7, M7, 9, sus2, sus4, m7b5 / o7 on a diminished triad */
static void test_cards(void) {
    static const struct { int key, scale, slot_scale, deg, colour, names; const char *want; } T[] = {
        {4, SCALE_MINOR, SCALE_MINOR, 3, COL_7TH, 0, "iv(7)"},
        {0, SCALE_MAJOR, SCALE_MINOR, 5, COL_ADD9, 0, "bVI(9)"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 0, COL_7TH, 0, "I(M7)"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 3, COL_7TH, 0, "IV(M7)"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 4, COL_7TH, 0, "V(7)"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 1, COL_7TH, 0, "ii(7)"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 4, COL_SUS2, 0, "V(s2)"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 5, COL_SUS4, 0, "vi(s4)"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 0, COL_7TH, 1, "CM7"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 4, COL_7TH, 1, "G7"},
        {4, SCALE_MINOR, SCALE_MINOR, 1, COL_7TH, 0, "ii" DIM "(7)"},   /* half-diminished: a minor 7th */
        {4, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 6, COL_7TH, 0, "vii" DIM "(" DIM "7)"},
        {4, SCALE_MINOR, SCALE_MINOR, 1, COL_TRIAD, 0, "ii" DIM},
        {4, SCALE_MINOR, SCALE_MINOR, 1, COL_SUS4, 0, "ii" DIM "(s4)"},
        {9, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 2, COL_7TH, 0, "III+(M7)"},
        {4, SCALE_MINOR, SCALE_MINOR, 3, COL_7TH, 1, "Am7"},
        {0, SCALE_MAJOR, SCALE_MAJOR, 0, COL_ADD9, 1, "C9"},
        {4, SCALE_MINOR, SCALE_MINOR, 1, COL_7TH, 1, "F#m7b5"},
        {4, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 6, COL_7TH, 1, "D#o7"},
        {4, SCALE_MINOR, SCALE_MINOR, 1, COL_TRIAD, 1, "F#dim"},
        {4, SCALE_MINOR, SCALE_MINOR, 0, COL_SUS2, 1, "Esus2"},
        {9, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 2, COL_7TH, 1, "C+M7"},
    };
    char b[16];
    int ok = 1, longest[2] = {0, 0};
    for (unsigned i = 0; i < sizeof T / sizeof T[0]; i++) {
        int n = theory_chord_card(T[i].key, T[i].scale, T[i].slot_scale, T[i].deg, T[i].colour, T[i].names, b, sizeof b);
        int good = !strcmp(b, T[i].want) && n == (int)strlen(b);
        if (!good) printf("     card %u: got %s, want %s\n", i, b, T[i].want);
        ok &= good;
    }
    CHECK(ok, "the card table (iv(7), bVI(9), I(M7), V(s2), vi(s4), ii\xC2\xB0(7); Am7, CM7, C9, F#m7b5, D#o7)");
    for (int k = 0; k < 12; k++)
        for (int s = 0; s < SCALE_COUNT; s++)
            for (int src = 0; src <= THEORY_NBORROW; src++)
                for (int d = 0; d < 7; d++)
                    for (int c = 0; c < COL_COUNT; c++)
                        for (int nm = 0; nm < 2; nm++) {
                            theory_chord_card(k, s, theory_slot_scale(s, src), d, c, nm, b, sizeof b);
                            if (theory_text_width(b) > longest[nm]) longest[nm] = theory_text_width(b);
                        }
    CHECK(longest[0] <= 9 && longest[1] <= 8, "every card chord is at most 9 characters, 8 in names (longest %d, %d)", longest[0], longest[1]);
}

/* n == 3 on triads equals theory_voice_next: chains over every key, scale and preset, each step also trying every degree */
static void test_voice_triads(void) {
    int bad = 0, steps = 0;
    for (int k = 0; k < 12; k++)
        for (int s = 0; s < SCALE_COUNT; s++)
            for (int p = 0; p < THEORY_NPRESETS; p++) {
                const theory_preset_t *r = &THEORY_PRESETS[p];
                int v[3], w[3], x[THEORY_CHORD_MAX], t[THEORY_CHORD_MAX], pv[THEORY_CHORD_MAX];
                theory_triad(k, s, r->deg[0], v);
                for (int i = 0; i < 3 * r->len; i++) {
                    for (int d = 0; d < 7; d++) {
                        theory_voice_next(k, s, d, v, w);
                        theory_chord_notes(k, s, d, COL_TRIAD, t);
                        memcpy(pv, v, sizeof v);
                        pv[3] = -1;   /* never read with np == 3 */
                        bad += theory_voice_chord(t, 3, pv, 3, x) != 3 || memcmp(x, w, sizeof w);
                        steps++;
                    }
                    theory_voice_next(k, s, r->deg[(i + 1) % r->len], v, w);
                    memcpy(v, w, sizeof v);
                }
            }
    CHECK(bad == 0, "theory_voice_chord equals theory_voice_next on %d triad moves (%d differ)", steps, bad);
}

static void test_voice_four(void) {
    int g[THEORY_CHORD_MAX], o[THEORY_CHORD_MAX], prev[THEORY_CHORD_MAX] = {52, 55, 59, -1}, ok = 1;
    theory_chord_notes(0, SCALE_MAJOR, 4, COL_7TH, g);   /* G7: 55 59 62 65 */
    CHECK(theory_voice_chord(g, 4, prev, 3, o) == 4 && o[0] == 53 && o[1] == 55 && o[2] == 59 && o[3] == 62,
          "Em (52 55 59) to G7 moves least: F3 G3 B3 D4 (got %d %d %d %d)", o[0], o[1], o[2], o[3]);
    int prev4[THEORY_CHORD_MAX] = {53, 55, 59, 62};
    theory_chord_notes(0, SCALE_MAJOR, 0, COL_TRIAD, g);   /* G7 to C: 4 notes back to 3 */
    theory_voice_chord(g, 3, prev4, 4, o);
    CHECK(o[0] == 52 && o[1] == 55 && o[2] == 60, "G7 (53 55 59 62) to C: 52 55 60 (got %d %d %d)", o[0], o[1], o[2]);
    /* add9's 9th is beyond the octave: every voicing still ascending, lowest in range, for every key and degree */
    for (int k = 0; k < 12; k++)
        for (int d = 0; d < 7; d++)
            for (int c = 0; c < COL_COUNT; c++) {
                int n = theory_chord_notes(k, SCALE_MINOR, d, c, g);
                theory_voice_chord(g, n, prev4, 4, o);
                ok &= o[0] >= 43 && o[0] <= 62;
                for (int i = 1; i < n; i++) ok &= o[i] > o[i - 1];
            }
    CHECK(ok, "every colour on every key and degree voices ascending with its lowest note in 43..62");
    int one[THEORY_CHORD_MAX] = {60, 0, 0, 0};   /* one previous note: every voice is measured against it */
    theory_chord_notes(0, SCALE_MAJOR, 0, COL_ADD9, g);   /* C E G D: 48 52 55 62 */
    theory_voice_chord(g, 4, one, 1, o);
    CHECK(o[0] == 55 && o[1] == 60 && o[2] == 62 && o[3] == 64, "Cadd9 near C4 alone: G3 C4 D4 E4 (got %d %d %d %d)", o[0], o[1], o[2], o[3]);
}

/* 0.17: Power to 13th and 6th colours, the colour knob's levels, voicing modes, drift score, names */
static void test_17_colour_notes(void) {
    static const struct { int colour, n, want[5]; } T[] = {
        {COL_POWER, 2, {52, 59}}, {COL_9TH, 5, {52, 55, 59, 62, 66}}, {COL_11TH, 5, {52, 55, 62, 66, 69}},
        {COL_13TH, 5, {52, 55, 62, 66, 72}}, {COL_6TH, 4, {52, 55, 59, 60}}};
    int g[THEORY_CHORD_MAX], ok = 1;
    for (unsigned i = 0; i < sizeof T / sizeof T[0]; i++) {
        int n = theory_chord_notes(4, SCALE_MINOR, 0, T[i].colour, g);
        int good = n == T[i].n && !memcmp(g, T[i].want, n * sizeof g[0]);
        if (!good) printf("     colour %d: got %d notes from %d, want %d\n", T[i].colour, n, g[0], T[i].n);
        ok &= good;
    }
    CHECK(ok, "E minor Power {52,59}, 9th, 11th, 13th (b13 = C) and 6th {52,55,59,60}");
    ok = COL_COUNT == 10 && COL_POWER == COL_ADD9 + 1 && COL_6TH == COL_ADD9 + 5 && !strcmp(THEORY_COLOUR_NAME[COL_POWER], "Power") &&
         !strcmp(THEORY_COLOUR_NAME[COL_13TH], "13th") && !strcmp(THEORY_COLOUR_NAME[COL_6TH], "6th");
    for (int k = 0; k < 12; k++)
        for (int s = 0; s < SCALE_COUNT; s++)
            for (int d = 0; d < 7; d++)
                for (int c = 0; c < COL_COUNT; c++) {
                    int n = theory_chord_notes(k, s, d, c, g);
                    ok &= n >= 2 && n <= 5 && g[0] >= 48 && g[0] < 60;
                    for (int i = 1; i < n; i++) ok &= g[i] > g[i - 1];
                }
    CHECK(ok, "new colours append after add9; every colour on every key, scale and degree is 2..5 ascending notes");
}

static void test_17_level_map(void) {
    static const int WANT[] = {COL_POWER, COL_TRIAD, COL_7TH, COL_9TH, COL_11TH, COL_13TH};
    int ok = 1;
    for (int l = -1; l <= 4; l++) ok &= theory_colour_of_level(l) == WANT[l + 1];
    CHECK(ok && theory_colour_of_level(-2) == COL_TRIAD && theory_colour_of_level(5) == COL_TRIAD, "levels -1..4 map to Power..13th; out of range is Triad");
}

static void test_17_modes(void) {
    int c[THEORY_CHORD_MAX], em9[THEORY_CHORD_MAX], a[THEORY_CHORD_MAX], b[THEORY_CHORD_MAX];
    int prev[THEORY_CHORD_MAX] = {52, 55, 59}, n, ok = 1;
    theory_chord_notes(4, SCALE_MINOR, 0, COL_9TH, em9);
    theory_chord_notes(0, SCALE_MAJOR, 0, COL_TRIAD, c);
    CHECK(theory_voice_mode(c, 3, prev, 3, VMODE_SMOOTH, a) == 3 && theory_voice_chord(c, 3, prev, 3, b) == 3 && !memcmp(a, b, 3 * sizeof a[0]),
          "SMOOTH is theory_voice_chord (got %d %d %d)", a[0], a[1], a[2]);
    CHECK(theory_voice_mode(em9, 5, prev, 3, VMODE_CLOSE, a) == 5 && !memcmp(a, em9, 5 * sizeof a[0]), "CLOSE copies the notes");
    n = theory_voice_mode(em9, 5, prev, 3, VMODE_ROOTLESS, a);
    for (int i = 0; i < n; i++) ok &= a[i] % 12 != 4;
    CHECK(n == 4 && ok, "ROOTLESS on Em9 is 4 notes, none an E (got %d)", n);
    CHECK(theory_voice_mode(c, 3, prev, 3, VMODE_ROOTLESS, a) == 3 && theory_voice_mode(c, 3, prev, 3, VMODE_SMOOTH, b) == 3 &&
          !memcmp(a, b, 3 * sizeof a[0]), "ROOTLESS keeps a triad whole");
    n = theory_voice_mode(c, 3, prev, 3, VMODE_TOPLINE, a);
    CHECK(n == 3 && a[2] >= 57 && a[2] <= 61 && a[0] >= 43 && a[0] <= 62, "TOPLINE Em to C keeps the top within 2 of B3 (got %d %d %d)", a[0], a[1], a[2]);
    ok = 1;   /* every mode on every colour: ascending, lowest note in range, counts as promised */
    for (int col = 0; col < COL_COUNT; col++)
        for (int m = 0; m < VMODE_COUNT; m++) {
            int src[THEORY_CHORD_MAX], k = theory_chord_notes(9, SCALE_MINOR, 3, col, src);
            n = theory_voice_mode(src, k, prev, 3, m, a);
            ok &= n == (m == VMODE_ROOTLESS && k >= 4 ? k - 1 : k);
            if (m != VMODE_CLOSE) ok &= a[0] >= 43 && a[0] <= 62;
            for (int i = 1; i < n; i++) ok &= a[i] > a[i - 1];
        }
    CHECK(ok, "every mode on every colour is ascending with the promised count");
}

static void test_17_drift(void) {
    int g = theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_TRIAD, 0, 0);
    int bii = theory_drift(4, SCALE_MINOR, theory_slot_scale(SCALE_MINOR, 3), theory_borrow_degree(SCALE_MINOR, 3), COL_TRIAD, 0, 0);
    int g11 = theory_drift(4, SCALE_MINOR, SCALE_MINOR, 2, COL_11TH, 0, 0);
    int e13 = theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_13TH, 0, 0);
    CHECK(g == 0 && theory_drift_level(g) == 0, "Em triad drifts 0, level 0 (got %d)", g);
    CHECK(bii == 30 && theory_drift_level(bii) == 1, "F bII triad drifts 30, level 1 (got %d)", bii);
    CHECK(g11 == 58 && theory_drift_level(g11) == 2, "G 11th: 38 + 20 for C over B, level 2 (got %d)", g11);
    CHECK(e13 == 70 && theory_drift_level(e13) == 3, "Em 13th: 50 + 20 for the b13 over the 5th, level 3 (got %d)", e13);
    CHECK(theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_TRIAD, 1, 0) == 22 && theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_TRIAD, 0, -3) == 6 &&
          theory_drift(4, SCALE_MINOR, SCALE_MINOR, 1, COL_TRIAD, 0, 0) == 15, "cluster adds 22, inversion 2 per step either way, a diminished triad 15");
    CHECK(theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_7TH, 0, 0) == 10 && theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_9TH, 0, 0) == 22 &&
          theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_POWER, 0, 0) == 0 && theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_SUS4, 0, 0) == 12 &&
          theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_ADD9, 0, 0) == 12, "colour adds Power 0, 7th 10, 9th 22, sus and add9 12");
    CHECK(theory_drift(4, SCALE_MINOR, SCALE_MINOR, 0, COL_13TH, 1, 5) == 100, "the score stops at 100");
    CHECK(theory_drift_level(15) == 0 && theory_drift_level(16) == 1 && theory_drift_level(39) == 1 && theory_drift_level(40) == 2 &&
          theory_drift_level(64) == 2 && theory_drift_level(65) == 3, "levels change at 16, 40 and 65");
}

static void test_17_names(void) {
    char b[16];
    theory_chord_name_c(4, SCALE_MINOR, 0, COL_9TH, b, sizeof b);
    CHECK(!strcmp(b, "Em9"), "Em9 (got %s)", b);
    theory_chord_name_c(4, SCALE_MINOR, 0, COL_POWER, b, sizeof b);
    CHECK(!strcmp(b, "E5"), "E5 (got %s)", b);
    theory_chord_name_c(0, SCALE_MAJOR, 0, COL_13TH, b, sizeof b);
    CHECK(!strcmp(b, "Cmaj13"), "Cmaj13 (got %s)", b);
    theory_chord_name_c(0, SCALE_MAJOR, 4, COL_9TH, b, sizeof b);
    CHECK(!strcmp(b, "G9"), "G9 (got %s)", b);
    theory_chord_name_c(4, SCALE_MINOR, 6, COL_11TH, b, sizeof b);
    CHECK(!strcmp(b, "D11"), "D11 (got %s)", b);
    theory_chord_name_c(4, SCALE_MINOR, 0, COL_6TH, b, sizeof b);
    CHECK(!strcmp(b, "Em6"), "Em6 (got %s)", b);
    theory_chord_name_c(4, SCALE_MINOR, 1, COL_9TH, b, sizeof b);
    CHECK(!strcmp(b, "F#m9b5"), "F#m9b5 (got %s)", b);
    static const struct { int deg, colour; const char *num; } N[] = {
        {0, COL_9TH, "i9"}, {6, COL_11TH, "VII11"}, {0, COL_POWER, "i5"}, {0, COL_13TH, "i13"}, {0, COL_6TH, "i6"}, {0, COL_11TH, "i11"}};
    int ok = 1;
    for (unsigned i = 0; i < sizeof N / sizeof N[0]; i++) {
        theory_degree_c(4, SCALE_MINOR, SCALE_MINOR, N[i].deg, N[i].colour, b, sizeof b);
        if (strcmp(b, N[i].num)) printf("     numeral %u: got %s, want %s\n", i, b, N[i].num);
        ok &= !strcmp(b, N[i].num);
    }
    CHECK(ok, "numerals i9, VII11, i5, i13, i6, i11");
    theory_degree_c(0, SCALE_MAJOR, SCALE_MAJOR, 0, COL_9TH, b, sizeof b);
    CHECK(!strcmp(b, "Imaj9"), "Imaj9 (got %s)", b);
    theory_chord_card(4, SCALE_MINOR, SCALE_MINOR, 0, COL_9TH, 1, b, sizeof b);
    ok = !strcmp(b, "Em9");
    theory_chord_card(4, SCALE_MINOR, SCALE_MINOR, 0, COL_POWER, 1, b, sizeof b);
    ok &= !strcmp(b, "E5");
    theory_chord_card(0, SCALE_MAJOR, SCALE_MAJOR, 0, COL_13TH, 1, b, sizeof b);
    ok &= !strcmp(b, "Cmaj13");
    theory_chord_card(4, SCALE_MINOR, SCALE_MINOR, 0, COL_6TH, 1, b, sizeof b);
    ok &= !strcmp(b, "Em6");
    CHECK(ok, "cards in names: Em9, E5, Cmaj13, Em6");
}

/* 0.18: the avoid-note test on its own (COMPLEXITY's Tension layer skips these chords); theory_drift's +20 is exactly it */
static void test_18_theory_avoid(void) {
    static const unsigned char COST[COL_COUNT] = {0, 10, 12, 12, 12, 0, 22, 38, 50, 12};
    int ok = 1;
    CHECK(theory_avoid(4, SCALE_MINOR, SCALE_MINOR, 2, COL_11TH) == 1, "G 11th in E minor: C a semitone above B, an avoid note");
    CHECK(theory_avoid(4, SCALE_MINOR, SCALE_MINOR, 0, COL_9TH) == 0, "Em9: F# sits a tone above E, no avoid note");
    CHECK(theory_avoid(4, SCALE_MINOR, SCALE_MINOR, 0, COL_11TH) == 0 && theory_avoid(4, SCALE_MINOR, SCALE_MINOR, 0, COL_13TH) == 1 &&
          theory_avoid(4, SCALE_MINOR, SCALE_MINOR, 0, COL_TRIAD) == 0 && theory_avoid(4, SCALE_MINOR, SCALE_MINOR, 0, COL_7TH) == 0,
          "Em11 none, Em13 the b13 over the 5th, a triad and a 7th never");
    for (int k = 0; k < 12; k++)
        for (int sc = 0; sc < SCALE_COUNT; sc++)
            for (int d = 0; d < 7; d++)
                for (int c = 0; c < COL_COUNT; c++)
                    ok &= theory_drift(k, sc, sc, d, c, 0, 0) == theory_drift(k, sc, sc, d, COL_TRIAD, 0, 0) + COST[c] + 20 * theory_avoid(k, sc, sc, d, c);
    CHECK(ok, "every key, scale, degree and colour: drift = the triad's + the colour's cost + 20 x theory_avoid");
}

/* fork, docs/development/specs/01-key-aware-spelling.md, verification 1: the tonic and seven letters of every key and scale, written
 * by hand from the key signatures (each mode read from its relative major), not derived from the code under test. S01-D2 picks a black
 * key's name by the fewer accidentals in that scale, the sharp one on a tie: Eb major but D# Phrygian, F# major (6 against 6) */
static const char *const SPELL_TABLE[SCALE_COUNT][12] = {
    {"C D E F G A B", "Db Eb F Gb Ab Bb C", "D E F# G A B C#", "Eb F G Ab Bb C D", "E F# G# A B C# D#", "F G A Bb C D E",
     "F# G# A# B C# D# E#", "G A B C D E F#", "Ab Bb C Db Eb F G", "A B C# D E F# G#", "Bb C D Eb F G A", "B C# D# E F# G# A#"},
    {"C D Eb F G Ab Bb", "C# D# E F# G# A B", "D E F G A Bb C", "D# E# F# G# A# B C#", "E F# G A B C D", "F G Ab Bb C Db Eb",
     "F# G# A B C# D E", "G A Bb C D Eb F", "G# A# B C# D# E F#", "A B C D E F G", "Bb C Db Eb F Gb Ab", "B C# D E F# G A"},
    {"C D Eb F G A Bb", "C# D# E F# G# A# B", "D E F G A B C", "Eb F Gb Ab Bb C Db", "E F# G A B C# D", "F G Ab Bb C D Eb",
     "F# G# A B C# D# E", "G A Bb C D E F", "G# A# B C# D# E# F#", "A B C D E F# G", "Bb C Db Eb F G Ab", "B C# D E F# G# A"},
    {"C D E F G A Bb", "C# D# E# F# G# A# B", "D E F# G A B C", "Eb F G Ab Bb C Db", "E F# G# A B C# D", "F G A Bb C D Eb",
     "F# G# A# B C# D# E", "G A B C D E F", "Ab Bb C Db Eb F Gb", "A B C# D E F# G", "Bb C D Eb F G Ab", "B C# D# E F# G# A"},
    {"C Db Eb F G Ab Bb", "C# D E F# G# A B", "D Eb F G A Bb C", "D# E F# G# A# B C#", "E F G A B C D", "F Gb Ab Bb C Db Eb",
     "F# G A B C# D E", "G Ab Bb C D Eb F", "G# A B C# D# E F#", "A Bb C D E F G", "A# B C# D# E# F# G#", "B C D E F# G A"},
    {"C D E F# G A B", "Db Eb F G Ab Bb C", "D E F# G# A B C#", "Eb F G A Bb C D", "E F# G# A# B C# D#", "F G A B C D E",
     "Gb Ab Bb C Db Eb F", "G A B C# D E F#", "Ab Bb C D Eb F G", "A B C# D# E F# G#", "Bb C D E F G A", "B C# D# E# F# G# A#"},
    {"C D Eb F G Ab B", "C# D# E F# G# A B#", "D E F G A Bb C#", "Eb F Gb Ab Bb Cb D", "E F# G A B C D#", "F G Ab Bb C Db E",
     "F# G# A B C# D E#", "G A Bb C D Eb F#", "G# A# B C# D# E F##", "A B C D E F G#", "Bb C Db Eb F Gb A", "B C# D E F# G A#"},
};

static void test_fork_spelling_table(void) {
    int bad = 0;
    for (int sc = 0; sc < SCALE_COUNT; sc++)
        for (int k = 0; k < 12; k++) {
            char got[64] = "";
            for (int d = 0; d < 7; d++) {
                if (d) strcat(got, " ");
                strcat(got, theory_spelling_text(theory_spell_degree(k, sc, d)));
            }
            if (strcmp(got, SPELL_TABLE[sc][k]) || strcmp(theory_key_name_s(k, sc), theory_spelling_text(theory_spell_degree(k, sc, 0)))) {
                printf("     %s on key %d: '%s', want '%s'\n", theory_scale_name(sc), k, got, SPELL_TABLE[sc][k]);
                bad++;
            }
        }
    CHECK(!bad, "spelling: the 84 keys and scales against the hand-written table (%d wrong)", bad);
}

/* Verification 2: chord names, against expected strings. Majors: I..vii, then iv bVI bVII borrowed; minors: i..VII, then IV V bII */
static void test_fork_chord_names(void) {
    static const struct { int key, scale; const char *names; } T[] = {
        {5, SCALE_MAJOR, "F Gm Am Bb C Dm Edim | Bbm Db Eb"},
        {3, SCALE_MAJOR, "Eb Fm Gm Ab Bb Cm Ddim | Abm B Db"},      /* bVI Cb respelled B (S01-D3: Cb outside the key) */
        {8, SCALE_MAJOR, "Ab Bbm Cm Db Eb Fm Gdim | Dbm E Gb"},     /* bVI Fb respelled E */
        {1, SCALE_MAJOR, "Db Ebm Fm Gb Ab Bbm Cdim | Gbm A B"},     /* bVI Bbb respelled A (S01-D3: a double flat), bVII Cb B */
        {11, SCALE_MAJOR, "B C#m D#m E F# G#m A#dim | Em G A"},
        {6, SCALE_MAJOR, "F# G#m A#m B C# D#m E#dim | Bm D E"},     /* E#dim kept: E# is in the key */
        {1, SCALE_MINOR, "C#m D#dim E F#m G#m A B | F# G# D"},
        {3, SCALE_MINOR, "D#m E#dim F# G#m A#m B C# | G# A# E"},
        {8, SCALE_MINOR, "G#m A#dim B C#m D#m E F# | C# D# A"},
    };
    int bad = 0;
    for (size_t i = 0; i < sizeof T / sizeof T[0]; i++) {
        char got[96] = "", nm[24];
        for (int d = 0; d < 7 + THEORY_NBORROW; d++) {
            int ss = d < 7 ? T[i].scale : theory_borrow_scale(T[i].scale, d - 6), deg = d < 7 ? d : theory_borrow_degree(T[i].scale, d - 6);
            theory_chord_name_s(T[i].key, T[i].scale, ss, deg, COL_TRIAD, nm, sizeof nm);
            strcat(got, d == 7 ? " | " : d ? " " : "");
            strcat(got, nm);
        }
        if (strcmp(got, T[i].names)) printf("     key %d %s: '%s', want '%s'\n", T[i].key, theory_scale_name(T[i].scale), got, T[i].names), bad++;
    }
    CHECK(!bad, "spelling: triads and borrowed chords in F, Eb, Ab, Db, B, F# major and C#, D#, G# minor (%d wrong)", bad);
    static const struct { int key, scale, src, deg, colour; const char *want; } C[] = {
        {5, SCALE_MAJOR, 0, 0, COL_7TH, "Fmaj7"}, {5, SCALE_MAJOR, 0, 1, COL_7TH, "Gm7"}, {5, SCALE_MAJOR, 0, 4, COL_7TH, "C7"},
        {5, SCALE_MAJOR, 0, 6, COL_7TH, "Em7b5"}, {5, SCALE_MAJOR, 0, 3, COL_SUS4, "Bbsus4"}, {3, SCALE_MAJOR, 0, 3, COL_7TH, "Abmaj7"},
        {1, SCALE_MAJOR, 0, 0, COL_9TH, "Dbmaj9"}, {10, SCALE_MINOR, 0, 0, COL_7TH, "Bbm7"}, {6, SCALE_MAJOR, 0, 6, COL_7TH, "E#m7b5"},
        {0, SCALE_MAJOR, 4 + 1 * CQ_COUNT + CQ_DOM, 0, COL_7TH, "Db7"},   /* C major's chromatic bII7: Db7 beside its numeral */
        {1, SCALE_MAJOR, 4 + 8 * CQ_COUNT + CQ_MAJOR, 0, COL_TRIAD, "A"},   /* Db major's chromatic bVI: A, not Bbb (S01-D3) */
        {5, SCALE_MAJOR, 4 + 6 * CQ_COUNT + CQ_DOM, 0, COL_7TH, "B7"},   /* F major, root B: the tritone, #IV (S01-A1) */
    };
    for (size_t i = 0; i < sizeof C / sizeof C[0]; i++) {
        char nm[24], num[16];
        int ss = theory_slot_scale(C[i].scale, C[i].src);
        theory_chord_name_s(C[i].key, C[i].scale, ss, C[i].deg, C[i].colour, nm, sizeof nm);
        theory_numeral(C[i].key, C[i].scale, ss, C[i].deg, num, sizeof num);
        CHECK(!strcmp(nm, C[i].want), "spelling: %s in key %d %s, numeral %s (got %s)", C[i].want, C[i].key, theory_scale_name(C[i].scale), num, nm);
    }
}

/* S01-A1: a chromatic root's degree, by the harmonic chromatic scale (1 b2 2 b3 3 4 #4 5 b6 6 b7 7 above the tonic) and, for a chord
 * whose triad is diminished (a leading-tone chord), the raised degree below its target; names and numerals agree (S01-A2). Textbook
 * cases, written by hand */
static void test_fork_chromatic_degrees(void) {
    static const struct { int key, scale, root, q, colour; const char *name, *num; } T[] = {
        {2, SCALE_MINOR, 4, CQ_DOM, COL_7TH, "F#7", "#III"},          /* D minor's major chord a major third up: 0.28.1 read bIV, Gb7 */
        {0, SCALE_MAJOR, 1, CQ_MAJOR, COL_TRIAD, "Db", "bII"},        /* the Neapolitan */
        {0, SCALE_MAJOR, 1, CQ_DIM, COL_7TH, "C#dim7", "#i" THEORY_DIM},   /* vii dim7 of ii */
        {0, SCALE_MAJOR, 3, CQ_MAJOR, COL_TRIAD, "Eb", "bIII"},
        {0, SCALE_MAJOR, 3, CQ_DIM, COL_7TH, "D#dim7", "#ii" THEORY_DIM},  /* vii dim7 of iii */
        {0, SCALE_MAJOR, 6, CQ_DIM, COL_7TH, "F#dim7", "#iv" THEORY_DIM},  /* vii dim7 of V */
        {0, SCALE_MAJOR, 6, CQ_DOM, COL_7TH, "F#7", "#IV"},           /* the tritone: #4 in the harmonic chromatic scale */
        {0, SCALE_MAJOR, 8, CQ_MAJOR, COL_TRIAD, "Ab", "bVI"},
        {0, SCALE_MAJOR, 8, CQ_DIM, COL_7TH, "G#dim7", "#v" THEORY_DIM},   /* vii dim7 of vi */
        {0, SCALE_MAJOR, 10, CQ_MAJOR, COL_TRIAD, "Bb", "bVII"},
        {9, SCALE_MINOR, 1, CQ_MAJOR, COL_TRIAD, "Bb", "bII"},        /* A minor's Neapolitan */
        {9, SCALE_MINOR, 11, CQ_DIM, COL_7TH, "G#dim7", "#vii" THEORY_DIM},   /* A minor's leading-tone seventh */
        {9, SCALE_MINOR, 4, CQ_MAJOR, COL_TRIAD, "C#", "#III"},
        {0, SCALE_HARM_MINOR, 10, CQ_DIM, COL_TRIAD, "Bbdim", "bvii" THEORY_DIM},   /* no degree below: the one above */
    };
    int bad = 0;
    for (size_t i = 0; i < sizeof T / sizeof T[0]; i++) {
        char nm[24], nu[16];
        int ss = theory_slot_scale(T[i].scale, theory_chroma_src(T[i].root, T[i].q));
        theory_chord_name_s(T[i].key, T[i].scale, ss, 0, T[i].colour, nm, sizeof nm);
        theory_numeral(T[i].key, T[i].scale, ss, 0, nu, sizeof nu);
        if (strcmp(nm, T[i].name) || strcmp(nu, T[i].num)) printf("     key %d root +%d: %s %s, want %s %s\n", T[i].key, T[i].root, nm, nu, T[i].name, T[i].num), bad++;
    }
    CHECK(!bad, "chromatic roots by the harmonic chromatic scale, leading-tone chords raised: %d textbook cases (%d wrong)",
          (int)(sizeof T / sizeof T[0]), bad);
}

/* S01-D4, S01-D5 and the octave of a spelled note */
static void test_fork_notes(void) {
    char b[16];
    theory_chord_t d7 = {theory_slot_scale(SCALE_MAJOR, 4 + 9 * CQ_COUNT + CQ_DOM), 0, COL_7TH};   /* D7 in F major */
    theory_chord_t dim = {SCALE_HARM_MINOR, 6, COL_7TH};   /* G# harmonic minor's vii (F##, as named Gdim7) */
    CHECK(!strcmp(theory_pc_text_s(5, SCALE_MAJOR, NULL, 10), "Bb"), "a scale note takes the scale's letter: Bb in F");
    CHECK(!strcmp(theory_pc_text_s(5, SCALE_MAJOR, &d7, 6), "F#"), "a chord tone takes the chord's letter: D7's F# in F");
    CHECK(!strcmp(theory_pc_text_s(5, SCALE_MAJOR, NULL, 6), "Gb"), "a passing note leans to the key's side: Gb in F");
    CHECK(!strcmp(theory_pc_text_s(0, SCALE_MAJOR, NULL, 1), "C#"), "and to sharps in C");
    theory_note_name_s(6, SCALE_MAJOR, NULL, 65, b, sizeof b);
    CHECK(!strcmp(b, "E#4"), "MIDI 65 in F# major is E#4 (got %s)", b);
    theory_note_name_s(1, SCALE_HARM_MINOR, NULL, 60, b, sizeof b);
    CHECK(!strcmp(b, "B#3"), "MIDI 60 in C# harmonic minor is B#3, the octave of its letter (got %s)", b);
    theory_note_name_s(0, SCALE_MAJOR, NULL, 0, b, sizeof b);
    CHECK(!strcmp(b, "C-1"), "MIDI 0 is C-1, as theory_note_name says (got %s)", b);
    CHECK(!strcmp(theory_spelling_text(theory_spell_degree(8, SCALE_HARM_MINOR, 6)), "F##") &&
          !strcmp(theory_pc_text_s(8, SCALE_HARM_MINOR, &dim, 7), "G"),
          "G# harmonic minor: F## in the scale's spelling, shown G (S01-D3's rule for a double accidental)");
    theory_chord_name_s(8, SCALE_HARM_MINOR, SCALE_HARM_MINOR, 6, COL_TRIAD, b, sizeof b);
    CHECK(!strcmp(b, "Gdim"), "its vii is Gdim (got %s)", b);
    int t[THEORY_CHORD_MAX], n = theory_chord_tones(COL_SUS4, t);
    CHECK(n == 3 && t[0] == 1 && t[1] == 4 && t[2] == 5, "a sus4's tones are 1 4 5");
    CHECK(!strcmp(theory_spelling_text(theory_spell_tone(0, SCALE_MAJOR, SCALE_MAJOR, 0, 4)), "F"), "Csus4's 4th is F, not E#");
}

int main(void) {
    test_borrow();
    test_colours();
    test_names_match_triads();
    test_numerals();
    test_degrees();
    test_cards();
    test_voice_triads();
    test_voice_four();
    test_17_colour_notes();
    test_17_level_map();
    test_17_modes();
    test_17_drift();
    test_17_names();
    test_18_theory_avoid();
    test_fork_spelling_table();
    test_fork_chord_names();
    test_fork_notes();
    test_fork_chromatic_degrees();
    printf(fails ? "\nFAILED: %d\n" : "\nPASSED\n", fails);
    return fails != 0;
}
