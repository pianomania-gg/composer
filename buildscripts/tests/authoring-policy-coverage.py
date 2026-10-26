#!/usr/bin/env python3
"""Generate exact policy data from the approved immutable authoring inventory.

Overrides are explicit coordinator decisions, not edits to the original audit.
This source check does not establish native UI or score round-trip acceptance.
"""
import argparse
import collections
import hashlib
import json
from pathlib import Path

APPROVED_SHA256 = "25c816c2b64fb4b26ee62481692b803a7102839b0b67002e69e2ba4675cd1bca"
OVERRIDES = {
    "action.dip": "hide creation",
    "palette.layout.action-icon-type-fframe": "hide creation",
}
# Explicit solo-piano scope correction. Keep the immutable audit and its other
# pending decisions; apply each restriction to the equivalent creation routes.
SOLO_PIANO_OVERRIDES = {
    "action.add-down-bow",
    "action.add-up-bow",
    "palette.articulations.sym-id-strings-harmonic",
    "palette.articulations.sym-id-plucked-snap-pizzicato-above",
    "palette.articulations.sym-id-wiggle-vibrato-large-faster",
    "palette.articulations.sym-id-wiggle-vibrato-large-slowest",
    "palette.text.arco",
    "palette.text.d-tach",
    "palette.text.martel",
    "palette.text.sul-pont",
    "palette.text.harmonics",
    "palette.text.mute",
    "palette.text.open",
    "palette.text.pizz",
    "palette.text.playing-technique-type-detache",
    "palette.text.playing-technique-type-martele",
    "palette.text.playing-technique-type-sul-ponticello",
    "palette.text.playing-technique-type-sul-tasto",
    "palette.text.playing-technique-type-pizzicato",
    "palette.text.playing-technique-type-vibrato",
    "master-symbol.strings-down-bow",
    "master-symbol.strings-down-bow-turned",
    "master-symbol.strings-up-bow",
    "master-symbol.strings-up-bow-turned",
    "master-symbol.strings-change-bow-direction",
    "master-symbol.strings-harmonic",
    "master-symbol.strings-half-harmonic",
    "master-symbol.strings-mute-on",
    "master-symbol.strings-mute-off",
    "master-symbol.strings-bow-behind-bridge",
    "master-symbol.strings-bow-behind-bridge-one-string",
    "master-symbol.strings-bow-behind-bridge-two-strings",
    "master-symbol.strings-bow-behind-bridge-three-strings",
    "master-symbol.strings-bow-behind-bridge-four-strings",
    "master-symbol.strings-bow-on-bridge",
    "master-symbol.strings-bow-on-tailpiece",
    "master-symbol.strings-overpressure-down-bow",
    "master-symbol.strings-overpressure-up-bow",
    "master-symbol.strings-overpressure-possibile-down-bow",
    "master-symbol.strings-overpressure-possibile-up-bow",
    "master-symbol.strings-overpressure-no-direction",
    "master-symbol.strings-jete-above",
    "master-symbol.strings-jete-below",
    "master-symbol.strings-fouette",
    "master-symbol.strings-vibrato-pulse",
    "master-symbol.plucked-snap-pizzicato-above",
    "master-symbol.plucked-snap-pizzicato-below",
    "master-symbol.plucked-buzz-pizzicato",
    "master-symbol.plucked-left-hand-pizzicato",
    "master-symbol.plucked-plectrum",
    "master-symbol.wiggle-vibrato-large-faster",
    "master-symbol.wiggle-vibrato-large-slowest",
}
# Reviewed chant/dance systems and instrument-specific aliases. Mixed ranges
# keep all other glyphs, including ordinary piano tremolos and fingerings.
SOLO_PIANO_OVERRIDES.update({
    "master-symbol.accdn-diatonic-clef",
    "master-symbol.indian-drum-clef",
    "master-symbol.fingering-qlower",
    "master-symbol.fingering-slower",
    "action.hammer-on-pull-off",
    "instrument.harpsichord",
    "master-symbol-range.daseian-notation",
    "master-symbol-range.kahnotation",
    "master-symbol-range.kievan-square-notation",
    "master-symbol-range.medieval-and-renaissance-plainchant-articulations",
    "master-symbol-range.medieval-and-renaissance-plainchant-multiple-note-forms",
    "master-symbol-range.medieval-and-renaissance-plainchant-single-note-forms",
    "master-symbol-range.techniques-noteheads",
    "master-symbol.buzz-roll",
    "master-symbol.chant-accentus-above",
    "master-symbol.chant-accentus-below",
    "master-symbol.chant-auctum-asc",
    "master-symbol.chant-auctum-desc",
    "master-symbol.chant-augmentum",
    "master-symbol.chant-circulus-above",
    "master-symbol.chant-circulus-below",
    "master-symbol.chant-connecting-line-asc2nd",
    "master-symbol.chant-connecting-line-asc3rd",
    "master-symbol.chant-connecting-line-asc4th",
    "master-symbol.chant-connecting-line-asc5th",
    "master-symbol.chant-connecting-line-asc6th",
    "master-symbol.chant-deminutum-lower",
    "master-symbol.chant-deminutum-upper",
    "master-symbol.chant-entry-line-asc2nd",
    "master-symbol.chant-entry-line-asc3rd",
    "master-symbol.chant-entry-line-asc4th",
    "master-symbol.chant-entry-line-asc5th",
    "master-symbol.chant-entry-line-asc6th",
    "master-symbol.chant-episema",
    "master-symbol.chant-ictus-above",
    "master-symbol.chant-ictus-below",
    "master-symbol.chant-ligatura-desc2nd",
    "master-symbol.chant-ligatura-desc3rd",
    "master-symbol.chant-ligatura-desc4th",
    "master-symbol.chant-ligatura-desc5th",
    "master-symbol.chant-oriscus-ascending",
    "master-symbol.chant-oriscus-descending",
    "master-symbol.chant-oriscus-liquescens",
    "master-symbol.chant-podatus-lower",
    "master-symbol.chant-podatus-upper",
    "master-symbol.chant-punctum",
    "master-symbol.chant-punctum-cavum",
    "master-symbol.chant-punctum-deminutum",
    "master-symbol.chant-punctum-inclinatum",
    "master-symbol.chant-punctum-inclinatum-auctum",
    "master-symbol.chant-punctum-inclinatum-deminutum",
    "master-symbol.chant-punctum-linea",
    "master-symbol.chant-punctum-linea-cavum",
    "master-symbol.chant-punctum-virga",
    "master-symbol.chant-punctum-virga-reversed",
    "master-symbol.chant-quilisma",
    "master-symbol.chant-semicirculus-above",
    "master-symbol.chant-semicirculus-below",
    "master-symbol.chant-strophicus",
    "master-symbol.chant-strophicus-auctus",
    "master-symbol.chant-strophicus-liquescens2nd",
    "master-symbol.chant-strophicus-liquescens3rd",
    "master-symbol.chant-strophicus-liquescens4th",
    "master-symbol.chant-strophicus-liquescens5th",
    "master-symbol.daseian-excellentes1",
    "master-symbol.daseian-excellentes2",
    "master-symbol.daseian-excellentes3",
    "master-symbol.daseian-excellentes4",
    "master-symbol.daseian-finales1",
    "master-symbol.daseian-finales2",
    "master-symbol.daseian-finales3",
    "master-symbol.daseian-finales4",
    "master-symbol.daseian-graves1",
    "master-symbol.daseian-graves2",
    "master-symbol.daseian-graves3",
    "master-symbol.daseian-graves4",
    "master-symbol.daseian-residua1",
    "master-symbol.daseian-residua2",
    "master-symbol.daseian-superiores1",
    "master-symbol.daseian-superiores2",
    "master-symbol.daseian-superiores3",
    "master-symbol.daseian-superiores4",
    "master-symbol.double-lateral-roll-stevens",
    "master-symbol.fingering-alower",
    "master-symbol.fingering-clower",
    "master-symbol.fingering-elower",
    "master-symbol.fingering-ilower",
    "master-symbol.fingering-mlower",
    "master-symbol.fingering-olower",
    "master-symbol.fingering-plower",
    "master-symbol.fingering-tlower",
    "master-symbol.fingering-tupper",
    "master-symbol.fingering-xlower",
    "master-symbol.guitar-vibrato-stroke",
    "master-symbol.guitar-wide-vibrato-stroke",
    "master-symbol.kahn-back-chug",
    "master-symbol.kahn-back-flap",
    "master-symbol.kahn-back-riff",
    "master-symbol.kahn-back-rip",
    "master-symbol.kahn-ball-change",
    "master-symbol.kahn-ball-dig",
    "master-symbol.kahn-brush-backward",
    "master-symbol.kahn-brush-forward",
    "master-symbol.kahn-chug",
    "master-symbol.kahn-clap",
    "master-symbol.kahn-double-snap",
    "master-symbol.kahn-double-wing",
    "master-symbol.kahn-draw-step",
    "master-symbol.kahn-draw-tap",
    "master-symbol.kahn-flam",
    "master-symbol.kahn-flap",
    "master-symbol.kahn-flap-step",
    "master-symbol.kahn-flat",
    "master-symbol.kahn-flea-hop",
    "master-symbol.kahn-flea-tap",
    "master-symbol.kahn-grace-tap",
    "master-symbol.kahn-grace-tap-change",
    "master-symbol.kahn-grace-tap-hop",
    "master-symbol.kahn-grace-tap-stamp",
    "master-symbol.kahn-heel",
    "master-symbol.kahn-heel-change",
    "master-symbol.kahn-heel-click",
    "master-symbol.kahn-heel-drop",
    "master-symbol.kahn-heel-step",
    "master-symbol.kahn-heel-tap",
    "master-symbol.kahn-hop",
    "master-symbol.kahn-jump-apart",
    "master-symbol.kahn-jump-together",
    "master-symbol.kahn-knee-inward",
    "master-symbol.kahn-knee-outward",
    "master-symbol.kahn-leap",
    "master-symbol.kahn-leap-flat-foot",
    "master-symbol.kahn-leap-heel-click",
    "master-symbol.kahn-left-catch",
    "master-symbol.kahn-left-cross",
    "master-symbol.kahn-left-foot",
    "master-symbol.kahn-left-toe-strike",
    "master-symbol.kahn-left-turn",
    "master-symbol.kahn-over-the-top",
    "master-symbol.kahn-over-the-top-tap",
    "master-symbol.kahn-pull",
    "master-symbol.kahn-push",
    "master-symbol.kahn-riff",
    "master-symbol.kahn-riffle",
    "master-symbol.kahn-right-catch",
    "master-symbol.kahn-right-cross",
    "master-symbol.kahn-right-foot",
    "master-symbol.kahn-right-toe-strike",
    "master-symbol.kahn-right-turn",
    "master-symbol.kahn-rip",
    "master-symbol.kahn-ripple",
    "master-symbol.kahn-scrape",
    "master-symbol.kahn-scuff",
    "master-symbol.kahn-scuffle",
    "master-symbol.kahn-shuffle",
    "master-symbol.kahn-slam",
    "master-symbol.kahn-slap",
    "master-symbol.kahn-slide-step",
    "master-symbol.kahn-slide-tap",
    "master-symbol.kahn-snap",
    "master-symbol.kahn-stamp",
    "master-symbol.kahn-stamp-stamp",
    "master-symbol.kahn-step",
    "master-symbol.kahn-step-stamp",
    "master-symbol.kahn-stomp",
    "master-symbol.kahn-stomp-brush",
    "master-symbol.kahn-tap",
    "master-symbol.kahn-toe",
    "master-symbol.kahn-toe-click",
    "master-symbol.kahn-toe-drop",
    "master-symbol.kahn-toe-step",
    "master-symbol.kahn-toe-tap",
    "master-symbol.kahn-trench",
    "master-symbol.kahn-wing",
    "master-symbol.kahn-wing-change",
    "master-symbol.kahn-zank",
    "master-symbol.kahn-zink",
    "master-symbol.kievan-accidental-flat",
    "master-symbol.kievan-accidental-sharp",
    "master-symbol.kievan-augmentation-dot",
    "master-symbol.kievan-cclef",
    "master-symbol.kievan-ending-symbol",
    "master-symbol.kievan-note-beam",
    "master-symbol.kievan-note-half-staff-line",
    "master-symbol.kievan-note-half-staff-space",
    "master-symbol.kievan-note-quarter-stem-down",
    "master-symbol.kievan-note-quarter-stem-up",
    "master-symbol.kievan-note-reciting",
    "master-symbol.kievan-note-whole",
    "master-symbol.kievan-note-whole-final",
    "master-symbol.kievan-note8th-stem-down",
    "master-symbol.kievan-note8th-stem-up",
    "master-symbol.one-handed-roll-stevens",
    "master-symbol.semipitched-percussion-clef1",
    "master-symbol.semipitched-percussion-clef2",
    "master-symbol.stem-bow-on-bridge",
    "master-symbol.stem-bow-on-tailpiece",
    "master-symbol.stem-buzz-roll",
    "master-symbol.stem-harp-string-noise",
    "master-symbol.stem-rim-shot",
    "master-symbol.stem-sprechgesang",
    "master-symbol.stem-sul-ponticello",
    "master-symbol.swiss-rudiments-notehead-black-double",
    "master-symbol.swiss-rudiments-notehead-black-flam",
    "master-symbol.swiss-rudiments-notehead-half-double",
    "master-symbol.swiss-rudiments-notehead-half-flam",
    "master-symbol.tremolo-divisi-dots2",
    "master-symbol.tremolo-divisi-dots3",
    "master-symbol.tremolo-divisi-dots4",
    "master-symbol.tremolo-divisi-dots6",
    "master-symbol.unpitched-percussion-clef1",
    "master-symbol.unpitched-percussion-clef2",
    "palette.text.col-legno",
    "palette.text.distort",
    "palette.text.jazz-tone",
    "palette.text.playing-technique-type-distortion",
    "palette.tremolo.sym-id-tremolo-divisi-dots2",
    "palette.tremolo.sym-id-tremolo-divisi-dots3",
    "palette.tremolo.sym-id-tremolo-divisi-dots4",
    "palette.tremolo.sym-id-tremolo-divisi-dots6",
    "palette.tremolo.tremolo-type-buzz-roll",
})
OVERRIDES.update({feature_id: "hide creation" for feature_id in sorted(SOLO_PIANO_OVERRIDES)})
DECISIONS = {"keep": "Allowed", "hide creation": "Hidden", "retain pending review": "ReviewRetained"}


