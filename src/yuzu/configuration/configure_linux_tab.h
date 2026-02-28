// SPDX-FileCopyrightText: Copyright 2023 yuzu Emulator Project
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

class ConfigureLinuxTab : public ConfigurationShared::Tab {
    Q_OBJECT

public:
    explicit ConfigureLinuxTab(const Core::System& system_,
                               std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group,
                               const ConfigurationShared::Builder& builder,
                               QWidget* parent = nullptr);
    ~ConfigureLinuxTab() override;

    void ApplyConfiguration() override;
    void SetConfiguration() override;

private:
    QQuickWidget* quick_widget = nullptr;
    SettingsModel* settings_model = nullptr;
    const Core::System& system;
};
