// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    signal accepted()
    signal rejected()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Description
        Text {
            Layout.fillWidth: true
            text: qsTr("Please confirm these are the files you wish to install.")
            font.pixelSize: 13
            color: paletteWindowText
            wrapMode: Text.WordWrap
        }

        // Update description
        Text {
            Layout.fillWidth: true
            text: qsTr("Installing an Update or DLC will overwrite the previously installed one.")
            font.pixelSize: 13
            color: paletteWindowText
            wrapMode: Text.WordWrap
        }

        // File list
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: paletteBase
            border.width: 1
            border.color: paletteMid
            radius: 2

            ListView {
                id: fileListView
                anchors.fill: parent
                anchors.margins: 2
                model: installFileModel
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                delegate: Rectangle {
                    width: fileListView.width
                    height: 28
                    color: delegateMouse.containsMouse ? paletteHighlight : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 8

                        // Checkbox
                        Rectangle {
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            border.width: 1
                            border.color: paletteMid
                            radius: 2
                            color: model.checked ? paletteHighlight : paletteBase

                            Text {
                                anchors.centerIn: parent
                                text: "\u2713"
                                font.pixelSize: 12
                                color: paletteHighlightedText
                                visible: model.checked
                            }
                        }

                        // File name
                        Text {
                            Layout.fillWidth: true
                            text: model.displayName
                            font.pixelSize: 12
                            color: delegateMouse.containsMouse ? paletteHighlightedText
                                                               : paletteWindowText
                            elide: Text.ElideRight
                        }
                    }

                    MouseArea {
                        id: delegateMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: installFileModel.toggleItem(index)
                    }
                }

                // Scrollbar
                Rectangle {
                    visible: fileListView.contentHeight > fileListView.height
                    anchors.right: parent.right
                    anchors.rightMargin: 1
                    y: fileListView.contentHeight > 0
                       ? (fileListView.contentY / fileListView.contentHeight) * parent.height
                       : 0
                    width: 6
                    height: fileListView.contentHeight > 0
                            ? Math.max(20, (fileListView.height / fileListView.contentHeight)
                              * parent.height)
                            : 0
                    radius: 3
                    color: "#80808080"
                }
            }
        }

        // Buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Item { Layout.fillWidth: true }

            // Cancel button
            Rectangle {
                Layout.preferredWidth: cancelBtnText.implicitWidth + 32
                Layout.preferredHeight: 30
                radius: 4
                color: cancelBtnMouse.pressed ? Qt.darker(paletteButton, 1.2)
                       : cancelBtnMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                       : paletteButton
                border.width: 1
                border.color: paletteMid

                Text {
                    id: cancelBtnText
                    anchors.centerIn: parent
                    text: qsTr("Cancel")
                    font.pixelSize: 12
                    color: paletteButtonText
                }

                MouseArea {
                    id: cancelBtnMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.rejected()
                }
            }

            // Install button
            Rectangle {
                Layout.preferredWidth: installBtnText.implicitWidth + 32
                Layout.preferredHeight: 30
                radius: 4
                color: installBtnMouse.pressed ? Qt.darker(paletteHighlight, 1.2)
                       : installBtnMouse.containsMouse ? Qt.lighter(paletteHighlight, 1.1)
                       : paletteHighlight
                border.width: 1
                border.color: paletteMid

                Text {
                    id: installBtnText
                    anchors.centerIn: parent
                    text: qsTr("Install")
                    font.pixelSize: 12
                    color: paletteHighlightedText
                }

                MouseArea {
                    id: installBtnMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.accepted()
                }
            }
        }
    }
}
