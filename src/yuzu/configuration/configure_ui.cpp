// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QFileDialog>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/fs/path_util.h"
#include "common/logging/log.h"
#include "common/settings.h"
#include "core/core.h"
#include "yuzu/configuration/configure_ui.h"
#include "yuzu/configuration/configure_ui_model.h"
#include "yuzu/qml_bridge.h"
#include "yuzu/uisettings.h"

ConfigureUi::ConfigureUi(Core::System& system_, QWidget* parent)
    : QWidget(parent), system{system_} {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    ui_model = new UiConfigModel(this);
    ui_model->initialize(Settings::values.aspect_ratio.GetValue(),
                         Settings::values.resolution_setup.GetValue());

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("uiModel"), ui_model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureUi.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureUi QML Error: {}",
                      error.toString().toStdString());
        }
    }

    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        connect(root, SIGNAL(browseScreenshotPath()), this, SLOT(onBrowseScreenshotPath()));
    }

    connect(ui_model, &UiConfigModel::languageChanged, this, &ConfigureUi::LanguageChanged);

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureUi::~ConfigureUi() = default;

void ConfigureUi::ApplyConfiguration() {
    ui_model->apply();
    system.ApplySettings();
}

void ConfigureUi::UpdateScreenshotInfo(Settings::AspectRatio ratio,
                                       Settings::ResolutionSetup resolution_info) {
    ui_model->updateScreenshotInfo(ratio, resolution_info);
}

void ConfigureUi::onBrowseScreenshotPath() {
    auto dir = QFileDialog::getExistingDirectory(
        this, tr("Select Screenshots Path..."),
        QString::fromStdString(
            Common::FS::GetYuzuPathString(Common::FS::YuzuPath::ScreenshotsDir)));
    if (!dir.isEmpty()) {
        if (dir.back() != QChar::fromLatin1('/')) {
            dir.append(QChar::fromLatin1('/'));
        }
        ui_model->setScreenshotPath(dir);
    }
}
