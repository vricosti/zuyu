// SPDX-FileCopyrightText: 2015 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <map>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QThreadPool>

#include <fmt/format.h>

#include "common/common_types.h"
#include "common/logging/log.h"
#include "common/string_util.h"
#include "core/core.h"
#include "core/file_sys/patch_manager.h"
#include "core/file_sys/registered_cache.h"
#include "yuzu/compatibility_list.h"
#include "yuzu/game_list.h"
#include "yuzu/game_list_p.h"
#include "yuzu/game_list_worker.h"
#include "yuzu/main.h"
#include "yuzu/uisettings.h"
#include "yuzu/util/util.h"

const QStringList GameList::supported_file_extensions = {
    QStringLiteral("nso"), QStringLiteral("nro"), QStringLiteral("nca"),
    QStringLiteral("xci"), QStringLiteral("nsp"), QStringLiteral("kip")};

GameList::GameList(std::shared_ptr<FileSys::VfsFilesystem> vfs_,
                   FileSys::ManualContentProvider* provider_,
                   PlayTime::PlayTimeManager& play_time_manager_, Core::System& system_,
                   GMainWindow* parent)
    : QWidget{parent}, vfs{std::move(vfs_)}, provider{provider_},
      main_window{parent}, play_time_manager{play_time_manager_}, system{system_} {

    watcher = new QFileSystemWatcher(this);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &GameList::RefreshGameDirectory);

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Create QML model and proxy
    qml_model = new GameListModel(this);
    proxy_model = new GameListSortFilterProxy(this);
    proxy_model->setSourceModel(qml_model);

    // Create icon provider (ownership transferred to QML engine below)
    icon_provider = new GameIconProvider();

    // Create QQuickWidget
    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // Register image provider with the QML engine
    quick_widget->engine()->addImageProvider(QStringLiteral("gameicon"), icon_provider);

    // Set context properties
    QQmlContext* ctx = quick_widget->rootContext();
    ctx->setContextProperty(QStringLiteral("gameListModel"), qml_model);
    ctx->setContextProperty(QStringLiteral("gameListProxyModel"), proxy_model);
    ctx->setContextProperty(QStringLiteral("gameListShowCompat"),
                            UISettings::values.show_compat.GetValue());
    ctx->setContextProperty(QStringLiteral("gameListShowAddOns"),
                            UISettings::values.show_add_ons.GetValue());
    ctx->setContextProperty(QStringLiteral("gameListShowSize"),
                            UISettings::values.show_size.GetValue());
    ctx->setContextProperty(QStringLiteral("gameListShowFileType"),
                            UISettings::values.show_types.GetValue());
    ctx->setContextProperty(QStringLiteral("gameListShowPlayTime"),
                            UISettings::values.show_play_time.GetValue());
    ctx->setContextProperty(QStringLiteral("gameListSearchBarVisible"), false);
    ctx->setContextProperty(QStringLiteral("gameListIconSize"),
                            static_cast<int>(UISettings::values.game_icon_size.GetValue()));

    // Pass palette colors to QML so we don't depend on SystemPalette QML type
    const QPalette pal = QApplication::palette();
    ctx->setContextProperty(QStringLiteral("paletteBase"), pal.color(QPalette::Base));
    ctx->setContextProperty(QStringLiteral("paletteAlternateBase"),
                            pal.color(QPalette::AlternateBase));
    ctx->setContextProperty(QStringLiteral("paletteWindow"), pal.color(QPalette::Window));
    ctx->setContextProperty(QStringLiteral("paletteWindowText"),
                            pal.color(QPalette::WindowText));
    ctx->setContextProperty(QStringLiteral("paletteButton"), pal.color(QPalette::Button));
    ctx->setContextProperty(QStringLiteral("paletteButtonText"),
                            pal.color(QPalette::ButtonText));
    ctx->setContextProperty(QStringLiteral("paletteHighlight"), pal.color(QPalette::Highlight));
    ctx->setContextProperty(QStringLiteral("paletteHighlightedText"),
                            pal.color(QPalette::HighlightedText));
    ctx->setContextProperty(QStringLiteral("paletteMid"), pal.color(QPalette::Mid));

    // Load QML
    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/GameListView.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "QML Error: {}", error.toString().toStdString());
        }
    }

    // Connect QML signals to C++ slots
    QQuickItem* rootItem = quick_widget->rootObject();
    if (rootItem) {
        connect(rootItem, SIGNAL(gameDoubleClicked(QString, QVariant)), this,
                SLOT(OnGameDoubleClicked(QString, QVariant)));
        connect(rootItem, SIGNAL(contextMenuRequested(int, qreal, qreal)), this,
                SLOT(OnContextMenuRequested(int, qreal, qreal)));
        connect(rootItem, SIGNAL(addDirectoryRequested()), this, SIGNAL(AddDirectory()));
        connect(rootItem, SIGNAL(searchClosed()), this, SLOT(OnSearchClosed()));
        connect(rootItem, SIGNAL(singleResultLaunched(QString)), this,
                SLOT(OnSingleResultLaunched(QString)));
    }

    // Connect model signals
    connect(qml_model, &GameListModel::favoriteToggled, this, &GameList::OnFavoriteToggled);

    layout->addWidget(quick_widget);
    setLayout(layout);

    // Register meta types for worker
    qRegisterMetaType<QList<QStandardItem*>>("QList<QStandardItem*>");
}

