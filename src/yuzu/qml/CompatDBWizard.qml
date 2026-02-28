// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: paletteWindow

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Title bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: paletteBase

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                text: {
                    if (compatModel.currentPage === 7 && compatModel.isFinished)
                        return qsTr("Thank you for your submission!")
                    return qsTr("Report Game Compatibility")
                }
                font.pixelSize: 14
                font.bold: true
                color: paletteWindowText
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: paletteMid
        }

        // Page content area
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Page 0: Intro
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: compatModel.currentPage === 0
                spacing: 12

                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                    font.pixelSize: 13
                    color: paletteWindowText
                    text: qsTr("<p><span style=\"font-size:10pt;\">Should you choose to submit " +
                          "a test case to the yuzu Compatibility List, " +
                          "the following information will be collected and displayed on the " +
                          "site:</span></p>" +
                          "<ul>" +
                          "<li>Hardware Information (CPU / GPU / Operating System)</li>" +
                          "<li>Which version of yuzu you are running</li>" +
                          "<li>The connected yuzu account</li>" +
                          "</ul>")
                    onLinkActivated: function(link) { Qt.openUrlExternally(link) }
                }

                Item { Layout.fillHeight: true }
            }

            // Page 1: Game Boot
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: compatModel.currentPage === 1
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Does the game boot?")
                    font.pixelSize: 13
                    color: paletteWindowText
                    wrapMode: Text.WordWrap
                }

                Item { Layout.fillHeight: true }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("Yes   The game starts to output video or audio")
                    checked: compatModel.gameBootSelection === 0
                    onClicked: compatModel.gameBootSelection = 0
                }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("No    The game doesn't get past the \"Launching...\" screen")
                    checked: compatModel.gameBootSelection === 1
                    onClicked: compatModel.gameBootSelection = 1
                }
            }

            // Page 2: Gameplay
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: compatModel.currentPage === 2
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Does the game reach gameplay?")
                    font.pixelSize: 13
                    color: paletteWindowText
                    wrapMode: Text.WordWrap
                }

                Item { Layout.fillHeight: true }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("Yes   The game gets past the intro/menu and into gameplay")
                    checked: compatModel.gameplaySelection === 0
                    onClicked: compatModel.gameplaySelection = 0
                }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("No    The game crashes or freezes while loading or using the menu")
                    checked: compatModel.gameplaySelection === 1
                    onClicked: compatModel.gameplaySelection = 1
                }
            }

            // Page 3: Freeze
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: compatModel.currentPage === 3
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Does the game work without crashing, freezing or locking up " +
                          "during gameplay?")
                    font.pixelSize: 13
                    color: paletteWindowText
                    wrapMode: Text.WordWrap
                }

                Item { Layout.fillHeight: true }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("Yes   The game works without crashes")
                    checked: compatModel.freezeSelection === 0
                    onClicked: compatModel.freezeSelection = 0
                }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("No    The game crashes or freezes during gameplay")
                    checked: compatModel.freezeSelection === 1
                    onClicked: compatModel.freezeSelection = 1
                }
            }

            // Page 4: Completion
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: compatModel.currentPage === 4
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Is the game completely playable from start to finish?")
                    font.pixelSize: 13
                    color: paletteWindowText
                    wrapMode: Text.WordWrap
                }

                Item { Layout.fillHeight: true }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("Yes   The game can be finished without any workarounds")
                    checked: compatModel.completionSelection === 0
                    onClicked: compatModel.completionSelection = 0
                }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("No    The game can't progress past a certain area")
                    checked: compatModel.completionSelection === 1
                    onClicked: compatModel.completionSelection = 1
                }
            }

            // Page 5: Graphical
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: compatModel.currentPage === 5
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Does the game have any graphical glitches?")
                    font.pixelSize: 13
                    color: paletteWindowText
                    wrapMode: Text.WordWrap
                }

                Item { Layout.fillHeight: true }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("Major   The game has major graphical errors")
                    checked: compatModel.graphicalSelection === 0
                    onClicked: compatModel.graphicalSelection = 0
                }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("Minor   The game has minor graphical errors")
                    checked: compatModel.graphicalSelection === 1
                    onClicked: compatModel.graphicalSelection = 1
                }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("None     Everything is rendered as it looks on the Nintendo Switch")
                    checked: compatModel.graphicalSelection === 2
                    onClicked: compatModel.graphicalSelection = 2
                }
            }

            // Page 6: Audio
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: compatModel.currentPage === 6
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Does the game have any audio glitches / missing effects?")
                    font.pixelSize: 13
                    color: paletteWindowText
                    wrapMode: Text.WordWrap
                }

                Item { Layout.fillHeight: true }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("Major   The game has major audio errors")
                    checked: compatModel.audioSelection === 0
                    onClicked: compatModel.audioSelection = 0
                }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("Minor   The game has minor audio errors")
                    checked: compatModel.audioSelection === 1
                    onClicked: compatModel.audioSelection = 1
                }

                RadioOption {
                    Layout.fillWidth: true
                    text: qsTr("None     Audio is played perfectly")
                    checked: compatModel.audioSelection === 2
                    onClicked: compatModel.audioSelection = 2
                }
            }

            // Page 7: Final / Thank You
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: compatModel.currentPage === 7
                spacing: 12

                Text {
                    Layout.fillWidth: true
                    visible: compatModel.isFinished
                    text: qsTr("Thank you for your submission!")
                    font.pixelSize: 16
                    font.bold: true
                    color: paletteWindowText
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    Layout.fillWidth: true
                    visible: compatModel.isSubmitting
                    text: qsTr("Submitting compatibility report...")
                    font.pixelSize: 13
                    color: paletteWindowText
                    horizontalAlignment: Text.AlignHCenter
                }

                Item { Layout.fillHeight: true }
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: paletteMid
        }

        // Button bar
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            spacing: 8

            Item { Layout.fillWidth: true }

            // Cancel button
            Rectangle {
                visible: compatModel.cancelVisible
                Layout.preferredWidth: cancelText.implicitWidth + 32
                Layout.preferredHeight: 30
                radius: 4
                color: cancelMouse.pressed ? Qt.darker(paletteButton, 1.2)
                       : cancelMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                       : paletteButton
                border.width: 1
                border.color: paletteMid

                Text {
                    id: cancelText
                    anchors.centerIn: parent
                    text: qsTr("Cancel")
                    font.pixelSize: 12
                    color: paletteButtonText
                }

                MouseArea {
                    id: cancelMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: compatModel.cancelRequested()
                }
            }

            // Back button
            Rectangle {
                visible: compatModel.backVisible
                Layout.preferredWidth: backText.implicitWidth + 32
                Layout.preferredHeight: 30
                radius: 4
                color: backMouse.pressed ? Qt.darker(paletteButton, 1.2)
                       : backMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                       : paletteButton
                border.width: 1
                border.color: paletteMid

                Text {
                    id: backText
                    anchors.centerIn: parent
                    text: qsTr("Back")
                    font.pixelSize: 12
                    color: paletteButtonText
                }

                MouseArea {
                    id: backMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: compatModel.previousPage()
                }
            }

            // Next button
            Rectangle {
                Layout.preferredWidth: nextText.implicitWidth + 32
                Layout.preferredHeight: 30
                radius: 4
                opacity: compatModel.nextEnabled ? 1.0 : 0.5
                color: nextMouse.pressed ? Qt.darker(paletteHighlight, 1.2)
                       : nextMouse.containsMouse ? Qt.lighter(paletteHighlight, 1.1)
                       : paletteHighlight
                border.width: 1
                border.color: paletteMid

                Text {
                    id: nextText
                    anchors.centerIn: parent
                    text: compatModel.nextButtonText
                    font.pixelSize: 12
                    color: paletteHighlightedText
                }

                MouseArea {
                    id: nextMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: compatModel.nextEnabled ? Qt.PointingHandCursor
                                                        : Qt.ArrowCursor
                    onClicked: {
                        if (compatModel.nextEnabled) {
                            compatModel.nextPage()
                        }
                    }
                }
            }
        }
    }

    // RadioOption inline component
    component RadioOption: Rectangle {
        id: radioOpt
        property string text: ""
        property bool checked: false
        signal clicked()

        height: 32
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            spacing: 8

            // Radio circle
            Rectangle {
                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
                radius: 9
                color: "transparent"
                border.width: 2
                border.color: radioOpt.checked ? paletteHighlight : paletteMid

                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: 10
                    radius: 5
                    color: paletteHighlight
                    visible: radioOpt.checked
                }
            }

            Text {
                Layout.fillWidth: true
                text: radioOpt.text
                font.pixelSize: 12
                color: paletteWindowText
                wrapMode: Text.WordWrap
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: radioOpt.clicked()
        }
    }
}
