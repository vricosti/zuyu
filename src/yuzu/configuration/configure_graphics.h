// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <functional>
#include <vector>
#include <QWidget>
#include "common/settings_enums.h"
#include "vk_device_info.h"
#include "yuzu/configuration/configuration_shared.h"

namespace Core {
class System;
}

namespace ConfigurationShared {
class Builder;
}

class QQuickWidget;
class GraphicsConfigModel;
class SettingsModel;

class ConfigureGraphics : public ConfigurationShared::Tab {
    Q_OBJECT

public:
    explicit ConfigureGraphics(
        const Core::System& system_, std::vector<VkDeviceInfo::Record>& records,
        const std::function<void()>& expose_compute_option,
        const std::function<void(Settings::AspectRatio, Settings::ResolutionSetup)>&
            update_aspect_ratio,
        std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group,
        const ConfigurationShared::Builder& builder, QWidget* parent = nullptr);
    ~ConfigureGraphics() override;

    void ApplyConfiguration() override;
    void SetConfiguration() override;

private:
    QQuickWidget* quick_widget = nullptr;
    GraphicsConfigModel* graphics_model = nullptr;
    SettingsModel* settings_model = nullptr;
    const Core::System& system;
    const std::function<void(Settings::AspectRatio, Settings::ResolutionSetup)> update_aspect_ratio;
};
