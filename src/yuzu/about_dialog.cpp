// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QIcon>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include <fmt/format.h>
#include "common/logging/log.h"
#include "common/scm_rev.h"
#include "yuzu/about_dialog.h"
#include "yuzu/qml_bridge.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("About yuzu"));
    resize(616, 294);

    const auto branch_name = std::string(Common::g_scm_branch);
    const auto description = std::string(Common::g_scm_desc);
    const auto build_id = std::string(Common::g_build_id);

    const auto yuzu_build =
        fmt::format("yuzu Development Build | {}-{}", branch_name, description);
    const auto override_build =
        fmt::format(fmt::runtime(std::string(Common::g_title_bar_format_idle)), build_id);
    const auto yuzu_build_version = override_build.empty() ? yuzu_build : override_build;

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);

    // Build info string
    QString build_info = QString::fromStdString(yuzu_build_version);
    build_info.append(QStringLiteral(" ("));
    build_info.append(QString::fromUtf8(Common::g_build_date).left(10));
    build_info.append(QStringLiteral(")"));
    ctx->setContextProperty(QStringLiteral("aboutBuildInfo"), build_info);

    // Logo: try theme icon, fall back to resource
    const QIcon yuzu_logo = QIcon::fromTheme(QStringLiteral("org.yuzu_emu.yuzu"));
    if (!yuzu_logo.isNull()) {
        // Save to temp file for QML Image to load
        // For now, use the resource path directly
        ctx->setContextProperty(QStringLiteral("aboutLogoSource"),
                                QStringLiteral("qrc:/icons/default/256x256/yuzu.png"));
    } else {
        ctx->setContextProperty(QStringLiteral("aboutLogoSource"),
                                QStringLiteral("qrc:/icons/default/256x256/yuzu.png"));
    }

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/AboutDialog.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "AboutDialog QML Error: {}", error.toString().toStdString());
        }
    }

    // Connect QML accepted signal to QDialog accept
    QQuickItem* rootItem = quick_widget->rootObject();
    if (rootItem) {
        connect(rootItem, SIGNAL(accepted()), this, SLOT(accept()));
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

AboutDialog::~AboutDialog() = default;
