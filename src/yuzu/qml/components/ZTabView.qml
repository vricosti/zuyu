// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property var tabTitles: []
    property int currentIndex: 0

    default property alias content: stack.children

    implicitWidth: 400
    implicitHeight: tabBar.height + stack.height

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ZTabBar {
            id: tabBar
            Layout.fillWidth: true
            model: root.tabTitles
            currentIndex: root.currentIndex

            onTabClicked: function(index) {
                root.currentIndex = index;
            }
        }

        // Tab content stack
        Item {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            onChildrenChanged: updateVisibility()
        }
    }

    onCurrentIndexChanged: updateVisibility()

    function updateVisibility() {
        for (var i = 0; i < stack.children.length; ++i) {
            stack.children[i].visible = (i === currentIndex);
            if (stack.children[i].visible) {
                stack.children[i].anchors.fill = stack;
            }
        }
    }

    Component.onCompleted: updateVisibility()
}
