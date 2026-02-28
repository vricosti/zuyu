// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    signal browseDirectory(string target)
    signal resetCacheClicked()

    Flickable {
        anchors.fill: parent
        anchors.margins: 12
        contentHeight: content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: content
            width: parent.width
            spacing: 8

            // Storage Directories
            Text {
                text: qsTr("Storage Directories")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            PathRow {
                label: qsTr("NAND Directory:")
                path: fsModel.nandDir
                target: "nand"
            }

            PathRow {
                label: qsTr("SD Card Directory:")
                path: fsModel.sdmcDir
                target: "sdmc"
            }

            PathRow {
                label: qsTr("Dump Directory:")
                path: fsModel.dumpDir
                target: "dump"
            }

            PathRow {
                label: qsTr("Mod Load Directory:")
                path: fsModel.loadDir
                target: "load"
            }

            // Gamecard
            Text {
                Layout.topMargin: 12
                text: qsTr("Gamecard")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            // Gamecard inserted
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Rectangle {
                    width: 18
                    height: 18
                    radius: 2
                    border.width: 1
                    border.color: paletteMid
                    color: fsModel.gamecardInserted ? paletteHighlight : paletteBase

                    Text {
                        anchors.centerIn: parent
                        text: "\u2713"
                        font.pixelSize: 12
                        color: paletteHighlightedText
                        visible: fsModel.gamecardInserted
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: fsModel.gamecardInserted = !fsModel.gamecardInserted
                    }
                }

                Text {
                    text: qsTr("Gamecard Inserted")
                    font.pixelSize: 12
                    color: paletteWindowText
                }
            }

            // Gamecard current game
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                opacity: fsModel.gamecardInserted ? 1.0 : 0.5

                Rectangle {
                    width: 18
                    height: 18
                    radius: 2
                    border.width: 1
                    border.color: paletteMid
                    color: fsModel.gamecardCurrentGame ? paletteHighlight : paletteBase

                    Text {
                        anchors.centerIn: parent
                        text: "\u2713"
                        font.pixelSize: 12
                        color: paletteHighlightedText
                        visible: fsModel.gamecardCurrentGame
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: fsModel.gamecardInserted
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                        onClicked: fsModel.gamecardCurrentGame = !fsModel.gamecardCurrentGame
                    }
                }

                Text {
                    text: qsTr("Current Game")
                    font.pixelSize: 12
                    color: paletteWindowText
                }
            }

            // Gamecard path
            PathRow {
                label: qsTr("Gamecard Path:")
                path: fsModel.gamecardPath
                target: "gamecard"
                enabled: fsModel.gamecardInserted && !fsModel.gamecardCurrentGame
            }

            // Dump options
            Text {
                Layout.topMargin: 12
                text: qsTr("Dump Options")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            CheckRow { text: qsTr("Dump ExeFS"); checked: fsModel.dumpExefs
                       onToggled: fsModel.dumpExefs = !fsModel.dumpExefs }
            CheckRow { text: qsTr("Dump NSOs"); checked: fsModel.dumpNso
                       onToggled: fsModel.dumpNso = !fsModel.dumpNso }

            // Game List
            Text {
                Layout.topMargin: 12
                text: qsTr("Game List")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            CheckRow { text: qsTr("Cache Game List"); checked: fsModel.cacheGameList
                       onToggled: fsModel.cacheGameList = !fsModel.cacheGameList }

            Rectangle {
                Layout.preferredWidth: resetCacheText.implicitWidth + 24
                Layout.preferredHeight: 30
                radius: 4
                color: resetCacheMouse.pressed ? Qt.darker(paletteButton, 1.2)
                       : resetCacheMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                       : paletteButton
                border.width: 1
                border.color: paletteMid

                Text {
                    id: resetCacheText
                    anchors.centerIn: parent
                    text: qsTr("Reset Metadata Cache")
                    font.pixelSize: 12
                    color: paletteButtonText
                }

                MouseArea {
                    id: resetCacheMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.resetCacheClicked()
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // PathRow component
    component PathRow: RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: 32
        spacing: 8

        property string label: ""
        property string path: ""
        property string target: ""

        Text {
            Layout.preferredWidth: 140
            text: label
            font.pixelSize: 12
            color: paletteWindowText
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 26
            color: paletteBase
            border.width: 1
            border.color: paletteMid
            radius: 3
            opacity: parent.enabled ? 1.0 : 0.5

            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 6
                anchors.verticalCenter: parent.verticalCenter
                text: path
                font.pixelSize: 11
                color: paletteWindowText
                elide: Text.ElideMiddle
            }
        }

        Rectangle {
            Layout.preferredWidth: 30
            Layout.preferredHeight: 26
            radius: 3
            color: browseMouse.pressed ? Qt.darker(paletteButton, 1.2)
                   : browseMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                   : paletteButton
            border.width: 1
            border.color: paletteMid
            opacity: parent.enabled ? 1.0 : 0.5

            Text {
                anchors.centerIn: parent
                text: "..."
                font.pixelSize: 12
                color: paletteButtonText
            }

            MouseArea {
                id: browseMouse
                anchors.fill: parent
                hoverEnabled: true
                enabled: parent.parent.enabled
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                onClicked: root.browseDirectory(target)
            }
        }
    }

    // CheckRow component
    component CheckRow: RowLayout {
        Layout.fillWidth: true
        spacing: 8

        property string text: ""
        property bool checked: false
        signal toggled()

        Rectangle {
            width: 18
            height: 18
            radius: 2
            border.width: 1
            border.color: paletteMid
            color: checked ? paletteHighlight : paletteBase

            Text {
                anchors.centerIn: parent
                text: "\u2713"
                font.pixelSize: 12
                color: paletteHighlightedText
                visible: checked
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: parent.parent.toggled()
            }
        }

        Text {
            text: parent.text
            font.pixelSize: 12
            color: paletteWindowText
        }
    }
}
