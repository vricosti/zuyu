// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15

Rectangle {
    id: root

    property alias text: label.text
    property bool checked: false
    property bool enabled: true

    signal clicked()

    width: label.implicitWidth + 12
    height: parent.height
    color: !root.enabled ? "transparent"
           : mouseArea.pressed ? Qt.darker(paletteButton, 1.2)
           : mouseArea.containsMouse ? Qt.darker(paletteButton, 1.05)
           : checked ? Qt.darker(paletteButton, 1.02)
           : "transparent"
    opacity: root.enabled ? 1.0 : 0.5

    Text {
        id: label
        anchors.centerIn: parent
        color: paletteButtonText
        font.pixelSize: 11
        font.bold: root.checked
    }

    // Left separator
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: 3
        anchors.bottomMargin: 3
        width: 1
        color: paletteMid
        opacity: 0.5
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.enabled
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: function(mouse) {
            if (!root.enabled) return;
            if (mouse.button === Qt.LeftButton) {
                root.clicked();
            }
        }
    }
}
