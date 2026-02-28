// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QStringList>

struct InstallFileEntry {
    QString displayName;
    QString fullPath;
    bool checked = true;
};

class InstallFileModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        DisplayNameRole = Qt::UserRole + 1,
        FullPathRole,
        CheckedRole,
    };

    explicit InstallFileModel(const QStringList& files, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void toggleItem(int row);

    QStringList checkedFiles() const;

    int minimumWidth() const;

private:
    QList<InstallFileEntry> m_entries;
};
