// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property int volume: 100
    property bool isMuted: false
    property string volumeText: ""

    signal volumeSliderChanged(int value)
    signal toggleMute()
    signal resetVolume()

    width: volumeBtn.width
    height: parent.height

    // Volume button
    Rectangle {
        id: volumeBtn
        width: volLabel.implicitWidth + 12
        height: parent.height
        color: mouseArea.pressed ? Qt.darker(paletteButton, 1.2)
               : mouseArea.containsMouse ? Qt.darker(paletteButton, 1.05)
               : "transparent"

        // Left separator
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: 3
            anchors.bottomMargin: 3
            width: 1
            color: paletteMid
            opacity: 0.5
        }

        Text {
            id: volLabel
            anchors.centerIn: parent
            text: root.volumeText
            color: paletteButtonText
            font.pixelSize: 11
            font.bold: !root.isMuted
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            cursorShape: Qt.PointingHandCursor

            onClicked: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    volumePopup.visible = !volumePopup.visible;
                }
            }

            onWheel: function(wheel) {
                var delta = wheel.angleDelta.y / 120;
                var newVol = Math.max(0, Math.min(200, root.volume + delta * 5));
                root.volumeSliderChanged(newVol);
            }
        }

        // Context menu
        Rectangle {
            id: contextMenu
            visible: false
            width: 140
            height: contextCol.implicitHeight + 8
            x: 0
            y: -height - 2
            radius: 3
            border.width: 1
            border.color: paletteMid
            color: paletteBase
            z: 2000

            Column {
                id: contextCol
                anchors.fill: parent
                anchors.margins: 4

                Rectangle {
                    width: parent.width
                    height: 24
                    radius: 2
                    color: muteMouseArea.containsMouse ? paletteHighlight : "transparent"

                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        text: root.isMuted ? qsTr("Unmute") : qsTr("Mute")
                        color: muteMouseArea.containsMouse ? paletteHighlightedText
                                                           : paletteWindowText
                        font.pixelSize: 12
                    }

                    MouseArea {
                        id: muteMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.toggleMute();
                            contextMenu.visible = false;
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 24
                    radius: 2
                    color: resetMouseArea.containsMouse ? paletteHighlight : "transparent"

                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Reset Volume")
                        color: resetMouseArea.containsMouse ? paletteHighlightedText
                                                            : paletteWindowText
                        font.pixelSize: 12
                    }

                    MouseArea {
                        id: resetMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.resetVolume();
                            contextMenu.visible = false;
                        }
                    }
                }
            }
        }
    }

    // Volume slider popup
    Rectangle {
        id: volumePopup
        visible: false
        width: volumeBtn.width
        height: 36
        x: 0
        y: -height - 2
        radius: 3
        border.width: 1
        border.color: paletteMid
        color: paletteWindow
        z: 1000

        MouseArea {
            anchors.fill: parent
            // Prevent click-through
        }

        Item {
            anchors.fill: parent
            anchors.margins: 6

            // Track
            Rectangle {
                id: sliderTrack
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                height: 4
                radius: 2
                color: paletteMid

                Rectangle {
                    width: (root.volume / 200.0) * parent.width
                    height: parent.height
                    radius: 2
                    color: paletteHighlight
                }

                Rectangle {
                    id: sliderHandle
                    width: 14
                    height: 14
                    radius: 7
                    y: (sliderTrack.height - height) / 2
                    x: Math.max(0, Math.min(sliderTrack.width - width,
                       (root.volume / 200.0) * (sliderTrack.width - width)))
                    color: sliderDragArea.pressed ? Qt.darker(paletteHighlight, 1.2) : paletteHighlight
                    border.width: 1
                    border.color: Qt.darker(paletteHighlight, 1.3)

                    MouseArea {
                        id: sliderDragArea
                        anchors.fill: parent
                        anchors.margins: -4
                        drag.target: sliderHandle
                        drag.axis: Drag.XAxis
                        drag.minimumX: 0
                        drag.maximumX: sliderTrack.width - sliderHandle.width

                        onPositionChanged: {
                            if (pressed) {
                                var ratio = sliderHandle.x / (sliderTrack.width - sliderHandle.width);
                                var newVol = Math.round(ratio * 200);
                                root.volumeSliderChanged(Math.max(0, Math.min(200, newVol)));
                            }
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -6
                    z: -1
                    onClicked: function(mouse) {
                        var ratio = mouse.x / sliderTrack.width;
                        var newVol = Math.round(ratio * 200);
                        root.volumeSliderChanged(Math.max(0, Math.min(200, newVol)));
                    }
                }
            }
        }
    }
}
