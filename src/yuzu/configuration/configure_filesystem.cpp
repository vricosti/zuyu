// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QFileDialog>
#include <QMessageBox>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "common/fs/fs.h"
#include "common/fs/path_util.h"
#include "yuzu/configuration/configure_filesystem.h"
#include "yuzu/configuration/configure_filesystem_model.h"
#include "yuzu/qml_bridge.h"
#include "yuzu/uisettings.h"

ConfigureFilesystem::ConfigureFilesystem(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    fs_model = new FilesystemConfigModel(this);
    fs_model->initialize();

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("fsModel"), fs_model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureFilesystem.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureFilesystem QML Error: {}",
                      error.toString().toStdString());
        }
    }

    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        connect(root, SIGNAL(browseDirectory(QString)), this,
                SLOT(onBrowseDirectory(QString)));
        connect(root, SIGNAL(resetCacheClicked()), this, SLOT(onResetCache()));
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureFilesystem::~ConfigureFilesystem() = default;

void ConfigureFilesystem::ApplyConfiguration() {
    fs_model->apply();
}

void ConfigureFilesystem::onBrowseDirectory(const QString& target) {
    QString caption;
    QString current_path;
    bool is_file = false;

    if (target == QStringLiteral("nand")) {
        caption = tr("Select Emulated NAND Directory...");
        current_path = fs_model->nandDir();
    } else if (target == QStringLiteral("sdmc")) {
        caption = tr("Select Emulated SD Directory...");
        current_path = fs_model->sdmcDir();
    } else if (target == QStringLiteral("gamecard")) {
        caption = tr("Select Gamecard Path...");
        current_path = fs_model->gamecardPath();
        is_file = true;
    } else if (target == QStringLiteral("dump")) {
        caption = tr("Select Dump Directory...");
        current_path = fs_model->dumpDir();
    } else if (target == QStringLiteral("load")) {
        caption = tr("Select Mod Load Directory...");
        current_path = fs_model->loadDir();
    }

    QString str;
    if (is_file) {
        str = QFileDialog::getOpenFileName(this, caption,
                                           QFileInfo(current_path).dir().path(),
                                           QStringLiteral("NX Gamecard;*.xci"));
    } else {
        str = QFileDialog::getExistingDirectory(this, caption, current_path);
    }

    if (str.isNull() || str.isEmpty()) {
        return;
    }

    if (!is_file && str.back() != QChar::fromLatin1('/')) {
        str.append(QChar::fromLatin1('/'));
    }

    if (target == QStringLiteral("nand")) {
        fs_model->setNandDir(str);
    } else if (target == QStringLiteral("sdmc")) {
        fs_model->setSdmcDir(str);
    } else if (target == QStringLiteral("gamecard")) {
        fs_model->setGamecardPath(str);
    } else if (target == QStringLiteral("dump")) {
        fs_model->setDumpDir(str);
    } else if (target == QStringLiteral("load")) {
        fs_model->setLoadDir(str);
    }
}

void ConfigureFilesystem::onResetCache() {
    if (!Common::FS::Exists(Common::FS::GetYuzuPath(Common::FS::YuzuPath::CacheDir) /
                            "game_list/")) {
        QMessageBox::information(this, tr("Reset Metadata Cache"),
                                 tr("The metadata cache is already empty."));
    } else if (Common::FS::RemoveDirRecursively(
                   Common::FS::GetYuzuPath(Common::FS::YuzuPath::CacheDir) / "game_list")) {
        QMessageBox::information(this, tr("Reset Metadata Cache"),
                                 tr("The operation completed successfully."));
        UISettings::values.is_game_list_reload_pending.exchange(true);
    } else {
        QMessageBox::warning(
            this, tr("Reset Metadata Cache"),
            tr("The metadata cache couldn't be deleted. It might be in use or non-existent."));
    }
}
