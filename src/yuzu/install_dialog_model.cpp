// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QFileInfo>
#include <QFontMetrics>
#include <QApplication>

#include "yuzu/install_dialog_model.h"

InstallFileModel::InstallFileModel(const QStringList& files, QObject* parent)
    : QAbstractListModel(parent) {
    m_entries.reserve(files.size());
    for (const QString& file : files) {
        InstallFileEntry entry;
        entry.displayName = QFileInfo(file).fileName();
        entry.fullPath = file;
        entry.checked = true;
        m_entries.append(entry);
    }
}

int InstallFileModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.size();
}

QVariant InstallFileModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return {};
    }

    const auto& entry = m_entries[index.row()];
    switch (role) {
    case DisplayNameRole:
        return entry.displayName;
    case FullPathRole:
        return entry.fullPath;
    case CheckedRole:
        return entry.checked;
    default:
        return {};
    }
}

bool InstallFileModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return false;
    }

    if (role == CheckedRole) {
        m_entries[index.row()].checked = value.toBool();
        emit dataChanged(index, index, {CheckedRole});
        return true;
    }
    return false;
}

QHash<int, QByteArray> InstallFileModel::roleNames() const {
    return {
        {DisplayNameRole, "displayName"},
        {FullPathRole, "fullPath"},
        {CheckedRole, "checked"},
    };
}

void InstallFileModel::toggleItem(int row) {
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    m_entries[row].checked = !m_entries[row].checked;
    const auto idx = index(row);
    emit dataChanged(idx, idx, {CheckedRole});
}

QStringList InstallFileModel::checkedFiles() const {
    QStringList files;
    for (const auto& entry : m_entries) {
        if (entry.checked) {
            files.append(entry.fullPath);
        }
    }
    return files;
}

int InstallFileModel::minimumWidth() const {
    const QFontMetrics fm(QApplication::font());
    int maxWidth = 0;
    for (const auto& entry : m_entries) {
        maxWidth = std::max(maxWidth, fm.horizontalAdvance(entry.displayName));
    }
    return (maxWidth * 11) / 10;
}
