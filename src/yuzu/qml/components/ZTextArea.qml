// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15

Rectangle {
    id: root

    property alias text: edit.text
    property alias placeholderText: placeholder.text
    property alias readOnly: edit.readOnly
    property alias wrapMode: edit.wrapMode
    property bool enabled: true

    implicitWidth: 200
    implicitHeight: 100
    radius: 3
    border.width: 1
    border.color: !root.enabled ? paletteMid
                  : edit.activeFocus ? paletteHighlight
                  : paletteMid
    color: root.enabled ? paletteBase : Qt.lighter(paletteButton, 1.05)
    opacity: root.enabled ? 1.0 : 0.6

    Flickable {
        id: flickable
        anchors.fill: parent
        anchors.margins: 4
        contentWidth: width
        contentHeight: edit.contentHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        function ensureVisible(r) {
            if (contentX >= r.x) contentX = r.x;
            else if (contentX + width <= r.x + r.width)
                contentX = r.x + r.width - width;
            if (contentY >= r.y) contentY = r.y;
            else if (contentY + height <= r.y + r.height)
                contentY = r.y + r.height - height;
        }

        TextEdit {
            id: edit
            width: flickable.width
            wrapMode: TextEdit.Wrap
            color: paletteWindowText
            selectionColor: paletteHighlight
            selectedTextColor: paletteHighlightedText
            selectByMouse: true
            font.pixelSize: 13
            enabled: root.enabled
            onCursorRectangleChanged: flickable.ensureVisible(cursorRectangle)

            Text {
                id: placeholder
                anchors.fill: parent
                color: paletteMid
                font: edit.font
                visible: !edit.text && !edit.activeFocus
                wrapMode: Text.Wrap
            }
        }
    }

    // Scrollbar
    Rectangle {
        visible: flickable.contentHeight > flickable.height
        anchors.right: parent.right
        anchors.rightMargin: 2
        y: flickable.y + (flickable.contentY / flickable.contentHeight) * flickable.height
        width: 4
        height: Math.max(20, (flickable.height / flickable.contentHeight) * flickable.height)
        radius: 2
        color: paletteMid
        opacity: 0.5
    }
}
