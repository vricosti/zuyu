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

            // CPU settings
            Text {
                Layout.fillWidth: true
                text: qsTr("CPU")
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
                model: cpuModel

                SettingDelegate {
                    Layout.fillWidth: true
                    settingsModel: cpuModel
                }
            }

            // Unsafe settings (visible when cpu_accuracy is "Unsafe")
            Text {
                Layout.fillWidth: true
                Layout.topMargin: 16
                visible: typeof showUnsafe !== "undefined" ? showUnsafe : false
                text: qsTr("Unsafe CPU Optimizations")
                font.pixelSize: 13
                font.bold: true
                color: "#cc0000"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                visible: typeof showUnsafe !== "undefined" ? showUnsafe : false
                color: "#cc0000"
            }

            Text {
                Layout.fillWidth: true
                visible: typeof showUnsafe !== "undefined" ? showUnsafe : false
                text: qsTr("These settings reduce accuracy for speed. Use with caution.")
                font.pixelSize: 11
                color: paletteMid
                wrapMode: Text.WordWrap
            }

            Repeater {
                model: (typeof showUnsafe !== "undefined" && showUnsafe) ? unsafeModel : null

                SettingDelegate {
                    Layout.fillWidth: true
                    settingsModel: unsafeModel
                }
            }
        }
    }
}
