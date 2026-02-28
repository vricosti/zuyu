// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <functional>
#include <utility>
#include <QMessageBox>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/settings.h"
#include "core/core.h"
#include "yuzu/configuration/configuration_shared.h"
#include "yuzu/configuration/configure_general.h"
#include "yuzu/configuration/settings_model.h"
#include "yuzu/qml_bridge.h"
#include "yuzu/uisettings.h"

ConfigureGeneral::ConfigureGeneral(const Core::System& system_,
                                   std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group_,
                                   const ConfigurationShared::Builder& builder, QWidget* parent)
    : Tab(group_, parent), system{system_} {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Create settings models
    settings_model = new SettingsModel(this);
    settings_model->setRuntimeLock(system.IsPoweredOn());

    std::vector<Settings::BasicSetting*> general_settings;
    for (auto* s : UISettings::values.linkage.by_category[Settings::Category::UiGeneral]) {
        if (s->Save()) {
            general_settings.push_back(s);
        }
    }
    settings_model->populate(general_settings);

    // Linux settings
    linux_settings_model = new SettingsModel(this);
    linux_settings_model->setRuntimeLock(system.IsPoweredOn());

    std::vector<Settings::BasicSetting*> linux_settings;
    for (auto* s : Settings::values.linkage.by_category[Settings::Category::Linux]) {
        if (s->Save()) {
            linux_settings.push_back(s);
        }
    }
    linux_settings_model->populate(linux_settings);

    // Set up QML
    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("settingsModel"), settings_model);
    ctx->setContextProperty(QStringLiteral("linuxSettingsModel"), linux_settings_model);
    ctx->setContextProperty(QStringLiteral("showResetButton"),
                            Settings::IsConfiguringGlobal());
#ifdef __unix__
    ctx->setContextProperty(QStringLiteral("showLinuxGroup"), true);
#else
    ctx->setContextProperty(QStringLiteral("showLinuxGroup"), false);
#endif

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureGeneral.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureGeneral QML Error: {}",
                      error.toString().toStdString());
        }
    }

    // Connect reset signal
    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        connect(root, SIGNAL(resetDefaultsClicked()), this, SLOT(ResetDefaults()));
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureGeneral::~ConfigureGeneral() = default;

void ConfigureGeneral::SetConfiguration() {}

void ConfigureGeneral::SetResetCallback(std::function<void()> callback) {
    reset_callback = std::move(callback);
}

void ConfigureGeneral::ResetDefaults() {
    QMessageBox::StandardButton answer = QMessageBox::question(
        this, tr("yuzu"),
        tr("This reset all settings and remove all per-game configurations. This will not delete "
           "game directories, profiles, or input profiles. Proceed?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::No) {
        return;
    }
    UISettings::values.reset_to_defaults = true;
    UISettings::values.is_game_list_reload_pending.exchange(true);
    if (reset_callback) {
        reset_callback();
    }
}

void ConfigureGeneral::ApplyConfiguration() {
    // Settings are written directly via SettingsModel::setValue/LoadString
    // No additional apply step needed
}
