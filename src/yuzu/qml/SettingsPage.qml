// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

// Generic settings page that displays a list of settings using SettingDelegate.
// The settingsModel context property must be set before loading this QML.
Rectangle {
    id: root
    color: paletteWindow

    Flickable {
        id: flickable
        anchors.fill: parent
        anchors.margins: 12
        contentHeight: settingsList.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: settingsList
            width: flickable.width
            spacing: 2

            Repeater {
                model: settingsModel

                SettingDelegate {
                    Layout.fillWidth: true
                    settingsModel: root.settingsModel !== undefined ? root.settingsModel : null
                }
            }
        }
    }

    // Scrollbar
    Rectangle {
        visible: flickable.contentHeight > flickable.height
        anchors.right: parent.right
        anchors.rightMargin: 2
        y: flickable.contentHeight > 0
           ? (flickable.contentY / flickable.contentHeight) * parent.height : 0
        width: 6
        height: flickable.contentHeight > 0
                ? Math.max(20, (flickable.height / flickable.contentHeight) * parent.height) : 0
        radius: 3
        color: "#80808080"
    }
}
