// SPDX-FileCopyrightText: 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QQmlContext>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/settings.h"
#include "core/core.h"
#include "yuzu/configuration/configuration_shared.h"
#include "yuzu/configuration/configure_applets.h"
#include "yuzu/configuration/settings_model.h"
#include "yuzu/qml_bridge.h"

ConfigureApplets::ConfigureApplets(Core::System& system_,
                                   std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group_,
                                   const ConfigurationShared::Builder& builder, QWidget* parent)
    : Tab(group_, parent), system{system_} {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    settings_model = new SettingsModel(this);
    settings_model->setRuntimeLock(system.IsPoweredOn());

    std::vector<Settings::BasicSetting*> settings;
    for (auto* s : Settings::values.linkage.by_category[Settings::Category::LibraryApplet]) {
        if (s->Save()) {
            // Skip untested applets
            if (s->Id() == Settings::values.data_erase_applet_mode.Id() ||
                s->Id() == Settings::values.net_connect_applet_mode.Id() ||
                s->Id() == Settings::values.shop_applet_mode.Id() ||
                s->Id() == Settings::values.login_share_applet_mode.Id() ||
                s->Id() == Settings::values.wifi_web_auth_applet_mode.Id() ||
                s->Id() == Settings::values.my_page_applet_mode.Id()) {
                continue;
            }
            settings.push_back(s);
        }
    }
    settings_model->populate(settings);

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("settingsModel"), settings_model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureApplets.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureApplets QML Error: {}",
                      error.toString().toStdString());
        }
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureApplets::~ConfigureApplets() = default;

void ConfigureApplets::SetConfiguration() {}

void ConfigureApplets::ApplyConfiguration() {
    // Settings are written directly via SettingsModel
}
