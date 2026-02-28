// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    // The model must provide: display, hasChildren, isExpanded, childCount, depth
    // The model should be flat with depth info (like QAbstractItemModel flattened)
    property var model: null
    property int currentIndex: -1
    property int indentWidth: 20

    signal itemClicked(int index)
    signal itemDoubleClicked(int index)
    signal toggleExpanded(int index)

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        model: root.model
        boundsBehavior: Flickable.StopAtBounds
        currentIndex: root.currentIndex

        delegate: Rectangle {
            id: delegateRoot
            width: listView.width
            height: 24
            color: index === root.currentIndex ? paletteHighlight
                   : delegateMouseArea.containsMouse ? Qt.darker(paletteBase, 1.05)
                   : "transparent"

            property int itemDepth: typeof depth !== "undefined" ? depth : 0
            property bool itemHasChildren: typeof hasChildren !== "undefined" ? hasChildren : false
            property bool itemExpanded: typeof isExpanded !== "undefined" ? isExpanded : false

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: delegateRoot.itemDepth * root.indentWidth + 4
                spacing: 4

                // Expand/collapse arrow
                Text {
                    visible: delegateRoot.itemHasChildren
                    text: delegateRoot.itemExpanded ? "\u25BC" : "\u25B6"
                    color: index === root.currentIndex ? paletteHighlightedText : paletteMid
                    font.pixelSize: 8
                    Layout.preferredWidth: 12

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -4
                        onClicked: root.toggleExpanded(index)
                    }
                }

                Item {
                    visible: !delegateRoot.itemHasChildren
                    Layout.preferredWidth: 12
                }

                Text {
                    Layout.fillWidth: true
                    text: typeof display !== "undefined" ? display : ""
                    color: index === root.currentIndex ? paletteHighlightedText : paletteWindowText
                    font.pixelSize: 13
                    elide: Text.ElideRight
                }
            }

            MouseArea {
                id: delegateMouseArea
                anchors.fill: parent
                hoverEnabled: true
                z: -1

                onClicked: {
                    root.currentIndex = index;
                    root.itemClicked(index);
                }
                onDoubleClicked: {
                    root.itemDoubleClicked(index);
                }
            }
        }
    }
}
