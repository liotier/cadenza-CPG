// The palette table (docs/development/specs/04-palette-table.md): the CPG's 184 catalogue palettes in their Classic style and its seven
// Scale Mode palettes, as Cadenza's chords, computed by the CPG's own modules (S04-D1) and written as constant C data. Node 20 or later.
//   node tools/cpg/generate.mjs <CPG folder> <chord listing> <names listing> <out dir>
// <chord listing>: tools/cpg/enumerate.c's output; <names listing>: its --names output. Writes into <out dir>: cpg_palettes.c (the table
// and its lookups, src/cpg_palettes.h's interface), cpg_expected.h (each stored chord's expected pitch classes, for
// tests/test_cpg_palettes.c only) and palettes.txt (the report, S04-D12). Strict (S04-D7): any chord with no exact representation, a
// palette the CPG builds otherwise in another key than C's transposed (the table stores C's, and the CPG guarantees the rest since its
// spec 18), an identifier collision or a missing nickname stops it, naming the entry. Deterministic: tools/cpg/generate.sh compares the
// output with the committed files.
import { readFileSync, writeFileSync } from 'node:fs';
import { pathToFileURL } from 'node:url';
import { resolve } from 'node:path';

const [cpgDir, listing, namesListing, outDir] = process.argv.slice(2);
if (!outDir) { console.error('usage: node tools/cpg/generate.mjs <CPG folder> <chord listing> <names listing> <out dir>'); process.exit(2); }
const mod = name => import(pathToFileURL(resolve(cpgDir, 'modules', name)).href);
const mt = await mod('musicTheory.js');
const gen = await mod('generation.js');
const en = JSON.parse(readFileSync(resolve(cpgDir, 'locales', 'en.json'), 'utf8'));
const fail = msg => { console.error(`generate: FAIL: ${msg}`); process.exit(1); };

// Cadenza's side (theory.h), checked against the listing below
const SCALES = ['Major', 'Minor', 'Dorian', 'Mixolydian', 'Phrygian', 'Lydian', 'Harmonic minor'];
const CPG_MODE = { 'Harmonic minor': 'Harmonic Minor' };
const NBORROW = 3, SRC_CHROMA = 4, CQ_COUNT = 4, SRC2 = 52, CQ2_COUNT = 5, SRC2_MAX = 111, COL_ALL = 13;
const QUALITY = ['major', 'minor', 'dominant', 'diminished', 'melodic minor', 'Locrian', 'dominant b5', 'Phrygian dominant', 'minor #5'];
const COLOUR = ['Triad', '7th', 'sus2', 'sus4', 'add9', 'Power', '9th', '11th', '13th', '6th', 'quartal', '7(no5)', '7sus4'];
const FIFTHLESS = new Set(['dom13', 'minor11']);   // S04-D6: decision 12's fifth rule, for these CPG types only
const second = src => src >= SRC2;
const qualityOf = src => second(src) ? CQ_COUNT + (src - SRC2) % CQ2_COUNT : (src - SRC_CHROMA) % CQ_COUNT;
const rootOfSrc = src => second(src) ? Math.floor((src - SRC2) / CQ2_COUNT) : Math.floor((src - SRC_CHROMA) / CQ_COUNT);

