#include "authoringcapabilitypolicy.h"
#include "authoringpolicycore.h"

#include <algorithm>
#include <QCoreApplication>
#include <QDir>
#include "engraving/dom/actionicon.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/playtechannotation.h"
#include "engraving/dom/score.h"
#include "engraving/dom/symbol.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/types/symnames.h"

using namespace mu::notation;
using namespace mu::engraving;

AuthoringCapabilityPolicy::AuthoringCapabilityPolicy()
{
    for (const auto& row : authoring::capabilityRows) {
        if (row.decision != authoring::Decision::Hidden) {
            continue;
        }
        if (row.family == "action") {
            m_hiddenActions.emplace(row.variant);
        } else if (row.family == "instrument") {
            m_hiddenInstruments.emplace(row.variant);
        } else if (row.family == "template") {
            m_hiddenTemplates.emplace(row.variant);
        } else if (row.family == "master_symbol_variant") {
            m_hiddenSymbols.emplace(SymNames::symIdByName(muse::String::fromUtf8(std::string(row.variant))));
        }
    }
    // A raw Symbol must not bypass a hidden semantic Articulation preset.
    m_hiddenSymbols.insert(authoring::hiddenArticulationSymbols.begin(), authoring::hiddenArticulationSymbols.end());
}

bool AuthoringCapabilityPolicy::isRestrictedAction(std::string_view code) const
{
    return m_hiddenActions.count(std::string(code)) != 0;
}

bool AuthoringCapabilityPolicy::mayCreateAction(std::string_view code) const
{
    return !isRestrictedAction(code);
}

bool AuthoringCapabilityPolicy::mayCreateInstrument(std::string_view templateId) const
{
    return m_hiddenInstruments.count(std::string(templateId)) == 0;
}

bool AuthoringCapabilityPolicy::mayCreateTemplate(const muse::io::path_t& path) const
{
    const QString root = QDir::cleanPath((globalConfiguration()->appDataPath() + "/templates").toQString()) + "/";
    const QString candidate = QDir::cleanPath(path.toQString());
    // User templates remain pending, including user files with bundled names.
    if (!candidate.startsWith(root)) {
        return true;
    }
    return m_hiddenTemplates.count(candidate.mid(root.size()).toStdString()) == 0;
}

bool AuthoringCapabilityPolicy::mayCreateSymbol(SymId symbol) const
{
    return m_hiddenSymbols.count(symbol) == 0;
}

bool AuthoringCapabilityPolicy::mayCreateSymbolRange(std::string_view description) const
{
    return std::find(authoring::hiddenRangeDescriptions.begin(), authoring::hiddenRangeDescriptions.end(), description)
        == authoring::hiddenRangeDescriptions.end();
}

bool AuthoringCapabilityPolicy::mayEditExisting(const EngravingItem* item, const Score* activeScore) const
{
    return item && activeScore && item->score() == activeScore;
}

bool AuthoringCapabilityPolicy::mayCreateElement(const EngravingItem& item, std::string_view origin) const
{
    // These are dedicated presentation routes. Generic retained routes remain available.
    if (origin == "guitar" || origin == "accordion" || origin == "handbells"
        || origin == "harp" || origin == "bagpipe-embellishment" || origin == "fretboard-diagram") {
        return false;
    }
    if (item.isActionIcon()) {
        return mayCreateAction(static_cast<const ActionIcon&>(item).actionCode());
    }
    if (item.isSymbol()) {
        return mayCreateSymbol(static_cast<const Symbol&>(item).sym());
    }
    if (item.isArticulation()) {
        const auto& articulation = static_cast<const Articulation&>(item);
        if (!mayCreateSymbol(articulation.symId())) {
            return false;
        }
        return std::find(authoring::hiddenArticulationTexts.begin(), authoring::hiddenArticulationTexts.end(), articulation.textType())
            == authoring::hiddenArticulationTexts.end();
    }
    if (item.isClef()) {
        const auto type = static_cast<const Clef&>(item).clefType();
        return std::find(authoring::hiddenClefs.begin(), authoring::hiddenClefs.end(), type) == authoring::hiddenClefs.end();
    }
    if (item.isFingering()) {
        const auto& fingering = static_cast<const Fingering&>(item);
        const auto text = fingering.xmlText().toStdString();
        switch (fingering.textStyleType()) {
        case TextStyleType::LH_GUITAR_FINGERING: return text.size() != 1 || std::string_view("012345T").find(text[0]) == std::string_view::npos;
        case TextStyleType::RH_GUITAR_FINGERING: return text.size() != 1 || std::string_view("pimac").find(text[0]) == std::string_view::npos;
        case TextStyleType::STRING_NUMBER: return text.size() != 1 || std::string_view("0123456789").find(text[0]) == std::string_view::npos;
        default: return true;
        }
    }
    if (item.isChordLine()) {
        switch (static_cast<const ChordLine&>(item).chordLineType()) {
        case ChordLineType::FALL:
        case ChordLineType::DOIT:
        case ChordLineType::PLOP:
        case ChordLineType::SCOOP: return false;
        default: return true;
        }
    }
    if (item.isPlayTechAnnotation()) {
        const auto& annotation = static_cast<const PlayTechAnnotation&>(item);
        const auto technique = annotation.techniqueType();
        // Natural also represents the retained "normal" preset. Arco is the
        // dedicated bowing preset, including its translated palette version.
        if (!authoring::allowed(authoring::decide("palette.text.arco")) && technique == PlayingTechniqueType::Natural
            && (annotation.xmlText() == u"arco"
                || annotation.xmlText() == muse::String::fromQString(QCoreApplication::translate("palette", "arco")))) {
            return false;
        }
        return std::find(authoring::hiddenPlayingTechniques.begin(), authoring::hiddenPlayingTechniques.end(), technique)
            == authoring::hiddenPlayingTechniques.end();
    }
    if (item.type() == ElementType::TREMOLO_SINGLECHORD) {
        const auto& tremolo = static_cast<const TremoloSingleChord&>(item);
        return !tremolo.isBuzzRoll()
               || authoring::allowed(authoring::decide("palette.tremolo.tremolo-type-buzz-roll"));
    }
    switch (item.type()) {
    case ElementType::FRET_DIAGRAM:
    case ElementType::HARP_DIAGRAM:
    case ElementType::BAGPIPE_EMBELLISHMENT:
    case ElementType::CAPO:
    case ElementType::STRING_TUNINGS:
    case ElementType::PALM_MUTE:
    case ElementType::WHAMMY_BAR:
    case ElementType::TAPPING:
    case ElementType::HAMMER_ON_PULL_OFF: return false;
    default: return true;
    }
}
