// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    required property int index
    required property string name
    required property string filePath
    required property var programId
    required property string fileType
    required property string sizeText
    required property string compatibilityText
    required property string compatibilityColor
    required property string addOns
    required property string playTimeText
    required property string iconSource
    required property bool isFavorite

    // Column visibility from context properties
    property bool showCompat: typeof gameListShowCompat !== "undefined" ? gameListShowCompat : true
    property bool showAddOns: typeof gameListShowAddOns !== "undefined" ? gameListShowAddOns : true
    property bool showSize: typeof gameListShowSize !== "undefined" ? gameListShowSize : true
    property bool showFileType: typeof gameListShowFileType !== "undefined" ? gameListShowFileType : true
    property bool showPlayTime: typeof gameListShowPlayTime !== "undefined" ? gameListShowPlayTime : true

    signal gameDoubleClicked(string path, var titleId)
    signal contextMenuRequested(int index, real globalX, real globalY)

    height: Math.max(iconSize + 8, 48)
    color: mouseArea.containsMouse ? Qt.darker(paletteBase, 1.1)
                                   : (index % 2 === 0 ? paletteBase : paletteAlternateBase)

    // Icon size from context property or default
    property int iconSize: typeof gameListIconSize !== "undefined" ? gameListIconSize : 64

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 12

        // Favorite indicator
        Text {
            text: root.isFavorite ? "\u2605" : ""
            font.pixelSize: 16
            color: "#f2d624"
            Layout.preferredWidth: 20
        }

        // Game icon
        Image {
            source: root.iconSource
            sourceSize.width: root.iconSize
            sourceSize.height: root.iconSize
            Layout.preferredWidth: root.iconSize
            Layout.preferredHeight: root.iconSize
            fillMode: Image.PreserveAspectFit
            cache: true
        }

        // Game name
        Text {
            text: root.name
            color: paletteWindowText
            elide: Text.ElideRight
            Layout.fillWidth: true
            font.pixelSize: 14
        }

        // Compatibility indicator
        Rectangle {
            visible: root.showCompat && root.compatibilityColor.length > 0
            width: 16
            height: 16
            radius: 8
            color: root.compatibilityColor
            Layout.alignment: Qt.AlignVCenter

            // Tooltip on hover
            Rectangle {
                id: compatTooltip
                visible: compatMouseArea.containsMouse && root.compatibilityText.length > 0
                parent: root
                x: compatMouseArea.mapToItem(root, compatMouseArea.mouseX, 0).x + 8
                y: 0
                width: tooltipText.width + 12
                height: tooltipText.height + 8
                color: "#333"
                radius: 3
                z: 100

                Text {
                    id: tooltipText
                    anchors.centerIn: parent
                    text: root.compatibilityText
                    color: "#eee"
                    font.pixelSize: 12
                }
            }

            MouseArea {
                id: compatMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        // Add-ons
        Text {
            visible: root.showAddOns
            text: root.addOns
            color: paletteWindowText
            Layout.preferredWidth: 120
            elide: Text.ElideRight
            font.pixelSize: 12
        }

        // File type
        Text {
            visible: root.showFileType
            text: root.fileType
            color: paletteWindowText
            Layout.preferredWidth: 60
            font.pixelSize: 12
        }

        // Size
        Text {
            visible: root.showSize
            text: root.sizeText
            color: paletteWindowText
            Layout.preferredWidth: 80
            horizontalAlignment: Text.AlignRight
            font.pixelSize: 12
        }

        // Play time
        Text {
            visible: root.showPlayTime
            text: root.playTimeText
            color: paletteWindowText
            Layout.preferredWidth: 80
            horizontalAlignment: Text.AlignRight
            font.pixelSize: 12
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true

        onDoubleClicked: function(mouse) {
            if (mouse.button === Qt.LeftButton) {
                root.gameDoubleClicked(root.filePath, root.programId);
            }
        }

        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                var globalPos = mapToGlobal(mouse.x, mouse.y);
                root.contextMenuRequested(root.index, globalPos.x, globalPos.y);
            }
        }
    }
}