// The listing: per scale, "root|pcs" -> every representation; the chromatic ones are the same in every scale
const space = SCALES.map(() => new Map()), chroma = new Map();
const add = (m, k, v) => { if (!m.has(k)) m.set(k, []); m.get(k).push(v); };
for (const line of readFileSync(listing, 'utf8').split('\n')) {
  if (!line.trim()) continue;
  const [scale, src, deg, col, root, pcs] = line.split(' ').map((x, i) => i < 5 ? +x : x);
  const key = `${root}|${pcs}`, rep = { src, deg, col };
  add(space[scale], key, rep);
  if (scale === 0 && src >= SRC_CHROMA) add(chroma, key, rep);
}
if (Math.max(...[...chroma.values()].flat().map(r => r.src)) !== SRC2_MAX) fail('the listing does not reach THEORY_SRC2_MAX: rebuild tools/cpg/enumerate.c');
// S04-D6: the first range before the second, then the lowest colour, then the lowest quality
const canon = reps => [...reps].sort((a, b) => second(a.src) - second(b.src) || a.col - b.col || qualityOf(a.src) - qualityOf(b.src))[0];
// S05-D5, for the report: a degree of the project scale, else a borrowed chord, else the chromatic form; the lowest colour
const kind = src => src === 0 ? 0 : src <= NBORROW ? 1 : second(src) ? 3 : 2;
const inScale = (scale, key) => {
  const reps = space[scale].get(key);
  return reps && [...reps].sort((a, b) => kind(a.src) - kind(b.src) || a.col - b.col || a.src - b.src)[0];
};
const names = new Map();
for (const line of readFileSync(namesListing, 'utf8').split('\n')) {
  if (!line.trim()) continue;
  const [k, scale, src, deg, name] = line.split(' ');
  names.set(`${k} ${scale} ${src} ${deg}`, name);
}