GameList::~GameList() = default;

void GameList::UnloadController() {
    // Controller navigation not yet ported to QML
}

QString GameList::GetLastFilterResultItem() const {
    for (int i = proxy_model->rowCount() - 1; i >= 0; --i) {
        const QModelIndex proxyIdx = proxy_model->index(i, 0);
        const QModelIndex srcIdx = proxy_model->mapToSource(proxyIdx);
        const auto& entry = qml_model->Entries()[srcIdx.row()];
        if (entry.itemType == GameListEntry::ItemType::Game) {
            return entry.filePath;
        }
    }
    return {};
}

void GameList::ClearFilter() {
    proxy_model->setFilterText(QString());
    QQuickItem* rootItem = quick_widget->rootObject();
    if (rootItem) {
        QMetaObject::invokeMethod(rootItem, "clearFilter");
    }
}

void GameList::SetFilterFocus() {
    QQuickItem* rootItem = quick_widget->rootObject();
    if (rootItem) {
        QMetaObject::invokeMethod(rootItem, "setFilterFocus");
    }
}

void GameList::SetFilterVisible(bool visibility) {
    quick_widget->rootContext()->setContextProperty(QStringLiteral("gameListSearchBarVisible"),
                                                    visibility);
}

bool GameList::IsEmpty() const {
    // Remove empty SDMC/NAND/SysNAND sections (matching original QWidget behavior).
    // In the flat model, a section is "empty" if no Game entries follow it before the next section.
    const auto& entries = qml_model->Entries();
    for (int i = entries.size() - 1; i >= 0; --i) {
        if (entries[i].itemType != GameListEntry::ItemType::Section) {
            continue;
        }
        const auto sectionType = static_cast<GameListItemType>(entries[i].sectionType);
        if (sectionType != GameListItemType::SdmcDir &&
            sectionType != GameListItemType::UserNandDir &&
            sectionType != GameListItemType::SysNandDir) {
            continue;
        }
        // Check if there are any game entries following this section before the next section
        bool hasChildren = false;
        for (int j = i + 1; j < entries.size(); ++j) {
            if (entries[j].itemType == GameListEntry::ItemType::Section) {
                break;
            }
            if (entries[j].itemType == GameListEntry::ItemType::Game) {
                hasChildren = true;
                break;
            }
        }
        if (!hasChildren) {
            qml_model->RemoveRow(i);
        }
    }

    for (const auto& entry : qml_model->Entries()) {
        if (entry.itemType == GameListEntry::ItemType::Game) {
            return false;
        }
    }
    return true;
}

