/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

import Muse.Ui
import Muse.UiComponents

import MuseScore.AppShell

StyledDialogView {
    id: root

    title: qsTrc("appshell/welcome", "Welcome")

    contentHeight: contentColumn.height + footerArea.height
    contentWidth: 900

    WelcomeDialogModel {
        id: welcomeModel

        Component.onCompleted: {
            welcomeModel.init()
        }
    }

    QtObject {
        id: prv
        readonly property int imageWidth: 740
        readonly property bool isWelcome: Boolean(welcomeModel.currentItem && welcomeModel.currentItem.isWelcome)
        readonly property bool hasMenuAction: Boolean(welcomeModel.currentItem && welcomeModel.currentItem.menuAction)
        readonly property string titleText: welcomeModel.currentItem ? welcomeModel.currentItem.title : ""
        readonly property string descText: welcomeModel.currentItem ? welcomeModel.currentItem.description : ""
    }

    function openCurrent() {
        if (!welcomeModel.currentItem) {
            return
        }
        api.launcher.openUrl(welcomeModel.currentItem.destinationUrl)
    }

    Column {
        id: contentColumn

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: contentColumn.spacing
        }

        StyledTextLabel {
            id: titleLabel

            height: 80
            width: prv.imageWidth
            anchors.horizontalCenter: contentColumn.horizontalCenter

            text: prv.titleText
            font: ui.theme.headerBoldFont
            wrapMode: Text.WordWrap

            maximumLineCount: 2
        }

        Row {
            id: imageAndArrowsRow

            height: 416
            anchors {
                left: contentColumn.left
                right: contentColumn.right
            }

            NavigationPanel {
                id: arrowButtonsPanel
                name: "ArrowButtonsPanel"
                order: 2
                section: root.navigationSection
                direction: NavigationPanel.Horizontal
            }

            Item {
                id: prevButtonArea

                height: imageAndArrowsRow.height
                width: (imageAndArrowsRow.width - image.width) / 2

                FlatButton {
                    id: prevButton
                    enabled: welcomeModel.currentIndex > 0

                    height: 48
                    width: prevButton.height
                    anchors.centerIn: prevButtonArea

                    contentItem: StyledIconLabel {
                        iconCode: IconCode.CHEVRON_LEFT
                        font.pixelSize: 30
                    }

                    navigation.panel: arrowButtonsPanel
                    navigation.column: 0
                    navigation.accessible.description: qsTrc("appshell/welcome", "Previous item")

                    onClicked: {
                        welcomeModel.prevItem()
                    }
                }
            }

            Rectangle {
                id: image
                width: prv.imageWidth
                height: imageAndArrowsRow.height
                radius: 12
                color: "#171e2d"
                border.color: "#36475e"
                clip: true

                Column {
                    visible: prv.isWelcome
                    anchors.centerIn: parent
                    width: parent.width - 64
                    spacing: 28

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 10

                        StyledTextLabel {
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTrc("appshell/welcome", "Powered by")
                            color: "#c3cede"
                            font: ui.theme.largeBodyFont
                        }
                        Image {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 28
                            height: 28
                            source: "resources/mu_logo.svg"
                            sourceSize: Qt.size(56, 56)
                        }
                        Image {
                            anchors.verticalCenter: parent.verticalCenter
                            height: 22
                            width: 143
                            source: "qrc:/resources/welcomedialog/MuseScoreStudioWordmark.png"
                            fillMode: Image.PreserveAspectFit
                            mipmap: true
                        }
                    }
                    StyledTextLabel {
                        width: parent.width
                        text: qsTrc("appshell/welcome", "Add your own music to Pianomania!")
                        color: "#f1f4f8"
                        font.pixelSize: 36
                        font.bold: true
                        wrapMode: Text.WordWrap
                    }
                }

                Image {
                    visible: !prv.isWelcome
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: prv.hasMenuAction ? 498 : parent.width
                    height: parent.height
                    source: !prv.isWelcome && welcomeModel.currentItem ? welcomeModel.currentItem.imageUrl : ""
                    // Keep the highlighted account/export section readable within the tutorial.
                    sourceClipRect: prv.hasMenuAction ? Qt.rect(12, 330, 500, 385) : Qt.rect(0, 0, 0, 0)
                    fillMode: Image.PreserveAspectFit
                }

                Column {
                    visible: prv.hasMenuAction
                    anchors.right: parent.right
                    anchors.rightMargin: 24
                    anchors.verticalCenter: parent.verticalCenter
                    width: 192
                    spacing: 18

                    StyledTextLabel {
                        width: parent.width
                        text: qsTrc("appshell/welcome", "File")
                        horizontalAlignment: Text.AlignLeft
                        color: "#a6c8ef"
                        font: ui.theme.tabBoldFont
                    }
                    Rectangle { width: 32; height: 2; color: "#a6c8ef" }
                    StyledTextLabel {
                        width: parent.width
                        text: prv.hasMenuAction ? welcomeModel.currentItem.menuAction : ""
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        color: "#f1f4f8"
                        font: ui.theme.headerBoldFont
                    }
                    StyledTextLabel {
                        width: parent.width
                        text: prv.hasMenuAction ? welcomeModel.currentItem.menuHint : ""
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        color: "#c3cede"
                        font: ui.theme.largeBodyFont
                    }
                }
            }

            Item {
                id: nextButtonArea

                width: (imageAndArrowsRow.width - image.width) / 2
                height: imageAndArrowsRow.height

                FlatButton {
                    id: nextButton
                    enabled: welcomeModel.currentIndex < welcomeModel.count - 1

                    height: 48
                    width: nextButton.height
                    anchors.centerIn: nextButtonArea

                    contentItem: StyledIconLabel {
                        iconCode: IconCode.CHEVRON_RIGHT
                        font.pixelSize: 30
                    }

                    navigation.panel: arrowButtonsPanel
                    navigation.column: 1
                    navigation.accessible.description: qsTrc("appshell/welcome", "Next item")

                    onClicked: {
                        welcomeModel.nextItem()
                    }
                }
            }
        }

        StyledTextLabel {
            id: descriptionLabel

            height: 96
            width: prv.imageWidth
            anchors.horizontalCenter: contentColumn.horizontalCenter

            text: prv.descText

            font: ui.theme.largeBodyFont
            wrapMode: Text.WordWrap
            maximumLineCount: 3
        }

        FlatButton {
            id: contentButton

            height: 40
            anchors.horizontalCenter: contentColumn.horizontalCenter

            text: welcomeModel.currentItem ? welcomeModel.currentItem.buttonText : ""
            textFont: ui.theme.tabBoldFont
            accentButton: true

            navigation.panel: NavigationPanel {
                name: "ContentButton"
                order: 0
                section: root.navigationSection
            }
            navigation.accessible.description: prv.titleText + "; " + prv.descText

            onClicked: {
                root.openCurrent()
            }
        }

        Item {
            id: indicatorArea

            height: 44
            width: contentColumn.width

            PageIndicator {
                anchors.centerIn: indicatorArea

                indicatorSize: 10

                count: welcomeModel.count
                currentIndex: welcomeModel.currentIndex
            }
        }
    }

    Rectangle {
        id: footerArea

        height: 60
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        color: ui.theme.backgroundSecondaryColor

        NavigationPanel {
            id: footerPanel
            name: "FooterPanel"
            order: 1
            section: root.navigationSection
            direction: NavigationPanel.Horizontal
        }

        CheckBox {
            id: showOnStartup

            anchors {
                margins: 24
                left: footerArea.left
                verticalCenter: footerArea.verticalCenter
            }

            text: qsTrc("appshell/welcome", "Don’t show welcome dialog on startup")
            checked: !welcomeModel.showOnStartup

            navigation.panel: footerPanel
            navigation.column: 1
            navigation.accessible.description: showOnStartup.text

            onClicked: {
                welcomeModel.showOnStartup = !welcomeModel.showOnStartup
            }
        }

        FlatButton {
            id: okButton

            width: 154
            height: 30
            anchors {
                margins: 24
                right: footerArea.right
                verticalCenter: footerArea.verticalCenter
            }

            text: welcomeModel.currentIndex < welcomeModel.count - 1
                  ? qsTrc("global", "Next") : qsTrc("global", "Done")

            navigation.panel: footerPanel
            navigation.column: 0
            navigation.accessible.description: okButton.text

            onClicked: {
                if (welcomeModel.currentIndex < welcomeModel.count - 1) {
                    welcomeModel.nextItem()
                } else {
                    root.accept()
                }
            }
        }
    }
}
