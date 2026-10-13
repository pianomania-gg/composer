/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

pragma ComponentBehavior: Bound

import QtQuick
import Qt.labs.platform as PLATFORM

import MuseScore.AppShell

Item {
    id: root

    readonly property bool available: menuModel.isGlobalMenuAvailable()

    PLATFORM.MenuBar {
        id: menuBar
    }

    PlatformAppMenuModel {
        id: menuModel
    }

    function load() {
        menuModel.load()

        var items = menuModel.items
        for (var i in items) {
            var menu = makeMenu(items[i])

            menuBar.addMenu(menu)

            menu.load()
        }

        menuModel.itemsChanged.connect(function() {
            for (var i in menuModel.items) {
                var menu = menuBar.menus[i]
                menu.subitems = menuModel.items[i].subitems
                menu.load()
            }
        })
    }

    function makeMenu(menuInfo) {
        var menu = menuComponent.createObject(menuBar)

        setUpMenu(menu, menuInfo)

        // With the menu as receiver, the handler goes away when the menu is destroyed
        menuInfo.subitemsChanged.connect(menu, function(subitems, menuId) {
            if (menu.id === menuId) {
                menu.subitems = subitems
                menu.load()
            }
        })

        return menu
    }

    function setUpMenu(menu, menuInfo) {
        menu.id = menuInfo.id
        menu.title = menuInfo.title
        menu.enabled = menuInfo.enabled
        menu.subitems = menuInfo.subitems
    }

    function makeMenuItem(parentMenu, itemInfo) {
        var menuItem = menuItemComponent.createObject(parentMenu)

        setUpMenuItem(menuItem, itemInfo)

        return menuItem
    }

    function setUpMenuItem(menuItem, itemInfo) {
        menuItem.id = itemInfo.id
        menuItem.text = itemInfo.title + "\t" + itemInfo.portableShortcuts
        menuItem.enabled = itemInfo.enabled
        menuItem.checked = itemInfo.checked
        menuItem.checkable = itemInfo.checkable
        menuItem.separator = !Boolean(itemInfo.title)
        menuItem.role = itemInfo.role
    }

    Component {
        id: menuComponent

        PLATFORM.Menu {
            property string id: ""
            property var subitems: []

            // Submenus made by load(). clear() deletes only the items that show them,
            // so the submenus are destroyed here before the menu is rebuilt.
            property var submenus: []
            function releaseSubmenus() {
                for (let menu of submenus) {
                    removeMenu(menu)
                    menu.releaseSubmenus()
                    menu.clear()
                    menu.destroy()
                }
                submenus = []
            }

            function shownSubitems() {
                let shown = []
                for (var i in subitems) {
                    if (Boolean(subitems[i])) {
                        shown.push(subitems[i])
                    }
                }
                return shown
            }

            function isSubmenu(itemInfo) {
                return Boolean(itemInfo.subitems) && itemInfo.subitems.length > 0
            }

            function load() {
                releaseSubmenus()
                clear()

                let made = []
                for (let item of shownSubitems()) {
                    if (isSubmenu(item)) {
                        let menu = root.makeMenu(item)
                        made.push(menu)
                        addMenu(menu)
                        menu.load()
                    } else {
                        let menuItem = root.makeMenuItem(this, item)
                        addItem(menuItem)
                    }
                }
                submenus = made
            }

            function update() {
                let shown = shownSubitems()

                // Open recent and similar items turn into submenus once they have entries, and back
                // when emptied. Rebuild instead of updating native items that no longer match.
                let matches = shown.length === items.length
                for (let i = 0; matches && i < shown.length; ++i) {
                    matches = isSubmenu(shown[i]) === Boolean(items[i].subMenu)
                }

                if (!matches) {
                    load()
                    return
                }

                for (let i = 0; i < shown.length; ++i) {
                    if (items[i].subMenu) {
                        root.setUpMenu(items[i].subMenu, shown[i])
                    } else {
                        root.setUpMenuItem(items[i], shown[i])
                    }
                }
            }

            onAboutToShow: {
                update()
            }
        }
    }

    Component {
        id: menuItemComponent

        PLATFORM.MenuItem {
            property string id: ""

            onTriggered: {
                Qt.callLater(menuModel.handleMenuItem, id)
            }
        }
    }
}