void GameList::LoadCompatibilityList() {
    QFile compat_list{QStringLiteral(":compatibility_list/compatibility_list.json")};

    if (!compat_list.open(QFile::ReadOnly | QFile::Text)) {
        LOG_ERROR(Frontend, "Unable to open game compatibility list");
        return;
    }

    if (compat_list.size() == 0) {
        LOG_WARNING(Frontend, "Game compatibility list is empty");
        return;
    }

    const QByteArray content = compat_list.readAll();
    if (content.isEmpty()) {
        LOG_ERROR(Frontend, "Unable to completely read game compatibility list");
        return;
    }

    const QJsonDocument json = QJsonDocument::fromJson(content);
    const QJsonArray arr = json.array();

    for (const QJsonValue value : arr) {
        const QJsonObject game = value.toObject();
        const QString compatibility_key = QStringLiteral("compatibility");

        if (!game.contains(compatibility_key) || !game[compatibility_key].isDouble()) {
            continue;
        }

        const int compatibility = game[compatibility_key].toInt();
        const QString directory = game[QStringLiteral("directory")].toString();
        const QJsonArray ids = game[QStringLiteral("releases")].toArray();

        for (const QJsonValue id_ref : ids) {
            const QJsonObject id_object = id_ref.toObject();
            const QString id = id_object[QStringLiteral("id")].toString();

            compatibility_list.emplace(id.toUpper().toStdString(),
                                       std::make_pair(QString::number(compatibility), directory));
        }
    }
}

void GameList::PopulateAsync(QVector<UISettings::GameDir>& game_dirs) {
    // Update visibility context properties
    QQmlContext* ctx = quick_widget->rootContext();
    ctx->setContextProperty(QStringLiteral("gameListShowCompat"),
                            UISettings::values.show_compat.GetValue());
    ctx->setContextProperty(QStringLiteral("gameListShowAddOns"),
                            UISettings::values.show_add_ons.GetValue());
    ctx->setContextProperty(QStringLiteral("gameListShowFileType"),
                            UISettings::values.show_types.GetValue());
    ctx->setContextProperty(QStringLiteral("gameListShowSize"),
                            UISettings::values.show_size.GetValue());
    ctx->setContextProperty(QStringLiteral("gameListShowPlayTime"),
                            UISettings::values.show_play_time.GetValue());

    // Cancel any existing worker
    current_worker.reset();

    // Clear existing data
    qml_model->Clear();
    section_start_indices.clear();
    ClearFilter();
    compat_model_dirty = true;

    current_worker = std::make_unique<GameListWorker>(vfs, provider, game_dirs, compatibility_list,
                                                      play_time_manager, system);

    connect(current_worker.get(), &GameListWorker::DataAvailable, this, &GameList::WorkerEvent,
            Qt::QueuedConnection);

    QThreadPool::globalInstance()->start(current_worker.get());
}

void GameList::WorkerEvent() {
    current_worker->ProcessEvents(this);
}

void GameList::AddDirEntry(GameListDir* entry_items) {
    const int dirType = entry_items->type();
    const QString displayName = entry_items->data(Qt::DisplayRole).toString();
    const int gameDirIndex = entry_items->data(GameListDir::GameDirRole).toInt();

    bool expanded = true;
    if (gameDirIndex >= 0 && gameDirIndex < UISettings::values.game_dirs.size()) {
        expanded = UISettings::values.game_dirs[gameDirIndex].expanded;
    }

    qml_model->AddSection(displayName, dirType, gameDirIndex, expanded);
    section_start_indices[entry_items] = qml_model->rowCount() - 1;
}

