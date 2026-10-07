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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.AppShell

StyledDialogView {
    id: root

    title: qsTrc("appshell/gettingstarted", "Getting started")
    contentWidth: 576
    contentHeight: 384
    margins: 20

    FirstLaunchSetupModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
        navigationActiveTimer.start()
    }

    onAboutToClose: model.finish()

    Timer {
        id: navigationActiveTimer
        interval: 1000
        repeat: false
        onTriggered: {
            doneButton.navigation.accessible.ignored = true
            doneButton.navigation.requestActive()
            themePage.readInfo()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 28
        anchors.rightMargin: 28
        spacing: 24

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ThemesPage {
                id: themePage
                navigationSection: root.navigationSection
                activeButtonTitle: doneButton.text
            }
        }

        FlatButton {
            id: doneButton
            Layout.alignment: Qt.AlignRight
            text: qsTrc("global", "Done")
            accentButton: true

            navigation.name: "DoneButton"
            navigation.panel: NavigationPanel {
                name: "ButtonsPanel"
                enabled: doneButton.enabled && doneButton.visible
                section: root.navigationSection
                order: 1
                direction: NavigationPanel.Horizontal
            }
            navigation.column: 0
            navigation.onActiveChanged: {
                if (!navigation.active) {
                    accessible.ignored = false
                    accessible.focused = true
                    themePage.resetFocus()
                }
            }

            onClicked: {
                model.finish()
                root.hide()
            }
        }
    }
}
