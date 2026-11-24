/* The palette's front end (fork, docs/development/specs/05-part-palette.md, §3.2): what the core cannot answer without the CPG's catalogue,
 * which only the front end links (S04-D11). No state: each call reads the core. */
#pragma once
#include "seq_core.h"
#include "cpg_palettes.h"

/* A palette chord as the project holds it: where its scale places it (theory_palette_place), its colour, spice and start */
typedef struct { signed char deg, src, colour, spice, start; } palette_pad_t;

/* The playing part's palette, else its scale's (S05-D3: Scale Mode's, cpg_scale_palette), as the project's scale places each chord, into
 * out; the count (16 for a scale's) */
int palette_shown(const seq_t *s, palette_pad_t out[THEORY_PALETTE_MAX]);
/* S05-D7: the catalogue entry (cpg_entry's index) holding the most of the playing part's distinct slot chords (same root and pitch
 * classes), among the entries of the project scale's family (a minor tonic triad: the minor entries; else the major ones); ties to
 * catalogue order; -1 when none holds one. A suggestion: nothing is loaded */
int palette_nearest(const seq_t *s);
/* Loads catalogue entry i with its starting order into the playing part (seq_palette_load): 1, or 0 for a bad index */
int palette_load_entry(seq_t *s, int i);
