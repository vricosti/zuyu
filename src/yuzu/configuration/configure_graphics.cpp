// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QQmlContext>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/logging/log.h"
#include "common/settings.h"
#include "core/core.h"
#include "yuzu/configuration/configure_graphics.h"
#include "yuzu/configuration/configure_graphics_model.h"
#include "yuzu/configuration/settings_model.h"
#include "yuzu/qml_bridge.h"

ConfigureGraphics::ConfigureGraphics(
    const Core::System& system_, std::vector<VkDeviceInfo::Record>& records,
    const std::function<void()>& expose_compute_option,
    const std::function<void(Settings::AspectRatio, Settings::ResolutionSetup)>&
        update_aspect_ratio_,
    std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group_,
    const ConfigurationShared::Builder& builder, QWidget* parent)
    : ConfigurationShared::Tab(group_, parent), system{system_},
      update_aspect_ratio{update_aspect_ratio_} {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Create the model for API/device/shader/VSync/bg color
    graphics_model =
        new GraphicsConfigModel(records, expose_compute_option, system.IsPoweredOn(), this);

    // Create SettingsModel for remaining Renderer settings (excluding specials)
    settings_model = new SettingsModel(this);

    // Build the list of non-special Renderer settings
    std::vector<Settings::BasicSetting*> renderer_settings;
    auto& linkage = Settings::values.linkage;
    auto it = linkage.by_category.find(Settings::Category::Renderer);
    if (it != linkage.by_category.end()) {
        for (auto* setting : it->second) {
            if (!setting->Save()) {
                continue;
            }
            // Skip settings managed by GraphicsConfigModel
            const auto id = setting->Id();
            if (id == Settings::values.renderer_backend.Id() ||
                id == Settings::values.vulkan_device.Id() ||
                id == Settings::values.shader_backend.Id() ||
                id == Settings::values.vsync_mode.Id() ||
                id == Settings::values.bg_red.Id() ||
                id == Settings::values.bg_green.Id() ||
                id == Settings::values.bg_blue.Id()) {
                continue;
            }
            renderer_settings.push_back(setting);
        }
    }
    settings_model->populate(renderer_settings);
    settings_model->setRuntimeLock(system.IsPoweredOn());

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("graphicsModel"), graphics_model);
    ctx->setContextProperty(QStringLiteral("settingsModel"), settings_model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureGraphics.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureGraphics QML Error: {}",
                      error.toString().toStdString());
        }
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureGraphics::~ConfigureGraphics() = default;

void ConfigureGraphics::SetConfiguration() {}

void ConfigureGraphics::ApplyConfiguration() {
    graphics_model->applyConfiguration();
    // SettingsModel values are already applied via LoadString
}
