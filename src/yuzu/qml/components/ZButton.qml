// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property alias text: label.text
    property bool checked: false
    property bool checkable: false
    property bool flat: false
    property bool enabled: true
    property alias font: label.font
    property alias iconText: iconLabel.text

    signal clicked()
    signal pressAndHold()

    implicitWidth: contentRow.implicitWidth + 24
    implicitHeight: Math.max(28, contentRow.implicitHeight + 8)
    radius: 3
    border.width: flat ? 0 : 1
    border.color: !root.enabled ? Qt.darker(paletteButton, 1.1)
                  : mouseArea.containsMouse ? paletteHighlight
                  : paletteMid
    color: !root.enabled ? Qt.lighter(paletteButton, 1.05)
           : mouseArea.pressed ? Qt.darker(paletteButton, 1.2)
           : (root.checked || mouseArea.containsMouse) ? Qt.darker(paletteButton, 1.1)
           : flat ? "transparent"
           : paletteButton
    opacity: root.enabled ? 1.0 : 0.6

    RowLayout {
        id: contentRow
        anchors.centerIn: parent
        spacing: 6

        Text {
            id: iconLabel
            visible: text.length > 0
            color: root.enabled ? paletteButtonText : paletteMid
            font.pixelSize: 14
        }

        Text {
            id: label
            color: root.enabled ? paletteButtonText : paletteMid
            font.pixelSize: 13
            elide: Text.ElideRight
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.enabled
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

        onClicked: {
            if (!root.enabled) return;
            if (root.checkable) root.checked = !root.checked;
            root.clicked();
        }

        onPressAndHold: {
            if (!root.enabled) return;
            root.pressAndHold();
        }
    }
}
