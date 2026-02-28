// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    signal browseScreenshotPath()

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

            // General
            Text {
                text: qsTr("General")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            Text {
                text: qsTr("Interface language will change after restarting the application.")
                font.pixelSize: 11
                color: paletteMid
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            DropdownRow {
                label: qsTr("Language:")
                model: uiModel.languageList
                currentIndex: uiModel.languageIndex
                onSelected: function(idx) { uiModel.languageIndex = idx }
            }

            DropdownRow {
                label: qsTr("Theme:")
                model: uiModel.themeList
                currentIndex: uiModel.themeIndex
                onSelected: function(idx) { uiModel.themeIndex = idx }
            }

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

            CheckRow { text: qsTr("Show Compatibility List"); checked: uiModel.showCompat
                       onToggled: uiModel.showCompat = !uiModel.showCompat }
            CheckRow { text: qsTr("Show Add-Ons Column"); checked: uiModel.showAddOns
                       onToggled: uiModel.showAddOns = !uiModel.showAddOns }
            CheckRow { text: qsTr("Show Size Column"); checked: uiModel.showSize
                       onToggled: uiModel.showSize = !uiModel.showSize }
            CheckRow { text: qsTr("Show File Types Column"); checked: uiModel.showTypes
                       onToggled: uiModel.showTypes = !uiModel.showTypes }
            CheckRow { text: qsTr("Show Play Time Column"); checked: uiModel.showPlayTime
                       onToggled: uiModel.showPlayTime = !uiModel.showPlayTime }

            DropdownRow {
                label: qsTr("Game Icon Size:")
                model: uiModel.gameIconSizeList
                currentIndex: uiModel.gameIconSizeIndex
                onSelected: function(idx) { uiModel.gameIconSizeIndex = idx }
            }

            DropdownRow {
                label: qsTr("Folder Icon Size:")
                model: uiModel.folderIconSizeList
                currentIndex: uiModel.folderIconSizeIndex
                onSelected: function(idx) { uiModel.folderIconSizeIndex = idx }
            }

            DropdownRow {
                label: qsTr("Row 1 Text:")
                model: uiModel.row1TextList
                currentIndex: uiModel.row1TextIndex
                onSelected: function(idx) { uiModel.row1TextIndex = idx }
            }

            DropdownRow {
                label: qsTr("Row 2 Text:")
                model: uiModel.row2TextList
                currentIndex: uiModel.row2TextIndex
                onSelected: function(idx) { uiModel.row2TextIndex = idx }
            }

            // Screenshots
            Text {
                Layout.topMargin: 12
                text: qsTr("Screenshots")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            CheckRow {
                text: qsTr("Ask Where To Save Screenshots")
                checked: uiModel.enableScreenshotSaveAs
                onToggled: uiModel.enableScreenshotSaveAs = !uiModel.enableScreenshotSaveAs
            }

            // Screenshot path
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    Layout.preferredWidth: 140
                    text: qsTr("Screenshots Path:")
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

                    Text {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: uiModel.screenshotPath
                        font.pixelSize: 11
                        color: paletteWindowText
                        elide: Text.ElideMiddle
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 26
                    radius: 3
                    color: browseSsMouse.pressed ? Qt.darker(paletteButton, 1.2)
                           : browseSsMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                           : paletteButton
                    border.width: 1
                    border.color: paletteMid

                    Text {
                        anchors.centerIn: parent
                        text: "..."
                        font.pixelSize: 12
                        color: paletteButtonText
                    }

                    MouseArea {
                        id: browseSsMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.browseScreenshotPath()
                    }
                }
            }

            // Screenshot resolution
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    Layout.preferredWidth: 140
                    text: qsTr("Screenshot Resolution:")
                    font.pixelSize: 12
                    color: paletteWindowText
                }

                Text {
                    text: uiModel.screenshotWidthText
                    font.pixelSize: 12
                    color: paletteWindowText
                }

                DropdownRow {
                    label: ""
                    model: uiModel.screenshotHeightList
                    currentIndex: uiModel.screenshotHeightIndex
                    onSelected: function(idx) { uiModel.screenshotHeightIndex = idx }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // DropdownRow component
    component DropdownRow: RowLayout {
        Layout.fillWidth: true
        spacing: 8

        property string label: ""
        property var model: []
        property int currentIndex: 0
        signal selected(int idx)

        Text {
            visible: label.length > 0
            Layout.preferredWidth: 140
            text: label
            font.pixelSize: 12
            color: paletteWindowText
        }

        Rectangle {
            Layout.preferredWidth: 250
            Layout.preferredHeight: 26
            radius: 3
            color: dropMouse.containsMouse ? Qt.lighter(paletteButton, 1.1) : paletteButton
            border.width: 1
            border.color: paletteMid

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.right: dropArrow.left
                anchors.verticalCenter: parent.verticalCenter
                text: currentIndex >= 0 && currentIndex < model.length ? model[currentIndex] : ""
                font.pixelSize: 12
                color: paletteButtonText
                elide: Text.ElideRight
            }

            Text {
                id: dropArrow
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                text: dropPopup.visible ? "\u25B2" : "\u25BC"
                font.pixelSize: 8
                color: paletteButtonText
            }

            MouseArea {
                id: dropMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: dropPopup.visible = !dropPopup.visible
            }

            Rectangle {
                id: dropPopup
                visible: false
                z: 200
                y: parent.height + 2
                width: parent.width
                height: Math.min(dropListView.contentHeight + 4, 200)
                color: paletteBase
                border.width: 1
                border.color: paletteMid
                radius: 3

                ListView {
                    id: dropListView
                    anchors.fill: parent
                    anchors.margins: 2
                    model: parent.parent.parent.model
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    delegate: Rectangle {
                        width: dropListView.width
                        height: 24
                        color: optMouse.containsMouse ? paletteHighlight : "transparent"

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData
                            font.pixelSize: 12
                            color: optMouse.containsMouse ? paletteHighlightedText
                                                          : paletteWindowText
                        }

                        MouseArea {
                            id: optMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                selected(index)
                                dropPopup.visible = false
                            }
                        }
                    }
                }
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
