// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15

Item {
    id: root

    property real value: 0
    property real from: 0
    property real to: 100
    property bool indeterminate: false
    property color barColor: paletteHighlight

    implicitWidth: 200
    implicitHeight: 8

    Rectangle {
        id: background
        anchors.fill: parent
        radius: height / 2
        color: Qt.lighter(paletteMid, 1.3)

        // Determinate fill
        Rectangle {
            visible: !root.indeterminate
            width: root.to > root.from
                   ? ((root.value - root.from) / (root.to - root.from)) * parent.width
                   : 0
            height: parent.height
            radius: parent.radius
            color: root.barColor

            Behavior on width { NumberAnimation { duration: 200 } }
        }

        // Indeterminate animation
        Rectangle {
            id: indeterminateBar
            visible: root.indeterminate
            width: parent.width * 0.3
            height: parent.height
            radius: parent.radius
            color: root.barColor

            SequentialAnimation on x {
                running: root.indeterminate && root.visible
                loops: Animation.Infinite
                NumberAnimation {
                    from: -indeterminateBar.width
                    to: background.width
                    duration: 1500
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
