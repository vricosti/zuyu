// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;

class ConfigureTasDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigureTasDialog(QWidget* parent);
    ~ConfigureTasDialog() override;

    void ApplyConfiguration();

private:
    void LoadConfiguration();
    void SetDirectory(QLineEdit* edit);
    void HandleApplyButtonClicked();

    QCheckBox* tas_enable;
    QCheckBox* tas_loop_script;
    QCheckBox* tas_pause_on_load;
    QLineEdit* tas_path_edit;
};
