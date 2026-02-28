// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15

Item {
    id: root

    property real value: 0
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property bool enabled: true
    property bool showValue: false

    signal moved()

    implicitWidth: 200
    implicitHeight: 28

    function _clamp(val) {
        return Math.max(from, Math.min(to, val));
    }

    function _snap(val) {
        if (stepSize > 0) {
            var steps = Math.round((val - from) / stepSize);
            return from + steps * stepSize;
        }
        return val;
    }

    // Track
    Rectangle {
        id: track
        anchors.left: parent.left
        anchors.right: valueLabel.visible ? valueLabel.left : parent.right
        anchors.rightMargin: valueLabel.visible ? 8 : 0
        anchors.verticalCenter: parent.verticalCenter
        height: 4
        radius: 2
        color: paletteMid
        opacity: root.enabled ? 1.0 : 0.5

        // Filled portion
        Rectangle {
            width: handle.x + handle.width / 2
            height: parent.height
            radius: 2
            color: root.enabled ? paletteHighlight : paletteMid
        }

        // Handle
        Rectangle {
            id: handle
            width: 16
            height: 16
            radius: 8
            y: (track.height - height) / 2
            x: root.to > root.from
               ? ((root.value - root.from) / (root.to - root.from))
                 * (track.width - width)
               : 0
            color: !root.enabled ? paletteMid
                   : dragArea.pressed ? Qt.darker(paletteHighlight, 1.2)
                   : dragArea.containsMouse ? Qt.lighter(paletteHighlight, 1.1)
                   : paletteHighlight
            border.width: 1
            border.color: Qt.darker(paletteHighlight, 1.3)

            MouseArea {
                id: dragArea
                anchors.fill: parent
                anchors.margins: -4
                hoverEnabled: root.enabled
                drag.target: handle
                drag.axis: Drag.XAxis
                drag.minimumX: 0
                drag.maximumX: track.width - handle.width
                cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                onPositionChanged: {
                    if (!root.enabled || !pressed) return;
                    var ratio = (handle.x) / (track.width - handle.width);
                    var newVal = root.from + ratio * (root.to - root.from);
                    root.value = root._clamp(root._snap(newVal));
                    root.moved();
                }
            }
        }

        // Click on track to jump
        MouseArea {
            anchors.fill: parent
            anchors.margins: -6
            z: -1
            enabled: root.enabled

            onClicked: function(mouse) {
                var ratio = mouse.x / track.width;
                var newVal = root.from + ratio * (root.to - root.from);
                root.value = root._clamp(root._snap(newVal));
                root.moved();
            }
        }
    }

    // Optional value display
    Text {
        id: valueLabel
        visible: root.showValue
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: Math.round(root.value)
        color: paletteWindowText
        font.pixelSize: 13
        width: 40
        horizontalAlignment: Text.AlignRight
    }
}
