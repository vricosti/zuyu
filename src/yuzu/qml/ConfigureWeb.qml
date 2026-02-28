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
            spacing: 8

            // yuzu Web Service
            Text {
                text: qsTr("yuzu Web Service")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            // Disable info
            Text {
                visible: !webModel.webServiceEnabled
                text: qsTr("Web Service is not available when a game is running.")
                font.pixelSize: 11
                color: paletteMid
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            // Web service content
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                opacity: webModel.webServiceEnabled ? 1.0 : 0.5
                enabled: webModel.webServiceEnabled

                Text {
                    text: qsTr("By providing your token, you agree to allow yuzu to collect "
                               + "additional usage data, which may include user-identifying "
                               + "information.")
                    font.pixelSize: 11
                    color: paletteWindowText
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                // Username
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        Layout.preferredWidth: 80
                        text: qsTr("Username:")
                        font.pixelSize: 12
                        color: paletteWindowText
                    }

                    Text {
                        text: webModel.username
                        font.pixelSize: 12
                        color: paletteWindowText
                    }
                }

                // Token
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        Layout.preferredWidth: 80
                        text: qsTr("Token:")
                        font.pixelSize: 12
                        color: paletteWindowText
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.maximumWidth: 300
                        Layout.preferredHeight: 26
                        color: paletteBase
                        border.width: 1
                        border.color: paletteMid
                        radius: 3

                        TextInput {
                            anchors.fill: parent
                            anchors.margins: 6
                            text: webModel.token
                            font.pixelSize: 12
                            color: paletteWindowText
                            echoMode: TextInput.Password
                            maximumLength: 80
                            clip: true
                            onTextChanged: webModel.token = text
                        }
                    }

                    // Verify status icon
                    Text {
                        font.pixelSize: 14
                        text: {
                            switch (webModel.verifyStatus) {
                            case "verifying": return "\u21BB"  // rotating arrow
                            case "verified": return "\u2713"   // checkmark
                            case "failed": return "\u2717"     // X mark
                            case "changed": return "\u2139"    // info
                            default: return ""
                            }
                        }
                        color: {
                            switch (webModel.verifyStatus) {
                            case "verified": return "#4CAF50"
                            case "failed": return "#F44336"
                            case "changed": return "#2196F3"
                            default: return paletteWindowText
                            }
                        }
                    }
                }

                // Verify button
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Item { Layout.preferredWidth: 80 }

                    Rectangle {
                        Layout.preferredWidth: verifyText.implicitWidth + 24
                        Layout.preferredHeight: 26
                        radius: 3
                        color: verifyMouse.pressed ? Qt.darker(paletteButton, 1.2)
                               : verifyMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                               : paletteButton
                        border.width: 1
                        border.color: paletteMid
                        opacity: webModel.verifyStatus === "verifying" ? 0.5 : 1.0

                        Text {
                            id: verifyText
                            anchors.centerIn: parent
                            text: webModel.verifyStatus === "verifying"
                                  ? qsTr("Verifying...") : qsTr("Verify")
                            font.pixelSize: 12
                            color: paletteButtonText
                        }

                        MouseArea {
                            id: verifyMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            enabled: webModel.verifyStatus !== "verifying"
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: webModel.verifyLogin()
                        }
                    }
                }
            }

            // Telemetry
            Text {
                Layout.topMargin: 12
                text: qsTr("Telemetry")
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
                text: qsTr("Share anonymous usage data with the yuzu team")
                checked: webModel.enableTelemetry
                onToggled: webModel.enableTelemetry = !webModel.enableTelemetry
            }

            // Telemetry ID + Regenerate
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Telemetry ID: %1").arg(webModel.telemetryId)
                    font.pixelSize: 12
                    color: paletteWindowText
                }

                Rectangle {
                    Layout.preferredWidth: regenText.implicitWidth + 24
                    Layout.preferredHeight: 26
                    radius: 3
                    color: regenMouse.pressed ? Qt.darker(paletteButton, 1.2)
                           : regenMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                           : paletteButton
                    border.width: 1
                    border.color: paletteMid

                    Text {
                        id: regenText
                        anchors.centerIn: parent
                        text: qsTr("Regenerate")
                        font.pixelSize: 12
                        color: paletteButtonText
                    }

                    MouseArea {
                        id: regenMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: webModel.regenerateTelemetryId()
                    }
                }
            }

            // Discord Presence
            ColumnLayout {
                visible: webModel.discordVisible
                Layout.fillWidth: true
                spacing: 8

                Text {
                    Layout.topMargin: 12
                    text: qsTr("Discord Presence")
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
                    text: qsTr("Show Current Game in your Discord Status")
                    checked: webModel.enableDiscordRpc
                    onToggled: webModel.enableDiscordRpc = !webModel.enableDiscordRpc
                }
            }

            Item { Layout.fillHeight: true }
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
