// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15

Item {
    id: root

    default property alias content: flickable.contentItem.children
    property alias contentWidth: flickable.contentWidth
    property alias contentHeight: flickable.contentHeight
    property alias contentItem: flickable.contentItem

    Flickable {
        id: flickable
        anchors.fill: parent
        anchors.rightMargin: vScrollbar.visible ? vScrollbar.width + 2 : 0
        clip: true
        boundsBehavior: Flickable.StopAtBounds
    }

    // Vertical scrollbar
    Rectangle {
        id: vScrollbar
        visible: flickable.contentHeight > flickable.height
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 6

        color: "transparent"

        Rectangle {
            id: vHandle
            anchors.right: parent.right
            width: parent.width
            radius: 3
            color: vHandleArea.pressed ? Qt.darker(paletteMid, 1.2)
                   : vHandleArea.containsMouse ? paletteMid
                   : Qt.lighter(paletteMid, 1.2)
            opacity: flickable.moving || vHandleArea.containsMouse ? 0.8 : 0.4

            y: flickable.contentHeight > 0
               ? (flickable.contentY / flickable.contentHeight) * vScrollbar.height
               : 0
            height: flickable.contentHeight > 0
                    ? Math.max(20, (flickable.height / flickable.contentHeight) * vScrollbar.height)
                    : 0

            Behavior on opacity { NumberAnimation { duration: 200 } }

            MouseArea {
                id: vHandleArea
                anchors.fill: parent
                hoverEnabled: true
                drag.target: parent
                drag.axis: Drag.YAxis
                drag.minimumY: 0
                drag.maximumY: vScrollbar.height - vHandle.height

                onPositionChanged: {
                    if (pressed) {
                        flickable.contentY = (vHandle.y / vScrollbar.height)
                                             * flickable.contentHeight;
                    }
                }
            }
        }
    }
}
