import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Muse.UiComponents
import MuseScore.Project

StyledDialogView {
    id: root
    objectName: "NotationCompatibilitySummary"
    title: NotationCompatibility.musicianTitle
    modal: false
    resizable: true
    contentWidth: 620
    contentHeight: 420
    margins: 24

    Connections {
        target: NotationCompatibility
        function onReportChanged() {
            if (!NotationCompatibility.needsFix && NotationCompatibility.musicianFindings.length === 0)
                root.close()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        ScrollView {
            id: findingsScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth
            contentHeight: findingsBody.implicitHeight
            clip: true
            Item {
                id: findingsBody
                width: findingsScroll.availableWidth
                height: implicitHeight
                implicitHeight: Math.max(findingsScroll.availableHeight, findingsColumn.implicitHeight)
                ColumnLayout {
                    id: findingsColumn
                    width: parent.width
                    anchors.centerIn: parent
                    spacing: 20
                    Repeater {
                        model: NotationCompatibility.musicianFindings
                        delegate: ColumnLayout {
                            required property int index
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: 8
                            StyledTextLabel {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                font.bold: true
                                text: modelData.title + (modelData.hasCount
                                    ? " · " + modelData.count + (modelData.count === 1 ? " mark" : " marks") : "")
                                wrapMode: Text.WordWrap
                            }
                            StyledTextLabel {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: modelData.description
                                wrapMode: Text.WordWrap
                            }
                            StyledTextLabel {
                                visible: modelData.locationLabel.length > 0
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                textFormat: Text.PlainText
                                text: modelData.locationLabel
                                wrapMode: Text.WordWrap
                            }
                            Button {
                                Layout.alignment: Qt.AlignHCenter
                                visible: modelData.locationLabel.length > 0
                                enabled: modelData.canNavigate
                                text: "Go to problem area"
                                onClicked: {
                                    if (NotationCompatibility.goToProblem(index)) root.close()
                                }
                            }
                        }
                    }
                }
            }
        }
        Button {
            Layout.alignment: Qt.AlignHCenter
            text: "Close"
            onClicked: root.close()
        }
    }
}
