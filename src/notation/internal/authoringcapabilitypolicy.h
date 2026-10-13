#pragma once

#include <set>
#include <string>
#include <unordered_set>
#include "../iauthoringcapabilitypolicy.h"
#include "global/iglobalconfiguration.h"
#include "modularity/ioc.h"

namespace mu::notation {
class AuthoringCapabilityPolicy final : public IAuthoringCapabilityPolicy
{
    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;
public:
    AuthoringCapabilityPolicy();
    bool isRestrictedAction(std::string_view code) const override;
    bool mayCreateAction(std::string_view code) const override;
    bool mayCreateInstrument(std::string_view templateId) const override;
    bool mayCreateTemplate(const muse::io::path_t& path) const override;
    bool mayCreateElement(const engraving::EngravingItem& item, std::string_view origin = {}) const override;
    bool mayCreateSymbol(engraving::SymId symbol) const override;
    bool mayCreateSymbolRange(std::string_view description) const override;
    bool mayEditExisting(const engraving::EngravingItem* item, const engraving::Score* activeScore) const override;
private:
    std::unordered_set<std::string> m_hiddenActions;
    std::unordered_set<std::string> m_hiddenInstruments;
    std::unordered_set<std::string> m_hiddenTemplates;
    std::set<engraving::SymId> m_hiddenSymbols;
};
}
