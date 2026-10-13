#include "notationcompatibility.h"

#include <algorithm>
#include <cmath>
#include <QCoreApplication>
#include <QMap>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QXmlStreamReader>

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/spanner.h"
#include "engraving/types/symnames.h"

using namespace mu::project::notationcompatibility;
namespace {
// BEGIN GENERATED NOTATION CATALOG
static constexpr const char* NOTATION_CATALOG_DIGEST = "b6a7b1c3561e7a6eadacc8fea506a31aefca7303553ed021e48e7155ce348a03";
static constexpr const char* NOTATION_CATALOG_JSON = R"PMCAT({"acceptedLayerChildren":["note","chord","beam","tuplet","graceGrp","rest","mRest","clef"],"acceptedStructuralContainers":["mei","music","body","mdiv","score","scoreDef","section","ending","measure","staff"],"arpeggioGeometry":{"brackets":"order=nonarp, arrow=false, solid","coordinates":"page inches, bottom-left origin, Y-up, CCW degrees","members":"hash references to emitted existing written note IDs; no synthesized identity","normal":"order=up, arrow=false, wavy","stageEvidence":"Source contract accepted for implementation; native/render/MIDI/input/scoring unverified.","straight":"explicit order, arrow=true, solid, pm:arpeggio-style=straight","up_down":"explicit order, arrow=true, wavy","version":1},"articulationStageEvidence":{"input":"unverified","model":"existing_component_models_source_inspected","parse":"Original five-token component parser inspected against original source; token groups are already supported.","playback":"unverified","render":"existing_component_and_composite_paths_source_inspected; no new feature claim","scoring":"unverified"},"capabilityCatalogVersion":"pianomania.notation-capabilities.v1","creationPolicy":"Independent of compatibility admission. Preserve imported items. Pending features stay available.","evidenceBoundary":"Catalog records inspected source behavior. It does not attest rendered, MIDI, input, scoring or device acceptance.","fatalRules":["malformed_xml","duplicate_identity","invalid_note_identity_pitch_timing","unsupported_playable_wrapper","invalid_tie_anchor","unbound_ordinary_midi_occurrence","ambiguous_traversal"],"fidelityReviews":[{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"featureId":"source-visibility","humanName":"Source visibility","observedBehavior":"unverified","reasonCode":"source_visibility_requires_correspondence","stage":"export"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"featureId":"rehearsal-mark","humanName":"Rehearsal mark fidelity","observedBehavior":"unverified","reasonCode":"rehearsal_fidelity_unverified","stage":"render"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"featureId":"symbol-parentheses","humanName":"Parenthesis and enclosure fidelity","observedBehavior":"unverified","reasonCode":"enclosure_fidelity_unverified","stage":"export"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"featureId":"ornament-recognized","humanName":"Recognized ornament variants","observedBehavior":"unverified","reasonCode":"ornament_variant_unverified","stage":"render"},{"affectedModes":["practice","game_solo","flow"],"featureId":"note-playback-disabled","humanName":"Playback-disabled note performance","observedBehavior":"unverified","reasonCode":"muted_note_performance_unverified","stage":"playback"}],"hostParity":{"generator":"scripts/generate-notation-capabilities.py","requiredCheck":"--check --host all after Unity source release; --check --host helper while Unity frozen","rule":"Each feature support change updates this catalog and regenerates both hosts in one integration; no host may silently promote a stage from unverified. Pending Composer implementation must consume this same version and vocabulary."},"modes":["practice","game_solo","flow","native_preview","webgl_preview"],"parsedArticulationCombinations":["acc stacc","marc stacc","marc ten","acc ten","stacc ten"],"parsedArticulationTokens":["stacc","stacciss","ten","marc","acc"],"recognizedOrnaments":["trill","mordent","turn"],"runtimeOmissions":[{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"arpeg","featureId":"arpeggio","humanName":"Rolled-chord signs","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"gliss","featureId":"glissando","humanName":"Glissando signs","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"ornam","featureId":"ornament-generic","humanName":"Generic ornaments","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"harm","featureId":"harmony","humanName":"Chord symbols","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"breath","featureId":"breath-caesura","humanName":"Breath and caesura marks","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"caesura","featureId":"breath-caesura","humanName":"Breath and caesura marks","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"fb","featureId":"figured-bass","humanName":"Figured bass","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"verse","featureId":"lyrics","humanName":"Lyrics","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"}],"schemaVersion":"pianomania.notation-capability-catalog.v1","sourceVariantComparisons":{"arpeggio":{"meiElement":"arpeg","meiProperties":["order","form","arrow","startid","plist","pm:arpeggio-style","pm:arpeggio-play","lform","pm:arpeggio-members","pm:arpeggio-geometry-version","pm:arpeggio-page","pm:arpeggio-visible","pm:arpeggio-bounds","pm:arpeggio-line-width","pm:arpeggio-lines","pm:arpeggio-glyphs"],"sourceElement":"Arpeggio","sourceProperty":"subtype","variants":{"0":"normal","1":"up","2":"down","3":"bracket","4":"up-straight","5":"down-straight"}}},"stages":["source","export","parse","model","render","playback","input","scoring","traversal"],"supportedArticulationCombinations":["acc stacc","marc stacc","marc ten","acc ten","stacc ten"],"supportedArticulationTokens":["stacc","stacciss","ten","marc","acc"],"topology":{"currentRuntimeStaffIds":["1","2"],"featureId":"piano-staff-topology","importedEditing":"preserved","migration":"Preserve N source/display staff identities and voices. Note hand remains independent. Never silently collapse a playable staff.","sourceCreation":"allowed","unsupportedExportedIdentity":"fatal"},"unsupportedArticulationNames":{"spicc":"Wedge staccatissimo"}})PMCAT";
// END GENERATED NOTATION CATALOG

QJsonObject catalog()
{
    return QJsonDocument::fromJson(NOTATION_CATALOG_JSON).object();
}
QString sha256(const QByteArray& bytes)
{
    return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}
