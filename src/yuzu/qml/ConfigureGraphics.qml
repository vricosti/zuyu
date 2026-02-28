// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    // Inline dropdown row component
    component DropdownRow: RowLayout {
        property string label: ""
        property var options: []
        property int currentIdx: 0
        property bool rowEnabled: true
        property bool rowVisible: true
        signal indexChanged(int idx)

        visible: rowVisible
        Layout.fillWidth: true
        Layout.preferredHeight: visible ? 32 : 0
        opacity: rowEnabled ? 1.0 : 0.5
        spacing: 8

        Text {
            text: parent.label
            color: paletteWindowText
            font.pixelSize: 12
            Layout.preferredWidth: 180
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            color: paletteBase
            border.color: paletteMid
            border.width: 1
            radius: 3

            Text {
                anchors.fill: parent
                anchors.margins: 4
                verticalAlignment: Text.AlignVCenter
                text: parent.parent.options.length > parent.parent.currentIdx
                      ? parent.parent.options[parent.parent.currentIdx] : ""
                color: paletteWindowText
                font.pixelSize: 12
                elide: Text.ElideRight
            }

            MouseArea {
                anchors.fill: parent
                enabled: parent.parent.rowEnabled
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    popup.visible = !popup.visible
                }
            }

            // Dropdown popup
            Rectangle {
                id: popup
                visible: false
                z: 100
                y: parent.height
                width: parent.width
                height: Math.min(popupList.contentHeight + 4, 200)
                color: paletteBase
                border.color: paletteMid
                border.width: 1
                radius: 3
                clip: true

                ListView {
                    id: popupList
                    anchors.fill: parent
                    anchors.margins: 2
                    model: parent.parent.parent.options
                    delegate: Rectangle {
                        width: popupList.width
                        height: 26
                        color: popupMa.containsMouse ? paletteHighlight : "transparent"

                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 4
                            verticalAlignment: Text.AlignVCenter
                            text: modelData
                            color: popupMa.containsMouse ? paletteHighlightedText
                                                         : paletteWindowText
                            font.pixelSize: 12
                        }

                        MouseArea {
                            id: popupMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                parent.parent.parent.parent.parent.parent.indexChanged(index)
                                popup.visible = false
                            }
                        }
                    }
                }
            }
        }
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        anchors.margins: 12
        contentHeight: content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: content
            width: flickable.width
            spacing: 4

            // -- API Settings group --
            Text {
                text: qsTr("API Settings")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            DropdownRow {
                label: qsTr("Graphics API")
                options: graphicsModel.apiOptions
                currentIdx: graphicsModel.apiIndex
                rowEnabled: graphicsModel.apiEnabled
                onIndexChanged: function(idx) { graphicsModel.apiIndex = idx }
            }

            DropdownRow {
                label: qsTr("Vulkan Device")
                options: graphicsModel.deviceOptions
                currentIdx: graphicsModel.deviceIndex
                rowVisible: graphicsModel.deviceVisible
                rowEnabled: !graphicsModel.runtimeLocked
                onIndexChanged: function(idx) { graphicsModel.deviceIndex = idx }
            }

            DropdownRow {
                label: qsTr("Shader Backend")
                options: graphicsModel.shaderOptions
                currentIdx: graphicsModel.shaderIndex
                rowVisible: graphicsModel.shaderVisible
                rowEnabled: !graphicsModel.runtimeLocked
                onIndexChanged: function(idx) { graphicsModel.shaderIndex = idx }
            }

            DropdownRow {
                label: qsTr("VSync Mode")
                options: graphicsModel.vsyncOptions
                currentIdx: graphicsModel.vsyncIndex
                rowEnabled: graphicsModel.vsyncOptions.length > 0
                onIndexChanged: function(idx) { graphicsModel.vsyncIndex = idx }
            }

            Item { height: 8 }

            // -- Graphics Settings group --
            Text {
                text: qsTr("Graphics Settings")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            Repeater {
                model: settingsModel
                delegate: SettingDelegate {
                    width: content.width
                    settingsModel: root.parent ? settingsModel : null
                }
            }

            // Background color picker
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                spacing: 8

                Text {
                    text: qsTr("Background Color:")
                    color: paletteWindowText
                    font.pixelSize: 12
                    Layout.preferredWidth: 180
                }

                Rectangle {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 24
                    color: graphicsModel.bgColorHex
                    border.color: paletteMid
                    border.width: 1
                    radius: 3

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: graphicsModel.pickBackgroundColor()
                    }
                }

                Item { Layout.fillWidth: true }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // Scrollbar
    Rectangle {
        visible: flickable.contentHeight > flickable.height
        anchors.right: parent.right
        anchors.rightMargin: 2
        y: flickable.contentHeight > 0
           ? (flickable.contentY / flickable.contentHeight) * parent.height : 0
        width: 6
        height: flickable.contentHeight > 0
                ? Math.max(20, (flickable.height / flickable.contentHeight) * parent.height) : 0
        radius: 3
        color: "#80808080"
    }
}
