// How much of the CPG's catalogue Cadenza's chord space represents exactly (docs/development/specs/00-infrastructure.md, 3.4;
// the study's Appendix A, items 2 and 3). Node 20 or later.
//   node tools/cpg/coverage.mjs <CPG folder> <chord listing>
// <CPG folder>: AkaiMPCChordProgressionGenerator (third_party/AkaiMPC/...); <chord listing>: tools/cpg/enumerate.c's output.
// Every template is rendered by the CPG's own modules in C major, assigned the Cadenza scale holding most of its template
// chords (diatonic first, then borrowed, then chromatic or near), and each chord matched on (root, pitch-class set):
//   diatonic   a degree of that scale (src 0)        borrowed  one of its three borrowed chords (src 1-3)
//   chromatic  a chromatic chord (src 4-111)         near      the same root and set but for the perfect fifth
//   absent     none of these
// A chord is exact when diatonic, borrowed or chromatic, or near with the fifth omitted by Cadenza (decision 12 of the study: its
// 11ths and 13ths leave the fifth out by design); the other near and absent chords need a new sonority. Step 3 (specs/03) added the
// second chromatic range (src 52-111) and three colours past COL_COUNT (10-12): the report counts the pads exact only with them, the
// Classic pads with no chromatic form (step 4's table stores every chord as one), and the Scale Mode palettes (generateScaleExploration,
// decision 18's scale palettes) holding a chord Cadenza cannot voice.
// Template chords: generateProgressionChords, repeats kept, root from the parser's scaleDegree. Pads: generateVariant in each of
// the six styles, root from the spelled chord name. Prints a deterministic report: tools/cpg/coverage.sh writes it to
// docs/development/measurements/coverage.txt.
import { readFileSync } from 'node:fs';
import { pathToFileURL } from 'node:url';
import { resolve } from 'node:path';

const [cpgDir, listing] = process.argv.slice(2);
if (!cpgDir || !listing) { console.error('usage: node tools/cpg/coverage.mjs <CPG folder> <chord listing>'); process.exit(2); }
const mod = name => import(pathToFileURL(resolve(cpgDir, 'modules', name)).href);
const mt = await mod('musicTheory.js');
const gen = await mod('generation.js');

const SCALES = ['Major', 'Minor', 'Dorian', 'Mixolydian', 'Phrygian', 'Lydian', 'Harmonic minor'];
const CLASSES = ['diatonic', 'borrowed', 'chromatic', 'near', 'absent'];
const RANK = { diatonic: 0, borrowed: 1, chromatic: 2 };

// Cadenza's chord space: per scale, "root|pcs" -> the best class that voices it; `older`, the same before step 3 (src 0-51, colours
// 0-9); `chroma`, the chromatic chords alone, the same in every scale
const space = SCALES.map(() => new Map()), older = SCALES.map(() => new Map()), chroma = [new Map()];
const SRC_MAX = 51, COL_COUNT = 10;
let chords = 0, olderChords = 0;
for (const line of readFileSync(listing, 'utf8').split('\n')) {
  if (!line.trim()) continue;
  const [scale, src, , col, root, pcs] = line.split(' ');
  const cls = +src === 0 ? 'diatonic' : +src <= 3 ? 'borrowed' : 'chromatic';
  const key = `${root}|${pcs}`, old = +src <= SRC_MAX && +col < COL_COUNT;
  for (const m of [space[+scale], ...(old ? [older[+scale]] : []), ...(cls === 'chromatic' ? [chroma[0]] : [])])
    if (!m.has(key) || RANK[cls] < RANK[m.get(key)]) m.set(key, cls);
  chords++;
  olderChords += old;
}

