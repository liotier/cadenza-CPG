#include "palette.h"
#include <string.h>

/* A chord's root over C and the pitch classes it voices, as one key: root x 4096 + the 12-bit set */
static int chord_key(int slot_scale, int deg, int colour) {
    int n[THEORY_CHORD_MAX], k = theory_chord_notes(0, slot_scale, deg, colour, n), m = 0;
    while (k--) m |= 1 << n[k] % 12;
    return theory_degree_pc(0, slot_scale, deg) * 4096 + m;
}

static int pal_key(theory_palette_chord_t c) { return chord_key(theory_slot_scale(SCALE_MAJOR, theory_palette_src(c)), 0, c.colour); }

int palette_shown(const seq_t *s, palette_pad_t out[THEORY_PALETTE_MAX]) {
    theory_palette_chord_t c[THEORY_PALETTE_MAX];
    int n = seq_palette(s, c), scale = seq_scale(s);
    if (!n) {
        const cpg_scale_chord_t *p = cpg_scale_palette(scale);
        for (int k = 0; p && k < CPG_PADS; k++) out[k] = (palette_pad_t){p[k].deg, p[k].src, p[k].colour, (signed char)(k / 4), 0};
        return p ? CPG_PADS : 0;
    }
    for (int k = 0; k < n; k++) {
        int d, sr;
        theory_palette_place(scale, c[k], &d, &sr);
        out[k] = (palette_pad_t){(signed char)d, (signed char)sr, c[k].colour, c[k].spice, c[k].start};
    }
    return n;
}

int palette_nearest(const seq_t *s) {
    int have[SEQ_MAX_PROG], nh = 0, scale = seq_scale(s), minor = theory_triad_quality(0, scale, 0) == 1, best = -1, most = 0;
    for (int i = 0; i < seq_slots(s); i++) {   /* the part's distinct chords */
        int k = chord_key(theory_slot_scale(scale, seq_slot_src(s, i)), seq_slot_deg(s, i), seq_slot_colour(s, i)), dup = 0;
        for (int j = 0; j < nh; j++) dup |= have[j] == k;
        if (!dup) have[nh++] = k;
    }
    for (int e = 0; e < cpg_count(); e++) {
        const cpg_entry_t *x = cpg_entry(e);
        int keys[CPG_PADS], shared = 0;
        if (x->minor != minor) continue;
        for (int k = 0; k < CPG_PADS; k++) keys[k] = pal_key(cpg_chord(x, k));
        for (int j = 0; j < nh; j++) {
            int in = 0;
            for (int k = 0; k < CPG_PADS; k++) in |= keys[k] == have[j];
            shared += in;
        }
        if (shared > most) most = shared, best = e;
    }
    return best;
}

int palette_load_entry(seq_t *s, int i) {
    const cpg_entry_t *e = cpg_entry(i);
    theory_palette_chord_t c[CPG_PADS];
    unsigned char order[CPG_MAX_BARS];
    if (!e) return 0;
    for (int k = 0; k < CPG_PADS; k++) c[k] = cpg_chord(e, k);
    return seq_palette_load(s, e->id, c, CPG_PADS, order, cpg_order(e, order));
}