QByteArray asciiJson(QByteArray value)
{
    QString unicode = QString::fromUtf8(value);
    QByteArray result;
    for (QChar character : unicode) {
        if (character.unicode() < 128) result += char(character.unicode());
        else result += QByteArray("\\u") + QByteArray::number(character.unicode(), 16).rightJustified(4, '0');
    }
    return result;
}
QString itemIdentity(const mu::engraving::EngravingItem* item)
{
    return item && item->eid().isValid() ? QString::fromStdString(item->eid().toStdString()) : QString();
}
QJsonArray strings(const QStringList& values)
{
    QJsonArray result;
    for (const QString& value : values) result.append(value);
    return result;
}
QStringList values(const QJsonArray& array)
{
    QStringList result;
    for (const QJsonValue& value : array) result.append(value.toString());
    return result;
}
QJsonObject reportBase(const QString& sourcePath)
{
    auto capabilities = catalog();
    QStringList modes = values(capabilities["modes"].toArray());
    std::sort(modes.begin(), modes.end());
    QJsonObject completeness;
    for (const QString& mode : modes) {
        QJsonObject stages;
        for (const QJsonValue& value : capabilities["stages"].toArray()) {
            QString stage = value.toString();
            stages[stage] = mode.endsWith("_preview") && (stage == "input" || stage == "scoring")
                ? "not_applicable" : "unverified";
        }
        stages["source"] = "partial";
        completeness[mode] = stages;
    }
    QFile source(sourcePath);
    QString sourceDigest;
    if (source.exists() && source.open(QIODevice::ReadOnly)) sourceDigest = sha256(source.readAll());
    return {
        {"schemaVersion", "pianomania.notation-compatibility-report.v1"},
        {"capabilityCatalogVersion", capabilities["capabilityCatalogVersion"]},
        {"capabilityCatalogDigest", NOTATION_CATALOG_DIGEST},
        {"scoreIdentity", QFileInfo(sourcePath).fileName()}, {"sourceDigest", sourceDigest}, {"meiDigest", ""},
        {"exporterProvenance", "native-composer-compatibility-v1"},
        {"inspectedStages", QJsonArray{"source"}}, {"targetModes", strings(modes)},
        {"completeness", completeness}, {"findings", QJsonArray()}
    };
}
void finishReport(QJsonObject& report)
{
    QJsonArray findings = report["findings"].toArray();
    QList<QJsonObject> sorted;
    for (const QJsonValue& value : findings) sorted.append(value.toObject());
    std::sort(sorted.begin(), sorted.end(), [](const auto& left, const auto& right) {
        return left["findingId"].toString() < right["findingId"].toString();
    });
    findings = {};
    for (const auto& item : sorted) findings.append(item);
    report["findings"] = findings;
    report.remove("reportDigest");
    report["reportDigest"] = sha256(mu::project::notationcompatibility::canonicalJson(report));
}
void addFinding(QJsonObject& report, const QString& feature, const QString& variant,
                const QString& human, const QString& reason, const QJsonObject& location,
                const QString& stage, const QString& unit, bool fatal = false,
                const QString& behavior = "", const QJsonArray& omitted = QJsonArray{"visual"},
                const QJsonArray& affectedModes = {})
{
    QString observed = behavior.isEmpty() ? (fatal ? "rejected" : "omitted") : behavior;
    QJsonArray modes = report["targetModes"].toArray();
    if (!affectedModes.isEmpty()) {
        QJsonArray selected;
        for (const QJsonValue& mode : modes) if (affectedModes.contains(mode)) selected.append(mode);
        modes = selected;
    }
    if (modes.isEmpty()) return;
    QJsonArray key {feature, variant, fatal ? "fatal" : "warning", reason, stage, modes, observed, omitted, unit};
    QString id = sha256(asciiJson(QJsonDocument(key).toJson(QJsonDocument::Compact)));
    QJsonArray findings = report["findings"].toArray();
    int found = -1;
    for (int index = 0; index < findings.size(); ++index) {
        if (findings[index].toObject()["findingId"] == id) { found = index; break; }
    }
    QJsonObject row = found >= 0 ? findings[found].toObject() : QJsonObject {
        {"findingId", id}, {"featureId", feature}, {"variant", variant}, {"humanName", human},
        {"severity", fatal ? "fatal" : "warning"}, {"reasonCode", reason}, {"stage", stage},
        {"affectedModes", modes}, {"observedBehavior", observed}, {"omittedBehavior", omitted},
        {"countUnit", unit}, {"count", 0}, {"locations", QJsonArray()}, {"representativeExamples", QJsonArray()}
    };
    QJsonArray locations = row["locations"].toArray(); locations.append(location);
    QJsonArray examples = row["representativeExamples"].toArray();
    if (examples.size() < 3) examples.append(location["itemIdentity"]);
    row["count"] = locations.size(); row["locations"] = locations; row["representativeExamples"] = examples;
    if (found >= 0) findings[found] = row; else findings.append(row);
    report["findings"] = findings;
}
struct XmlItem {
    QString name;
    QString namespaceUri;
    QString text;
    QJsonObject attributes;
    QJsonObject location;
    int parent = -1;
};
// Match the placed single-segment display contract; stage acceptance stays unverified.
QString glissandoDisplayOmission(const QList<XmlItem>& items, int index, bool& textOmitted)
{
    const auto& attributes = items[index].attributes;
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    QList<int> segments;
    for (int child = index + 1; child < items.size(); ++child)
        if (items[child].parent == index && items[child].name == "gliss-segment"
            && items[child].namespaceUri == "http://pianomania.gg/ns/mei") segments.append(child);
    if (segments.isEmpty()) return "glissando_geometry_missing";
    if (segments.size() != 1) return "glissando_multisegment_omission";
    const auto number = [](const QString& value, double& result) {
        bool valid = false; result = value.toDouble(&valid);
        return !value.isEmpty() && valid && std::isfinite(result);
    };
    const auto tuple = [&number](const QString& value, int count, QList<double>& result) {
        const auto fields = value.split(',');
        if (fields.size() != count) return false;
        result.clear();
        for (const auto& field : fields) {
            double parsed = 0; if (!number(field, parsed)) return false;
            result.append(parsed);
        }
        return true;
    };
    QList<double> start, end, center;
    const QString form = attributes["lform"].toString();
    const auto flag = [](const QString& value) { return value == "true" || value == "false"; };
    const auto anchor = [](const QString& value) {
        return value.startsWith('#') && value.size() > 1
            && std::none_of(value.cbegin(), value.cend(), [](QChar c) { return c.isSpace(); });
    };
    if ((form != "solid" && form != "wavy")
        || !anchor(attributes["startid"].toString()) || !anchor(attributes["endid"].toString())
        || !flag(attributes["visible"].toString()) || !flag(attributes[prefix + "gliss-play"].toString())
        || !tuple(attributes[prefix + "gliss-start-center"].toString(), 2, center)
        || !tuple(attributes[prefix + "gliss-end-center"].toString(), 2, center)) return "glissando_display_omission";
    const int segmentIndex = segments.first();
    const auto segment = items[segmentIndex].attributes;
    bool validPage = false; int page = segment["page"].toString().toInt(&validPage);
    double width = 0;
    if (!validPage || page < 0 || !tuple(segment["start"].toString(), 2, start)
        || !tuple(segment["end"].toString(), 2, end) || start == end
        || !number(segment["width"].toString(), width) || width <= 0) return "glissando_display_omission";
    for (int child = segmentIndex + 1; child < items.size(); ++child) {
        if (items[child].parent != segmentIndex) continue;
        const auto& ink = items[child];
        const auto inkAttributes = ink.attributes;
        if (ink.name != "glyph" && ink.name != "text") return "glissando_display_omission";
        const QString font = inkAttributes["font"].toString();
        const auto corners = inkAttributes["corners"].toString().split(';');
        QList<QList<double>> points;
        if (font.isEmpty() || corners.size() != 4) return "glissando_display_omission";
        for (const auto& corner : corners) {
            QList<double> point;
            if (!tuple(corner, 2, point)) return "glissando_display_omission";
            points.append(point);
        }
        const auto& a = points[0]; const auto& b = points[1]; const auto& c = points[2];
        if ((b[0] - a[0]) * (c[1] - a[1]) == (b[1] - a[1]) * (c[0] - a[0])) return "glissando_display_omission";
        if (ink.name == "glyph") {
            if (inkAttributes["name"] != "wiggleGlissando"
                || (form == "wavy" && font != "Leland" && font != "Bravura")) return "glissando_display_omission";
        } else {
            bool validStyle = false; int style = inkAttributes["font-style"].toString().toInt(&validStyle);
            if (ink.text.isEmpty() || !validStyle || style < 0 || style > 15) return "glissando_display_omission";
            const bool supported = ((font == "Edwin" && style == 2) || (font == "Academico" && style == 1))
                && std::all_of(ink.text.cbegin(), ink.text.cend(), [](QChar c) { return c >= ' ' && c <= '~'; });
            textOmitted |= !supported;
        }
    }
    return {};
}
QString arpeggioVariant(const QJsonObject& attributes)
{
    QString order = attributes["order"].toString("normal");
    if (order == "nonarp") return "bracket";
    QString style = attributes["{http://pianomania.gg/ns/mei}arpeggio-style"].toString();
    bool straight = style == "straight" || attributes["form"] == "straight" || attributes["lform"] == "solid";
    if (order == "up" && attributes["arrow"] == "false" && (style == "wavy" || attributes["lform"] == "wavy")) return "normal";
    return straight ? order + "-straight" : order;
}
}

