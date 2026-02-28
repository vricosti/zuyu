// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QWidget>

class QQuickWidget;

namespace Core {
class System;
}

class ConfigureNetwork : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureNetwork(const Core::System& system_, QWidget* parent = nullptr);
    ~ConfigureNetwork() override;

    void ApplyConfiguration();

public slots:
    void onInterfaceChanged(const QString& name);

private:
    QQuickWidget* quick_widget = nullptr;
    QString selected_interface;

    const Core::System& system;
};
