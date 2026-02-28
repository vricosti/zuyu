// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

// Delegate that automatically displays the correct control for a setting
Item {
    id: root
    implicitHeight: settingRow.implicitHeight + 4

    property var settingsModel: null

    RowLayout {
        id: settingRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        // Per-game: restore to global button
        Rectangle {
            visible: model.settingIsSwitchable && !settingsModel.configureGlobal
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            radius: 2
            color: restoreMouse.containsMouse ? paletteHighlight : "transparent"
            border.width: 1
            border.color: model.settingIsGlobal ? paletteMid : paletteHighlight

            Text {
                anchors.centerIn: parent
                text: model.settingIsGlobal ? "\u21BA" : "\u2022"
                font.pixelSize: 12
                color: model.settingIsGlobal ? paletteMid : paletteHighlightedText
            }

            MouseArea {
                id: restoreMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    settingsModel.setGlobal(index, !model.settingIsGlobal)
                }
            }
        }

        // Label
        Text {
            Layout.preferredWidth: 200
            Layout.minimumWidth: 120
            text: model.settingName
            font.pixelSize: 12
            color: model.settingIsLocked ? paletteMid : paletteWindowText
            elide: Text.ElideRight
            wrapMode: Text.NoWrap

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                property bool showingTooltip: false

                onContainsMouseChanged: {
                    if (containsMouse && model.settingTooltip.length > 0) {
                        tooltipItem.visible = true
                    } else {
                        tooltipItem.visible = false
                    }
                }
            }
        }

        // Control area - loader picks the right control
        Loader {
            Layout.fillWidth: true
            Layout.preferredHeight: item ? item.implicitHeight : 28
            active: true
            sourceComponent: {
                switch (model.settingType) {
                case "checkbox":
                    return checkboxComponent
                case "combobox":
                    return comboboxComponent
                case "slider":
                    return sliderComponent
                case "spinbox":
                    return spinboxComponent
                case "lineedit":
                    return lineeditComponent
                default:
                    return lineeditComponent
                }
            }
        }
    }

    // Tooltip overlay
    Rectangle {
        id: tooltipItem
        visible: false
        z: 100
        x: 0
        y: settingRow.height + 2
        width: tooltipText.implicitWidth + 16
        height: tooltipText.implicitHeight + 8
        color: paletteToolTipBase
        border.width: 1
        border.color: paletteMid
        radius: 4

        Text {
            id: tooltipText
            anchors.centerIn: parent
            text: model.settingTooltip
            font.pixelSize: 11
            color: paletteToolTipText
            wrapMode: Text.WordWrap
            maximumLineCount: 5
            width: Math.min(implicitWidth, 300)
        }
    }

    // Checkbox component
    Component {
        id: checkboxComponent
        Rectangle {
            implicitHeight: 24
            color: "transparent"

            RowLayout {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                Rectangle {
                    width: 18
                    height: 18
                    radius: 2
                    border.width: 1
                    border.color: paletteMid
                    color: model.settingValue === "true" ? paletteHighlight : paletteBase
                    opacity: model.settingIsLocked ? 0.5 : 1.0

                    Text {
                        anchors.centerIn: parent
                        text: "\u2713"
                        font.pixelSize: 12
                        color: paletteHighlightedText
                        visible: model.settingValue === "true"
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: !model.settingIsLocked
                        cursorShape: model.settingIsLocked ? Qt.ForbiddenCursor
                                                           : Qt.PointingHandCursor
                        onClicked: {
                            var newVal = model.settingValue === "true" ? "false" : "true"
                            settingsModel.setValue(index, newVal)
                        }
                    }
                }
            }
        }
    }

    // Combobox component
    Component {
        id: comboboxComponent
        Rectangle {
            implicitHeight: 28
            color: "transparent"

            property var options: model.settingOptions
            property var optionValues: model.settingOptionValues

            Rectangle {
                id: comboRect
                width: Math.min(parent.width, 250)
                height: 26
                radius: 3
                color: comboMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                                                : paletteButton
                border.width: 1
                border.color: paletteMid
                opacity: model.settingIsLocked ? 0.5 : 1.0

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.right: comboArrow.left
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 12
                    color: paletteButtonText
                    elide: Text.ElideRight
                    text: {
                        var val = parseInt(model.settingValue)
                        var opts = model.settingOptions
                        var vals = model.settingOptionValues
                        if (opts && vals) {
                            for (var i = 0; i < vals.length; i++) {
                                if (vals[i] === val) return opts[i]
                            }
                        }
                        return model.settingValue
                    }
                }

                Text {
                    id: comboArrow
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: comboPopup.visible ? "\u25B2" : "\u25BC"
                    font.pixelSize: 8
                    color: paletteButtonText
                }

                MouseArea {
                    id: comboMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: !model.settingIsLocked
                    cursorShape: model.settingIsLocked ? Qt.ForbiddenCursor
                                                       : Qt.PointingHandCursor
                    onClicked: comboPopup.visible = !comboPopup.visible
                }

                // Popup
                Rectangle {
                    id: comboPopup
                    visible: false
                    z: 200
                    y: parent.height + 2
                    width: parent.width
                    height: Math.min(comboListView.contentHeight + 4,
                                     200)
                    color: paletteBase
                    border.width: 1
                    border.color: paletteMid
                    radius: 3

                    ListView {
                        id: comboListView
                        anchors.fill: parent
                        anchors.margins: 2
                        model: options
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds

                        delegate: Rectangle {
                            width: comboListView.width
                            height: 24
                            color: optMouse.containsMouse ? paletteHighlight : "transparent"

                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 8
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData
                                font.pixelSize: 12
                                color: optMouse.containsMouse ? paletteHighlightedText
                                                              : paletteWindowText
                            }

                            MouseArea {
                                id: optMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    var vals = optionValues
                                    if (vals && index < vals.length) {
                                        settingsModel.setValue(
                                            root.settingsModel ? settingRow.parent.index
                                                               : 0,
                                            vals[index].toString())
                                    }
                                    comboPopup.visible = false
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Slider component
    Component {
        id: sliderComponent
        RowLayout {
            implicitHeight: 28
            spacing: 8

            property int minVal: parseInt(model.settingMin) || 0
            property int maxVal: parseInt(model.settingMax) || 100
            property int currentVal: parseInt(model.settingValue) || 0

            Rectangle {
                Layout.fillWidth: true
                Layout.maximumWidth: 200
                Layout.preferredHeight: 20
                color: "transparent"
                opacity: model.settingIsLocked ? 0.5 : 1.0

                // Track
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: 4
                    radius: 2
                    color: paletteMid

                    // Fill
                    Rectangle {
                        width: {
                            var range = maxVal - minVal
                            if (range <= 0) return 0
                            return Math.max(0, Math.min(1, (currentVal - minVal) / range))
                                   * parent.width
                        }
                        height: parent.height
                        radius: 2
                        color: paletteHighlight
                    }
                }

                // Handle
                Rectangle {
                    id: sliderHandle
                    x: {
                        var range = maxVal - minVal
                        if (range <= 0) return 0
                        return Math.max(0, Math.min(1, (currentVal - minVal) / range))
                               * (parent.width - width)
                    }
                    anchors.verticalCenter: parent.verticalCenter
                    width: 14
                    height: 14
                    radius: 7
                    color: sliderDrag.pressed ? Qt.darker(paletteHighlight, 1.2)
                           : paletteHighlight
                    border.width: 1
                    border.color: Qt.darker(paletteHighlight, 1.3)

                    MouseArea {
                        id: sliderDrag
                        anchors.fill: parent
                        enabled: !model.settingIsLocked
                        drag.target: sliderHandle
                        drag.axis: Drag.XAxis
                        drag.minimumX: 0
                        drag.maximumX: sliderHandle.parent.width - sliderHandle.width

                        onPositionChanged: {
                            if (pressed) {
                                var ratio = sliderHandle.x / (sliderHandle.parent.width
                                            - sliderHandle.width)
                                var newVal = Math.round(minVal + ratio * (maxVal - minVal))
                                settingsModel.setValue(index, newVal.toString())
                            }
                        }
                    }
                }
            }

            // Value label
            Text {
                Layout.preferredWidth: 50
                text: model.settingIsPercentage ? (currentVal + "%") : currentVal.toString()
                font.pixelSize: 12
                color: paletteWindowText
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    // Spinbox component
    Component {
        id: spinboxComponent
        RowLayout {
            implicitHeight: 28
            spacing: 4

            property int currentVal: parseInt(model.settingValue) || 0
            property int minVal: parseInt(model.settingMin) || 0
            property int maxVal: parseInt(model.settingMax) || 999999

            // Minus button
            Rectangle {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 3
                color: minusMouse.pressed ? Qt.darker(paletteButton, 1.2)
                       : minusMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                       : paletteButton
                border.width: 1
                border.color: paletteMid
                opacity: model.settingIsLocked ? 0.5 : 1.0

                Text {
                    anchors.centerIn: parent
                    text: "-"
                    font.pixelSize: 14
                    color: paletteButtonText
                }

                MouseArea {
                    id: minusMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: !model.settingIsLocked && currentVal > minVal
                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                    onClicked: settingsModel.setValue(index, (currentVal - 1).toString())
                }
            }

            // Value display
            Rectangle {
                Layout.preferredWidth: 80
                Layout.preferredHeight: 24
                color: paletteBase
                border.width: 1
                border.color: paletteMid
                radius: 3

                TextInput {
                    anchors.fill: parent
                    anchors.margins: 4
                    text: model.settingIsPercentage ? (currentVal + "%") : currentVal.toString()
                    font.pixelSize: 12
                    color: paletteWindowText
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    readOnly: model.settingIsLocked
                    validator: IntValidator { bottom: minVal; top: maxVal }

                    onAccepted: {
                        var val = parseInt(text)
                        if (!isNaN(val)) {
                            settingsModel.setValue(index, val.toString())
                        }
                    }
                }
            }

            // Plus button
            Rectangle {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 3
                color: plusMouse.pressed ? Qt.darker(paletteButton, 1.2)
                       : plusMouse.containsMouse ? Qt.lighter(paletteButton, 1.1)
                       : paletteButton
                border.width: 1
                border.color: paletteMid
                opacity: model.settingIsLocked ? 0.5 : 1.0

                Text {
                    anchors.centerIn: parent
                    text: "+"
                    font.pixelSize: 14
                    color: paletteButtonText
                }

                MouseArea {
                    id: plusMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: !model.settingIsLocked && currentVal < maxVal
                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                    onClicked: settingsModel.setValue(index, (currentVal + 1).toString())
                }
            }
        }
    }

    // Line edit component
    Component {
        id: lineeditComponent
        Rectangle {
            implicitHeight: 28
            color: "transparent"

            Rectangle {
                width: Math.min(parent.width, 300)
                height: 26
                color: paletteBase
                border.width: 1
                border.color: paletteMid
                radius: 3
                opacity: model.settingIsLocked ? 0.5 : 1.0

                TextInput {
                    anchors.fill: parent
                    anchors.margins: 6
                    text: model.settingValue
                    font.pixelSize: 12
                    color: paletteWindowText
                    readOnly: model.settingIsLocked
                    clip: true

                    onAccepted: settingsModel.setValue(index, text)
                    onEditingFinished: settingsModel.setValue(index, text)
                }
            }
        }
    }
}