QByteArray mu::project::notationcompatibility::canonicalJson(const QJsonObject& object)
{
    return asciiJson(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

SourceInventory mu::project::notationcompatibility::inventoryScore(mu::engraving::Score* score, const QString& sourcePath)
{
    SourceInventory inventory;
    inventory.report = reportBase(sourcePath);
    if (!score) {
        auto completeness = inventory.report["completeness"].toObject();
        for (const QString& mode : completeness.keys()) {
            auto stages = completeness[mode].toObject(); stages["source"] = "unverified"; completeness[mode] = stages;
        }
        inventory.report["completeness"] = completeness;
        inventory.report["inspectedStages"] = QJsonArray();
        finishReport(inventory.report); return inventory;
    }
    using namespace mu::engraving;
    QSet<const EngravingItem*> visited;
    QList<Spanner*> authoredGlissandi;
    QSet<const Spanner*> collectedGlissandi;
    auto collectGlissando = [&](Spanner* spanner) {
        if (!spanner || !spanner->isGlissando() || collectedGlissandi.contains(spanner)) return;
        collectedGlissandi.insert(spanner);
        authoredGlissandi.append(spanner);
    };
    QJsonArray content;
    const QJsonObject capabilities = catalog();
    const QStringList variants {"normal", "up", "down", "bracket", "up-straight", "down-straight"};
    auto visit = [&](EngravingItem* item) {
        if (item && item->isSpannerSegment()) {
            Spanner* spanner = toSpannerSegment(item)->spanner();
            if (spanner && spanner->isGlissando()) item = spanner;
        }
        if (!item || visited.contains(item) || item->generated()) return;
        visited.insert(item);
        QString identity = itemIdentity(item);
        if (identity.isEmpty()) identity = "source:" + QString::number(visited.size());
        auto measure = score->tick2measure(item->tick());
        QJsonObject attributes {{"sourceType", item->typeName()}, {"visible", item->visible() ? "1" : "0"}};
        if (item->isNote()) {
            auto note = toNote(item);
            attributes["pitch"] = QString::number(note->pitch());
            attributes["play"] = note->play() ? "1" : "0";
            for (Spanner* spanner : note->spannerFor()) collectGlissando(spanner);
        }
        QJsonObject location {
            {"itemIdentity", identity}, {"inventoryOrdinal", int(visited.size() - 1)},
            {"measureId", itemIdentity(measure)},
            {"displayedMeasureNumber", measure ? QString::number(measure->no() + 1) : QString()},
            {"staffId", item->hasStaff() ? QString::number(int(item->staffIdx()) + 1) : QString()},
            {"voice", item->hasStaff() ? QString::number(int(item->voice()) + 1) : QString()},
            {"anchorIds", QJsonArray()}, {"attributes", attributes}
        };
        content.append(location);
        if (!item->visible()) {
            addFinding(inventory.report, "source-visibility", "hidden", "Source visibility fidelity",
                       "source_visibility_requires_correspondence", location, "export", "source_item", false, "unverified", {});
        }
        if (item->isNote() && !toNote(item)->play()) {
            addFinding(inventory.report, "note-playback-disabled", "play-disabled", "Playback-disabled note performance",
                       "muted_note_performance_unverified", location, "playback", "source_item", false, "unverified", {}, {"practice", "game_solo", "flow"});
        }
        if (item->isArpeggio() || item->type() == ElementType::CHORD_BRACKET) {
            auto arpeggio = static_cast<Arpeggio*>(item);
            int subtype = int(arpeggio->arpeggioType());
            QString variant = item->type() == ElementType::CHORD_BRACKET ? "bracket"
                : subtype >= 0 && subtype < variants.size() ? variants[subtype] : "unknown:" + QString::number(subtype);
            auto chord = arpeggio->chord();
            bool expected = item->staff() && item->staff()->show() && item->staff()->part()->show() && chord && chord->visible();
            bool visibleNote = false;
            QJsonArray anchors;
            if (chord) for (const Note* note : chord->notes()) { anchors.append(itemIdentity(note)); visibleNote |= note->visible(); }
            expected &= visibleNote;
            location["anchorIds"] = anchors;
            auto attrs = location["attributes"].toObject(); attrs["subtype"] = QString::number(subtype); location["attributes"] = attrs;
            inventory.items.append(QJsonObject {{"featureId", "arpeggio"}, {"variant", variant}, {"location", location}, {"expectedExport", expected}});
            if (expected) addFinding(inventory.report, "arpeggio", variant, "Rolled-chord signs", "arpeggio_render_unverified", location, "render", "source_item", false, "unverified", {});
        }
        if (item->isGlissando()) {
            Spanner* glissando = toSpanner(item);
            EngravingItem* start = glissando->startElement();
            EngravingItem* end = glissando->endElement();
            QJsonArray anchors;
            if (start) anchors.append(itemIdentity(start));
            if (end) anchors.append(itemIdentity(end));
            location["anchorIds"] = anchors;
            auto visibleAnchor = [](EngravingItem* anchor) {
                return anchor && anchor->isNote() && anchor->visible()
                    && toNote(anchor)->chord() && toNote(anchor)->chord()->visible();
            };
            bool expected = item->visible() && item->staff() && item->staff()->show()
                && item->staff()->part()->show() && visibleAnchor(start) && visibleAnchor(end);
            if (expected) addFinding(inventory.report, "glissando", "gliss", "Glissando signs",
                                     "glissando_render_unverified", location, "render", "source_item", false, "unverified", {});
        }
        if (item->isArticulation()) {
            auto articulation = toArticulation(item);
            QString name = QString::fromUtf8(SymNames::nameForSymId(articulation->symId()).ascii());
            auto attrs = location["attributes"].toObject(); attrs["symId"] = name; location["attributes"] = attrs;
            bool core = name.startsWith("articStaccato") || name.startsWith("articStaccatissimoAbove")
                || name.startsWith("articStaccatissimoBelow") || name.startsWith("articTenutoAbove")
                || name.startsWith("articTenutoBelow") || name.startsWith("articAccentAbove")
                || name.startsWith("articAccentBelow") || name.startsWith("articMarcatoAbove") || name.startsWith("articMarcatoBelow")
                || name == "articAccentStaccatoAbove" || name == "articAccentStaccatoBelow"
                || name == "articMarcatoStaccatoAbove" || name == "articMarcatoStaccatoBelow"
                || name == "articMarcatoTenutoAbove" || name == "articMarcatoTenutoBelow"
                || name == "articTenutoAccentAbove" || name == "articTenutoAccentBelow"
                || name == "articTenutoStaccatoAbove" || name == "articTenutoStaccatoBelow";
            QString token;
            QString human;
            if (name.contains("StaccatissimoWedge")) { token = "spicc"; human = "Wedge staccatissimo"; }
            else if (name.contains("StaccatissimoStroke")) token = "stroke";
            else if (name.contains("SoftAccent")) token = "acc-soft";
            else if (name.contains("Unstress")) token = "unstress";
            else if (name.contains("Stress")) token = "stress";
            if (!token.isEmpty() && !values(capabilities["supportedArticulationTokens"].toArray()).contains(token)) {
                bool parsed = values(capabilities["parsedArticulationTokens"].toArray()).contains(token);
                addFinding(inventory.report, "articulation-uncommon", token, human.isEmpty() ? "Articulation " + token : human,
                           parsed ? "articulation_render_unverified" : "unsupported_articulation", location,
                           parsed ? "render" : "parse", "source_item", false, parsed ? "unverified" : "omitted",
                           parsed ? QJsonArray() : QJsonArray{"visual"});
            } else if (!core && token.isEmpty()) {
                addFinding(inventory.report, "articulation-uncommon", name, "Articulation variant fidelity", "source_articulation_unverified", location, "render", "source_item", false, "unverified", {});
            }
        }
    };
    score->scanElements(visit);
    // Scan authored tracks explicitly: rendered-page scans omit hidden staves.
    for (Segment* segment = score->firstSegment(SegmentType::All); segment; segment = segment->next1(SegmentType::All)) {
        for (EngravingItem* item : segment->elist()) if (item) item->scanElements(visit);
        for (EngravingItem* item : segment->annotations()) if (item) item->scanElements(visit);
    }
    // Note-anchored spanners do not visit their physical owner through scanElements.
    for (const auto& entry : score->spanner()) collectGlissando(entry.second);
    for (Spanner* glissando : authoredGlissandi) visit(glissando);
    int shown = 0;
    for (Staff* staff : score->staves()) if (staff->show() && staff->part()->show()) ++shown;
    if (shown > 2) {
        QJsonObject location {{"itemIdentity", "source-staff-topology"}, {"anchorIds", QJsonArray()}, {"attributes", QJsonObject{{"visibleStaffCount", QString::number(shown)}}}};
        addFinding(inventory.report, "piano-staff-topology", QString::number(shown), "Piano staff topology",
                   "source_topology_export_unverified", location, "export", "source_item", false, "unverified", {});
    }
    inventory.report["scoreIdentity"] = QFileInfo(sourcePath).fileName() + ":" + sha256(asciiJson(QJsonDocument(content).toJson(QJsonDocument::Compact)));
    finishReport(inventory.report);
    return inventory;
}

QJsonObject mu::project::notationcompatibility::analyzeMei(const QByteArray& mei, const SourceInventory& source,
                        const QHash<QString, QString>& sourceToCanonical, bool canonicalMidiValidated)
{
    QJsonObject report = source.report;
    report["findings"] = QJsonArray();
    report["meiDigest"] = sha256(mei);
    bool inspectedSource = values(source.report["inspectedStages"].toArray()).contains("source");
    report["inspectedStages"] = inspectedSource ? QJsonArray{"source", "export", "parse"} : QJsonArray{"parse"};
    QJsonObject capabilities = catalog();
    QStringList allowed = values(capabilities["acceptedStructuralContainers"].toArray())
        + values(capabilities["acceptedLayerChildren"].toArray())
        + QStringList{"layer", "beam", "tuplet", "graceGrp", "chord"};
    QStringList core = values(capabilities["supportedArticulationTokens"].toArray());
    QList<XmlItem> items;
    QList<int> stack;
    QHash<QString, int> byId;
    QXmlStreamReader reader(mei);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            XmlItem item; item.name = reader.name().toString(); item.namespaceUri = reader.namespaceUri().toString();
            item.parent = stack.isEmpty() ? -1 : stack.last();
            for (const auto& attribute : reader.attributes()) {
                QString key = attribute.namespaceUri().isEmpty() ? attribute.name().toString()
                    : "{" + attribute.namespaceUri().toString() + "}" + attribute.name().toString();
                item.attributes[key] = attribute.value().toString();
            }
            QString id = item.attributes["{http://www.w3.org/XML/1998/namespace}id"].toString();
            item.location = {{"itemIdentity", id.isEmpty() ? "mei:" + QString::number(items.size()) : id},
                             {"inventoryOrdinal", int(items.size())}, {"anchorIds", QJsonArray()}, {"attributes", item.attributes}};
            for (int ancestor = item.parent; ancestor >= 0; ancestor = items[ancestor].parent) {
                if (items[ancestor].name == "measure") {
                    item.location["measureId"] = items[ancestor].attributes["{http://www.w3.org/XML/1998/namespace}id"];
                    item.location["displayedMeasureNumber"] = items[ancestor].attributes["n"];
                }
                if (items[ancestor].name == "staff") item.location["staffId"] = items[ancestor].attributes["n"];
                if (items[ancestor].name == "layer") item.location["voice"] = items[ancestor].attributes["n"];
            }
            QJsonArray anchors;
            for (const QString& name : {QString("startid"), QString("endid"), QString("plist"), QString("{http://pianomania.gg/ns/mei}arpeggio-members")}) {
                for (QString value : item.attributes[name].toString().split(' ', Qt::SkipEmptyParts)) { if (value.startsWith('#')) value.remove(0, 1); anchors.append(value); }
            }
            item.location["anchorIds"] = anchors;
            if (!id.isEmpty()) {
                if (byId.contains(id)) addFinding(report, "note-chord-identity", "duplicate", "Duplicate notation identities", "duplicate_identity", item.location, "parse", "mei_item", true, "", {"structure"});
                byId[id] = items.size();
            }
            items.append(item); stack.append(items.size() - 1);
        } else if (reader.isCharacters() && !stack.isEmpty()) items[stack.last()].text += reader.text().toString();
        else if (reader.isEndElement() && !stack.isEmpty()) stack.removeLast();
    }
    if (reader.hasError()) {
        addFinding(report, "note-chord-identity", "xml", "Malformed MEI", "malformed_xml", {{"itemIdentity", "document"}, {"anchorIds", QJsonArray()}}, "parse", "mei_item", true, "", {"structure"});
    }
    QSet<int> reportedPlayableWrappers;
    for (int index = 0; index < items.size(); ++index) {
        const XmlItem& item = items[index];
        if (item.name == "note") {
            const XmlItem& timing = item.parent >= 0 && items[item.parent].name == "chord" ? items[item.parent] : item;
            bool idxOk = false, durationOk = false, beatOk = false, octaveOk = false;
            int idx = item.attributes["idx"].toString().toInt(&idxOk);
            int duration = (item.attributes.contains("dur") ? item.attributes["dur"] : timing.attributes["dur"]).toString().toInt(&durationOk);
            double beat = timing.attributes["beat"].toString().toDouble(&beatOk);
            int octave = item.attributes["oct"].toString().toInt(&octaveOk);
            QString pitch = item.attributes["pname"].toString();
            bool valid = !item.attributes["{http://www.w3.org/XML/1998/namespace}id"].toString().isEmpty() && idxOk && idx >= 0
                && durationOk && duration > 0 && beatOk && std::isfinite(beat) && beat >= 0 && octaveOk && octave >= 0 && octave <= 9
                && pitch.size() == 1 && QString("abcdefg").contains(pitch);
            if (!valid) addFinding(report, "note-chord-identity", "note", "Invalid playable notes", "invalid_note_identity_pitch_timing", item.location, "parse", "mei_item", true, "", {"structure", "playback", "input", "scoring"});
            for (int ancestor = item.parent; ancestor >= 0; ancestor = items[ancestor].parent) {
                if (!allowed.contains(items[ancestor].name) && !reportedPlayableWrappers.contains(ancestor)) {
                    reportedPlayableWrappers.insert(ancestor);
                    const QString& wrapper = items[ancestor].name;
                    if (canonicalMidiValidated && (wrapper == "bTrem" || wrapper == "fTrem")) {
                        addFinding(report, "tremolo", wrapper, "Tremolo runtime semantics", "tremolo_runtime_unverified",
                                   items[ancestor].location, "model", "mei_item", false, "unverified", {});
                    } else {
                        addFinding(report, "unknown-element-wrapper", wrapper, "Unsupported playable wrappers", "unsupported_playable_wrapper", items[ancestor].location, "parse", "mei_item", true, "", {"structure", "playback", "input", "scoring"});
                    }
                }
            }
        }
        if ((item.name == "staffDef" || item.name == "staff") && !values(capabilities["topology"].toObject()["currentRuntimeStaffIds"].toArray()).contains(item.attributes["n"].toString())) {
            addFinding(report, "piano-staff-topology", item.attributes["n"].toString(), "Unsupported piano staff topology", "unsupported_staff_identity", item.location, "parse", "mei_item", true, "", {"structure", "playback", "input", "scoring"});
        }
        if (item.name == "tie") {
            QString start = item.attributes["startid"].toString(); if (start.startsWith('#')) start.remove(0, 1);
            QString end = item.attributes["endid"].toString(); if (end.startsWith('#')) end.remove(0, 1);
            QStringList terminal = item.attributes["{http://pianomania.gg/ns/mei}terminal-ids"].toString().split(' ', Qt::SkipEmptyParts);
            bool terminalValid = !terminal.isEmpty();
            for (QString id : terminal) { if (id.startsWith('#')) id.remove(0, 1); terminalValid &= byId.contains(id); }
            if (start.isEmpty() || !byId.contains(start) || (!end.isEmpty() && !byId.contains(end)) || (end.isEmpty() && !terminalValid)) {
                addFinding(report, "ties-slurs", "tie", "Unresolved ties", "invalid_tie_anchor", item.location, "parse", "mei_item", true, "", {"structure", "playback", "input", "scoring"});
            } else if (end.isEmpty()) {
                addFinding(report, "ties-slurs", "hanging-tie", "Hanging ties", "hanging_tie_visual_omission", item.location, "render", "mei_item");
            }
        }
        for (const QJsonValue& value : capabilities["runtimeOmissions"].toArray()) {
            auto omission = value.toObject();
            if (item.name != omission["element"].toString()) continue;
            if (item.name == "gliss") {
                bool textOmitted = false;
                QString reason = glissandoDisplayOmission(items, index, textOmitted);
                QString behavior = reason.isEmpty() ? (textOmitted ? "partial" : "unverified") : "omitted";
                if (reason.isEmpty()) reason = textOmitted ? "glissando_text_omission" : "glissando_render_unverified";
                addFinding(report, "glissando", "gliss", "Glissandos", reason, item.location, "render", "mei_item",
                           false, behavior, behavior == "unverified" ? QJsonArray() : QJsonArray{"visual"}, {"practice", "game_solo"});
                continue;
            }
            addFinding(report, omission["featureId"].toString(), item.name == "arpeg" ? arpeggioVariant(item.attributes) : item.name, omission["humanName"].toString(), "unsupported_notation", item.location, omission["stage"].toString(), "mei_item", false, "", {"visual"}, omission["affectedModes"].toArray());
        }
        if (item.name == "artic") {
            QStringList tokens = item.attributes["artic"].toString().split(' ', Qt::SkipEmptyParts);
            if (tokens.size() > 1 && std::any_of(tokens.begin(), tokens.end(), [&](const QString& token) { return !core.contains(token); }))
                addFinding(report, "articulation-combinations", tokens.join(' '), "Combined articulation glyphs", "unsupported_articulation_combination", item.location, "render", "mei_item", false, "partial");
            else if (tokens.size() == 1 && !core.contains(tokens.first())) {
                bool parsed = values(capabilities["parsedArticulationTokens"].toArray()).contains(tokens.first());
                addFinding(report, "articulation-uncommon", tokens.first(), tokens.first() == "spicc" ? "Wedge staccatissimo" : "Articulation " + tokens.first(),
                           parsed ? "articulation_render_unverified" : "unsupported_articulation", item.location,
                           parsed ? "render" : "parse", "mei_item", false, parsed ? "unverified" : "omitted",
                           parsed ? QJsonArray() : QJsonArray{"visual"});
            }
        }
    }
    for (const QJsonValue& value : source.items) {
        auto item = value.toObject(); auto location = item["location"].toObject();
        if (!item["expectedExport"].toBool()) continue;
        QString publicId = sourceToCanonical.value(location["itemIdentity"].toString().replace('/', '.').replace('+', '-'));
        QJsonArray anchors;
        for (const QJsonValue& anchor : location["anchorIds"].toArray()) {
            QString id = sourceToCanonical.value(anchor.toString().replace('/', '.').replace('+', '-')); if (!id.isEmpty()) anchors.append(id);
        }
        location["anchorIds"] = anchors;
        if (!publicId.isEmpty() && byId.contains(publicId)) {
            const auto& exported = items[byId[publicId]];
            if (exported.name == "arpeg" && arpeggioVariant(exported.attributes) != item["variant"].toString())
                addFinding(report, "arpeggio", item["variant"].toString(), "Rolled-chord signs", "source_variant_missing_from_mei", location, "export", "source_item");
        } else if (!sourceToCanonical.isEmpty()) {
            addFinding(report, "arpeggio", item["variant"].toString(), "Rolled-chord signs", "source_item_missing_from_mei", location, "export", "source_item");
        } else {
            addFinding(report, "arpeggio", item["variant"].toString(), "Rolled-chord correspondence", "source_variant_correspondence_unverified", location, "export", "source_item", false, "unverified", {});
        }
    }
    // Source property reviews remain uncertain until their exact export policy is verified.
    for (const QJsonValue& value : source.report["findings"].toArray()) {
        auto row = value.toObject();
        if (row["observedBehavior"] == "unverified") {
            auto findings = report["findings"].toArray(); findings.append(row); report["findings"] = findings;
        }
    }
    auto completeness = report["completeness"].toObject();
    for (const QJsonValue& value : report["targetModes"].toArray()) {
        QString mode = value.toString(); auto stages = completeness[mode].toObject();
        stages["parse"] = admits(report) ? "partial" : "rejected";
        stages["export"] = inspectedSource ? "partial" : "unverified";
        completeness[mode] = stages;
    }
    report["completeness"] = completeness;
    finishReport(report);
    return report;
}

