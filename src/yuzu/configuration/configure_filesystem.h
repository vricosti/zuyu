// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QWidget>

class FilesystemConfigModel;
class QQuickWidget;

class ConfigureFilesystem : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureFilesystem(QWidget* parent = nullptr);
    ~ConfigureFilesystem() override;

    void ApplyConfiguration();

public slots:
    void onBrowseDirectory(const QString& target);
    void onResetCache();

private:
    QQuickWidget* quick_widget = nullptr;
    FilesystemConfigModel* fs_model = nullptr;
};
