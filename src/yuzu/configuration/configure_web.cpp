// SPDX-FileCopyrightText: 2017 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QMessageBox>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/logging/log.h"
#include "yuzu/configuration/configure_web.h"
#include "yuzu/configuration/configure_web_model.h"
#include "yuzu/qml_bridge.h"

ConfigureWeb::ConfigureWeb(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    web_model = new WebConfigModel(this);
    web_model->initialize();

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("webModel"), web_model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureWeb.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureWeb QML Error: {}",
                      error.toString().toStdString());
        }
    }

    connect(web_model, &WebConfigModel::verifyFailed, this, &ConfigureWeb::onVerifyFailed);

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureWeb::~ConfigureWeb() = default;

void ConfigureWeb::ApplyConfiguration() {
    web_model->apply();
}

void ConfigureWeb::SetWebServiceConfigEnabled(bool enabled) {
    web_model->setWebServiceEnabled(enabled);
}

void ConfigureWeb::onVerifyFailed(const QString& message) {
    QMessageBox::critical(this, tr("Verification failed"), message);
}
