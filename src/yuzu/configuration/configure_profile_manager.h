// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <QWidget>

namespace Common {
struct UUID;
}

namespace Core {
class System;
}

namespace Service::Account {
class ProfileManager;
}

class QQuickWidget;
class QStandardItemModel;

class ConfigureProfileManager : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureProfileManager(Core::System& system_, QWidget* parent = nullptr);
    ~ConfigureProfileManager() override;

    void ApplyConfiguration();

public slots:
    void onSelectUser(int index);
    void onAddUser();
    void onRenameUser();
    void onDeleteUser();
    void onSetUserImage();

private:
    void PopulateUserList();
    void UpdateCurrentUser();
    void RefreshModel();

    QQuickWidget* quick_widget = nullptr;
    QStandardItemModel* user_model = nullptr;

    bool enabled = false;
    Service::Account::ProfileManager& profile_manager;
    const Core::System& system;
};
