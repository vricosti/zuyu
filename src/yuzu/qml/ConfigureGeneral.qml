// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    signal resetDefaultsClicked()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Settings list
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentHeight: settingsColumn.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: settingsColumn
                width: parent.width
                spacing: 2

                // General settings section
                Text {
                    Layout.fillWidth: true
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

                Repeater {
                    model: settingsModel

                    SettingDelegate {
                        Layout.fillWidth: true
                        settingsModel: root.parent ? settingsModel : null
                    }
                }

                // Linux section (Unix only)
                Text {
                    Layout.fillWidth: true
                    Layout.topMargin: 16
                    visible: typeof showLinuxGroup !== "undefined" ? showLinuxGroup : false
                    text: qsTr("Linux")
                    font.pixelSize: 13
                    font.bold: true
                    color: paletteWindowText
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    visible: typeof showLinuxGroup !== "undefined" ? showLinuxGroup : false
                    color: paletteMid
                }

                Repeater {
                    model: typeof linuxSettingsModel !== "undefined" ? linuxSettingsModel : null

                    SettingDelegate {
                        Layout.fillWidth: true
                        settingsModel: typeof linuxSettingsModel !== "undefined"
                                       ? linuxSettingsModel : null
                    }
                }
            }
        }

        // Reset defaults button (global config only)
        Rectangle {
            visible: typeof showResetButton !== "undefined" ? showResetButton : false
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: resetText.implicitWidth + 24
            Layout.preferredHeight: 30
            radius: 4
            color: resetMouse.pressed ? Qt.darker(paletteButton, 1.2)
                   : resetMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                   : paletteButton
            border.width: 1
            border.color: paletteMid

            Text {
                id: resetText
                anchors.centerIn: parent
                text: qsTr("Reset All Settings")
                font.pixelSize: 12
                color: paletteButtonText
            }

            MouseArea {
                id: resetMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.resetDefaultsClicked()
            }
        }
    }
}
