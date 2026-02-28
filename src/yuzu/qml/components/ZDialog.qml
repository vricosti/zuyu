// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property alias title: titleText.text
    property string acceptText: qsTr("OK")
    property string rejectText: qsTr("Cancel")
    property bool showRejectButton: true
    property bool acceptEnabled: true

    default property alias content: contentArea.children

    signal accepted()
    signal rejected()

    color: "#80000000"

    // Block clicks through to parent
    MouseArea {
        anchors.fill: parent
        onClicked: {} // absorb
    }

    Rectangle {
        id: dialog
        anchors.centerIn: parent
        width: Math.min(root.width * 0.8, 500)
        height: dialogLayout.implicitHeight + 32
        radius: 6
        color: paletteWindow
        border.width: 1
        border.color: paletteMid

        ColumnLayout {
            id: dialogLayout
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            // Title
            Text {
                id: titleText
                Layout.fillWidth: true
                font.pixelSize: 16
                font.bold: true
                color: paletteWindowText
                visible: text.length > 0
            }

            // Content area
            Item {
                id: contentArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 40
            }

            // Button row
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Item { Layout.fillWidth: true }

                ZButton {
                    visible: root.showRejectButton
                    text: root.rejectText
                    onClicked: root.rejected()
                }

                ZButton {
                    text: root.acceptText
                    enabled: root.acceptEnabled
                    // Highlight the accept button
                    color: !enabled ? Qt.lighter(paletteButton, 1.05)
                           : mouseAreaAccept.pressed ? Qt.darker(paletteHighlight, 1.2)
                           : mouseAreaAccept.containsMouse ? Qt.lighter(paletteHighlight, 1.1)
                           : paletteHighlight
                    border.color: paletteHighlight

                    Text {
                        // Override text color for highlighted button
                        anchors.centerIn: parent
                        text: root.acceptText
                        color: paletteHighlightedText
                        font.pixelSize: 13
                    }

                    // Hide the default label
                    font.pixelSize: 0

                    MouseArea {
                        id: mouseAreaAccept
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: if (root.acceptEnabled) root.accepted()
                    }
                }
            }
        }
    }
}
