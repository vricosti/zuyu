// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    signal openLogLocation()

    Flickable {
        id: flickable
        anchors.fill: parent
        anchors.margins: 12
        contentHeight: content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: content
            width: flickable.width
            spacing: 4

            Text {
                text: pageTitle
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            // Open Log Location button
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                color: openLogMa.containsMouse ? paletteHighlight : paletteButton
                border.color: paletteMid
                border.width: 1
                radius: 4

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Open Log Location")
                    color: openLogMa.containsMouse ? paletteHighlightedText
                                                   : paletteButtonText
                    font.pixelSize: 12
                }

                MouseArea {
                    id: openLogMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.openLogLocation()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            Repeater {
                model: settingsModel
                delegate: SettingDelegate {
                    width: content.width
                    settingsModel: root.parent ? settingsModel : null
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // Scrollbar
    Rectangle {
        visible: flickable.contentHeight > flickable.height
        anchors.right: parent.right
        anchors.rightMargin: 2
        y: flickable.contentHeight > 0
           ? (flickable.contentY / flickable.contentHeight) * parent.height : 0
        width: 6
        height: flickable.contentHeight > 0
                ? Math.max(20, (flickable.height / flickable.contentHeight) * parent.height)
                : 0
        radius: 3
        color: "#80808080"
    }
}
