#include "authoringcapabilitypolicy.h"
#include "authoringpolicycore.h"

#include <algorithm>
#include <utility>
#include <QDir>
#include "engraving/dom/actionicon.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/playtechannotation.h"
#include "engraving/dom/score.h"
#include "engraving/dom/symbol.h"
#include "engraving/types/symnames.h"

using namespace mu::notation;
using namespace mu::engraving;

AuthoringCapabilityPolicy::AuthoringCapabilityPolicy(std::function<bool()> enabled, std::function<void(bool)> setEnabled, muse::async::Notification changed)
    : m_enabled(std::move(enabled)), m_setEnabled(std::move(setEnabled)), m_changed(std::move(changed))
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
}

bool AuthoringCapabilityPolicy::enabled() const
{
    return m_enabled();
}

void AuthoringCapabilityPolicy::setEnabled(bool enabled)
{
    m_setEnabled(enabled);
}

muse::async::Notification AuthoringCapabilityPolicy::changed() const
{
    return m_changed;
}

bool AuthoringCapabilityPolicy::isRestrictedAction(std::string_view code) const
{
    return m_hiddenActions.count(std::string(code)) != 0;
}

bool AuthoringCapabilityPolicy::mayCreateAction(std::string_view code) const
{
    return !enabled() || !isRestrictedAction(code);
}

bool AuthoringCapabilityPolicy::mayCreateInstrument(std::string_view templateId) const
{
    return !enabled() || m_hiddenInstruments.count(std::string(templateId)) == 0;
}

bool AuthoringCapabilityPolicy::mayCreateTemplate(const muse::io::path_t& path) const
{
    if (!enabled()) {
        return true;
    }
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
    return !enabled() || m_hiddenSymbols.count(symbol) == 0;
}

bool AuthoringCapabilityPolicy::mayCreateSymbolRange(std::string_view description) const
{
    return !enabled() || std::find(authoring::hiddenRangeDescriptions.begin(), authoring::hiddenRangeDescriptions.end(), description)
        == authoring::hiddenRangeDescriptions.end();
}

bool AuthoringCapabilityPolicy::mayEditExisting(const EngravingItem* item, const Score* activeScore) const
{
    return item && activeScore && item->score() == activeScore;
}

bool AuthoringCapabilityPolicy::mayCreateElement(const EngravingItem& item, std::string_view origin) const
{
    if (!enabled()) {
        return true;
    }
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
        if (std::find(authoring::hiddenArticulationSymbols.begin(), authoring::hiddenArticulationSymbols.end(), articulation.symId())
            != authoring::hiddenArticulationSymbols.end()) {
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
        const auto technique = static_cast<const PlayTechAnnotation&>(item).techniqueType();
        return std::find(authoring::hiddenPlayingTechniques.begin(), authoring::hiddenPlayingTechniques.end(), technique)
            == authoring::hiddenPlayingTechniques.end();
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
