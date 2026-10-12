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
import QtQuick.Layouts
import Muse.Ui
import Muse.UiComponents
import MuseScore.Project
import "internal/ScoresPage"

FocusScope {
    id: root
    readonly property int sideMargin: width < 900 ? 28 : 46
    readonly property bool hasRecentScores: recentScoresModel.rowCount > 2

    FontLoader { id: controlsFont; source: "qrc:/composer/song-details/Inter-Regular.ttf" }
    FontLoader { id: titleFont; source: "qrc:/composer/song-details/Inter-Medium.ttf" }
    FontLoader { id: actionFont; source: "qrc:/composer/song-details/BarlowCondensed-ExtraBoldItalic.ttf" }
    ScoresPageModel { id: scoresPageModel }
    RecentScoresModel { id: recentScoresModel }
    Component.onCompleted: recentScoresModel.load()

    NavigationSection {
        id: navSec
        name: "RecentScores"
        enabled: root.enabled && root.visible
        order: 3
        onActiveChanged: function(active) {
            if (active) root.forceActiveFocus()
        }
    }
    NavigationPanel {
        id: actionsNav
        name: "ScoreActions"
        section: navSec
        order: 1
        direction: NavigationPanel.Horizontal
    }

    function paintSurface(canvas, top, bottom, edge, radius) {
        const ctx = canvas.getContext("2d")
        ctx.reset()
        const w = canvas.width - 1
        const h = canvas.height - 4
        const inset = h * 0.12278456
        // Canvas arcTo uses the same circular tangent corners as SkewGeometry.
        const points = [[inset + 0.5, 0.5], [w, 0.5], [w - inset, h], [0.5, h]]
        const cornerRadius = Math.min(radius, h / 2)
        const halfAngle = Math.acos(inset / Math.hypot(inset, h)) / 2
        const tangent = cornerRadius / Math.tan(halfAngle)
        const factor = Math.min(1, (w - inset - 0.5) / (tangent * 2), Math.hypot(inset, h) / (tangent * 2))
        const roundedRadius = cornerRadius * factor
        ctx.beginPath()
        // Begin on the left edge, midway between corners, to give arcTo an incoming tangent.
        ctx.moveTo((points[3][0] + points[0][0]) / 2, (points[3][1] + points[0][1]) / 2)
        for (let i = 0; i < 4; ++i) {
            const corner = points[i]
            const next = points[(i + 1) % 4]
            ctx.arcTo(corner[0], corner[1], next[0], next[1], roundedRadius)
        }
        ctx.closePath()
        const fill = ctx.createLinearGradient(0, 0, 0, h)
        fill.addColorStop(0, top)
        fill.addColorStop(1, bottom)
        ctx.fillStyle = fill
        ctx.fill()
        ctx.strokeStyle = edge
        ctx.lineWidth = 1
        ctx.stroke()
    }

    component ScoreAction: FlatButton {
        id: button
        property bool primary: false
        implicitHeight: 48
        implicitWidth: primary ? 166 : 126
        Layout.maximumWidth: implicitWidth
        Layout.preferredWidth: implicitWidth
        textFont.family: primary ? actionFont.name : controlsFont.name
        textFont.pixelSize: primary ? 25 : 15
        textFont.italic: primary
        textFont.weight: primary ? Font.ExtraBold : Font.Normal
        backgroundItem: Canvas {
            id: face
            onPaint: root.paintSurface(face,
                button.primary ? (button.mouseArea.containsMouse ? "#3d9256" : "#347c4a") : "#273443",
                button.primary ? "#29643c" : "#17212d",
                button.navigation.active ? "#edf2f8" : button.primary ? "#65a878" : "#536377", 8)
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            Connections {
                target: button.mouseArea
                function onContainsMouseChanged() { face.requestPaint() }
                function onPressedChanged() { face.requestPaint() }
            }
            Connections {
                target: button.navigation
                function onActiveChanged() { face.requestPaint() }
            }
        }
        contentItem: Text {
            text: button.primary ? button.text.toUpperCase() : button.text
            color: "#f1f4f8"
            font: button.textFont
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0; color: "#0d1723" }
            GradientStop { position: 1; color: "#050c14" }
        }
    }
    RowLayout {
        id: heading
        anchors.top: parent.top
        anchors.topMargin: root.hasRecentScores ? 32 : Math.max(32, (root.height - height) / 2)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: root.hasRecentScores ? root.sideMargin : Math.max(root.sideMargin, (root.width - 304) / 2)
        anchors.rightMargin: anchors.leftMargin
        spacing: 12

        Text {
            Layout.fillWidth: true
            visible: root.hasRecentScores
            text: qsTrc("project", "Recent scores")
            color: "#f1f4f8"
            font.family: titleFont.name
            font.pixelSize: 32
            elide: Text.ElideRight
        }
        ScoreAction {
            primary: true
            text: qsTrc("project", "New score")
            navigation.name: "NewScore"
            navigation.panel: actionsNav
            navigation.column: 1
            onClicked: scoresPageModel.createNewScore()
        }
        ScoreAction {
            text: qsTrc("project", "Open…")
            navigation.name: "OpenScore"
            navigation.panel: actionsNav
            navigation.column: 2
            onClicked: scoresPageModel.openOther()
        }
    }
    Rectangle {
        id: rule
        visible: root.hasRecentScores
        height: 1
        color: "#283648"
        anchors.top: heading.bottom
        anchors.topMargin: 20
        anchors.left: heading.left
        anchors.right: heading.right
    }
    SearchField {
        id: searchField
        anchors.top: rule.bottom
        anchors.topMargin: 24
        anchors.right: heading.right
        width: 260
        visible: root.hasRecentScores
        hint: qsTrc("project", "Search recent scores")
        hintIcon: IconCode.NONE
        clearTextButtonVisible: false
        inputField.color: "#edf2f8"
        inputField.placeholderTextColor: "#98a9bf"
        inputField.selectedTextColor: "#ffffff"
        inputField.font.family: controlsFont.name
        inputField.font.pixelSize: 14
        background.color: "#02070d"
        background.border.color: "#34475f"
        navigation.name: "ScoresSearch"
        navigation.panel: actionsNav
        navigation.column: 3
    }
    SortFilterProxyModel {
        id: filteredScores
        sourceModel: recentScoresModel
        alwaysExcludeIndices: recentScoresModel.nonScoreItemIndices
        filters: FilterValue {
            roleName: "name"
            roleValue: searchField.searchText
            compareType: CompareType.Contains
        }
    }
    StyledListView {
        id: scores
        anchors.top: searchField.visible ? searchField.bottom : rule.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 28
        anchors.left: heading.left
        anchors.right: heading.right
        spacing: 12
        model: filteredScores
        navigation.name: "RecentScoresList"
        navigation.section: navSec
        navigation.order: 2
        accessible.name: qsTrc("project", "Recent scores")

        delegate: ListItemBlank {
            id: scoreRow
            required property var score
            required property int index
            width: scores.width
            height: 108
            normalColor: "transparent"
            hoverHitColor: "transparent"
            selectedColor: "transparent"
            hint: score.path ?? ""
            navigation.panel: scores.navigation
            navigation.row: index
            navigation.accessible.name: score.name ?? ""
            navigation.onActiveChanged: {
                face.requestPaint()
                if (navigation.active) scores.positionViewAtIndex(index, ListView.Contain)
            }
            onHovered: face.requestPaint()
            onClicked: Qt.callLater(scoresPageModel.openScore, score.path, score.name)

            Canvas {
                id: face
                anchors.fill: parent
                onPaint: root.paintSurface(face, "#1b2735", "#0b131e",
                    scoreRow.navigation.active || scoreRow.mouseArea.containsMouse ? "#9bafc8" : "#3a4e68", 9)
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
            }
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 28
                anchors.rightMargin: 28
                spacing: 24
                ScoreThumbnail {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 64
                    path: scoreRow.score.path ?? ""
                    suffix: scoreRow.score.suffix ?? ""
                    thumbnailUrl: scoreRow.score.thumbnailUrl ?? ""
                }
                Text {
                    Layout.fillWidth: true
                    text: scoreRow.score.name ?? ""
                    color: "#f1f4f8"
                    font.family: titleFont.name
                    font.pixelSize: 25
                    elide: Text.ElideRight
                    textFormat: Text.PlainText
                }
                Text {
                    text: scoreRow.score.timeSinceModified ?? ""
                    color: "#98a9bf"
                    font.family: controlsFont.name
                    font.pixelSize: 13
                }
            }
        }
    }
}
