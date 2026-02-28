// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    signal regionOrLanguageChanged(int regionIndex, int languageIndex)

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

            // Core Settings
            Text {
                text: qsTr("Core")
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
                model: coreSettingsModel
                delegate: SettingDelegate {
                    width: content.width
                    settingsModel: coreSettingsModel
                }
            }

            // System Settings
            Text {
                Layout.topMargin: 12
                text: qsTr("System")
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
                model: systemSettingsModel
                delegate: SettingDelegate {
                    width: content.width
                    settingsModel: systemSettingsModel

                    // Detect region or language changes
                    Connections {
                        target: systemSettingsModel
                        function onDataChanged() {
                            systemConfigModel.onRegionOrLanguageChanged(
                                getSettingValue("region_index"),
                                getSettingValue("language_index"))
                        }
                    }
                }
            }

            // Locale warning
            Text {
                visible: systemConfigModel.localeWarning.length > 0
                text: systemConfigModel.localeWarning
                font.pixelSize: 11
                color: "#FF6B00"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            // RTC Settings
            Text {
                Layout.topMargin: 12
                text: qsTr("Real Time Clock")
                font.pixelSize: 13
                font.bold: true
                color: paletteWindowText
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: paletteMid
            }

            // RTC Enabled checkbox
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Rectangle {
                    width: 18
                    height: 18
                    radius: 2
                    border.width: 1
                    border.color: paletteMid
                    color: systemConfigModel.rtcEnabled ? paletteHighlight : paletteBase

                    Text {
                        anchors.centerIn: parent
                        text: "\u2713"
                        font.pixelSize: 12
                        color: paletteHighlightedText
                        visible: systemConfigModel.rtcEnabled
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: systemConfigModel.rtcEnabled = !systemConfigModel.rtcEnabled
                    }
                }

                Text {
                    text: qsTr("Custom RTC")
                    font.pixelSize: 12
                    color: paletteWindowText
                }
            }

            // RTC Offset
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                opacity: systemConfigModel.rtcEnabled ? 1.0 : 0.5

                Text {
                    Layout.preferredWidth: 200
                    text: qsTr("RTC Offset (seconds):")
                    font.pixelSize: 12
                    color: paletteWindowText
                }

                Rectangle {
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 26
                    color: paletteBase
                    border.width: 1
                    border.color: paletteMid
                    radius: 3

                    TextInput {
                        anchors.fill: parent
                        anchors.margins: 6
                        text: systemConfigModel.rtcOffset.toString()
                        font.pixelSize: 12
                        color: paletteWindowText
                        enabled: systemConfigModel.rtcEnabled
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        validator: IntValidator { bottom: -2147483648; top: 2147483647 }
                        onAccepted: systemConfigModel.rtcOffset = parseInt(text)
                        onEditingFinished: systemConfigModel.rtcOffset = parseInt(text)
                    }
                }
            }

            // RTC Date/Time display
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                opacity: systemConfigModel.rtcEnabled ? 1.0 : 0.5

                Text {
                    Layout.preferredWidth: 200
                    text: qsTr("Effective RTC:")
                    font.pixelSize: 12
                    color: paletteWindowText
                }

                Text {
                    text: {
                        var d = new Date(systemConfigModel.rtcDateTime * 1000)
                        return d.toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm:ss")
                    }
                    font.pixelSize: 12
                    color: paletteMid
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // Helper function to get a setting value by label
    function getSettingValue(label) {
        // This is used for locale validation
        // We'll get the values from the model
        return 0
    }
}
