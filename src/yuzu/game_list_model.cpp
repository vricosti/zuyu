// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QMutexLocker>
#include "yuzu/game_list_model.h"

// --- GameListModel ---

GameListModel::GameListModel(QObject* parent) : QAbstractListModel(parent) {}

int GameListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(entries_.size());
}

QVariant GameListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) {
        return {};
    }

    const auto& entry = entries_[index.row()];

    switch (role) {
    case NameRole:
        return entry.name;
    case FilePathRole:
        return entry.filePath;
    case ProgramIdRole:
        return QVariant::fromValue(static_cast<quint64>(entry.programId));
    case FileTypeRole:
        return entry.fileType;
    case SizeBytesRole:
        return QVariant::fromValue(entry.sizeBytes);
    case SizeTextRole:
        return entry.sizeText;
    case CompatibilityRole:
        return entry.compatibility;
    case CompatibilityTextRole:
        return entry.compatibilityText;
    case CompatibilityColorRole:
        return entry.compatibilityColor;
    case AddOnsRole:
        return entry.addOns;
    case PlayTimeTextRole:
        return entry.playTimeText;
    case IconSourceRole:
        if (entry.itemType == GameListEntry::ItemType::Game && entry.programId != 0) {
            return QStringLiteral("image://gameicon/%1").arg(entry.programId);
        }
        return QString();
    case IsFavoriteRole:
        return entry.isFavorite;
    case SectionNameRole:
        return entry.sectionName;
    case SectionTypeRole:
        return entry.sectionType;
    case ItemTypeRole:
        return static_cast<int>(entry.itemType);
    case GameDirIndexRole:
        return entry.gameDirIndex;
    case SectionExpandedRole:
        return entry.sectionExpanded;
    default:
        return {};
    }
}

QHash<int, QByteArray> GameListModel::roleNames() const {
    return {
        {NameRole, "name"},
        {FilePathRole, "filePath"},
        {ProgramIdRole, "programId"},
        {FileTypeRole, "fileType"},
        {SizeBytesRole, "sizeBytes"},
        {SizeTextRole, "sizeText"},
        {CompatibilityRole, "compatibility"},
        {CompatibilityTextRole, "compatibilityText"},
        {CompatibilityColorRole, "compatibilityColor"},
        {AddOnsRole, "addOns"},
        {PlayTimeTextRole, "playTimeText"},
        {IconSourceRole, "iconSource"},
        {IsFavoriteRole, "isFavorite"},
        {SectionNameRole, "sectionName"},
        {SectionTypeRole, "sectionType"},
        {ItemTypeRole, "itemType"},
        {GameDirIndexRole, "gameDirIndex"},
        {SectionExpandedRole, "sectionExpanded"},
    };
}

void GameListModel::AddGameEntry(const GameListEntry& entry) {
    const int row = static_cast<int>(entries_.size());
    beginInsertRows(QModelIndex(), row, row);
    entries_.append(entry);
    endInsertRows();
}

void GameListModel::AddSection(const QString& name, int type, int game_dir_index, bool expanded) {
    GameListEntry entry;
    entry.itemType = GameListEntry::ItemType::Section;
    entry.sectionName = name;
    entry.name = name;
    entry.sectionType = type;
    entry.gameDirIndex = game_dir_index;
    entry.sectionExpanded = expanded;

    const int row = static_cast<int>(entries_.size());
    beginInsertRows(QModelIndex(), row, row);
    entries_.append(entry);
    endInsertRows();
}

void GameListModel::RemoveRow(int row) {
    if (row < 0 || row >= entries_.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    entries_.removeAt(row);
    endRemoveRows();
}

void GameListModel::Clear() {
    beginResetModel();
    entries_.clear();
    endResetModel();
}

void GameListModel::DonePopulating() {
    emit populatingCompleted();
}

void GameListModel::toggleFavorite(quint64 programId) {
    for (int i = 0; i < entries_.size(); ++i) {
        if (entries_[i].itemType == GameListEntry::ItemType::Game &&
            entries_[i].programId == programId) {
            entries_[i].isFavorite = !entries_[i].isFavorite;
            const QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {IsFavoriteRole});
        }
    }
    emit favoriteToggled(programId);
}

void GameListModel::toggleSectionExpanded(int row) {
    if (row < 0 || row >= entries_.size()) {
        return;
    }
    if (entries_[row].itemType != GameListEntry::ItemType::Section) {
        return;
    }
    entries_[row].sectionExpanded = !entries_[row].sectionExpanded;
    const QModelIndex idx = index(row);
    emit dataChanged(idx, idx, {SectionExpandedRole});
}

// --- GameIconProvider ---

GameIconProvider::GameIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