void GameList::AddEntry(const QList<QStandardItem*>& entry_items, GameListDir* parent) {
    if (entry_items.isEmpty()) {
        return;
    }

    GameListEntry entry;
    entry.itemType = GameListEntry::ItemType::Game;

    // Extract data from QStandardItems (same roles as game_list_p.h)
    const QStandardItem* nameItem = entry_items[0];
    entry.filePath = nameItem->data(GameListItemPath::FullPathRole).toString();
    entry.name = nameItem->data(GameListItemPath::TitleRole).toString();
    entry.programId = nameItem->data(GameListItemPath::ProgramIdRole).toULongLong();
    entry.fileType = nameItem->data(GameListItemPath::FileTypeRole).toString();

    // If title is empty, use filename
    if (entry.name.isEmpty()) {
        std::string filename;
        Common::SplitPath(entry.filePath.toStdString(), nullptr, &filename, nullptr);
        entry.name = QString::fromStdString(filename);
    }

    // Extract icon from the decoration role
    const QVariant decoration = nameItem->data(Qt::DecorationRole);
    if (decoration.isValid()) {
        QPixmap pixmap;
        if (decoration.canConvert<QPixmap>()) {
            pixmap = decoration.value<QPixmap>();
        } else if (decoration.canConvert<QIcon>()) {
            pixmap = decoration.value<QIcon>().pixmap(64, 64);
        }
        if (!pixmap.isNull() && entry.programId != 0) {
            icon_provider->addIcon(entry.programId, pixmap);
        }
    }

    // Compatibility (item at index 1)
    if (entry_items.size() > 1) {
        const QStandardItem* compatItem = entry_items[1];
        entry.compatibility = compatItem->data(GameListItemCompat::CompatNumberRole).toString();
        entry.compatibilityText = compatItem->text();

        static const std::map<QString, QString> compat_colors = {
            {QStringLiteral("0"), QStringLiteral("#5c93ed")},
            {QStringLiteral("1"), QStringLiteral("#47d35c")},
            {QStringLiteral("2"), QStringLiteral("#f2d624")},
            {QStringLiteral("3"), QStringLiteral("#f2d624")},
            {QStringLiteral("4"), QStringLiteral("#FF0000")},
            {QStringLiteral("5"), QStringLiteral("#828282")},
            {QStringLiteral("99"), QStringLiteral("#000000")},
        };
        auto it = compat_colors.find(entry.compatibility);
        if (it != compat_colors.end()) {
            entry.compatibilityColor = it->second;
        }
    }

    // Add-ons (item at index 2)
    if (entry_items.size() > 2) {
        entry.addOns = entry_items[2]->text();
    }

    // Size (item at index 3)
    if (entry_items.size() > 3) {
        const QStandardItem* sizeItem = entry_items[3];
        entry.sizeBytes = sizeItem->data(GameListItemSize::SizeRole).toULongLong();
        entry.sizeText = sizeItem->text();
    }

    // Play time (item at index 4)
    if (entry_items.size() > 4) {
        entry.playTimeText = entry_items[4]->text();
    }

    // Determine section name from parent
    if (parent && section_start_indices.contains(parent)) {
        entry.sectionName = parent->data(Qt::DisplayRole).toString();
    }

    // Check if favorited
    entry.isFavorite = UISettings::values.favorited_ids.contains(entry.programId);

    qml_model->AddGameEntry(entry);
    compat_model_dirty = true;

    // Clean up the QStandardItems since we extracted what we need
    qDeleteAll(entry_items);
}

void GameList::DonePopulating(const QStringList& watch_list_in) {
    emit ShowList(!IsEmpty());

    // Clear out old watch directories and add new ones
    auto watch_dirs = watcher->directories();
    if (!watch_dirs.isEmpty()) {
        watcher->removePaths(watch_dirs);
    }

    constexpr int LIMIT_WATCH_DIRECTORIES = 5000;
    constexpr int SLICE_SIZE = 25;
    int len = std::min(static_cast<int>(watch_list_in.size()), LIMIT_WATCH_DIRECTORIES);
    for (int i = 0; i < len; i += SLICE_SIZE) {
        watcher->addPaths(watch_list_in.mid(i, i + SLICE_SIZE));
        QCoreApplication::processEvents();
    }

    compat_model_dirty = true;
    emit PopulatingCompleted();
}

