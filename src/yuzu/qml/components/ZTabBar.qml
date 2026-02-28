// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property var model: []
    property int currentIndex: 0
    property bool enabled: true

    signal tabClicked(int index)

    implicitHeight: 36
    color: paletteWindow

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        spacing: 2

        Repeater {
            model: root.model

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: tabText.implicitWidth + 24
                radius: 3
                color: index === root.currentIndex
                       ? paletteBase
                       : tabMouseArea.containsMouse ? Qt.darker(paletteWindow, 1.05) : "transparent"
                border.width: index === root.currentIndex ? 1 : 0
                border.color: paletteMid

                // Bottom highlight for active tab
                Rectangle {
                    visible: index === root.currentIndex
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 2
                    color: paletteHighlight
                }

                Text {
                    id: tabText
                    anchors.centerIn: parent
                    text: modelData
                    color: index === root.currentIndex ? paletteWindowText
                           : paletteMid
                    font.pixelSize: 13
                    font.bold: index === root.currentIndex
                }

                MouseArea {
                    id: tabMouseArea
                    anchors.fill: parent
                    hoverEnabled: root.enabled
                    cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                    onClicked: {
                        if (!root.enabled) return;
                        root.currentIndex = index;
                        root.tabClicked(index);
                    }
                }
            }
        }

        Item { Layout.fillWidth: true }
    }
}
