// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QWidget>

namespace Core {
class System;
}

class QQuickWidget;
class SettingsModel;

class ConfigureDebug : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureDebug(const Core::System& system_, QWidget* parent = nullptr);
    ~ConfigureDebug() override;

    void ApplyConfiguration();

private slots:
    void onOpenLogLocation();

private:
    QQuickWidget* quick_widget = nullptr;
    SettingsModel* settings_model = nullptr;
    const Core::System& system;
};