void GameList::OnGameDoubleClicked(const QString& path, const QVariant& titleId) {
    if (path.isEmpty()) {
        return;
    }

    const QFileInfo file_info(path);
    if (!file_info.exists()) {
        return;
    }

    if (file_info.isDir()) {
        const QDir dir{path};
        const QStringList matching_main = dir.entryList({QStringLiteral("main")}, QDir::Files);
        if (matching_main.size() == 1) {
            emit GameChosen(dir.path() + QDir::separator() + matching_main[0]);
        }
        return;
    }

    const u64 title_id = titleId.toULongLong();
    ClearFilter();
    emit GameChosen(path, title_id);
}

void GameList::OnContextMenuRequested(int sourceIndex, qreal globalX, qreal globalY) {
    PopupContextMenu(QPoint(static_cast<int>(globalX), static_cast<int>(globalY)), sourceIndex);
}

void GameList::OnSearchClosed() {
    main_window->filterBarSetChecked(false);
}

void GameList::OnSingleResultLaunched(const QString& path) {
    ClearFilter();
    emit GameChosen(path);
}

void GameList::OnFavoriteToggled(quint64 programId) {
    ToggleFavorite(programId);
}

void GameList::PopupContextMenu(const QPoint& globalPos, int sourceIndex) {
    if (sourceIndex < 0 || sourceIndex >= qml_model->Entries().size()) {
        return;
    }

    const auto& entry = qml_model->Entries()[sourceIndex];
    QMenu context_menu;

    if (entry.itemType == GameListEntry::ItemType::Game) {
        AddGamePopup(context_menu, entry.programId, entry.filePath.toStdString());
    } else if (entry.itemType == GameListEntry::ItemType::Section) {
        const auto sectionType = static_cast<GameListItemType>(entry.sectionType);
        switch (sectionType) {
        case GameListItemType::CustomDir:
            AddPermDirPopup(context_menu, sourceIndex);
            AddCustomDirPopup(context_menu, sourceIndex);
            break;
        case GameListItemType::SdmcDir:
        case GameListItemType::UserNandDir:
        case GameListItemType::SysNandDir:
            AddPermDirPopup(context_menu, sourceIndex);
            break;
        case GameListItemType::Favorites:
            AddFavoritesPopup(context_menu);
            break;
        default:
            return;
        }
    }

    context_menu.exec(globalPos);
}

