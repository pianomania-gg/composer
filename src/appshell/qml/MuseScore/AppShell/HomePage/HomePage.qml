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

import Muse.Ui
import Muse.UiComponents
import Muse.Dock

import MuseScore.Project

DockPage {
    id: root

    property string section: "scores"
    property string subSection: ""

    property var window: null

    objectName: "Home"
    uri: "musescore://home"

    onSetParamsRequested: function(params) {
        setCurrentCentral(params["section"] || "scores")
        if (Boolean(params["subSection"])) {
            subSection = params["subSection"]
        }
    }

    onSectionChanged: {
        Qt.callLater(root.setCurrentCentral, section)
    }

    function setCurrentCentral(name) {
        if (section === name || !Boolean(name)) {
            return
        }

        section = name

        switch (name) {
        case "scores": root.central = scoresComp; break
        case "plugins":
        case "extensions": root.central = extensionsComp; break
        }
    }

    panels: []

    central: scoresComp

    Component {
        id: scoresComp

        ScoresPage {}
    }

    Component {
        id: extensionsComp

        PluginsPage {
            onBackRequested: root.setCurrentCentral("scores")
        }
    }

}