def load_rows(path):
    raw = path.read_bytes()
    if hashlib.sha256(raw).hexdigest() != APPROVED_SHA256:
        raise ValueError("Approved authoring inventory digest changed")
    original = json.loads(raw)["rows"]
    if len(original) != 5131 or len({r["feature_id"] for r in original}) != 5131:
        raise ValueError("Unexpected inventory row identity set")
    if collections.Counter(r["recommendation"] for r in original) != {"keep": 1102, "hide creation": 1674, "retain pending review": 2355}:
        raise ValueError("Unexpected original recommendation counts")
    if not OVERRIDES.keys() <= {r["feature_id"] for r in original}:
        raise ValueError("An explicit override has no approved inventory identity")
    rows = sorted(({**r, "original_recommendation": r["recommendation"], "recommendation": OVERRIDES.get(r["feature_id"], r["recommendation"])} for r in original), key=lambda r: r["feature_id"])
    return rows


def generate(rows, ranges):
    hidden = [r for r in rows if r["recommendation"] == "hide creation"]
    hidden_symbols = {r["variant"] for r in hidden if r["family"] == "master_symbol_variant"}
    hidden_ranges = [r["variant"] for r in hidden if r["family"] == "master_symbol_range"]
    exact_symbols = {r["variant"] for r in hidden if r["family"] == "master_symbol_variant" and r["feature_id"] in SOLO_PIANO_OVERRIDES}
    if hidden_symbols != {g for key in hidden_ranges for g in ranges[key]["glyphs"]} | exact_symbols:
        raise ValueError("Approved hidden glyph set differs from hidden ranges")
    articulation_symbols = set()
    text_types, clefs, techniques = set(), set(), {"PlayingTechniqueType::Overdrive"}
    for row in hidden:
        if row["family"] != "palette_variant":
            continue
        v = row["variant"]
        family = row["feature_id"].split(".")[1]
        if v.startswith("SymId::") and family != "accordion" and v != "SymId::stringsThumbPosition":
            articulation_symbols.add(v)
        elif v.startswith("ArticulationTextType::"):
            text_types.add(v)
        elif v.startswith("ClefType::"):
            clefs.add(v)
        elif v.startswith("PlayingTechniqueType::") and v != "PlayingTechniqueType::Natural":
            techniques.add(v)
        elif row["feature_id"] in {"palette.text.harmonics", "palette.text.mute", "palette.text.open", "palette.text.distort", "palette.text.jazz-tone", "palette.text.col-legno"}:
            techniques.add({"palette.text.harmonics": "PlayingTechniqueType::Harmonics",
                            "palette.text.mute": "PlayingTechniqueType::Mute",
                            "palette.text.open": "PlayingTechniqueType::Open",
                            "palette.text.distort": "PlayingTechniqueType::Distortion",
                            "palette.text.jazz-tone": "PlayingTechniqueType::JazzTone",
                            "palette.text.col-legno": "PlayingTechniqueType::ColLegno"}[row["feature_id"]])
    lines = ["// Generated by buildscripts/tests/authoring-policy-coverage.py.", "// Immutable inventory SHA-256: " + APPROVED_SHA256, "#pragma once", "#include <array>", "#include <string_view>", '#include "engraving/dom/articulation.h"', '#include "engraving/types/types.h"', "namespace mu::notation::authoring {", "using namespace mu::engraving;", "enum class Decision { Allowed, Hidden, ReviewRetained };", "struct CapabilityRow { std::string_view id; std::string_view family; std::string_view variant; Decision decision; };", 'inline constexpr std::string_view catalogVersion = "pianomania.authoring-capabilities.v1";', "inline constexpr std::array<CapabilityRow, 5131> capabilityRows = {{"]
    for row in rows:
        strings = [json.dumps(row[k], ensure_ascii=True) for k in ["feature_id", "family", "variant"]]
        lines.append("    { " + ", ".join(strings) + ", Decision::" + DECISIONS[row["recommendation"]] + " },")
    lines.append("}};")
    for name, typename, values in [("hiddenArticulationSymbols", "SymId", sorted(articulation_symbols)), ("hiddenArticulationTexts", "ArticulationTextType", sorted(text_types)), ("hiddenClefs", "ClefType", sorted(clefs)), ("hiddenPlayingTechniques", "PlayingTechniqueType", sorted(techniques))]:
        lines += [f"inline constexpr std::array<{typename}, {len(values)}> {name} = {{{{", *["    " + v + "," for v in values], "}};"]
    descriptions = sorted(ranges[key]["description"] for key in hidden_ranges)
    lines += [f"inline constexpr std::array<std::string_view, {len(descriptions)}> hiddenRangeDescriptions = {{{{", *["    " + json.dumps(v, ensure_ascii=True) + "," for v in descriptions], "}};", "} // namespace mu::notation::authoring", ""]
    return "\n".join(lines).encode()


def coverage(rows):
    return {"schema": "pianomania.authoring-policy-effective-coverage.v1", "immutable_inventory_sha256": APPROVED_SHA256, "original_rows": 5131, "effective_rows": dict(collections.Counter(r["recommendation"] for r in rows)), "effective_hidden_recorded_obligations": sum(len(r["authoring_surfaces"]) for r in rows if r["recommendation"] == "hide creation"), "overrides": OVERRIDES, "pending_original_rows_preserved": all(r["recommendation"] == "retain pending review" for r in rows if r["original_recommendation"] == "retain pending review" and r["feature_id"] not in SOLO_PIANO_OVERRIDES), "native_behavior_verified": False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory", type=Path, required=True)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    rows = load_rows(args.inventory)
    ranges = json.loads((args.source / "fonts/smufl/ranges.json").read_text())
    data = generate(rows, ranges)
    target = args.source / "src/notation/internal/authoringcapabilitypolicydata.h"
    if args.check:
        if not target.is_file() or target.read_bytes() != data:
            raise SystemExit("Generated authoring policy header is stale")
    else:
        target.write_bytes(data)
    receipt = coverage(rows)
    if args.receipt:
        args.receipt.write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt, indent=2))


if __name__ == "__main__":
    main()