void GameList::AddGamePopup(QMenu& context_menu, u64 program_id, const std::string& path) {
    QAction* favorite = context_menu.addAction(tr("Favorite"));
    context_menu.addSeparator();
    QAction* start_game = context_menu.addAction(tr("Start Game"));
    QAction* start_game_global =
        context_menu.addAction(tr("Start Game without Custom Configuration"));
    context_menu.addSeparator();
    QAction* open_save_location = context_menu.addAction(tr("Open Save Data Location"));
    QAction* open_mod_location = context_menu.addAction(tr("Open Mod Data Location"));
    QAction* open_transferable_shader_cache =
        context_menu.addAction(tr("Open Transferable Pipeline Cache"));
    context_menu.addSeparator();
    QMenu* remove_menu = context_menu.addMenu(tr("Remove"));
    QAction* remove_update = remove_menu->addAction(tr("Remove Installed Update"));
    QAction* remove_dlc = remove_menu->addAction(tr("Remove All Installed DLC"));
    QAction* remove_custom_config = remove_menu->addAction(tr("Remove Custom Configuration"));
    QAction* remove_play_time_data = remove_menu->addAction(tr("Remove Play Time Data"));
    QAction* remove_cache_storage = remove_menu->addAction(tr("Remove Cache Storage"));
    QAction* remove_gl_shader_cache = remove_menu->addAction(tr("Remove OpenGL Pipeline Cache"));
    QAction* remove_vk_shader_cache = remove_menu->addAction(tr("Remove Vulkan Pipeline Cache"));
    remove_menu->addSeparator();
    QAction* remove_shader_cache = remove_menu->addAction(tr("Remove All Pipeline Caches"));
    QAction* remove_all_content = remove_menu->addAction(tr("Remove All Installed Contents"));
    QMenu* dump_romfs_menu = context_menu.addMenu(tr("Dump RomFS"));
    QAction* dump_romfs = dump_romfs_menu->addAction(tr("Dump RomFS"));
    QAction* dump_romfs_sdmc = dump_romfs_menu->addAction(tr("Dump RomFS to SDMC"));
    QAction* verify_integrity = context_menu.addAction(tr("Verify Integrity"));
    QAction* copy_tid = context_menu.addAction(tr("Copy Title ID to Clipboard"));
    QAction* navigate_to_gamedb_entry = context_menu.addAction(tr("Navigate to GameDB entry"));
#if !defined(__APPLE__)
    QMenu* shortcut_menu = context_menu.addMenu(tr("Create Shortcut"));
    QAction* create_desktop_shortcut = shortcut_menu->addAction(tr("Add to Desktop"));
    QAction* create_applications_menu_shortcut =
        shortcut_menu->addAction(tr("Add to Applications Menu"));
#endif
    context_menu.addSeparator();
    QAction* properties = context_menu.addAction(tr("Properties"));

    favorite->setVisible(program_id != 0);
    favorite->setCheckable(true);
    favorite->setChecked(UISettings::values.favorited_ids.contains(program_id));
    open_save_location->setVisible(program_id != 0);
    open_mod_location->setVisible(program_id != 0);
    open_transferable_shader_cache->setVisible(program_id != 0);
    remove_update->setVisible(program_id != 0);
    remove_dlc->setVisible(program_id != 0);
    remove_gl_shader_cache->setVisible(program_id != 0);
    remove_vk_shader_cache->setVisible(program_id != 0);
    remove_shader_cache->setVisible(program_id != 0);
    remove_all_content->setVisible(program_id != 0);
    auto it = FindMatchingCompatibilityEntry(compatibility_list, program_id);
    navigate_to_gamedb_entry->setVisible(it != compatibility_list.end() && program_id != 0);

    connect(favorite, &QAction::triggered, [this, program_id]() { ToggleFavorite(program_id); });
    connect(open_save_location, &QAction::triggered, [this, program_id, path]() {
        emit OpenFolderRequested(program_id, GameListOpenTarget::SaveData, path);
    });
    connect(start_game, &QAction::triggered,
            [this, path]() { emit BootGame(QString::fromStdString(path), StartGameType::Normal); });
    connect(start_game_global, &QAction::triggered,
            [this, path]() { emit BootGame(QString::fromStdString(path), StartGameType::Global); });
    connect(open_mod_location, &QAction::triggered, [this, program_id, path]() {
        emit OpenFolderRequested(program_id, GameListOpenTarget::ModData, path);
    });
    connect(open_transferable_shader_cache, &QAction::triggered,
            [this, program_id]() { emit OpenTransferableShaderCacheRequested(program_id); });
    connect(remove_all_content, &QAction::triggered, [this, program_id]() {
        emit RemoveInstalledEntryRequested(program_id, InstalledEntryType::Game);
    });
    connect(remove_update, &QAction::triggered, [this, program_id]() {
        emit RemoveInstalledEntryRequested(program_id, InstalledEntryType::Update);
    });
    connect(remove_dlc, &QAction::triggered, [this, program_id]() {
        emit RemoveInstalledEntryRequested(program_id, InstalledEntryType::AddOnContent);
    });
    connect(remove_gl_shader_cache, &QAction::triggered, [this, program_id, path]() {
        emit RemoveFileRequested(program_id, GameListRemoveTarget::GlShaderCache, path);
    });
    connect(remove_vk_shader_cache, &QAction::triggered, [this, program_id, path]() {
        emit RemoveFileRequested(program_id, GameListRemoveTarget::VkShaderCache, path);
    });
    connect(remove_shader_cache, &QAction::triggered, [this, program_id, path]() {
        emit RemoveFileRequested(program_id, GameListRemoveTarget::AllShaderCache, path);
    });
    connect(remove_custom_config, &QAction::triggered, [this, program_id, path]() {
        emit RemoveFileRequested(program_id, GameListRemoveTarget::CustomConfiguration, path);
    });
    connect(remove_play_time_data, &QAction::triggered,
            [this, program_id]() { emit RemovePlayTimeRequested(program_id); });
    connect(remove_cache_storage, &QAction::triggered, [this, program_id, path] {
        emit RemoveFileRequested(program_id, GameListRemoveTarget::CacheStorage, path);
    });
    connect(dump_romfs, &QAction::triggered, [this, program_id, path]() {
        emit DumpRomFSRequested(program_id, path, DumpRomFSTarget::Normal);
    });
    connect(dump_romfs_sdmc, &QAction::triggered, [this, program_id, path]() {
        emit DumpRomFSRequested(program_id, path, DumpRomFSTarget::SDMC);
    });
    connect(verify_integrity, &QAction::triggered,
            [this, path]() { emit VerifyIntegrityRequested(path); });
    connect(copy_tid, &QAction::triggered,
            [this, program_id]() { emit CopyTIDRequested(program_id); });
    connect(navigate_to_gamedb_entry, &QAction::triggered, [this, program_id]() {
        emit NavigateToGamedbEntryRequested(program_id, compatibility_list);
    });
#if !defined(__APPLE__)
    connect(create_desktop_shortcut, &QAction::triggered, [this, program_id, path]() {
        emit CreateShortcut(program_id, path, GameListShortcutTarget::Desktop);
    });
    connect(create_applications_menu_shortcut, &QAction::triggered, [this, program_id, path]() {
        emit CreateShortcut(program_id, path, GameListShortcutTarget::Applications);
    });
#endif
    connect(properties, &QAction::triggered,
            [this, path]() { emit OpenPerGameGeneralRequested(path); });
}

