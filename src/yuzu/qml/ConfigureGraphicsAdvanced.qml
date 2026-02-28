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
            spacing: 4

            Text {
                text: pageTitle
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
                delegate: SettingDelegate {
                    width: content.width
                    settingsModel: root.parent ? settingsModel : null
                    // Hide compute pipelines unless exposed
                    visible: {
                        if (model.settingId === computePipelinesSettingId) {
                            return showComputePipelines
                        }
                        return true
                    }
                    height: visible ? implicitHeight : 0
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
