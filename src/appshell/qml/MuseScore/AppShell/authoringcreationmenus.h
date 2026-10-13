// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <map>
#include <memory>
#include <optional>

#include "notation/iauthoringcapabilitypolicy.h"
#include "uicomponents/qml/Muse/UiComponents/menuitem.h"

namespace mu::appshell {
// Removes denied creation actions from the application menus. The policy is fixed, so the
// visible menus change only when the selection starts or stops being an existing percussion
// item, which keeps its percussion editors.
class AuthoringCreationMenus
{
public:
    AuthoringCreationMenus(const muse::uicomponents::MenuItemList& menus,
                           std::shared_ptr<const notation::IAuthoringCapabilityPolicy> policy);

    // Returns the visible top-level menus when the visible menus changed since the last update.
    std::optional<muse::uicomponents::MenuItemList> update(bool existingPercussionSelected);

private:
    bool recordCreationContainers(muse::uicomponents::MenuItem* item);
    bool filterCreation(muse::uicomponents::MenuItemList& source, bool existingPercussionSelected);

    const muse::uicomponents::MenuItemList m_menus;
    const std::shared_ptr<const notation::IAuthoringCapabilityPolicy> m_policy;
    // Original child lists of the containers that hold denied creation actions. Account,
    // recent-file and plugin updates keep using their live menu objects.
    std::map<muse::uicomponents::MenuItem*, muse::uicomponents::MenuItemList> m_completeChildren;
    std::optional<bool> m_existingPercussionSelected;
    muse::uicomponents::MenuItemList m_visibleMenus;
};
}
