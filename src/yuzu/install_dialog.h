// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDialog>

class InstallFileModel;
class QQuickWidget;

class InstallDialog : public QDialog {
    Q_OBJECT

public:
    explicit InstallDialog(QWidget* parent, const QStringList& files);
    ~InstallDialog() override;

    [[nodiscard]] QStringList GetFiles() const;
    [[nodiscard]] int GetMinimumWidth() const;

private:
    InstallFileModel* file_model = nullptr;
    QQuickWidget* quick_widget = nullptr;
};