const LETTER = { C: 0, D: 2, E: 4, F: 5, G: 7, A: 9, B: 11 };
const ascii = s => s.replaceAll('♭', 'b').replaceAll('♯', '#');
const comment = s => ascii(s).replaceAll('—', '-').replaceAll('°', 'o').replaceAll('ø', 'h').replace(/[^\x20-\x7e]/g, '?').replaceAll('*/', '* /');
const rootText = name => ascii(name).match(/^[A-G](##|bb|#|b)?/)[0];
const pcOfText = t => (LETTER[t[0]] + [...t.slice(1)].reduce((a, c) => a + (c === '#' ? 1 : -1), 0) + 24) % 12;
const pcsOf = notes => [...new Set(notes.map(n => ((n % 12) + 12) % 12))].sort((a, b) => a - b);
const maskOf = pcs => pcs.reduce((m, p) => m | 1 << p, 0);
const fnv1a = text => {
  let h = 0x811c9dc5;
  for (const b of Buffer.from(text, 'utf8')) h = Math.imul(h ^ b, 0x01000193) >>> 0;
  return h;
};

// A pad as a chromatic chord (S04-D4, S04-D6): its representation and the pitch classes that representation voices
function chromatic(where, pad, keyOffset = 0) {
  const root = (pcOfText(rootText(pad.chordName)) - keyOffset + 12) % 12, pcs = pcsOf(pad.notes.map(n => n - keyOffset));
  let rep = chroma.get(`${root}|${pcs.join(',')}`), voiced = pcs;
  if (!rep && FIFTHLESS.has(pad.chordType) && pcs.includes((root + 7) % 12)) {
    voiced = pcs.filter(p => p !== (root + 7) % 12);
    rep = chroma.get(`${root}|${voiced.join(',')}`);
  }
  if (!rep) fail(`${where}: ${pad.chordName} (${pad.chordType}, pitch classes ${pcs.join(' ')}) has no exact representation in Cadenza`);
  const r = canon(rep);
  if (rootOfSrc(r.src) !== root) fail(`${where}: ${pad.chordName}: representation on the wrong root`);
  return { root, quality: qualityOf(r.src), colour: r.col, src: r.src, pcs, voiced, fifthless: voiced !== pcs };
}

const genres = Object.keys(mt.progressions);
const entries = [], ids = new Map();
const stats = { quality: new Map(), colour: new Map(), mapped: [0, 0, 0, 0], layouts: new Map(), spellDiff: [], spellTotal: 0 };
const bump = (m, k) => m.set(k, (m.get(k) || 0) + 1);

for (const [g, genre] of genres.entries()) {
  for (const t of mt.progressions[genre]) {
    const where = `${genre} ${t.value}`;
    const id = fnv1a(`${genre}|${t.value}`);
    if (ids.has(id)) fail(`${where}: identifier ${id.toString(16)} already ${ids.get(id)}'s`);
    ids.set(id, where);
    const nickname = en.progressions?.[genre]?.[t.value]?.nickname;
    if (!nickname) fail(`${where}: no English nickname in locales/en.json`);
    const tonality = mt.templateTonality(t), minor = tonality === 'minor' ? 1 : 0, project = minor ? 1 : 0;
    const variant = gen.generateVariant('Classic', { key: 'C', mode: 'Major', progression: t.value, category: genre });
    if (variant.pads.length !== 16) fail(`${where}: ${variant.pads.length} pads`);
    const chords = variant.pads.map((p, k) => ({ ...chromatic(`${where} pad ${k + 1}`, p), spice: p.row - 1, start: p.isProgressionChord ? 1 : 0, pad: p }));
    // The table stores the palette once, as the CPG builds it in C: in every other key the CPG must build C's transposed (its spec 18,
    // which fixed row 4's key dependence), else the generation stops; and the report's spellings, in all twelve keys
    for (let key = 0; key < 12; key++) {
      const v = key ? gen.generateVariant('Classic', { key: mt.keys[key], mode: 'Major', progression: t.value, category: genre }) : variant;
      v.pads.forEach((p, k) => {
        const c = key ? chromatic(`${where} in ${mt.keys[key]} pad ${k + 1}`, p, key) : chords[k];
        if (c.root !== chords[k].root || c.pcs.join() !== chords[k].pcs.join())
          fail(`${where}: pad ${k + 1} in ${mt.keys[key]} is not C's transposed (the CPG's spec 18 guarantees it)`);
        const rep = inScale(project, `${c.root}|${c.voiced.join(',')}`) || { src: chords[k].src, deg: 0 };
        const cadenza = names.get(`${key} ${project} ${rep.src} ${rep.deg}`), cpg = rootText(p.chordName);
        stats.spellTotal++;
        if (cadenza !== cpg) stats.spellDiff.push(`${ascii(mt.keys[key]).padEnd(5)} ${comment(where)}: pad ${k + 1} ${ascii(p.chordName)}, Cadenza ${cadenza}`);
        if (!key) stats.mapped[kind(rep.src)]++;
      });
    }
    // The starting order: the parser's chords (never the text split: '12-bar-blues' is one token), each a palette chord, laid out by
    // decision 4's rule (S04-D9): whole bars in a loop of 1, 2, 4, 8 or 16, the spare bars to the tonic
    const tchords = mt.generateProgressionChords(t.value, 0, mt.getScaleDegrees('Major'), 'Major', mt.templateSpelling('C', tonality));
    const index = tchords.map((c, i) => {
      const root = ((c.scaleDegree % 12) + 12) % 12, pcs = pcsOf(c.notes).join();
      const k = chords.findIndex(x => x.start && x.root === root && x.pcs.join() === pcs);
      if (k < 0) fail(`${where}: template chord ${i + 1} (${c.romanNumeral}) is not a palette chord of the starting order`);
      return k;
    });
    const n = index.length;
    let bars = 1;
    while (bars < n) bars *= 2;
    if (bars > 16) fail(`${where}: ${n} chords, more than 16 bars`);
    const spare = bars - n, hold = index.map(() => 1), tonic = i => ((tchords[i].scaleDegree % 12) + 12) % 12 === 0;
    const rule = tonic(n - 1) ? 'ends on the tonic' : tonic(0) ? 'starts on the tonic' : 'neither';
    hold[rule === 'starts on the tonic' ? 0 : n - 1] += spare;
    const order = index.flatMap((k, i) => Array(hold[i]).fill(k));
    bump(stats.layouts, `${String(n).padStart(2)} chords: ${String(bars).padStart(2)} bars${spare ? `, ${rule}` : ''}`);
    for (const c of chords) bump(stats.quality, QUALITY[c.quality]), bump(stats.colour, COLOUR[c.colour]);
    entries.push({ id, g, genre, value: t.value, nickname, minor, bars, order, chords });
  }
}

// The scale palettes (S04-D5): Scale Mode in C, each chord its own scale's, as a slot holds it
const scalePalettes = SCALES.map((name, s) => gen.generateScaleExploration({ key: 'C', mode: CPG_MODE[name] || name }).pads.map((p, k) => {
  const root = pcOfText(rootText(p.chordName)), pcs = pcsOf(p.notes);
  const reps = (space[s].get(`${root}|${pcs.join(',')}`) || []).filter(r => r.src === 0).sort((a, b) => a.col - b.col);
  if (!reps.length) fail(`Scale Mode ${name} pad ${k + 1}: ${p.chordName} (${p.chordType}) is not a chord of the scale in Cadenza`);
  return { ...reps[0], mask: maskOf(pcs), name: ascii(p.chordName) };
}));
if (scalePalettes.some(p => p.length !== 16)) fail('a Scale Mode palette without 16 chords');

// C text: ASCII only, non-ASCII bytes of a nickname as octal escapes, comments transliterated
const cString = s => '"' + [...Buffer.from(s, 'utf8')].map(b => b === 0x22 || b === 0x5c ? '\\' + String.fromCharCode(b)
  : b >= 0x20 && b < 0x7f ? String.fromCharCode(b) : '\\' + b.toString(8).padStart(3, '0')).join('') + '"';
const hex2 = b => '0x' + b.toString(16).padStart(2, '0');
const sha = process.env.CPG_SHA || 'unknown';
const head = `Generated by tools/cpg/generate.sh from the MPC Chord Progression Generator (liotier/AkaiMPC at ${sha}): do not edit. CI regenerates it
 * and fails on any difference (docs/development/specs/04-palette-table.md).`;

const c = [];
c.push(`/* ${head} */`);
c.push('#include "cpg_palettes.h"', '#include "theory.h"', '#include <stddef.h>', '');
// The genre names and nicknames, one block of NUL-ended text, each entry an offset into it
const text = [], at = new Map();
let size = 0;
const put = s => { if (!at.has(s)) at.set(s, size), text.push(s), size += Buffer.byteLength(s, 'utf8') + 1; return at.get(s); };
const genreAt = genres.map(g => put(en.progressionCategories?.[g] || g));
for (const e of entries) e.nick = put(e.nickname);
if (size > 65535) fail('the text block passes 64 KB');
c.push(`static const char TEXT[${size}] =`);
text.forEach((s, i) => c.push(`    ${cString(s + (i < text.length - 1 ? '\0' : ''))}${i < text.length - 1 ? '' : ';'}`));
c.push(`static const unsigned short GENRE[${genres.length}] = {${genreAt.join(', ')}};`, '');
c.push('/* chord: root | quality << 4, colour | spice << 4 | start << 6; order: a palette index a bar, low nibble first */');
c.push(`static const cpg_entry_t ENTRY[${entries.length}] = {`);
for (const e of entries) {
  c.push(`    /* ${comment(e.genre)}: ${comment(e.value)} */`);
  const ch = e.chords.map(x => `{${hex2(x.root | x.quality << 4)}, ${hex2(x.colour | x.spice << 4 | x.start << 6)}}`);
  const ord = Array.from({ length: 8 }, (_, i) => hex2((e.order[2 * i] ?? 0) | (e.order[2 * i + 1] ?? 0) << 4));
  c.push(`    {0x${e.id.toString(16).padStart(8, '0')}u, ${e.nick}, ${e.g}, ${e.minor}, ${e.bars},   /* ${comment(e.nickname)} */`);
  c.push(`     {${ch.slice(0, 8).join(', ')},`, `      ${ch.slice(8).join(', ')}},`);
  c.push(`     {${ord.join(', ')}}},`);
}
c.push('};', '');
c.push('static const cpg_scale_chord_t SCALE_PALETTE[SCALE_COUNT][CPG_PADS] = {');
scalePalettes.forEach((p, s) => c.push(`    /* ${SCALES[s]} */ {${p.map(x => `{${x.deg}, ${x.src}, ${x.col}}`).join(', ')}},`));
c.push('};', '');
c.push(`int cpg_count(void) { return (int)(sizeof ENTRY / sizeof ENTRY[0]); }
const cpg_entry_t *cpg_entry(int i) { return i >= 0 && i < cpg_count() ? &ENTRY[i] : NULL; }
const cpg_entry_t *cpg_find(uint32_t id) {
    for (int i = 0; i < cpg_count(); i++)
        if (ENTRY[i].id == id) return &ENTRY[i];
    return NULL;
}
int cpg_genre_count(void) { return (int)(sizeof GENRE / sizeof GENRE[0]); }
const char *cpg_genre_name(int g) { return g >= 0 && g < cpg_genre_count() ? TEXT + GENRE[g] : NULL; }
const char *cpg_nickname(const cpg_entry_t *e) { return TEXT + e->nick; }
cpg_chord_t cpg_chord(const cpg_entry_t *e, int k) {
    const unsigned char *b = e->chord[k & (CPG_PADS - 1)];
    return (cpg_chord_t){(signed char)(b[0] & 15), (signed char)(b[0] >> 4), (signed char)(b[1] & 15), (signed char)(b[1] >> 4 & 3), (signed char)(b[1] >> 6 & 1)};
}
int cpg_order(const cpg_entry_t *e, unsigned char out[CPG_MAX_BARS]) {
    for (int i = 0; i < e->bars; i++) out[i] = (unsigned char)(e->order[i / 2] >> (i % 2 * 4) & 15);
    return e->bars;
}
int cpg_chord_src(cpg_chord_t c) { return c.quality < CQ_COUNT ? theory_chroma_src(c.root, c.quality) : theory_chroma2_src(c.root, c.quality - CQ_COUNT); }
const cpg_scale_chord_t *cpg_scale_palette(int scale) { return scale >= 0 && scale < SCALE_COUNT ? SCALE_PALETTE[scale] : NULL; }`);
writeFileSync(resolve(outDir, 'cpg_palettes.c'), c.join('\n') + '\n');

// The test's expectations (verification 2): each chord's pitch classes as the CPG voices it, and which take the fifth rule
const h = [`/* ${head} */`, '/* For tests/test_cpg_palettes.c only: the CPG pad\'s pitch classes (bit p: pitch class p above the tonic), the pads that take',
  ' * decision 12\'s fifth rule, each starting order, and the Scale Mode chords\' pitch classes. */', '#pragma once', ''];
h.push(`#define CPG_EXPECTED_ENTRIES ${entries.length}`);
h.push(`static const unsigned short CPG_EXPECTED_PCS[${entries.length}][16] = {`);
for (const e of entries) h.push(`    {${e.chords.map(x => '0x' + maskOf(x.pcs).toString(16).padStart(3, '0')).join(', ')}},`);
h.push('};');
h.push(`static const unsigned short CPG_EXPECTED_FIFTHLESS[${entries.length}] = {`);
for (let i = 0; i < entries.length; i += 12)
  h.push('    ' + entries.slice(i, i + 12).map(e => '0x' + e.chords.reduce((m, x, k) => m | (x.fifthless ? 1 << k : 0), 0).toString(16).padStart(4, '0')).join(', ') + ',');
h.push('};');
h.push(`static const unsigned char CPG_EXPECTED_ORDER[${entries.length}][17] = {   /* the bars, then a palette index a bar */`);
for (const e of entries) h.push(`    {${[e.bars, ...e.order].join(', ')}},`);
h.push('};');
h.push(`static const unsigned short CPG_EXPECTED_SCALE_PCS[${SCALES.length}][16] = {`);
for (const p of scalePalettes) h.push(`    {${p.map(x => '0x' + x.mask.toString(16).padStart(3, '0')).join(', ')}},`);
h.push('};');
writeFileSync(resolve(outDir, 'cpg_expected.h'), h.join('\n') + '\n');

// The report (S04-D12)
const sorted = m => [...m.entries()].sort((a, b) => b[1] - a[1] || (a[0] < b[0] ? -1 : 1));
const nickBytes = entries.reduce((a, e) => a + Buffer.byteLength(e.nickname, 'utf8') + 1, 0) + genres.reduce((a, g) => a + g.length + 1, 0);
const longest = entries.reduce((a, e) => [...e.nickname].length > [...a.nickname].length ? e : a);
const r = [];
r.push(`# ${head.replace('\n * ', ' ')}`, '');
r.push(`Entries: ${entries.length} in ${genres.length} genres; scale palettes: ${SCALES.length}; palettes: ${entries.length + SCALES.length}`);
r.push(`Data: chords ${entries.length} x 16 x 2 = ${entries.length * 32} bytes, starting orders ${entries.length} x 8 = ${entries.length * 8}, entries` +
  ` ${entries.length} x 52 = ${entries.length * 52}, nicknames and genre names ${nickBytes}, scale palettes ${SCALES.length * 48}: no pointer, no relocation`);
r.push(`Nicknames: longest ${[...longest.nickname].length} characters (${longest.nickname}); over 19 characters: ${entries.filter(e => [...e.nickname].length > 19).length}`);
r.push(`Tonality: ${entries.filter(e => !e.minor).length} major, ${entries.filter(e => e.minor).length} minor`, '');
r.push(`Chords stored (${entries.length * 16}), by quality:`);
for (const [k, v] of sorted(stats.quality)) r.push(`  ${String(v).padStart(6)}  ${k}`);
r.push('By colour:');
for (const [k, v] of sorted(stats.colour)) r.push(`  ${String(v).padStart(6)}  ${k}`);
r.push(`Pads taking decision 12's fifth rule: ${entries.reduce((a, e) => a + e.chords.filter(x => x.fifthless).length, 0)}`);
r.push('As a project in the entry\'s tonality (Major or Minor) would hold them (S05-D5):');
['a degree of the scale', 'a borrowed chord', 'a chromatic chord, first range', 'a chromatic chord, second range'].forEach((k, i) =>
  r.push(`  ${String(stats.mapped[i]).padStart(6)}  ${k}`));
r.push('', 'Starting orders, by template length (S04-D9: whole bars, the spare ones to the tonic):');
for (const [k, v] of [...stats.layouts.entries()].sort()) r.push(`  ${String(v).padStart(4)}  ${k}`);
r.push('Examples:');
const ex = (genre, value) => { const e = entries.find(x => x.genre === genre && x.value === value); return e && `  ${comment(value)}: ${e.order.map(k => ascii(e.chords[k].pad.chordName)).join(' ')}`; };
for (const [g, v] of [['Pop/Rock', 'I—V—vi—IV'], ['Jazz/Functional', 'ii—V—I'], ['Blues/Soul', '12-bar-blues'], ['Pop/Rock', 'I—V—vi—iii—IV']]) { const l = ex(g, v); if (l) r.push(l); }
r.push('', `Scale palettes (Scale Mode in C, each chord its scale's own): ${scalePalettes.map((p, s) => `${SCALES[s]} ${p.filter(x => x.col >= 10).length}`).join(', ')} with a colour past COL_COUNT`);
r.push('', `Every palette in every key is C's transposed (checked: the generation stops otherwise)`);
r.push('', `Root spellings, every chord in its entry's twelve keys, Cadenza (as S05-D5 writes it, spelled by spec 01) against the CPG: ${stats.spellTotal} compared, ${stats.spellDiff.length} differ`);
for (const l of stats.spellDiff.slice(0, 40)) r.push(`  ${l}`);
if (stats.spellDiff.length > 40) r.push(`  ... and ${stats.spellDiff.length - 40} more`);
writeFileSync(resolve(outDir, 'palettes.txt'), r.join('\n') + '\n');
