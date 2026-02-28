// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <functional>
#include <vector>
#include <QWidget>
#include "yuzu/configuration/configuration_shared.h"

namespace Core {
class System;
}

class ConfigureDialog;
class HotkeyRegistry;
class QQuickWidget;
class SettingsModel;

namespace ConfigurationShared {
class Builder;
}

class ConfigureGeneral : public ConfigurationShared::Tab {
    Q_OBJECT

public:
    explicit ConfigureGeneral(const Core::System& system_,
                              std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group,
                              const ConfigurationShared::Builder& builder,
                              QWidget* parent = nullptr);
    ~ConfigureGeneral() override;

    void SetResetCallback(std::function<void()> callback);
    void ApplyConfiguration() override;
    void SetConfiguration() override;

public slots:
    void ResetDefaults();

private:
    std::function<void()> reset_callback;

    QQuickWidget* quick_widget = nullptr;
    SettingsModel* settings_model = nullptr;
    SettingsModel* linux_settings_model = nullptr;

    const Core::System& system;
};
