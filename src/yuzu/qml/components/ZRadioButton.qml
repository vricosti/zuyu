// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property bool checked: false
    property alias text: label.text
    property bool enabled: true

    signal toggled()

    implicitWidth: contentRow.implicitWidth
    implicitHeight: Math.max(24, contentRow.implicitHeight)

    RowLayout {
        id: contentRow
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Rectangle {
            width: 18
            height: 18
            radius: 9
            border.width: 1
            border.color: !root.enabled ? paletteMid
                          : mouseArea.containsMouse ? paletteHighlight
                          : paletteMid
            color: paletteBase
            opacity: root.enabled ? 1.0 : 0.6

            Rectangle {
                anchors.centerIn: parent
                width: 10
                height: 10
                radius: 5
                color: paletteHighlight
                visible: root.checked
            }
        }

        Text {
            id: label
            color: root.enabled ? paletteWindowText : paletteMid
            font.pixelSize: 13
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.enabled
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

        onClicked: {
            if (!root.enabled) return;
            root.checked = true;
            root.toggled();
        }
    }
}
