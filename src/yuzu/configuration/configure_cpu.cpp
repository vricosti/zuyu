// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QQmlContext>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/settings.h"
#include "common/settings_enums.h"
#include "core/core.h"
#include "yuzu/configuration/configuration_shared.h"
#include "yuzu/configuration/configure_cpu.h"
#include "yuzu/configuration/settings_model.h"
#include "yuzu/qml_bridge.h"

ConfigureCpu::ConfigureCpu(const Core::System& system_,
                           std::shared_ptr<std::vector<ConfigurationShared::Tab*>> group_,
                           const ConfigurationShared::Builder& builder, QWidget* parent)
    : Tab(group_, parent), system{system_} {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // CPU settings model
    cpu_model = new SettingsModel(this);
    cpu_model->setRuntimeLock(system.IsPoweredOn());
    cpu_model->populate(Settings::Category::Cpu);

    // Unsafe settings model
    unsafe_model = new SettingsModel(this);
    unsafe_model->setRuntimeLock(system.IsPoweredOn());
    unsafe_model->populate(Settings::Category::CpuUnsafe);

    // Determine unsafe visibility
    const bool show_unsafe =
        Settings::values.cpu_accuracy.GetValue() == Settings::CpuAccuracy::Unsafe;

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("cpuModel"), cpu_model);
    ctx->setContextProperty(QStringLiteral("unsafeModel"), unsafe_model);
    ctx->setContextProperty(QStringLiteral("showUnsafe"), show_unsafe);

#ifdef HAS_NCE
    ctx->setContextProperty(QStringLiteral("showBackend"), true);
#else
    ctx->setContextProperty(QStringLiteral("showBackend"), false);
#endif

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureCpu.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureCpu QML Error: {}", error.toString().toStdString());
        }
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureCpu::~ConfigureCpu() = default;

void ConfigureCpu::SetConfiguration() {}

void ConfigureCpu::ApplyConfiguration() {
    // Settings are written directly via SettingsModel
}
