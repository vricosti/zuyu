// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    color: "#000000"

    // Fade container
    Item {
        id: fadeParent
        anchors.fill: parent
        opacity: loadingModel.fadeOpacity

        Behavior on opacity {
            NumberAnimation {
                duration: 500
                easing.type: Easing.OutBack
            }
        }

        // Logo (top-left corner)
        Image {
            visible: loadingModel.logoAvailable
            source: loadingModel.logoAvailable ? "image://loadingscreen/logo" : ""
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 30
            sourceSize.width: 200
            sourceSize.height: 200
            fillMode: Image.PreserveAspectFit
            cache: false
        }

        // Center content
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 15
            width: Math.min(parent.width * 0.7, 500)

            // Stage text
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: loadingModel.stageText
                color: "#ffffff"
                font.pixelSize: 20
                font.family: "Arial"
            }

            // Progress bar
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 40
                visible: loadingModel.progressVisible

                Rectangle {
                    id: progressTrack
                    anchors.fill: parent
                    radius: 4
                    border.width: 2
                    border.color: "#ffffff"
                    // Background color depends on stage
                    color: loadingModel.stage === 2 ? "#0ab9e6" : "#000000"

                    // Progress fill
                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.margins: 2
                        width: loadingModel.progressMax > 0
                               ? (loadingModel.progressValue / loadingModel.progressMax)
                                 * (parent.width - 4)
                               : 0
                        radius: 2
                        color: "#ff3c28"

                        Behavior on width {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    // Indeterminate animation for Complete stage
                    Rectangle {
                        id: indeterminate
                        visible: loadingModel.stage === 2
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.margins: 2
                        width: parent.width * 0.3
                        radius: 2
                        color: "#ff3c28"

                        SequentialAnimation on x {
                            running: indeterminate.visible
                            loops: Animation.Infinite
                            NumberAnimation {
                                from: 2
                                to: progressTrack.width - indeterminate.width - 2
                                duration: 1500
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                from: progressTrack.width - indeterminate.width - 2
                                to: 2
                                duration: 1500
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }
            }

            // ETA text
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: loadingModel.estimateText
                color: "#ffffff"
                font.pixelSize: 15
                font.family: "Arial"
                visible: text.length > 0
            }
        }

        // Banner (bottom-right corner)
        Image {
            visible: loadingModel.bannerAvailable
            source: loadingModel.bannerAvailable ? "image://loadingscreen/banner" : ""
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 30
            fillMode: Image.PreserveAspectFit
            cache: false
        }
    }
}
