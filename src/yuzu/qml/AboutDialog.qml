// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    signal accepted()

    color: paletteWindow

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // Logo
            Image {
                source: typeof aboutLogoSource !== "undefined" ? aboutLogoSource : ""
                Layout.preferredWidth: 200
                Layout.preferredHeight: 200
                Layout.alignment: Qt.AlignTop
                fillMode: Image.PreserveAspectFit
            }

            // Info column
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                // Title
                Text {
                    text: "yuzu"
                    font.pixelSize: 28
                    font.bold: true
                    color: paletteWindowText
                }

                // Build info
                Text {
                    text: typeof aboutBuildInfo !== "undefined" ? aboutBuildInfo : ""
                    font.pixelSize: 13
                    color: paletteWindowText
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                // Description
                Text {
                    text: qsTr("yuzu is an experimental open-source emulator for the Nintendo Switch licensed under GPLv3.0+.\n\nThis software should not be used to play games you have not legally obtained.")
                    font.pixelSize: 12
                    color: paletteWindowText
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Item { Layout.fillHeight: true }

                // Links
                Text {
                    text: '<a href="https://yuzu-emu.org/" style="color: #039be5;">Website</a>'
                          + ' | <a href="https://github.com/yuzu-emu" style="color: #039be5;">Source Code</a>'
                          + ' | <a href="https://github.com/yuzu-emu/yuzu/graphs/contributors" style="color: #039be5;">Contributors</a>'
                          + ' | <a href="https://github.com/yuzu-emu/yuzu/blob/master/LICENSE.txt" style="color: #039be5;">License</a>'
                    font.pixelSize: 13
                    color: paletteWindowText
                    onLinkActivated: function(link) { Qt.openUrlExternally(link) }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.NoButton
                        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
                }

                // Liability
                Text {
                    text: qsTr('"Nintendo Switch" is a trademark of Nintendo. yuzu is not affiliated with Nintendo in any way.')
                    font.pixelSize: 9
                    color: paletteMid
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        // OK button row
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 32

            Rectangle {
                anchors.right: parent.right
                width: okText.implicitWidth + 32
                height: 28
                radius: 3
                color: okMouseArea.pressed ? Qt.darker(paletteButton, 1.2)
                       : okMouseArea.containsMouse ? Qt.darker(paletteButton, 1.1)
                       : paletteButton
                border.width: 1
                border.color: paletteMid

                Text {
                    id: okText
                    anchors.centerIn: parent
                    text: qsTr("OK")
                    color: paletteButtonText
                    font.pixelSize: 13
                }

                MouseArea {
                    id: okMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.accepted()
                }
            }
        }
    }
}
