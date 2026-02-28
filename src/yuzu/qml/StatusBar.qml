// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 24
    color: paletteWindow

    // Separator line at top
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: paletteMid
    }

    RowLayout {
        anchors.fill: parent
        anchors.topMargin: 1
        spacing: 0

        // Message label (left-aligned, takes remaining space)
        Text {
            Layout.fillWidth: true
            Layout.leftMargin: 4
            text: statusBarModel.messageText
            color: paletteWindowText
            font.pixelSize: 12
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        // TAS label
        Text {
            visible: statusBarModel.tasText.length > 0
            text: statusBarModel.tasText
            color: paletteWindowText
            font.pixelSize: 12
            Layout.margins: 2
            verticalAlignment: Text.AlignVCenter
        }

        StatusBarSeparator { visible: statusBarModel.tasText.length > 0 }

        // Shader building label
        Text {
            visible: statusBarModel.shaderBuildingVisible
            text: statusBarModel.shaderBuildingText
            color: paletteWindowText
            font.pixelSize: 12
            Layout.margins: 2
            verticalAlignment: Text.AlignVCenter
        }

        StatusBarSeparator { visible: statusBarModel.shaderBuildingVisible }

        // Resolution scale
        Text {
            visible: statusBarModel.perfVisible
            text: statusBarModel.resScaleText
            color: paletteWindowText
            font.pixelSize: 12
            Layout.margins: 2
            verticalAlignment: Text.AlignVCenter
        }

        StatusBarSeparator { visible: statusBarModel.perfVisible }

        // Emulation speed
        Text {
            visible: statusBarModel.perfVisible && statusBarModel.emuSpeedVisible
            text: statusBarModel.emuSpeedText
            color: paletteWindowText
            font.pixelSize: 12
            Layout.margins: 2
            verticalAlignment: Text.AlignVCenter
        }

        StatusBarSeparator {
            visible: statusBarModel.perfVisible && statusBarModel.emuSpeedVisible
        }

        // Game FPS
        Text {
            visible: statusBarModel.perfVisible
            text: statusBarModel.gameFpsText
            color: paletteWindowText
            font.pixelSize: 12
            Layout.margins: 2
            verticalAlignment: Text.AlignVCenter
        }

        StatusBarSeparator { visible: statusBarModel.perfVisible }

        // Frame time
        Text {
            visible: statusBarModel.perfVisible
            text: statusBarModel.emuFrametimeText
            color: paletteWindowText
            font.pixelSize: 12
            Layout.margins: 2
            verticalAlignment: Text.AlignVCenter
        }

        StatusBarSeparator { visible: statusBarModel.perfVisible }

        // Firmware label
        Text {
            visible: statusBarModel.firmwareVisible
            text: statusBarModel.firmwareText
            color: paletteWindowText
            font.pixelSize: 12
            Layout.margins: 2
            verticalAlignment: Text.AlignVCenter

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                property bool showTip: false

                onContainsMouseChanged: showTip = containsMouse

                Rectangle {
                    visible: parent.showTip && statusBarModel.firmwareTooltip.length > 0
                    x: parent.mouseX + 8
                    y: -height - 4
                    width: fwTooltipText.width + 12
                    height: fwTooltipText.height + 8
                    radius: 3
                    color: "#333"
                    z: 100

                    Text {
                        id: fwTooltipText
                        anchors.centerIn: parent
                        text: statusBarModel.firmwareTooltip
                        color: "#eee"
                        font.pixelSize: 11
                    }
                }
            }
        }

        StatusBarSeparator { visible: statusBarModel.firmwareVisible }

        // Renderer API button
        StatusBarButton {
            text: statusBarModel.rendererText
            checked: statusBarModel.rendererChecked
            enabled: statusBarModel.rendererEnabled
            onClicked: statusBarModel.rendererClicked()
        }

        // GPU Accuracy button
        StatusBarButton {
            text: statusBarModel.gpuAccuracyText
            checked: statusBarModel.gpuAccuracyChecked
            onClicked: statusBarModel.gpuAccuracyClicked()
        }

        // Dock mode button
        StatusBarButton {
            text: statusBarModel.dockedText
            checked: statusBarModel.dockedChecked
            onClicked: statusBarModel.dockedClicked()
        }

        // Filter button
        StatusBarButton {
            text: statusBarModel.filterText
            checked: true
            onClicked: statusBarModel.filterClicked()
        }

        // AA button
        StatusBarButton {
            text: statusBarModel.aaText
            checked: true
            onClicked: statusBarModel.aaClicked()
        }

        // Volume button
        VolumeControl {
            volume: statusBarModel.volume
            isMuted: statusBarModel.isMuted
            volumeText: statusBarModel.volumeText
            onVolumeSliderChanged: function(val) {
                statusBarModel.volume = val;
            }
            onToggleMute: statusBarModel.muteRequested()
            onResetVolume: statusBarModel.resetVolumeRequested()
        }
    }
}