bool mu::project::notationcompatibility::admits(const QJsonObject& report)
{
    if (report["schemaVersion"] != "pianomania.notation-compatibility-report.v1"
        || report["capabilityCatalogDigest"] != NOTATION_CATALOG_DIGEST || !report["findings"].isArray()) return false;
    for (const QJsonValue& row : report["findings"].toArray()) if (row.toObject()["severity"] == "fatal") return false;
    return true;
}
QString mu::project::notationcompatibility::fatalSummary(const QJsonObject& report)
{
    QStringList problems;
    for (const QJsonValue& value : report["findings"].toArray()) {
        auto row = value.toObject();
        if (row["severity"] == "fatal") problems.append(row["humanName"].toString() + ": " + QString::number(row["count"].toInt()));
    }
    return "Score compatibility admission rejected: " + problems.join("; ");
}
SummaryModel::SummaryModel(QObject* parent) : QObject(parent) {}
SummaryModel* SummaryModel::instance()
{
    static SummaryModel* model = new SummaryModel(QCoreApplication::instance());
    return model;
}
QVariantMap SummaryModel::report() const { return m_report.toVariantMap(); }
QString SummaryModel::selectedMode() const { return m_mode; }
QVariantList SummaryModel::findings() const
{
    QVariantList result;
    for (const QJsonValue& value : m_report["findings"].toArray()) {
        auto row = value.toObject();
        if (values(row["affectedModes"].toArray()).contains(m_mode) || row["severity"] == "fatal") result.append(row.toVariantMap());
    }
    return result;
}
QString SummaryModel::summary() const
{
    return admits(m_report) ? QString("Score compatibility: %1 kinds of notation need review").arg(findings().size())
        : QString("Score files rejected: %1 compatibility problems").arg(findings().size());
}
namespace {
struct MusicianIssue {
    QString title;
    QString consequence;
    QString explanation;
    bool fatal = false;
    bool identitiesKnown = true;
    QSet<QString> units;
    QSet<QString> identities;
    QSet<int> measures;
    QSet<int> problemMeasures;
};

QString measureLabel(const QSet<int>& measures)
{
    QList<int> ordered = measures.values();
    std::sort(ordered.begin(), ordered.end());
    QStringList labels;
    for (int index = 0; index < ordered.size();) {
        int first = ordered[index];
        int last = first;
        while (++index < ordered.size() && ordered[index] == last + 1) last = ordered[index];
        labels.append(first == last ? QString("Measure %1").arg(first)
                                    : QString("Measures %1–%2").arg(first).arg(last));
    }
    return labels.join(", ");
}

QString musicalName(const QJsonObject& row)
{
    const QString feature = row["featureId"].toString();
    const QString variant = row["variant"].toString();
    if (feature == "arpeggio") {
        if (variant == "bracket") return "Non-arpeggio brackets";
        if (variant == "up" || variant == "up-straight") return "Upward arpeggios";
        if (variant == "down" || variant == "down-straight") return "Downward arpeggios";
        return "Arpeggios";
    }
    if (feature == "glissando") return "Glissandos";
    if (feature == "harmony") return "Chord symbols";
    if (feature == "lyrics") return "Lyrics";
    if (feature == "figured-bass") return "Figured bass";
    if (feature == "breath-caesura") return variant == "breath" ? "Breath marks" : "Caesuras";
    if (feature == "tremolo" || variant == "bTrem" || variant == "fTrem") return "Tremolos";
    if (feature.startsWith("ornament")) {
        if (variant == "trill") return "Trills";
        if (variant == "mordent") return "Mordents";
        if (variant == "turn") return "Turns";
        return "Ornaments";
    }
    const QMap<QString, QString> articulations {
        {"stacc", "Staccato"}, {"stacciss", "Staccatissimo"}, {"spicc", "Wedge staccatissimo"},
        {"stroke", "Stroke staccatissimo"}, {"ten", "Tenuto"}, {"marc", "Marcato"},
        {"acc", "Accents"}, {"acc-soft", "Soft accents"}, {"stress", "Stress marks"}, {"unstress", "Unstress marks"}
    };
    if (feature == "articulation-combinations") {
        QStringList names;
        for (const QString& token : variant.split(' ', Qt::SkipEmptyParts)) {
            if (!articulations.contains(token)) return "Articulation combinations";
            names.append(articulations[token]);
        }
        return names.isEmpty() ? QString("Articulation combinations") : names.join(" + ");
    }
    if (feature.startsWith("articulation")) return articulations.value(variant, "Articulations");
    if (feature == "piano-staff-topology") return "Staff layout";
    if (feature == "tie" || (feature == "ties-slurs" && row["reasonCode"] == "hanging_tie_visual_omission")) return "Ties";
    return "Score notation";
}

// This presentation-only check mirrors the current arpeggio parser's consumed v1 payload.
// Admission and exact written-note identity validation remain in the machine report.
bool usableArpeggioGeometry(const QJsonObject& row, const QJsonObject& attributes)
{
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    const QString variant = row["variant"].toString();
    if (!QStringList {"normal", "up", "down", "bracket", "up-straight", "down-straight"}.contains(variant)) return false;
    const QString order = attributes["order"].toString();
    if (attributes.contains("order") && !QStringList {"up", "down", "nonarp"}.contains(order)) return false;
    QString style = attributes[prefix + "arpeggio-style"].toString();
    if (attributes.contains(prefix + "arpeggio-style") && !QStringList {"wavy", "straight", "bracket"}.contains(style)) return false;
    if (attributes.contains("lform")) {
        const QString lineForm = attributes["lform"].toString();
        if (lineForm != "wavy" && lineForm != "solid") return false;
        const QString standardStyle = lineForm == "wavy" ? "wavy" : order == "nonarp" ? "bracket" : "straight";
        if (!style.isEmpty() && style != standardStyle) return false;
        style = standardStyle;
    }
    if ((style == "bracket" && order != "nonarp") || (order == "nonarp" && !style.isEmpty() && style != "bracket")) return false;
    const auto validBoolean = [](const QString& value) { return value == "true" || value == "1" || value == "false" || value == "0"; };
    for (const QString& name : {QString("arrow"), prefix + "arpeggio-play"})
        if (attributes.contains(name) && !validBoolean(attributes[name].toString())) return false;
    const auto number = [](const QString& value, double& result) {
        bool ok = false;
        result = value.toDouble(&ok);
        return ok && std::isfinite(result);
    };
    const auto tuple = [&number](const QString& value, int count, QList<double>& result) {
        const QStringList fields = value.split(',');
        if (fields.size() != count) return false;
        result.clear();
        for (const QString& field : fields) {
            double parsed = 0;
            if (!number(field, parsed)) return false;
            result.append(parsed);
        }
        return true;
    };
    QList<double> parsed;
    if (attributes.contains(prefix + "arpeggio-stretch")) {
        double stretch = 0;
        if (!number(attributes[prefix + "arpeggio-stretch"].toString(), stretch) || stretch <= 0) return false;
    }
    if (attributes[prefix + "arpeggio-geometry-version"] != "1") return false;
    const QString pageText = attributes[prefix + "arpeggio-page"].toString();
    bool validPage = false;
    const int page = pageText.toInt(&validPage);
    if (!validPage || page <= 0 || !std::all_of(pageText.cbegin(), pageText.cend(), [](QChar c) { return c >= '0' && c <= '9'; })) return false;
    const QString visibility = attributes[prefix + "arpeggio-visible"].toString();
    if (!validBoolean(visibility)) return false;
    const bool visible = visibility == "true" || visibility == "1";
    if (!tuple(attributes[prefix + "xy"].toString(), 2, parsed)) return false;
    if (!tuple(attributes[prefix + "arpeggio-bounds"].toString(), 4, parsed) || parsed[0] > parsed[2] || parsed[1] < parsed[3]) return false;
    double width = 0;
    if (!number(attributes[prefix + "arpeggio-line-width"].toString(), width) || width <= 0) return false;
    const QString font = attributes[prefix + "arpeggio-font"].toString();
    if ((font != "Leland" && font != "Bravura") || attributes[prefix + "arpeggio-line-cap"] != "flat") return false;
    if (!attributes.contains(prefix + "arpeggio-lines") || !attributes.contains(prefix + "arpeggio-glyphs")) return false;
    int primitives = 0;
    const QString lines = attributes[prefix + "arpeggio-lines"].toString();
    if (!lines.isEmpty()) for (const QString& line : lines.split(';')) {
        if (!tuple(line, 4, parsed) || (parsed[0] == parsed[2] && parsed[1] == parsed[3])) return false;
        ++primitives;
    }
    const QString glyphs = attributes[prefix + "arpeggio-glyphs"].toString();
    if (!glyphs.isEmpty()) for (const QString& glyph : glyphs.split(';')) {
        const QStringList fields = glyph.split(',');
        if (fields.size() != 8 || !QStringList {"wiggleArpeggiatoUp", "wiggleArpeggiatoUpArrow", "arrowheadBlackUp", "arrowheadBlackDown"}.contains(fields[0])) return false;
        if (!tuple(fields.mid(1).join(','), 7, parsed) || parsed[3] >= parsed[5] || parsed[4] <= parsed[6]) return false;
        ++primitives;
    }
    if ((visible && primitives == 0) || (!visible && primitives != 0)) return false;
    // Playing views also require exact notehead registration; incomplete legacy v1 remains a warning.
    if (visible) {
        if (!tuple(attributes[prefix + "arpeggio-anchor-center"].toString(), 2, parsed)) return false;
        const QStringList members = attributes[prefix + "arpeggio-members"].toString().simplified().split(' ', Qt::SkipEmptyParts);
        const QStringList centers = attributes[prefix + "arpeggio-member-centers"].toString().split(';');
        if (members.isEmpty() || centers.size() != members.size()) return false;
        QSet<QString> distinctMembers;
        for (const QString& member : members) {
            if (!member.startsWith('#') || member.size() < 2 || distinctMembers.contains(member)) return false;
            distinctMembers.insert(member);
        }
        for (const QString& center : centers) if (!tuple(center, 2, parsed)) return false;
    }
    return true;
}

// A source-only blanket catalog row cannot establish a concrete visual loss.
bool concreteArpeggioLoss(const QJsonObject& report, const QJsonObject& row, const QJsonObject& location)
{
    if (row["featureId"] != "arpeggio" || row["reasonCode"] != "unsupported_notation") return true;
    for (const QJsonValue& behavior : row["omittedBehavior"].toArray())
        if (behavior != "visual") return true;
    if (row["countUnit"] != "mei_item" || report["meiDigest"].toString().isEmpty()) return false;
    const auto attributes = location["attributes"].toObject();
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    if (attributes[prefix + "arpeggio-visible"] == "false" || attributes[prefix + "arpeggio-visible"] == "0"
        || attributes["visible"] == "false" || attributes["visible"] == "0") return false;
    return !usableArpeggioGeometry(row, attributes);
}

bool lossCoversProduct(const QJsonObject& report, const QJsonObject& row, const QString& behavior)
{
    const QStringList affected = values(row["affectedModes"].toArray());
    bool hasRelevantUse = false;
    for (const QString& use : values(report["targetModes"].toArray())) {
        if ((behavior == "input" || behavior == "scoring") && use.endsWith("_preview")) continue;
        hasRelevantUse = true;
        if (!affected.contains(use)) return false;
    }
    return hasRelevantUse;
}

MusicianIssue musicianIssue(const QJsonObject& report, const QJsonObject& row)
{
    MusicianIssue issue;
    issue.fatal = row["severity"] == "fatal";
    issue.title = musicalName(row);
    const QString reason = row["reasonCode"].toString();
    const QString feature = row["featureId"].toString();
    if (issue.fatal) {
        issue.consequence = reason;
        if (reason == "malformed_xml") {
            issue.title = "Score file";
            issue.explanation = "The score file couldn’t be read. Try exporting it again.";
        } else if (reason == "invalid_tie_anchor") {
            issue.title = "Ties";
            issue.explanation = "Some ties couldn’t be read. Check the affected measures, then retry.";
        } else if (reason == "invalid_note_identity_pitch_timing") {
            issue.title = "Notes and rhythms";
            issue.explanation = "Some notes or rhythms couldn’t be read. Check the affected measures, then retry.";
        } else if (reason == "duplicate_identity") {
            issue.title = "Notes";
            issue.explanation = "Some notes couldn’t be distinguished. Try exporting the score again.";
        } else if (feature == "piano-staff-topology") {
            issue.explanation = "This staff layout isn’t supported yet.";
        } else if (issue.title == "Tremolos") {
            issue.explanation = "Pianomania couldn’t read these tremolos. Check the affected measures, then retry.";
        } else {
            issue.title = "Score compatibility";
            issue.explanation = "Pianomania couldn’t check this score. Try exporting it again.";
        }
        return issue;
    }
    if (reason == "source_variant_missing_from_mei") {
        issue.consequence = "different-marking";
        issue.explanation = "These markings differ from the score in Pianomania.";
        return issue;
    }
    QStringList effects;
    const QJsonArray omitted = row["omittedBehavior"].toArray();
    QString visualSubject = issue.title;
    if (feature == "arpeggio" && row["variant"] != "bracket") visualSubject = "Arpeggio markings";
    else if (feature == "glissando") visualSubject = "Glissando markings";
    else if (feature == "tremolo") visualSubject = "Tremolo markings";
    else if (feature.startsWith("ornament")) visualSubject = "These ornament markings";
    else if (feature.startsWith("articulation")) visualSubject = "These articulation markings";
    if (omitted.contains("visual")) {
        effects.append(feature == "glissando" && reason == "glissando_text_omission"
            ? "Glissando text won’t appear in Pianomania."
            : row["observedBehavior"] == "partial" || !lossCoversProduct(report, row, "visual")
            ? "These markings aren’t shown in full in Pianomania."
            : visualSubject + " won’t appear in Pianomania.");
    }
    if (omitted.contains("playback")) effects.append(lossCoversProduct(report, row, "playback")
        ? "These markings won’t affect playback in Pianomania."
        : "Playback of these markings isn’t fully supported in Pianomania.");
    if (omitted.contains("input")) effects.append(lossCoversProduct(report, row, "input")
        ? "Pianomania won’t check these markings while you play."
        : "Pianomania doesn’t always check these markings while you play.");
    if (omitted.contains("scoring")) effects.append(lossCoversProduct(report, row, "scoring")
        ? "These markings aren’t supported for scoring in Pianomania."
        : "Scoring for these markings isn’t fully supported in Pianomania.");
    if (omitted.contains("traversal")) effects.append(lossCoversProduct(report, row, "traversal")
        ? "Pianomania won’t follow these repeats."
        : "These repeats aren’t fully supported in Pianomania.");
    if (omitted.contains("structure")) effects.append("Pianomania couldn’t read this part of the score.");
    QStringList losses = values(omitted);
    std::sort(losses.begin(), losses.end());
    QStringList coverage;
    for (const QString& loss : losses) coverage.append(loss + (lossCoversProduct(report, row, loss) ? ":all" : ":limited"));
    issue.consequence = row["observedBehavior"].toString() + ":" + coverage.join("|");
    issue.explanation = effects.join(" ");
    return issue;
}
}

