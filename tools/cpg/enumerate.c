/* Every chord Cadenza's theory layer can voice (docs/development/specs/00-infrastructure.md, 3.4; the study's Appendix A,
 * item 1): at key C, for each of the seven scales, every src from 0 to THEORY_SRC2_MAX (spec 03: the second chromatic range too), the
 * degrees each allows (any for src 0, the borrow's degree for 1..THEORY_NBORROW, degree 0 for the chromatic chords) and every colour up
 * to COL_ALL (spec 03: the colours no control offers too). One line a chord:
 *   scale src degree colour root-pc pitch-classes notes
 * pitch classes ascending and comma-separated, notes as semitones above C4 (theory_chord_notes, less 48), comma-separated.
 * Read by tools/cpg/coverage.mjs. Built against src/theory.c alone: tools/cpg/coverage.sh. */
#include <stdio.h>
#include "theory.h"

int main(void) {
    for (int scale = 0; scale < SCALE_COUNT; scale++)
        for (int src = 0; src <= THEORY_SRC2_MAX; src++)
            for (int deg = 0; deg < 7; deg++) {
                if (src >= 1 && src <= THEORY_NBORROW && deg != theory_borrow_degree(scale, src)) continue;
                if (src > THEORY_NBORROW && deg != 0) continue;
                int ss = theory_slot_scale(scale, src);
                for (int col = 0; col < COL_ALL; col++) {
                    int out[THEORY_CHORD_MAX], n = theory_chord_notes(0, ss, deg, col, out), mask = 0, first = 1;
                    for (int i = 0; i < n; i++) mask |= 1 << (out[i] % 12);
                    printf("%d %d %d %d %d ", scale, src, deg, col, theory_degree_pc(0, ss, deg));
                    for (int pc = 0; pc < 12; pc++)
                        if (mask >> pc & 1) { printf(first ? "%d" : ",%d", pc); first = 0; }
                    printf(" ");
                    for (int i = 0; i < n; i++) printf(i ? ",%d" : "%d", out[i] - 48);
                    printf("\n");
                }
            }
    return 0;
}
