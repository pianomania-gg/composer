// SPDX-License-Identifier: GPL-3.0-only
#include "authoringcreationmenus.h"

#include <algorithm>

using namespace mu::appshell;
using namespace muse::uicomponents;

AuthoringCreationMenus::AuthoringCreationMenus(const MenuItemList& menus,
                                               std::shared_ptr<const notation::IAuthoringCapabilityPolicy> policy)
    : m_menus(menus), m_policy(std::move(policy))
{
    for (MenuItem* item : m_menus) {
        recordCreationContainers(item);
    }
}

std::optional<MenuItemList> AuthoringCreationMenus::update(bool existingPercussionSelected)
{
    if (m_existingPercussionSelected == existingPercussionSelected) {
        return std::nullopt;
    }
    m_existingPercussionSelected = existingPercussionSelected;

    MenuItemList visibleMenus = m_menus;
    const bool subitemsChanged = filterCreation(visibleMenus, existingPercussionSelected);
    if (!subitemsChanged && visibleMenus == m_visibleMenus) {
        return std::nullopt;
    }

    m_visibleMenus = visibleMenus;
    return visibleMenus;
}

bool AuthoringCreationMenus::recordCreationContainers(MenuItem* item)
{
    if (!item) {
        return false;
    }
    bool affected = m_policy->isRestrictedAction(item->action().code);
    const MenuItemList children = item->subitems();
    for (MenuItem* child : children) {
        affected = recordCreationContainers(child) || affected;
    }
    if (affected && !children.empty()) {
        m_completeChildren.emplace(item, children);
    }
    return affected;
}

bool AuthoringCreationMenus::filterCreation(MenuItemList& source, bool existingPercussionSelected)
{
    bool subitemsChanged = false;
    for (auto it = source.begin(); it != source.end();) {
        MenuItem* item = *it;
        if (!item) {
            it = source.erase(it);
            continue;
        }
        const auto original = m_completeChildren.find(item);
        MenuItemList children = original == m_completeChildren.end() ? item->subitems() : original->second;
        const bool wasMenu = !children.empty();
        if (original != m_completeChildren.end()) {
            subitemsChanged = filterCreation(children, existingPercussionSelected) || subitemsChanged;
            if (item->subitems() != children) {
                item->setSubitems(children);
                subitemsChanged = true;
            }
        }
        const auto& code = item->action().code;
        const bool existingEditor = existingPercussionSelected && (code == "customize-kit" || code == "toggle-percussion-panel");
        const bool denied = !code.empty() && !m_policy->mayCreateAction(code) && !existingEditor;
        const bool emptyCreationContainer = original != m_completeChildren.end() && wasMenu
                                            && std::all_of(children.begin(), children.end(), [](const MenuItem* child) {
            return child->action().code.empty() && child->action().title.isEmpty() && child->subitems().empty();
        });
        if (denied || emptyCreationContainer) {
            it = source.erase(it);
        } else {
            ++it;
        }
    }
    return subitemsChanged;
}
