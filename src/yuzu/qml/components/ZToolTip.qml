// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15

Item {
    id: root

    property alias text: tipText.text
    property Item target: parent
    property int delay: 500
    property bool active: false

    visible: false
    z: 10000

    function show(globalX, globalY) {
        root.x = globalX + 8;
        root.y = globalY + 8;
        root.visible = true;
    }

    function hide() {
        root.visible = false;
        delayTimer.stop();
    }

    Timer {
        id: delayTimer
        interval: root.delay
        onTriggered: {
            if (root.active) {
                root.visible = true;
            }
        }
    }

    onActiveChanged: {
        if (active) {
            delayTimer.restart();
        } else {
            hide();
        }
    }

    Rectangle {
        id: background
        width: tipText.width + 12
        height: tipText.height + 8
        radius: 3
        color: typeof paletteToolTipBase !== "undefined" ? paletteToolTipBase : "#ffffdc"
        border.width: 1
        border.color: paletteMid

        Text {
            id: tipText
            anchors.centerIn: parent
            color: typeof paletteToolTipText !== "undefined" ? paletteToolTipText : "#000000"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            maximumLineCount: 5
        }
    }
}
