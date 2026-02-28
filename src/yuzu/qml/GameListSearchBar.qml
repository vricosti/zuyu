// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property alias text: searchField.text
    property int visibleCount: 0
    property int totalCount: 0

    signal closeRequested()

    height: 40
    color: paletteWindow

    function forceActiveFocus() {
        searchField.forceActiveFocus();
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 10

        Text {
            text: qsTr("Filter:")
            color: paletteWindowText
        }

        // TextField replacement: Rectangle + TextInput
        Rectangle {
            Layout.fillWidth: true
            height: 28
            border.color: searchField.activeFocus ? paletteHighlight : paletteMid
            border.width: 1
            radius: 3
            color: paletteBase

            TextInput {
                id: searchField
                anchors.fill: parent
                anchors.margins: 4
                verticalAlignment: TextInput.AlignVCenter
                clip: true
                color: paletteWindowText
                selectionColor: paletteHighlight
                selectedTextColor: paletteHighlightedText
                selectByMouse: true

                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Enter pattern to filter")
                    color: paletteMid
                    visible: !searchField.text && !searchField.activeFocus
                }

                Keys.onEscapePressed: {
                    if (text.length > 0) {
                        text = "";
                    } else {
                        root.closeRequested();
                    }
                }

                Keys.onReturnPressed: {
                    if (root.visibleCount === 1) {
                        gameListView.launchSingleResult();
                    }
                }

                Keys.onEnterPressed: Keys.onReturnPressed
            }
        }

        Text {
            text: qsTr("%1 of %2 result(s)").arg(root.visibleCount).arg(root.totalCount)
            color: paletteWindowText
        }

        // Close button
        Rectangle {
            width: 28
            height: 28
            radius: 3
            color: closeMouseArea.containsMouse ? "#E81123" : paletteButton

            Text {
                anchors.centerIn: parent
                text: "X"
                font.bold: true
                color: closeMouseArea.containsMouse ? "#EEEEEE" : paletteButtonText
            }

            MouseArea {
                id: closeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.closeRequested()
            }
        }
    }
}
