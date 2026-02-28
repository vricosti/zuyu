// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15

Rectangle {
    id: root

    property alias text: input.text
    property alias placeholderText: placeholder.text
    property alias readOnly: input.readOnly
    property alias echoMode: input.echoMode
    property alias validator: input.validator
    property alias inputMethodHints: input.inputMethodHints
    property alias maximumLength: input.maximumLength
    property bool enabled: true

    signal accepted()
    signal textEdited()

    function forceActiveFocus() {
        input.forceActiveFocus();
    }

    function selectAll() {
        input.selectAll();
    }

    implicitWidth: 200
    implicitHeight: 28
    radius: 3
    border.width: 1
    border.color: !root.enabled ? paletteMid
                  : input.activeFocus ? paletteHighlight
                  : paletteMid
    color: root.enabled ? paletteBase : Qt.lighter(paletteButton, 1.05)
    opacity: root.enabled ? 1.0 : 0.6

    TextInput {
        id: input
        anchors.fill: parent
        anchors.margins: 4
        verticalAlignment: TextInput.AlignVCenter
        clip: true
        color: paletteWindowText
        selectionColor: paletteHighlight
        selectedTextColor: paletteHighlightedText
        selectByMouse: true
        font.pixelSize: 13
        enabled: root.enabled

        onAccepted: root.accepted()
        onTextEdited: root.textEdited()

        Text {
            id: placeholder
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            color: paletteMid
            font: input.font
            visible: !input.text && !input.activeFocus
        }
    }
}
