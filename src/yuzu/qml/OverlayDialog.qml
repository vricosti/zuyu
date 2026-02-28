// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

// Full-screen overlay with centered dialog content
Rectangle {
    id: root

    property string titleText: typeof overlayTitleText !== "undefined" ? overlayTitleText : ""
    property string bodyText: typeof overlayBodyText !== "undefined" ? overlayBodyText : ""
    property string leftButtonText: typeof overlayLeftButtonText !== "undefined"
                                    ? overlayLeftButtonText : ""
    property string rightButtonText: typeof overlayRightButtonText !== "undefined"
                                     ? overlayRightButtonText : ""
    property bool useRichText: typeof overlayUseRichText !== "undefined" ? overlayUseRichText : false
    property real titleFontSize: typeof overlayTitleFontSize !== "undefined"
                                 ? overlayTitleFontSize : 14
    property real bodyFontSize: typeof overlayBodyFontSize !== "undefined"
                                ? overlayBodyFontSize : 18
    property real buttonFontSize: typeof overlayButtonFontSize !== "undefined"
                                  ? overlayButtonFontSize : 18

    signal leftButtonClicked()
    signal rightButtonClicked()

    color: "#B0000000"

    // Center content
    Rectangle {
        id: dialog
        anchors.centerIn: parent
        width: root.width * 0.6
        height: root.height * 0.42
        color: "transparent"

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // Title
            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: implicitHeight + 20
                visible: root.titleText.length > 0
                text: root.titleText
                font.pixelSize: root.titleFontSize
                color: "#ffffff"
                verticalAlignment: Text.AlignBottom
                wrapMode: Text.WordWrap
            }

            // Body text
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Flickable {
                    id: bodyFlickable
                    anchors.fill: parent
                    anchors.topMargin: 10
                    anchors.bottomMargin: 10
                    contentHeight: bodyTextItem.implicitHeight
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    Text {
                        id: bodyTextItem
                        width: bodyFlickable.width
                        text: root.bodyText
                        font.pixelSize: root.bodyFontSize
                        color: "#ffffff"
                        wrapMode: Text.WordWrap
                        textFormat: root.useRichText ? Text.RichText : Text.PlainText
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        onLinkActivated: function(link) {
                            Qt.openUrlExternally(link);
                        }
                    }
                }

                // Scrollbar
                Rectangle {
                    visible: bodyFlickable.contentHeight > bodyFlickable.height
                    anchors.right: parent.right
                    anchors.rightMargin: 2
                    y: bodyFlickable.contentHeight > 0
                       ? (bodyFlickable.contentY / bodyFlickable.contentHeight) * parent.height
                       : 0
                    width: 4
                    height: bodyFlickable.contentHeight > 0
                            ? Math.max(20, (bodyFlickable.height / bodyFlickable.contentHeight) * parent.height)
                            : 0
                    radius: 2
                    color: "#80ffffff"
                }
            }

            // Buttons row
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                visible: root.leftButtonText.length > 0 || root.rightButtonText.length > 0
                spacing: 16

                // Left (cancel) button
                Rectangle {
                    visible: root.leftButtonText.length > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 4
                    color: leftBtnMouse.pressed ? "#40ffffff"
                           : leftBtnMouse.containsMouse ? "#20ffffff"
                           : "#10ffffff"
                    border.width: leftBtnFocus.activeFocus ? 2 : 1
                    border.color: leftBtnFocus.activeFocus ? "#ffffff" : "#40ffffff"

                    Text {
                        anchors.centerIn: parent
                        text: root.leftButtonText
                        font.pixelSize: root.buttonFontSize
                        color: "#ffffff"
                    }

                    FocusScope {
                        id: leftBtnFocus
                        anchors.fill: parent
                    }

                    MouseArea {
                        id: leftBtnMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.leftButtonClicked()
                    }
                }

                // Right (ok) button
                Rectangle {
                    visible: root.rightButtonText.length > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 4
                    color: rightBtnMouse.pressed ? "#40ffffff"
                           : rightBtnMouse.containsMouse ? "#20ffffff"
                           : "#10ffffff"
                    border.width: rightBtnFocus.activeFocus ? 2 : 1
                    border.color: rightBtnFocus.activeFocus ? "#ffffff" : "#40ffffff"

                    Text {
                        anchors.centerIn: parent
                        text: root.rightButtonText
                        font.pixelSize: root.buttonFontSize
                        color: "#ffffff"
                    }

                    FocusScope {
                        id: rightBtnFocus
                        anchors.fill: parent
                    }

                    MouseArea {
                        id: rightBtnMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.rightButtonClicked()
                    }
                }
            }
        }
    }
}
