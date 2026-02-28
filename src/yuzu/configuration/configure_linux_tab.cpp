// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QQmlContext>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/logging/log.h"
#include "common/settings.h"
#include "core/core.h"
#include "yuzu/configuration/configure_linux_tab.h"
#include "yuzu/configuration/settings_model.h"
#include "yuzu/qml_bridge.h"

ConfigureLinuxTab::ConfigureLinuxTab(
    const Core::System& system_,
    std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group_,
    const ConfigurationShared::Builder& builder, QWidget* parent)
    : Tab(group_, parent), system{system_} {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    settings_model = new SettingsModel(this);
    settings_model->populate(Settings::Category::Linux);
    settings_model->setRuntimeLock(system.IsPoweredOn());

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("settingsModel"), settings_model);
    ctx->setContextProperty(QStringLiteral("pageTitle"), tr("Linux"));

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/SettingsPage.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureLinuxTab QML Error: {}",
                      error.toString().toStdString());
        }
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureLinuxTab::~ConfigureLinuxTab() = default;

void ConfigureLinuxTab::SetConfiguration() {}

void ConfigureLinuxTab::ApplyConfiguration() {
    // Values are already written to settings via LoadString in SettingsModel
}
