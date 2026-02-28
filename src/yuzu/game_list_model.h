// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QMutex>
#include <QPixmap>
#include <QQuickImageProvider>
#include <QSortFilterProxyModel>
#include <QString>
#include <QVector>

#include "common/common_types.h"

/// Represents a single entry (game or section header) in the flat QML game list model.
struct GameListEntry {
    enum class ItemType {
        Game,
        Section,
    };

    ItemType itemType = ItemType::Game;

    // Game fields
    QString name;
    QString filePath;
    u64 programId = 0;
    QString fileType;
    qulonglong sizeBytes = 0;
    QString sizeText;
    QString compatibility;
    QString compatibilityText;
    QString compatibilityColor;
    QString addOns;
    QString playTimeText;
    bool isFavorite = false;

    // Section fields
    QString sectionName;
    int sectionType = 0; // maps to GameListItemType
    int gameDirIndex = -1;
    bool sectionExpanded = true;
};

/// Flat list model exposing game entries with named roles for QML.
class GameListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        FilePathRole,
        ProgramIdRole,
        FileTypeRole,
        SizeBytesRole,
        SizeTextRole,
        CompatibilityRole,
        CompatibilityTextRole,
        CompatibilityColorRole,
        AddOnsRole,
        PlayTimeTextRole,
        IconSourceRole,
        IsFavoriteRole,
        SectionNameRole,
        SectionTypeRole,
        ItemTypeRole,
        GameDirIndexRole,
        SectionExpandedRole,
    };

    explicit GameListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void AddGameEntry(const GameListEntry& entry);
    void AddSection(const QString& name, int type, int game_dir_index, bool expanded);
    void RemoveRow(int row);
    void Clear();
    void DonePopulating();

    Q_INVOKABLE void toggleFavorite(quint64 programId);
    Q_INVOKABLE void toggleSectionExpanded(int index);

    const QVector<GameListEntry>& Entries() const {
        return entries_;
    }

signals:
    void favoriteToggled(quint64 programId);
    void populatingCompleted();

private:
    QVector<GameListEntry> entries_;
};

/// Serves game icons to QML via image://gameicon/<programId> URLs.
class GameIconProvider : public QQuickImageProvider {
public:
    GameIconProvider();

    QPixmap requestPixmap(const QString& id, QSize* size,
                          const QSize& requestedSize) override;

    void addIcon(u64 programId, const QPixmap& pixmap);

private:
    mutable QMutex mutex_;
    QHash<u64, QPixmap> icons_;
};

/// Sort/filter proxy for the QML game list. Handles text filtering and section collapsing.
class GameListSortFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(int visibleCount READ visibleCount NOTIFY visibleCountChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)

public:
    explicit GameListSortFilterProxy(QObject* parent = nullptr);

    QString filterText() const;
    void setFilterText(const QString& text);

    int visibleCount() const;
    int totalCount() const;

signals:
    void filterTextChanged();
    void visibleCountChanged();
    void totalCountChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    void updateCounts();

    QString filter_text_;
    int visible_count_ = 0;
    int total_count_ = 0;
};
