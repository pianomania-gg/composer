// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import Muse.Ui
import Muse.UiComponents
import MuseScore.AppShell

FlatButton {
    id: root
    ComposerAccountModel { id: account }
    buttonType: FlatButton.Custom
    implicitWidth: account.signedIn ? 212 : 152
    width: implicitWidth
    height: 30
    transparent: true
    textFormat: Text.PlainText
    text: account.error ? qsTrc("appshell", "Account unavailable")
                       : account.signedIn ? "" : qsTrc("appshell", "Sign in to Pianomania")
    toolTipTitle: account.signedIn ? qsTrc("appshell", "Signed in as %1").arg(account.email) : text
    toolTipDescription: account.error
    accessible.name: toolTipTitle
    onClicked: account.openAccount()

    Column {
        visible: account.signedIn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 0
        StyledTextLabel {
            width: parent.width
            text: qsTrc("appshell", "Signed in as")
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignLeft
            font: Qt.font({ family: ui.theme.bodyFont.family, pixelSize: Math.max(10, ui.theme.bodyFont.pixelSize - 2) })
            color: ui.theme.fontPrimaryColor
            opacity: 0.75
        }
        StyledTextLabel {
            width: parent.width
            text: account.email || qsTrc("appshell", "Pianomania account")
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideMiddle
            font: ui.theme.bodyFont
        }
    }
}
