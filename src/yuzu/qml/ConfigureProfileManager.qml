// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    signal selectUser(int index)
    signal addUser()
    signal renameUser()
    signal deleteUser()
    signal setUserImage()

    property int selectedIndex: -1
    property string currentUsername: ""
    property int userCount: 0
    property bool actionsEnabled: true

    function getSelectedIndex() {
        return selectedIndex
    }

    function updateCurrentUser(username, count, canEdit) {
        currentUsername = username
        userCount = count
        actionsEnabled = canEdit
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Profile Manager header
        Text {
            text: qsTr("Profile Manager")
            font.pixelSize: 13
            font.bold: true
            color: paletteWindowText
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: paletteMid
        }

        // Current user display
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: qsTr("Current User:")
                font.pixelSize: 12
                color: paletteWindowText
            }

            Text {
                text: currentUsername
                font.pixelSize: 12
                font.bold: true
                color: paletteWindowText
            }
        }

        // User list
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: paletteBase
            border.width: 1
            border.color: paletteMid
            radius: 3

            ListView {
                id: userListView
                anchors.fill: parent
                anchors.margins: 2
                model: profileModel
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                currentIndex: selectedIndex

                delegate: Rectangle {
                    width: userListView.width
                    height: 72
                    color: {
                        if (index === selectedIndex) return paletteHighlight
                        if (index % 2 === 0) return paletteBase
                        return Qt.darker(paletteBase, 1.03)
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 4
                        spacing: 8

                        // Icon placeholder
                        Rectangle {
                            Layout.preferredWidth: 64
                            Layout.preferredHeight: 64
                            color: paletteMid
                            radius: 4

                            // Icon from model decoration role
                            Image {
                                anchors.fill: parent
                                // QStandardItemModel icons accessible via decoration
                                source: ""
                                visible: false
                            }

                            Text {
                                anchors.centerIn: parent
                                text: "\uD83D\uDC64" // person silhouette
                                font.pixelSize: 32
                                color: paletteBase
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: display ? display : ""
                            font.pixelSize: 12
                            color: index === selectedIndex ? paletteHighlightedText
                                                          : paletteWindowText
                            wrapMode: Text.WordWrap
                            maximumLineCount: 3
                            elide: Text.ElideRight
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            selectedIndex = index
                            root.selectUser(index)
                        }
                    }
                }
            }
        }

        // Action buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            ActionButton {
                text: qsTr("Set Image")
                enabled: selectedIndex >= 0 && actionsEnabled
                onClicked: root.setUserImage()
            }

            Item { Layout.fillWidth: true }

            ActionButton {
                text: qsTr("Add")
                enabled: actionsEnabled && userCount < maxUsers
                onClicked: root.addUser()
            }

            ActionButton {
                text: qsTr("Rename")
                enabled: selectedIndex >= 0 && actionsEnabled
                onClicked: root.renameUser()
            }

            ActionButton {
                text: qsTr("Remove")
                enabled: selectedIndex >= 0 && actionsEnabled && userCount >= 2
                onClicked: root.deleteUser()
            }
        }

        // Info text
        Text {
            visible: !actionsEnabled
            text: qsTr("Profile management is available only when the game is not running.")
            font.pixelSize: 11
            color: paletteMid
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    component ActionButton: Rectangle {
        property string text: ""
        property bool enabled: true
        signal clicked()

        Layout.preferredWidth: btnText.implicitWidth + 24
        Layout.preferredHeight: 28
        radius: 3
        color: !enabled ? Qt.darker(paletteButton, 1.1)
               : btnMouse.pressed ? Qt.darker(paletteButton, 1.2)
               : btnMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
               : paletteButton
        border.width: 1
        border.color: paletteMid
        opacity: enabled ? 1.0 : 0.5

        Text {
            id: btnText
            anchors.centerIn: parent
            text: parent.text
            font.pixelSize: 12
            color: paletteButtonText
        }

        MouseArea {
            id: btnMouse
            anchors.fill: parent
            hoverEnabled: true
            enabled: parent.enabled
            cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
            onClicked: parent.clicked()
        }
    }
}
