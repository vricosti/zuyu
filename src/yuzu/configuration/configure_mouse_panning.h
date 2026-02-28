// SPDX-FileCopyrightText: 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDialog>

class QCheckBox;
class QLabel;
class QSpinBox;

namespace InputCommon {
class InputSubsystem;
}

class ConfigureMousePanning : public QDialog {
    Q_OBJECT

public:
    explicit ConfigureMousePanning(QWidget* parent, InputCommon::InputSubsystem* input_subsystem_,
                                   float right_stick_deadzone, float right_stick_range);
    ~ConfigureMousePanning() override;

public slots:
    void ApplyConfiguration();

private:
    void closeEvent(QCloseEvent* event) override;
    void SetConfiguration(float right_stick_deadzone, float right_stick_range);
    void SetDefaultConfiguration();

    InputCommon::InputSubsystem* input_subsystem;

    QCheckBox* enable;
    QSpinBox* x_sensitivity;
    QSpinBox* y_sensitivity;
    QSpinBox* deadzone_counterweight;
    QSpinBox* decay_strength;
    QSpinBox* min_decay;
    QLabel* warning_label;
};