void GameList::AddCustomDirPopup(QMenu& context_menu, int sourceIndex) {
    if (sourceIndex < 0 || sourceIndex >= qml_model->Entries().size()) {
        return;
    }

    const auto& entry = qml_model->Entries()[sourceIndex];
    const int gameDirIndex = entry.gameDirIndex;
    if (gameDirIndex < 0 || gameDirIndex >= UISettings::values.game_dirs.size()) {
        return;
    }

    UISettings::GameDir& game_dir = UISettings::values.game_dirs[gameDirIndex];

    QAction* deep_scan = context_menu.addAction(tr("Scan Subfolders"));
    QAction* delete_dir = context_menu.addAction(tr("Remove Game Directory"));

    deep_scan->setCheckable(true);
    deep_scan->setChecked(game_dir.deep_scan);

    connect(deep_scan, &QAction::triggered, [this, &game_dir] {
        game_dir.deep_scan = !game_dir.deep_scan;
        PopulateAsync(UISettings::values.game_dirs);
    });
    connect(delete_dir, &QAction::triggered, [this, &game_dir] {
        UISettings::values.game_dirs.removeOne(game_dir);
        PopulateAsync(UISettings::values.game_dirs);
    });
}

void GameList::AddPermDirPopup(QMenu& context_menu, int sourceIndex) {
    if (sourceIndex < 0 || sourceIndex >= qml_model->Entries().size()) {
        return;
    }

    const auto& entry = qml_model->Entries()[sourceIndex];
    const int gameDirIndex = entry.gameDirIndex;

    QAction* open_directory_location = context_menu.addAction(tr("Open Directory Location"));

    connect(open_directory_location, &QAction::triggered, [this, gameDirIndex] {
        if (gameDirIndex >= 0 && gameDirIndex < UISettings::values.game_dirs.size()) {
            emit OpenDirectory(
                QString::fromStdString(UISettings::values.game_dirs[gameDirIndex].path));
        }
    });
}

void GameList::AddFavoritesPopup(QMenu& context_menu) {
    QAction* clear = context_menu.addAction(tr("Clear"));

    connect(clear, &QAction::triggered, [this] {
        UISettings::values.favorited_ids.clear();
        // Update all entries' favorite status
        const auto& entries = qml_model->Entries();
        for (int i = 0; i < entries.size(); ++i) {
            if (entries[i].isFavorite) {
                qml_model->toggleFavorite(entries[i].programId);
            }
        }
    });
}