QPixmap GameIconProvider::requestPixmap(const QString& id, QSize* size,
                                        const QSize& requestedSize) {
    QMutexLocker locker(&mutex_);

    bool ok = false;
    const u64 programId = id.toULongLong(&ok);

    QPixmap pixmap;
    if (ok && icons_.contains(programId)) {
        pixmap = icons_[programId];
    } else {
        // Return a transparent fallback
        const int s = requestedSize.isValid() ? requestedSize.width() : 64;
        pixmap = QPixmap(s, s);
        pixmap.fill(Qt::transparent);
    }

    if (size) {
        *size = pixmap.size();
    }

    if (requestedSize.isValid() && requestedSize != pixmap.size()) {
        pixmap = pixmap.scaled(requestedSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    return pixmap;
}

void GameIconProvider::addIcon(u64 programId, const QPixmap& pixmap) {
    QMutexLocker locker(&mutex_);
    icons_[programId] = pixmap;
}

// --- GameListSortFilterProxy ---

GameListSortFilterProxy::GameListSortFilterProxy(QObject* parent)
    : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
}

QString GameListSortFilterProxy::filterText() const {
    return filter_text_;
}

void GameListSortFilterProxy::setFilterText(const QString& text) {
    if (filter_text_ != text) {
        filter_text_ = text.toLower();
        invalidateFilter();
        updateCounts();
        emit filterTextChanged();
    }
}

int GameListSortFilterProxy::visibleCount() const {
    return visible_count_;
}

int GameListSortFilterProxy::totalCount() const {
    return total_count_;
}

bool GameListSortFilterProxy::filterAcceptsRow(int sourceRow,
                                                const QModelIndex& sourceParent) const {
    const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    const auto itemType = static_cast<GameListEntry::ItemType>(
        idx.data(GameListModel::ItemTypeRole).toInt());

    // Sections are always shown (but may be hidden by QML if empty)
    if (itemType == GameListEntry::ItemType::Section) {
        return true;
    }

    // Check if the game is in a collapsed section
    // Walk backwards to find the parent section
    for (int i = sourceRow - 1; i >= 0; --i) {
        const QModelIndex secIdx = sourceModel()->index(i, 0, sourceParent);
        const auto secType = static_cast<GameListEntry::ItemType>(
            secIdx.data(GameListModel::ItemTypeRole).toInt());
        if (secType == GameListEntry::ItemType::Section) {
            if (!secIdx.data(GameListModel::SectionExpandedRole).toBool()) {
                return false;
            }
            break;
        }
    }

    // Apply text filter
    if (!filter_text_.isEmpty()) {
        const QString name = idx.data(GameListModel::NameRole).toString().toLower();
        const QString filePath = idx.data(GameListModel::FilePathRole).toString().toLower();
        const auto programId = idx.data(GameListModel::ProgramIdRole).toULongLong();
        const QString programIdStr =
            QStringLiteral("%1").arg(programId, 16, 16, QLatin1Char{'0'});

        // Extract filename from path
        const QString fileName =
            filePath.mid(filePath.lastIndexOf(QLatin1Char{'/'}) + 1);
        const QString searchable = fileName + QLatin1Char{' '} + name;

        // Word-order-insensitive search
        const QStringList words =
            filter_text_.split(QLatin1Char{' '}, Qt::SkipEmptyParts);
        const bool matchesName = std::all_of(words.begin(), words.end(),
                                             [&searchable](const QString& w) {
                                                 return searchable.contains(w);
                                             });
        const bool matchesProgramId =
            programIdStr.size() == 16 && programIdStr.contains(filter_text_);

        if (!matchesName && !matchesProgramId) {
            return false;
        }
    }

    return true;
}

bool GameListSortFilterProxy::lessThan(const QModelIndex& left,
                                        const QModelIndex& right) const {
    // Sections maintain their original order
    const auto leftType = static_cast<GameListEntry::ItemType>(
        left.data(GameListModel::ItemTypeRole).toInt());
    const auto rightType = static_cast<GameListEntry::ItemType>(
        right.data(GameListModel::ItemTypeRole).toInt());

    if (leftType == GameListEntry::ItemType::Section ||
        rightType == GameListEntry::ItemType::Section) {
        return left.row() < right.row();
    }

    // Sort games by name
    const QString leftName = left.data(GameListModel::NameRole).toString().toLower();
    const QString rightName = right.data(GameListModel::NameRole).toString().toLower();
    return leftName < rightName;
}

void GameListSortFilterProxy::updateCounts() {
    int total = 0;
    int visible = 0;

    if (sourceModel()) {
        for (int i = 0; i < sourceModel()->rowCount(); ++i) {
            const QModelIndex idx = sourceModel()->index(i, 0);
            const auto itemType = static_cast<GameListEntry::ItemType>(
                idx.data(GameListModel::ItemTypeRole).toInt());
            if (itemType == GameListEntry::ItemType::Game) {
                ++total;
                if (filterAcceptsRow(i, QModelIndex())) {
                    ++visible;
                }
            }
        }
    }

    if (visible != visible_count_) {
        visible_count_ = visible;
        emit visibleCountChanged();
    }
    if (total != total_count_) {
        total_count_ = total;
        emit totalCountChanged();
    }
}
