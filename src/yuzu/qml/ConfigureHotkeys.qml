// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    signal configureHotkey(int groupIndex, int actionIndex, int column)
    signal restoreDefaults()
    signal clearAll()
    signal restoreDefault(int groupIndex, int actionIndex, int column)
    signal clearHotkey(int groupIndex, int actionIndex, int column)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                Layout.fillWidth: true
                text: qsTr("Double-click on a binding to change it.")
                font.pixelSize: 12
                color: paletteMid
            }

            Rectangle {
                Layout.preferredWidth: clearAllText.implicitWidth + 24
                Layout.preferredHeight: 28
                radius: 3
                color: clearAllMouse.pressed ? Qt.darker(paletteButton, 1.2)
                       : clearAllMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                       : paletteButton
                border.width: 1
                border.color: paletteMid

                Text {
                    id: clearAllText
                    anchors.centerIn: parent
                    text: qsTr("Clear All")
                    font.pixelSize: 12
                    color: paletteButtonText
                }

                MouseArea {
                    id: clearAllMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.clearAll()
                }
            }

            Rectangle {
                Layout.preferredWidth: restoreText.implicitWidth + 24
                Layout.preferredHeight: 28
                radius: 3
                color: restoreMouse.pressed ? Qt.darker(paletteButton, 1.2)
                       : restoreMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                       : paletteButton
                border.width: 1
                border.color: paletteMid

                Text {
                    id: restoreText
                    anchors.centerIn: parent
                    text: qsTr("Restore Defaults")
                    font.pixelSize: 12
                    color: paletteButtonText
                }

                MouseArea {
                    id: restoreMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.restoreDefaults()
                }
            }
        }

        // Column headers
        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            Text {
                Layout.fillWidth: true
                Layout.minimumWidth: 150
                text: qsTr("Action")
                font.pixelSize: 12
                font.bold: true
                color: paletteWindowText
                leftPadding: 8
            }
            Text {
                Layout.preferredWidth: 180
                text: qsTr("Hotkey")
                font.pixelSize: 12
                font.bold: true
                color: paletteWindowText
            }
            Text {
                Layout.preferredWidth: 180
                text: qsTr("Controller Hotkey")
                font.pixelSize: 12
                font.bold: true
                color: paletteWindowText
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: paletteMid
        }

        // Tree view content
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentHeight: treeColumn.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: treeColumn
                width: parent.width
                spacing: 0

                Repeater {
                    id: groupRepeater
                    model: hotkeyModel ? hotkeyModel.rowCount() : 0

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        property int groupIndex: index
                        property var groupItem: hotkeyModel ? hotkeyModel.item(index, 0) : null
                        property bool expanded: true

                        // Group header
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 28
                            color: groupHeaderMouse.containsMouse
                                   ? Qt.lighter(paletteWindow, 1.1) : paletteWindow

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 4
                                spacing: 4

                                Text {
                                    text: expanded ? "\u25BC" : "\u25B6"
                                    font.pixelSize: 10
                                    color: paletteWindowText
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: groupItem ? groupItem.text : ""
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: paletteWindowText
                                }
                            }

                            MouseArea {
                                id: groupHeaderMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: expanded = !expanded
                            }
                        }

                        // Action rows
                        Repeater {
                            model: expanded && groupItem ? groupItem.rowCount() : 0

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 28
                                color: actionRowMouse.containsMouse
                                       ? Qt.lighter(paletteBase, 1.05)
                                       : (index % 2 === 0 ? paletteBase
                                                          : Qt.darker(paletteBase, 1.03))

                                property int actionIdx: index
                                property var actionItem: groupItem.child(index, 0)
                                property var hotkeyItem: groupItem.child(index, 1)
                                property var ctrlItem: groupItem.child(index, 2)

                                RowLayout {
                                    anchors.fill: parent
                                    spacing: 0

                                    Text {
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 150
                                        text: actionItem ? actionItem.text : ""
                                        font.pixelSize: 12
                                        color: paletteWindowText
                                        leftPadding: 24
                                        elide: Text.ElideRight
                                    }

                                    // Hotkey cell
                                    Rectangle {
                                        Layout.preferredWidth: 180
                                        Layout.fillHeight: true
                                        color: "transparent"

                                        Text {
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: hotkeyItem ? hotkeyItem.text : ""
                                            font.pixelSize: 12
                                            color: paletteWindowText
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onDoubleClicked: root.configureHotkey(
                                                groupIndex, actionIdx, 1)
                                        }
                                    }

                                    // Controller hotkey cell
                                    Rectangle {
                                        Layout.preferredWidth: 180
                                        Layout.fillHeight: true
                                        color: "transparent"

                                        Text {
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: ctrlItem ? ctrlItem.text : ""
                                            font.pixelSize: 12
                                            color: paletteWindowText
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onDoubleClicked: root.configureHotkey(
                                                groupIndex, actionIdx, 2)
                                        }
                                    }
                                }

                                MouseArea {
                                    id: actionRowMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    acceptedButtons: Qt.RightButton
                                    onClicked: function(mouse) {
                                        contextMenu.groupIdx = groupIndex
                                        contextMenu.actionIdx = actionIdx
                                        contextMenu.x = mouse.x
                                        contextMenu.y = mouse.y
                                        contextMenu.visible = true
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Context menu
    Rectangle {
        id: contextMenu
        visible: false
        z: 300
        width: 160
        height: ctxCol.implicitHeight + 8
        color: paletteBase
        border.width: 1
        border.color: paletteMid
        radius: 3

        property int groupIdx: -1
        property int actionIdx: -1

        ColumnLayout {
            id: ctxCol
            anchors.fill: parent
            anchors.margins: 4
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 26
                color: restoreCtxMouse.containsMouse ? paletteHighlight : "transparent"

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Restore Default")
                    font.pixelSize: 12
                    color: restoreCtxMouse.containsMouse ? paletteHighlightedText
                                                        : paletteWindowText
                }

                MouseArea {
                    id: restoreCtxMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.restoreDefault(contextMenu.groupIdx, contextMenu.actionIdx, 1)
                        contextMenu.visible = false
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 26
                color: clearCtxMouse.containsMouse ? paletteHighlight : "transparent"

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Clear")
                    font.pixelSize: 12
                    color: clearCtxMouse.containsMouse ? paletteHighlightedText
                                                      : paletteWindowText
                }

                MouseArea {
                    id: clearCtxMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.clearHotkey(contextMenu.groupIdx, contextMenu.actionIdx, 1)
                        contextMenu.visible = false
                    }
                }
            }
        }
    }

    // Click outside context menu to close
    MouseArea {
        anchors.fill: parent
        visible: contextMenu.visible
        z: 299
        onClicked: contextMenu.visible = false
    }
}
