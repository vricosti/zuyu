// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    Flickable {
        anchors.fill: parent
        anchors.margins: 12
        contentHeight: content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: content
            width: parent.width
            spacing: 2

            // Audio Engine section
            Text {
                Layout.fillWidth: true
                text: qsTr("Audio Output")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            // Sink selection
            AudioDropdown {
                label: qsTr("Output Engine:")
                model: audioModel.sinkList
                currentIndex: audioModel.sinkIndex
                onIndexChanged: function(idx) { audioModel.sinkIndex = idx }
            }

            // Output device
            AudioDropdown {
                label: qsTr("Output Device:")
                model: audioModel.outputDevices
                currentIndex: audioModel.outputDeviceIndex
                onIndexChanged: function(idx) { audioModel.outputDeviceIndex = idx }
            }

            // Input device
            AudioDropdown {
                label: qsTr("Input Device:")
                model: audioModel.inputDevices
                currentIndex: audioModel.inputDeviceIndex
                onIndexChanged: function(idx) { audioModel.inputDeviceIndex = idx }
            }

            // Other audio settings
            Text {
                Layout.fillWidth: true
                Layout.topMargin: 16
                text: qsTr("Audio Settings")
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

                SettingDelegate {
                    Layout.fillWidth: true
                    settingsModel: root.parent ? settingsModel : null
                }
            }
        }
    }

    // AudioDropdown inline component
    component AudioDropdown: RowLayout {
        id: dropdown
        Layout.fillWidth: true
        Layout.preferredHeight: 32
        spacing: 12

        property string label: ""
        property var model: []
        property int currentIndex: 0
        signal indexChanged(int idx)

        Text {
            Layout.preferredWidth: 200
            text: dropdown.label
            font.pixelSize: 12
            color: paletteWindowText
        }

        Rectangle {
            Layout.preferredWidth: 300
            Layout.preferredHeight: 26
            radius: 3
            color: ddMouse.containsMouse ? Qt.lighter(paletteButton, 1.1) : paletteButton
            border.width: 1
            border.color: paletteMid

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.right: ddArrow.left
                anchors.verticalCenter: parent.verticalCenter
                text: dropdown.model && dropdown.currentIndex >= 0
                      && dropdown.currentIndex < dropdown.model.length
                    ? dropdown.model[dropdown.currentIndex] : ""
                font.pixelSize: 12
                color: paletteButtonText
                elide: Text.ElideRight
            }

            Text {
                id: ddArrow
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                text: ddPopup.visible ? "\u25B2" : "\u25BC"
                font.pixelSize: 8
                color: paletteButtonText
            }

            MouseArea {
                id: ddMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: ddPopup.visible = !ddPopup.visible
            }

            Rectangle {
                id: ddPopup
                visible: false
                z: 200
                y: parent.height + 2
                width: parent.width
                height: Math.min(ddListView.contentHeight + 4, 200)
                color: paletteBase
                border.width: 1
                border.color: paletteMid
                radius: 3

                ListView {
                    id: ddListView
                    anchors.fill: parent
                    anchors.margins: 2
                    model: dropdown.model
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    delegate: Rectangle {
                        width: ddListView.width
                        height: 24
                        color: ddOptMouse.containsMouse ? paletteHighlight : "transparent"

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData
                            font.pixelSize: 12
                            color: ddOptMouse.containsMouse ? paletteHighlightedText
                                                            : paletteWindowText
                        }

                        MouseArea {
                            id: ddOptMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                dropdown.indexChanged(index)
                                ddPopup.visible = false
                            }
                        }
                    }
                }
            }
        }
    }
}
