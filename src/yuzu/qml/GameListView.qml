// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    // Signals forwarded to C++
    signal gameDoubleClicked(string path, var titleId)
    signal contextMenuRequested(int sourceIndex, real globalX, real globalY)
    signal addDirectoryRequested()
    signal searchClosed()
    signal singleResultLaunched(string path)

    property bool searchBarVisible: typeof gameListSearchBarVisible !== "undefined"
                                    ? gameListSearchBarVisible : false

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: gameListProxyModel
            boundsBehavior: Flickable.StopAtBounds

            delegate: Loader {
                id: delegateLoader
                width: listView.width

                // Properties from model
                required property int index
                required property string name
                required property string filePath
                required property var programId
                required property string fileType
                required property string sizeText
                required property string compatibilityText
                required property string compatibilityColor
                required property string addOns
                required property string playTimeText
                required property string iconSource
                required property bool isFavorite
                required property string sectionName
                required property int sectionType
                required property int itemType
                required property int gameDirIndex
                required property bool sectionExpanded

                sourceComponent: itemType === 1 ? sectionComponent : gameComponent

                Component {
                    id: sectionComponent
                    GameListSectionDelegate {
                        width: listView.width
                        index: delegateLoader.index
                        name: delegateLoader.sectionName.length > 0
                              ? delegateLoader.sectionName : delegateLoader.name
                        sectionType: delegateLoader.sectionType
                        sectionExpanded: delegateLoader.sectionExpanded
                        gameDirIndex: delegateLoader.gameDirIndex

                        onToggleExpanded: function(sourceIndex) {
                            var srcIdx = gameListProxyModel.mapToSource(
                                gameListProxyModel.index(sourceIndex, 0));
                            gameListModel.toggleSectionExpanded(srcIdx.row);
                        }
                    }
                }

                Component {
                    id: gameComponent
                    GameListDelegate {
                        width: listView.width
                        index: delegateLoader.index
                        name: delegateLoader.name
                        filePath: delegateLoader.filePath
                        programId: delegateLoader.programId
                        fileType: delegateLoader.fileType
                        sizeText: delegateLoader.sizeText
                        compatibilityText: delegateLoader.compatibilityText
                        compatibilityColor: delegateLoader.compatibilityColor
                        addOns: delegateLoader.addOns
                        playTimeText: delegateLoader.playTimeText
                        iconSource: delegateLoader.iconSource
                        isFavorite: delegateLoader.isFavorite

                        onGameDoubleClicked: function(path, titleId) {
                            root.gameDoubleClicked(path, titleId);
                        }
                        onContextMenuRequested: function(idx, gx, gy) {
                            var srcIdx = gameListProxyModel.mapToSource(
                                gameListProxyModel.index(idx, 0));
                            root.contextMenuRequested(srcIdx.row, gx, gy);
                        }
                    }
                }
            }
        }

        GameListSearchBar {
            id: searchBar
            Layout.fillWidth: true
            visible: root.searchBarVisible

            visibleCount: gameListProxyModel ? gameListProxyModel.visibleCount : 0
            totalCount: gameListProxyModel ? gameListProxyModel.totalCount : 0

            onTextChanged: {
                gameListProxyModel.filterText = text;
            }

            onCloseRequested: {
                root.searchClosed();
            }
        }
    }

    function clearFilter() {
        searchBar.text = "";
    }

    function setFilterFocus() {
        searchBar.forceActiveFocus();
    }

    function launchSingleResult() {
        for (var i = 0; i < gameListProxyModel.rowCount(); ++i) {
            var srcIdx = gameListProxyModel.mapToSource(gameListProxyModel.index(i, 0));
            var it = gameListModel.data(srcIdx, 273); // ItemTypeRole
            if (it === 0) { // Game
                var path = gameListModel.data(srcIdx, 258); // FilePathRole
                root.singleResultLaunched(path);
                return;
            }
        }
    }
}