bool SummaryModel::needsFix() const { return !admits(m_report); }
QString SummaryModel::musicianTitle() const
{
    return QString("Compatibility warning");
}
QVariantList SummaryModel::musicianFindings() const
{
    // This is a whole-product projection. Internal mode selection remains in the raw findings API only.
    QMap<QString, MusicianIssue> grouped;
    for (const QJsonValue& value : m_report["findings"].toArray()) {
        const auto row = value.toObject();
        const bool fatal = row["severity"] == "fatal";
        const QString observed = row["observedBehavior"].toString();
        if (!fatal && (observed == "unverified" || row["omittedBehavior"].toArray().isEmpty())) continue;
        MusicianIssue issue = musicianIssue(m_report, row);
        if (!fatal && issue.explanation.isEmpty()) continue;
        const QString key = (fatal ? "0:" : "1:") + row["featureId"].toString() + ":"
            + row["variant"].toString() + ":" + issue.consequence;
        const QJsonArray locations = row["locations"].toArray();
        bool hasConcreteIssue = fatal;
        for (const QJsonValue& location : locations)
            if (concreteArpeggioLoss(m_report, row, location.toObject())) hasConcreteIssue = true;
        if (locations.isEmpty() && row["featureId"] != "arpeggio") hasConcreteIssue = true;
        if (!hasConcreteIssue) continue;
        if (!grouped.contains(key)) grouped.insert(key, issue);
        MusicianIssue& target = grouped[key];
        const QString unit = row["countUnit"].toString();
        target.units.insert(unit);
        if (locations.isEmpty()) target.identitiesKnown = false;
        for (const QJsonValue& value : locations) {
            const auto location = value.toObject();
            if (!fatal && !concreteArpeggioLoss(m_report, row, location)) continue;
            const QString identity = location["itemIdentity"].toString();
            if (identity.isEmpty() || identity.startsWith("source:") || identity.startsWith("mei:")
                || (unit != "source_item" && unit != "mei_item")) target.identitiesKnown = false;
            else target.identities.insert(identity);
            bool validNumber = false;
            const int measure = location["displayedMeasureNumber"].toString().toInt(&validNumber);
            if (validNumber && measure >= 0) target.measures.insert(measure);
            const QString measureId = location["measureId"].toString();
            if (m_measureOrdinals.contains(measureId)) target.problemMeasures.insert(m_measureOrdinals.value(measureId));
        }
    }
    if (needsFix() && grouped.isEmpty()) {
        QJsonObject fatal {{"severity", "fatal"}};
        grouped.insert("0:compatibility", musicianIssue(m_report, fatal));
    }
    QVariantList result;
    for (auto iterator = grouped.cbegin(); iterator != grouped.cend(); ++iterator) {
        const MusicianIssue& issue = iterator.value();
        const bool hasCount = !issue.fatal && issue.identitiesKnown && issue.units.size() == 1
            && !issue.identities.isEmpty();
        const int problemMeasure = issue.problemMeasures.isEmpty() ? -1
            : *std::min_element(issue.problemMeasures.cbegin(), issue.problemMeasures.cend());
        const bool canNavigate = problemMeasure >= 0 && m_navigate && m_navigate(problemMeasure, false);
        result.append(QVariantMap {{"title", issue.title}, {"description", issue.explanation},
            {"fatal", issue.fatal}, {"hasCount", hasCount}, {"count", hasCount ? issue.identities.size() : 0},
            {"locationLabel", measureLabel(issue.measures)}, {"problemMeasureOrdinal", problemMeasure}, {"canNavigate", canNavigate}});
    }
    return result;
}

void SummaryModel::setSelectedMode(const QString& mode)
{
    if (mode == m_mode || !values(m_report["targetModes"].toArray()).contains(mode)) return;
    m_mode = mode; emit reportChanged();
}
bool SummaryModel::goToProblem(int findingIndex)
{
    const QVariantList rows = musicianFindings();
    if (!m_navigate || findingIndex < 0 || findingIndex >= rows.size()) return false;
    const QVariantMap row = rows[findingIndex].toMap();
    if (!row["canNavigate"].toBool()) return false;
    return m_navigate(row["problemMeasureOrdinal"].toInt(), true);
}

bool SummaryModel::setReport(const QJsonObject& report, const QHash<QString, int>& measureOrdinals,
                             std::function<bool(int, bool)> navigate)
{
    m_measureOrdinals = measureOrdinals;
    m_navigate = std::move(navigate);
    m_report = report; emit reportChanged();
    QString key = report["scoreIdentity"].toString() + report["sourceDigest"].toString()
        + report["meiDigest"].toString() + report["reportDigest"].toString();
    if (musicianFindings().isEmpty() || m_presented.contains(key)) return false;
    m_presented.insert(key); return true;
}
