// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/logging/log.h"
#include "common/settings.h"
#include "core/core.h"
#include "yuzu/configuration/configure_system.h"
#include "yuzu/configuration/configure_system_model.h"
#include "yuzu/configuration/settings_model.h"
#include "yuzu/qml_bridge.h"

ConfigureSystem::ConfigureSystem(
    Core::System& system_,
    std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group,
    const ConfigurationShared::Builder& builder, QWidget* parent)
    : Tab(group, parent), system{system_} {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Core settings model
    core_model = new SettingsModel(this);
    core_model->populate(Settings::Category::Core);
    core_model->setRuntimeLock(system.IsPoweredOn());

    // System settings model - filter out RTC-related and special settings
    std::vector<Settings::BasicSetting*> system_settings;
    auto& linkage = Settings::values.linkage;
    auto it = linkage.by_category.find(Settings::Category::System);
    if (it != linkage.by_category.end()) {
        for (auto* setting : it->second) {
            // Skip RTC settings - handled by SystemConfigModel
            if (setting->Id() == Settings::values.custom_rtc_enabled.Id() ||
                setting->Id() == Settings::values.custom_rtc.Id() ||
                setting->Id() == Settings::values.custom_rtc_offset.Id()) {
                continue;
            }
            // Skip use_docked_mode when configuring global
            if (setting->Id() == Settings::values.use_docked_mode.Id() &&
                Settings::IsConfiguringGlobal()) {
                continue;
            }
            // Skip current_user (managed by profile tab)
            if (setting->Id() == Settings::values.current_user.Id()) {
                continue;
            }
            if (!setting->Save()) {
                continue;
            }
            system_settings.push_back(setting);
        }
    }
    system_model = new SettingsModel(this);
    system_model->populate(system_settings);
    system_model->setRuntimeLock(system.IsPoweredOn());

    // System config model for RTC and locale
    system_config_model = new SystemConfigModel(system, this);
    system_config_model->initialize();

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("coreSettingsModel"), core_model);
    ctx->setContextProperty(QStringLiteral("systemSettingsModel"), system_model);
    ctx->setContextProperty(QStringLiteral("systemConfigModel"), system_config_model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureSystem.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureSystem QML Error: {}",
                      error.toString().toStdString());
        }
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureSystem::~ConfigureSystem() = default;

void ConfigureSystem::SetConfiguration() {}

void ConfigureSystem::ApplyConfiguration() {
    const bool powered_on = system.IsPoweredOn();
    system_config_model->apply(powered_on);
}
