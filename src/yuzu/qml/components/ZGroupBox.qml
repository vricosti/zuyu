// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property alias title: titleText.text
    property bool collapsible: false
    property bool collapsed: false

    default property alias content: contentColumn.children

    implicitWidth: 200
    implicitHeight: titleRow.height + (collapsed ? 0 : contentColumn.height + 8)

    Behavior on implicitHeight { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        // Title row
        RowLayout {
            id: titleRow
            Layout.fillWidth: true
            spacing: 6

            Text {
                visible: root.collapsible
                text: root.collapsed ? "\u25B6" : "\u25BC"
                color: paletteMid
                font.pixelSize: 10
            }

            Text {
                id: titleText
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            // Horizontal line
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: paletteMid
                Layout.alignment: Qt.AlignVCenter
            }

            MouseArea {
                visible: root.collapsible
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.collapsed = !root.collapsed
            }
        }

        // Content
        ColumnLayout {
            id: contentColumn
            visible: !root.collapsed
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.topMargin: 8
            spacing: 6
        }
    }
}
