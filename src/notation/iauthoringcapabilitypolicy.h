#pragma once

#include <string_view>
#include "modularity/imoduleinterface.h"
#include "engraving/types/symid.h"
#include "io/path.h"

namespace mu::engraving { class EngravingItem; class Score; }

namespace mu::notation {
class IAuthoringCapabilityPolicy : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IAuthoringCapabilityPolicy)
public:
    virtual ~IAuthoringCapabilityPolicy() = default;
    virtual bool isRestrictedAction(std::string_view code) const = 0;
    virtual bool mayCreateAction(std::string_view code) const = 0;
    virtual bool mayCreateInstrument(std::string_view templateId) const = 0;
    virtual bool mayCreateTemplate(const muse::io::path_t& path) const = 0;
    // Origin is an unlocalized creation route, such as "guitar" or "text".
    // Existing score editing and clipboard import do not use this predicate.
    virtual bool mayCreateElement(const engraving::EngravingItem& item, std::string_view origin = {}) const = 0;
    virtual bool mayCreateSymbol(engraving::SymId symbol) const = 0;
    virtual bool mayCreateSymbolRange(std::string_view description) const = 0;
    // The caller passes an item from the active selection and its active score.
    virtual bool mayEditExisting(const engraving::EngravingItem* item, const engraving::Score* activeScore) const = 0;
};
}
