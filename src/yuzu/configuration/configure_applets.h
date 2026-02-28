// SPDX-FileCopyrightText: 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QWidget>
#include "yuzu/configuration/configuration_shared.h"

namespace Core {
class System;
}

class QQuickWidget;
class SettingsModel;

namespace ConfigurationShared {
class Builder;
}

class ConfigureApplets : public ConfigurationShared::Tab {
public:
    explicit ConfigureApplets(Core::System& system_,
                              std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group,
                              const ConfigurationShared::Builder& builder,
                              QWidget* parent = nullptr);
    ~ConfigureApplets() override;

    void ApplyConfiguration() override;
    void SetConfiguration() override;

private:
    QQuickWidget* quick_widget = nullptr;
    SettingsModel* settings_model = nullptr;

    Core::System& system;
};
