// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <vector>

#include <QList>
#include <QWidget>

#include "core/file_sys/vfs/vfs_types.h"

namespace Core {
class System;
}

class QStandardItem;
class QStandardItemModel;
class QTreeView;

class ConfigurePerGameAddons : public QWidget {
    Q_OBJECT

public:
    explicit ConfigurePerGameAddons(Core::System& system_, QWidget* parent = nullptr);
    ~ConfigurePerGameAddons() override;

    void ApplyConfiguration();

    void LoadFromFile(FileSys::VirtualFile file_);

    void SetTitleId(u64 id);

private:
    void LoadConfiguration();

    FileSys::VirtualFile file;
    u64 title_id;

    QTreeView* tree_view;
    QStandardItemModel* item_model;

    std::vector<QList<QStandardItem*>> list_items;

    Core::System& system;
};
