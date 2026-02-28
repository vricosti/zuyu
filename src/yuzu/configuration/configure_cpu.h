// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <vector>
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

class ConfigureCpu : public ConfigurationShared::Tab {
    Q_OBJECT

public:
    explicit ConfigureCpu(const Core::System& system_,
                          std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group,
                          const ConfigurationShared::Builder& builder, QWidget* parent = nullptr);
    ~ConfigureCpu() override;

    void ApplyConfiguration() override;
    void SetConfiguration() override;

private:
    QQuickWidget* quick_widget = nullptr;
    SettingsModel* cpu_model = nullptr;
    SettingsModel* unsafe_model = nullptr;

    const Core::System& system;
};
