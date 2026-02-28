// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QWidget>

namespace Core {
class System;
}

class QQuickWidget;
class SettingsModel;

class ConfigureCpuDebug : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureCpuDebug(const Core::System& system_, QWidget* parent = nullptr);
    ~ConfigureCpuDebug() override;

    void ApplyConfiguration();

private:
    QQuickWidget* quick_widget = nullptr;
    SettingsModel* settings_model = nullptr;
    const Core::System& system;
};
