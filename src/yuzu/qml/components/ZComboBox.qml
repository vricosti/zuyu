// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property var model: []
    property int currentIndex: -1
    property string currentText: currentIndex >= 0 && currentIndex < model.length
                                 ? (typeof model[currentIndex] === "object"
                                    ? model[currentIndex].text : model[currentIndex])
                                 : ""
    property bool enabled: true
    property string textRole: ""

    signal activated(int index)

    function getItemText(item) {
        if (typeof item === "object" && root.textRole.length > 0)
            return item[root.textRole];
        if (typeof item === "object" && item.text !== undefined)
            return item.text;
        return String(item);
    }

    implicitWidth: 200
    implicitHeight: 28

    Rectangle {
        id: button
        anchors.fill: parent
        radius: 3
        border.width: 1
        border.color: !root.enabled ? paletteMid
                      : mouseArea.containsMouse ? paletteHighlight
                      : paletteMid
        color: root.enabled ? paletteBase : Qt.lighter(paletteButton, 1.05)
        opacity: root.enabled ? 1.0 : 0.6

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 6
            spacing: 4

            Text {
                Layout.fillWidth: true
                text: root.currentIndex >= 0 ? root.getItemText(root.model[root.currentIndex]) : ""
                color: paletteWindowText
                font.pixelSize: 13
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                text: popup.visible ? "\u25B2" : "\u25BC"
                color: paletteMid
                font.pixelSize: 8
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: root.enabled
            cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

            onClicked: {
                if (!root.enabled) return;
                popup.visible = !popup.visible;
            }
        }
    }

    // Dropdown popup
    Rectangle {
        id: popup
        visible: false
        y: button.height + 2
        width: root.width
        height: Math.min(listView.contentHeight + 4, 200)
        radius: 3
        border.width: 1
        border.color: paletteMid
        color: paletteBase
        z: 1000
        clip: true

        ListView {
            id: listView
            anchors.fill: parent
            anchors.margins: 2
            model: root.model
            boundsBehavior: Flickable.StopAtBounds
            currentIndex: root.currentIndex

            delegate: Rectangle {
                width: listView.width
                height: 26
                radius: 2
                color: delegateMouseArea.containsMouse ? paletteHighlight
                       : index === root.currentIndex ? Qt.lighter(paletteHighlight, 1.5)
                       : "transparent"

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    verticalAlignment: Text.AlignVCenter
                    text: root.getItemText(modelData)
                    color: delegateMouseArea.containsMouse ? paletteHighlightedText
                           : paletteWindowText
                    font.pixelSize: 13
                    elide: Text.ElideRight
                }

                MouseArea {
                    id: delegateMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        root.currentIndex = index;
                        root.activated(index);
                        popup.visible = false;
                    }
                }
            }
        }
    }

    // Close popup when clicking outside
    Connections {
        target: popup.visible ? root.Window : null
        function onActiveFocusItemChanged() {
            popup.visible = false;
        }
    }
}
