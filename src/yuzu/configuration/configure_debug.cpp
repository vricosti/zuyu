// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QDesktopServices>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "common/fs/path_util.h"
#include "common/logging/backend.h"
#include "common/logging/filter.h"
#include "common/logging/log.h"
#include "common/settings.h"
#include "core/core.h"
#include "yuzu/configuration/configure_debug.h"
#include "yuzu/configuration/settings_model.h"
#include "yuzu/debugger/console.h"
#include "yuzu/qml_bridge.h"
#include "yuzu/uisettings.h"

ConfigureDebug::ConfigureDebug(const Core::System& system_, QWidget* parent)
    : QWidget(parent), system{system_} {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    settings_model = new SettingsModel(this);

    // Build the list of settings to show — they span multiple categories
    std::vector<Settings::BasicSetting*> debug_settings{
        &Settings::values.use_gdbstub,
        &Settings::values.gdbstub_port,
        &Settings::values.log_filter,
        &Settings::values.program_args,
        &Settings::values.enable_fs_access_log,
        &Settings::values.reporting_services,
        &Settings::values.dump_audio_commands,
        &Settings::values.quest_flag,
        &Settings::values.use_debug_asserts,
        &Settings::values.use_auto_stub,
        &Settings::values.enable_all_controllers,
        &Settings::values.renderer_debug,
        &Settings::values.enable_renderdoc_hotkey,
        &Settings::values.disable_buffer_reorder,
        &Settings::values.renderer_shader_feedback,
        &Settings::values.cpu_debug_mode,
        &Settings::values.enable_nsight_aftermath,
        &Settings::values.dump_shaders,
        &Settings::values.dump_macros,
        &Settings::values.disable_shader_loop_safety_checks,
        &Settings::values.disable_macro_jit,
        &Settings::values.disable_macro_hle,
        &Settings::values.extended_logging,
        &Settings::values.perform_vulkan_check,
        &UISettings::values.show_console,
        &UISettings::values.disable_web_applet,
    };

    settings_model->populate(debug_settings);
    settings_model->setRuntimeLock(system.IsPoweredOn());

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("settingsModel"), settings_model);
    ctx->setContextProperty(QStringLiteral("pageTitle"), tr("Debug"));

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureDebug.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureDebug QML Error: {}",
                      error.toString().toStdString());
        }
    }

    // Connect "Open Log Location" signal from QML
    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        connect(root, SIGNAL(openLogLocation()), this, SLOT(onOpenLogLocation()));
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureDebug::~ConfigureDebug() = default;

void ConfigureDebug::onOpenLogLocation() {
    const auto path =
        QString::fromStdString(Common::FS::GetYuzuPathString(Common::FS::YuzuPath::LogDir));
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void ConfigureDebug::ApplyConfiguration() {
    // Values are already written to settings via LoadString in SettingsModel.
    // Apply side effects:
    Debugger::ToggleConsole();
    Common::Log::Filter filter;
    filter.ParseFilterString(Settings::values.log_filter.GetValue());
    Common::Log::SetGlobalFilter(filter);
}
