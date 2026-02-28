// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    property var interfaceList: typeof networkInterfaces !== "undefined" ? networkInterfaces : []
    property string selectedInterface: typeof currentInterface !== "undefined"
                                       ? currentInterface : ""

    signal interfaceChanged(string name)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: qsTr("Network")
            font.pixelSize: 13
            font.bold: true
            color: paletteWindowText
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: paletteMid
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                Layout.preferredWidth: 200
                text: qsTr("Network Interface:")
                font.pixelSize: 12
                color: paletteWindowText
            }

            Rectangle {
                Layout.preferredWidth: 250
                Layout.preferredHeight: 26
                radius: 3
                color: ifaceMouse.containsMouse ? Qt.lighter(paletteButton, 1.1) : paletteButton
                border.width: 1
                border.color: paletteMid
                opacity: typeof isLocked !== "undefined" && isLocked ? 0.5 : 1.0

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.right: ifaceArrow.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.selectedInterface.length > 0 ? root.selectedInterface
                                                            : qsTr("None")
                    font.pixelSize: 12
                    color: paletteButtonText
                    elide: Text.ElideRight
                }

                Text {
                    id: ifaceArrow
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: ifacePopup.visible ? "\u25B2" : "\u25BC"
                    font.pixelSize: 8
                    color: paletteButtonText
                }

                MouseArea {
                    id: ifaceMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: !(typeof isLocked !== "undefined" && isLocked)
                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                    onClicked: ifacePopup.visible = !ifacePopup.visible
                }

                Rectangle {
                    id: ifacePopup
                    visible: false
                    z: 200
                    y: parent.height + 2
                    width: parent.width
                    height: Math.min(ifaceListView.contentHeight + 4, 200)
                    color: paletteBase
                    border.width: 1
                    border.color: paletteMid
                    radius: 3

                    ListView {
                        id: ifaceListView
                        anchors.fill: parent
                        anchors.margins: 2
                        model: root.interfaceList
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds

                        delegate: Rectangle {
                            width: ifaceListView.width
                            height: 24
                            color: ifaceOptMouse.containsMouse ? paletteHighlight : "transparent"

                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 8
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData
                                font.pixelSize: 12
                                color: ifaceOptMouse.containsMouse ? paletteHighlightedText
                                                                   : paletteWindowText
                            }

                            MouseArea {
                                id: ifaceOptMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.selectedInterface = modelData
                                    root.interfaceChanged(modelData)
                                    ifacePopup.visible = false
                                }
                            }
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