const LETTER = { C: 0, D: 2, E: 4, F: 5, G: 7, A: 9, B: 11 };
const ACC = { '': 0, '#': 1, '##': 2, b: -1, bb: -2 };
const rootOf = name => { const m = name.match(/^([A-G])(##|bb|#|b)?/); return (LETTER[m[1]] + ACC[m[2] || ''] + 12) % 12; };
const pcsOf = notes => [...new Set(notes.map(n => ((n % 12) + 12) % 12))].sort((a, b) => a - b);

// { cls, kind }: kind, for near, says which side has the fifth; in spaces (default: Cadenza's whole space)
function classify(scale, root, pcs, spaces = space) {
  const m = spaces[scale], fifth = (root + 7) % 12;
  const hit = m.get(`${root}|${pcs.join(',')}`);
  if (hit) return { cls: hit };
  if (pcs.includes(fifth)) {
    if (m.has(`${root}|${pcs.filter(p => p !== fifth).join(',')}`)) return { cls: 'near', kind: 'Cadenza omits the fifth' };
  } else if (m.has(`${root}|${[...pcs, fifth].sort((a, b) => a - b).join(',')}`)) {
    return { cls: 'near', kind: 'Cadenza adds the fifth' };
  }
  return { cls: 'absent' };
}

const OMITS = 'Cadenza omits the fifth';
const exact = c => c.cls === 'diatonic' || c.cls === 'borrowed' || c.cls === 'chromatic' || c.kind === OMITS;
const tally = () => Object.fromEntries(CLASSES.map(c => [c, 0]));
const bump = (map, key) => map.set(key, (map.get(key) || 0) + 1);
const sorted = map => [...map.entries()].sort((a, b) => b[1] - a[1] || (a[0] < b[0] ? -1 : a[0] > b[0] ? 1 : 0));

const progT = tally(), padT = tally();
const styleT = Object.fromEntries(gen.VARIANT_STYLES.map(s => [s, tally()]));
const bestScale = new Map(SCALES.map(s => [s, 0]));
const progMissing = new Map(), padMissing = new Map();
let templates = 0, allDiatonic = 0, srcUpTo3 = 0, allExact = 0, padsOmit = 0, padsNew = 0, padsStep3 = 0, classicNoChroma = 0;
const step3Missing = new Map(), classicMissing = new Map();
const genres = Object.keys(mt.progressions);

for (const [category, list] of Object.entries(mt.progressions)) {
  for (const t of list) {
    templates++;
    const tchords = mt.generateProgressionChords(t.value, 0, mt.getScaleDegrees('Major'), 'Major',
      mt.templateSpelling('C', mt.templateTonality(t)));
    let best = null;
    for (let s = 0; s < SCALES.length; s++) {
      const cs = tchords.map(c => classify(s, c.scaleDegree % 12, pcsOf(c.notes)));
      const n = cls => cs.filter(c => cls.includes(c.cls)).length;
      const score = [n(['diatonic']), n(['borrowed']), n(['chromatic', 'near'])];
      if (!best || score[0] > best.score[0] || (score[0] === best.score[0] && (score[1] > best.score[1]
          || (score[1] === best.score[1] && score[2] > best.score[2])))) best = { s, cs, score };
    }
    bump(bestScale, SCALES[best.s]);
    best.cs.forEach((c, i) => {
      progT[c.cls]++;
      if (c.cls === 'near' || c.cls === 'absent') bump(progMissing, `${tchords[i].chordType} (${c.kind || 'absent'})`);
    });
    if (best.cs.every(c => c.cls === 'diatonic')) allDiatonic++;
    if (best.cs.every(c => c.cls === 'diatonic' || c.cls === 'borrowed')) srcUpTo3++;
    if (best.cs.every(exact)) allExact++;
    for (const style of gen.VARIANT_STYLES) {
      const v = gen.generateVariant(style, { key: 'C', mode: 'Major', progression: t.value, category });
      for (const p of v.pads) {
        const c = classify(best.s, rootOf(p.chordName), pcsOf(p.notes));
        padT[c.cls]++;
        styleT[style][c.cls]++;
        if (c.cls === 'near' || c.cls === 'absent') bump(padMissing, `${p.chordType} (${c.kind || 'absent'})`);
        if (c.kind === OMITS) padsOmit++; else if (!exact(c)) padsNew++;
        if (exact(c) && !exact(classify(best.s, rootOf(p.chordName), pcsOf(p.notes), older))) padsStep3++, bump(step3Missing, p.chordType);
        if (style === 'Classic' && !exact(classify(0, rootOf(p.chordName), pcsOf(p.notes), chroma)))
          classicNoChroma++, bump(classicMissing, p.chordType);
      }
    }
  }
}

// Scale Mode: each Cadenza scale's palette, in C, against that scale's space
const CPG_MODE = { 'Harmonic minor': 'Harmonic Minor' };
const scaleMode = SCALES.map((name, s) => {
  const pads = gen.generateScaleExploration({ key: 'C', mode: CPG_MODE[name] || name }).pads;
  return { name, missing: pads.filter(p => !exact(classify(s, rootOf(p.chordName), pcsOf(p.notes)))).map(p => p.chordName) };
});

const total = t => Object.values(t).reduce((a, b) => a + b, 0);
const pct = (v, n) => (100 * v / n).toFixed(1).padStart(5) + ' %';
const table = t => { const n = total(t); return CLASSES.map(c => `  ${c.padEnd(10)} ${String(t[c]).padStart(6)}  ${pct(t[c], n)}`); };
const out = [];
out.push(`Cadenza's chord space: ${chords} chords (${SCALES.length} scales, every src, degree and colour, at key C); ${olderChords} before step 3`);
out.push('');
out.push(`CPG catalogue: ${templates} templates in ${genres.length} genres`);
out.push(`Best Cadenza scale per template: ${sorted(bestScale).filter(([, v]) => v).map(([k, v]) => `${k} ${v}`).join(', ')}`);
out.push('');
out.push(`Template chords, repeats included: ${total(progT)}`);
out.push(...table(progT));
out.push(`Templates entirely diatonic in their best scale: ${allDiatonic}`);
out.push(`Templates within src 0-3 (diatonic and borrowed only): ${srcUpTo3}`);
out.push(`Templates whose every chord is exact: ${allExact}`);
out.push('Template chords not exact, by CPG chord type:');
for (const [k, v] of sorted(progMissing)) out.push(`  ${String(v).padStart(6)}  ${k}`);
out.push('');
out.push(`Pads, all six styles (${templates} x ${gen.VARIANT_STYLES.length} x 16): ${total(padT)}`);
out.push(...table(padT));
out.push('Pads per style (diatonic, borrowed, chromatic, near, absent):');
for (const s of gen.VARIANT_STYLES) out.push(`  ${s.padEnd(13)} ${CLASSES.map(c => String(styleT[s][c]).padStart(5)).join(' ')}`);
out.push(`Pads exact with the fifth omitted (decision 12): ${padsOmit}`);
out.push(`Pads needing a new sonority: ${padsNew}`);
out.push('Pads not exact, by CPG chord type:');
for (const [k, v] of sorted(padMissing)) out.push(`  ${String(v).padStart(6)}  ${k}`);
out.push(`Pads exact only with step 3's values (the second chromatic range, a colour past COL_COUNT): ${padsStep3}`);
for (const [k, v] of sorted(step3Missing)) out.push(`  ${String(v).padStart(6)}  ${k}`);
out.push(`Classic pads with no chromatic form (step 4's table stores each as a chromatic chord): ${classicNoChroma}`);
for (const [k, v] of sorted(classicMissing)) out.push(`  ${String(v).padStart(6)}  ${k}`);
out.push('');
out.push(`Scale Mode palettes (generateScaleExploration, in C) with a chord Cadenza cannot voice: ${scaleMode.filter(m => m.missing.length).length} of ${SCALES.length}`);
for (const m of scaleMode) if (m.missing.length) out.push(`  ${m.name.padEnd(15)} ${m.missing.join(' ')}`);
console.log(out.join('\n'));
