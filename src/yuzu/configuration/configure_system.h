// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>
#include <vector>

#include <QWidget>
#include "yuzu/configuration/configuration_shared.h"

namespace Core {
class System;
}

namespace ConfigurationShared {
class Builder;
}

class QQuickWidget;
class SettingsModel;
class SystemConfigModel;

class ConfigureSystem : public ConfigurationShared::Tab {
    Q_OBJECT

public:
    explicit ConfigureSystem(Core::System& system_,
                             std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group,
                             const ConfigurationShared::Builder& builder,
                             QWidget* parent = nullptr);
    ~ConfigureSystem() override;

    void ApplyConfiguration() override;
    void SetConfiguration() override;

private:
    QQuickWidget* quick_widget = nullptr;
    SettingsModel* core_model = nullptr;
    SettingsModel* system_model = nullptr;
    SystemConfigModel* system_config_model = nullptr;
    Core::System& system;
};
