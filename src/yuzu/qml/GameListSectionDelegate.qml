// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    required property int index
    required property string name
    required property int sectionType
    required property bool sectionExpanded
    required property int gameDirIndex

    signal toggleExpanded(int sourceIndex)

    height: 32
    color: paletteMid

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 8

        Text {
            text: root.sectionExpanded ? "\u25BC" : "\u25B6"
            font.pixelSize: 12
            color: paletteWindowText
        }

        Text {
            text: root.name
            font.bold: true
            color: paletteWindowText
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.toggleExpanded(root.index)
    }
}
