// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/logging/log.h"
#include "yuzu/install_dialog.h"
#include "yuzu/install_dialog_model.h"
#include "yuzu/qml_bridge.h"

InstallDialog::InstallDialog(QWidget* parent, const QStringList& files) : QDialog(parent) {
    setWindowTitle(tr("Install Files to NAND"));

    file_model = new InstallFileModel(files, this);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("installFileModel"), file_model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/InstallDialog.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "InstallDialog QML Error: {}", error.toString().toStdString());
        }
    }

    // Connect QML signals
    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        connect(root, SIGNAL(accepted()), this, SLOT(accept()));
        connect(root, SIGNAL(rejected()), this, SLOT(reject()));
    }

    layout->addWidget(quick_widget);
    setLayout(layout);

    // Set minimum width based on file names
    const int minWidth = file_model->minimumWidth();
    if (minWidth > 0) {
        setMinimumWidth(minWidth);
    }
}

InstallDialog::~InstallDialog() = default;

QStringList InstallDialog::GetFiles() const {
    return file_model->checkedFiles();
}

int InstallDialog::GetMinimumWidth() const {
    return quick_widget->width();
}
