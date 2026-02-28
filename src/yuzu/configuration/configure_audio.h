// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <vector>
#include <QWidget>
#include "yuzu/configuration/configuration_shared.h"

namespace Core {
class System;
}

class AudioConfigModel;
class QQuickWidget;
class SettingsModel;

namespace ConfigurationShared {
class Builder;
}

class ConfigureAudio : public ConfigurationShared::Tab {
    Q_OBJECT

public:
    explicit ConfigureAudio(const Core::System& system_,
                            std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group,
                            const ConfigurationShared::Builder& builder, QWidget* parent = nullptr);
    ~ConfigureAudio() override;

    void ApplyConfiguration() override;
    void SetConfiguration() override;

private:
    QQuickWidget* quick_widget = nullptr;
    SettingsModel* settings_model = nullptr;
    AudioConfigModel* audio_model = nullptr;

    const Core::System& system;
};
