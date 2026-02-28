// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>
#include <QWidget>

class ConfigureDebug;
class ConfigureCpuDebug;
class QTabWidget;

namespace Core {
class System;
}

class ConfigureDebugTab : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureDebugTab(const Core::System& system_, QWidget* parent = nullptr);
    ~ConfigureDebugTab() override;

    void ApplyConfiguration();

    void SetCurrentIndex(int index);

private:
    QTabWidget* tab_widget = nullptr;
    std::unique_ptr<ConfigureDebug> debug_tab;
    std::unique_ptr<ConfigureCpuDebug> cpu_debug_tab;
};