void GameList::ToggleFavorite(u64 program_id) {
    if (!UISettings::values.favorited_ids.contains(program_id)) {
        UISettings::values.favorited_ids.append(program_id);
    } else {
        UISettings::values.favorited_ids.removeOne(program_id);
    }

    // Update model entries
    const auto& entries = qml_model->Entries();
    for (int i = 0; i < entries.size(); ++i) {
        if (entries[i].itemType == GameListEntry::ItemType::Game &&
            entries[i].programId == program_id) {
            // Use toggleFavorite to update the model properly
            // But since we already changed UISettings, just update the data directly
            auto& mutableEntry = const_cast<GameListEntry&>(entries[i]);
            mutableEntry.isFavorite = UISettings::values.favorited_ids.contains(program_id);
            const QModelIndex idx = qml_model->index(i);
            emit qml_model->dataChanged(idx, idx, {GameListModel::IsFavoriteRole});
        }
    }

    compat_model_dirty = true;
    emit SaveConfig();
}

void GameList::SaveInterfaceLayout() {
    // QML manages its own layout
}

void GameList::LoadInterfaceLayout() {
    // QML manages its own layout
}

QStandardItemModel* GameList::GetModel() const {
    if (compat_model_dirty) {
        RebuildCompatModel();
    }
    return compat_model;
}

void GameList::RebuildCompatModel() const {
    if (!compat_model) {
        compat_model = new QStandardItemModel(const_cast<GameList*>(this));
    }
    compat_model->clear();

    for (const auto& entry : qml_model->Entries()) {
        if (entry.itemType != GameListEntry::ItemType::Game) {
            continue;
        }

        auto* item = new QStandardItem(entry.name);
        item->setData(entry.filePath, GameListItemPath::FullPathRole);
        item->setData(QVariant::fromValue(static_cast<qulonglong>(entry.programId)),
                      GameListItemPath::ProgramIdRole);
        item->setData(entry.name, GameListItemPath::TitleRole);
        item->setData(entry.fileType, GameListItemPath::FileTypeRole);
        item->setData(static_cast<int>(GameListItemType::Game), GameListItem::TypeRole);
        compat_model->appendRow(item);
    }

    compat_model_dirty = false;
}

void GameList::RefreshGameDirectory() {
    if (!UISettings::values.game_dirs.empty() && current_worker != nullptr) {
        LOG_INFO(Frontend, "Change detected in the games directory. Reloading game list.");
        PopulateAsync(UISettings::values.game_dirs);
    }
}

void GameList::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
}

// --- GameListPlaceholder ---

GameListPlaceholder::GameListPlaceholder(GMainWindow* parent) : QWidget{parent} {
    connect(parent, &GMainWindow::UpdateThemedIcons, this,
            &GameListPlaceholder::onUpdateThemedIcons);

    layout = new QVBoxLayout;
    image = new QLabel;
    text = new QLabel;
    layout->setAlignment(Qt::AlignCenter);
    image->setPixmap(QIcon::fromTheme(QStringLiteral("plus_folder")).pixmap(200));

    RetranslateUI();
    QFont font = text->font();
    font.setPointSize(20);
    text->setFont(font);
    text->setAlignment(Qt::AlignHCenter);
    image->setAlignment(Qt::AlignHCenter);

    layout->addWidget(image);
    layout->addWidget(text);
    setLayout(layout);
}

GameListPlaceholder::~GameListPlaceholder() = default;

void GameListPlaceholder::onUpdateThemedIcons() {
    image->setPixmap(QIcon::fromTheme(QStringLiteral("plus_folder")).pixmap(200));
}

void GameListPlaceholder::mouseDoubleClickEvent(QMouseEvent* event) {
    emit GameListPlaceholder::AddDirectory();
}

void GameListPlaceholder::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        RetranslateUI();
    }

    QWidget::changeEvent(event);
}

void GameListPlaceholder::RetranslateUI() {
    text->setText(tr("Double-click to add a new folder to the game list"));
}
