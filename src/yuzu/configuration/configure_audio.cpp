// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QQmlContext>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/settings.h"
#include "core/core.h"
#include "yuzu/configuration/configuration_shared.h"
#include "yuzu/configuration/configure_audio.h"
#include "yuzu/configuration/configure_audio_model.h"
#include "yuzu/configuration/settings_model.h"
#include "yuzu/qml_bridge.h"
#include "yuzu/uisettings.h"

ConfigureAudio::ConfigureAudio(const Core::System& system_,
                               std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group_,
                               const ConfigurationShared::Builder& builder, QWidget* parent)
    : Tab(group_, parent), system{system_} {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Audio engine/device model
    audio_model = new AudioConfigModel(this);
    audio_model->initialize();

    // Other audio settings (volume, etc.)
    settings_model = new SettingsModel(this);
    settings_model->setRuntimeLock(system.IsPoweredOn());

    std::vector<Settings::BasicSetting*> settings;
    auto push = [&](Settings::Category category, auto& linkage) {
        for (auto* s : linkage.by_category[category]) {
            // Skip sink_id, output_device, input_device - handled by AudioConfigModel
            if (s->Id() == Settings::values.sink_id.Id() ||
                s->Id() == Settings::values.audio_output_device_id.Id() ||
                s->Id() == Settings::values.audio_input_device_id.Id()) {
                continue;
            }
            if (s->Save()) {
                settings.push_back(s);
            }
        }
    };
    push(Settings::Category::Audio, Settings::values.linkage);
    push(Settings::Category::SystemAudio, Settings::values.linkage);
    push(Settings::Category::UiAudio, UISettings::values.linkage);
    settings_model->populate(settings);

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("audioModel"), audio_model);
    ctx->setContextProperty(QStringLiteral("settingsModel"), settings_model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureAudio.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureAudio QML Error: {}", error.toString().toStdString());
        }
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureAudio::~ConfigureAudio() = default;

void ConfigureAudio::SetConfiguration() {}

void ConfigureAudio::ApplyConfiguration() {
    audio_model->applySinkAndDevices();
    // Other settings are written directly via SettingsModel
}
