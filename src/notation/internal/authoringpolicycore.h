// Exact authoring-row decisions. Imported editing is validated by the host service.
#pragma once

#include "authoringcapabilitypolicydata.h"

namespace mu::notation::authoring {
enum class Operation { CreateFromDedicatedControl, EditExisting, PreserveImported };
struct Context {
    Operation operation = Operation::CreateFromDedicatedControl;
    // The host verifies a live document item. QML cannot supply this authority.
    bool existingItemVerified = false;
};
struct PolicyResult {
    Decision decision;
    bool validContext;
    bool classified;
};

inline const CapabilityRow* findCapability(std::string_view featureId)
{
    std::size_t first = 0;
    std::size_t last = capabilityRows.size();
    while (first < last) {
        const std::size_t mid = first + (last - first) / 2;
        if (capabilityRows[mid].id < featureId) {
            first = mid + 1;
        } else {
            last = mid;
        }
    }
    return first < capabilityRows.size() && capabilityRows[first].id == featureId
        ? &capabilityRows[first] : nullptr;
}

inline PolicyResult decide(std::string_view featureId, Context context = {})
{
    if (context.operation == Operation::PreserveImported) {
        return { Decision::Allowed, true, false };
    }
    if (context.operation == Operation::EditExisting) {
        return { Decision::Allowed, context.existingItemVerified, false };
    }
    if (context.operation != Operation::CreateFromDedicatedControl) {
        return { Decision::Allowed, false, false };
    }
    const CapabilityRow* row = findCapability(featureId);
    return { row ? row->decision : Decision::Allowed, true, row != nullptr };
}

inline bool allowed(PolicyResult result)
{
    return result.validContext && result.decision != Decision::Hidden;
}
} // namespace mu::notation::authoring
