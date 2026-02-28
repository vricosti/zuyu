// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property int value: 0
    property int from: 0
    property int to: 99
    property int stepSize: 1
    property string suffix: ""
    property bool enabled: true

    signal valueModified()

    implicitWidth: 120
    implicitHeight: 28

    function increment() {
        if (value + stepSize <= to) {
            value += stepSize;
            valueModified();
        }
    }

    function decrement() {
        if (value - stepSize >= from) {
            value -= stepSize;
            valueModified();
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Decrement button
        Rectangle {
            Layout.preferredWidth: 24
            Layout.fillHeight: true
            radius: 3
            color: !root.enabled ? Qt.lighter(paletteButton, 1.05)
                   : decMouseArea.pressed ? Qt.darker(paletteButton, 1.2)
                   : decMouseArea.containsMouse ? Qt.darker(paletteButton, 1.1)
                   : paletteButton
            border.width: 1
            border.color: paletteMid
            opacity: root.enabled ? 1.0 : 0.6

            Text {
                anchors.centerIn: parent
                text: "\u2212"
                color: paletteButtonText
                font.pixelSize: 14
            }

            MouseArea {
                id: decMouseArea
                anchors.fill: parent
                hoverEnabled: root.enabled
                onClicked: if (root.enabled) root.decrement()

                // Auto-repeat on press and hold
                onPressAndHold: repeatTimerDec.start()
                onReleased: repeatTimerDec.stop()
            }

            Timer {
                id: repeatTimerDec
                interval: 100
                repeat: true
                onTriggered: if (root.enabled) root.decrement()
            }
        }

        // Value display/input
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: root.enabled ? paletteBase : Qt.lighter(paletteButton, 1.05)
            border.width: 1
            border.color: input.activeFocus ? paletteHighlight : paletteMid
            opacity: root.enabled ? 1.0 : 0.6

            TextInput {
                id: input
                anchors.fill: parent
                anchors.margins: 2
                horizontalAlignment: TextInput.AlignHCenter
                verticalAlignment: TextInput.AlignVCenter
                text: root.value + root.suffix
                color: paletteWindowText
                font.pixelSize: 13
                selectByMouse: true
                enabled: root.enabled
                validator: IntValidator { bottom: root.from; top: root.to }

                onEditingFinished: {
                    var stripped = text.replace(root.suffix, "");
                    var num = parseInt(stripped);
                    if (!isNaN(num)) {
                        root.value = Math.max(root.from, Math.min(root.to, num));
                        root.valueModified();
                    }
                    text = Qt.binding(function() { return root.value + root.suffix; });
                }
            }
        }

        // Increment button
        Rectangle {
            Layout.preferredWidth: 24
            Layout.fillHeight: true
            radius: 3
            color: !root.enabled ? Qt.lighter(paletteButton, 1.05)
                   : incMouseArea.pressed ? Qt.darker(paletteButton, 1.2)
                   : incMouseArea.containsMouse ? Qt.darker(paletteButton, 1.1)
                   : paletteButton
            border.width: 1
            border.color: paletteMid
            opacity: root.enabled ? 1.0 : 0.6

            Text {
                anchors.centerIn: parent
                text: "+"
                color: paletteButtonText
                font.pixelSize: 14
            }

            MouseArea {
                id: incMouseArea
                anchors.fill: parent
                hoverEnabled: root.enabled
                onClicked: if (root.enabled) root.increment()

                onPressAndHold: repeatTimerInc.start()
                onReleased: repeatTimerInc.stop()
            }

            Timer {
                id: repeatTimerInc
                interval: 100
                repeat: true
                onTriggered: if (root.enabled) root.increment()
            }
        }
    }
}
