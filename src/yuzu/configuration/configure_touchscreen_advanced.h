// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDialog>

class QSpinBox;

class ConfigureTouchscreenAdvanced : public QDialog {
    Q_OBJECT

public:
    explicit ConfigureTouchscreenAdvanced(QWidget* parent);
    ~ConfigureTouchscreenAdvanced() override;

    void ApplyConfiguration();

private:
    void LoadConfiguration();
    void RestoreDefaults();

    QSpinBox* diameter_x_box;
    QSpinBox* diameter_y_box;
    QSpinBox* angle_box;
};
