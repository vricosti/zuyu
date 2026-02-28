// SPDX-FileCopyrightText: 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCheckBox>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "common/settings.h"
#include "yuzu/configuration/configure_mouse_panning.h"

ConfigureMousePanning::ConfigureMousePanning(QWidget* parent,
                                             InputCommon::InputSubsystem* input_subsystem_,
                                             float right_stick_deadzone, float right_stick_range)
    : QDialog(parent), input_subsystem{input_subsystem_} {
    setWindowTitle(tr("Configure mouse panning"));

    auto* main_layout = new QVBoxLayout(this);

    // Enable checkbox
    enable = new QCheckBox(tr("Enable mouse panning"), this);
    enable->setToolTip(tr("Can be toggled via a hotkey. Default hotkey is Ctrl + F9"));
    main_layout->addWidget(enable);

    // Three group boxes side by side
    auto* groups_layout = new QHBoxLayout;

    // Sensitivity group
    auto* sensitivity_group = new QGroupBox(tr("Sensitivity"), this);
    auto* sens_layout = new QGridLayout(sensitivity_group);

    sens_layout->addWidget(new QLabel(tr("Horizontal"), this), 0, 0);
    x_sensitivity = new QSpinBox(this);
    x_sensitivity->setAlignment(Qt::AlignCenter);
    x_sensitivity->setSuffix(QStringLiteral("%"));
    x_sensitivity->setMinimum(1);
    x_sensitivity->setMaximum(100);
    sens_layout->addWidget(x_sensitivity, 0, 1);

    sens_layout->addWidget(new QLabel(tr("Vertical"), this), 1, 0);
    y_sensitivity = new QSpinBox(this);
    y_sensitivity->setAlignment(Qt::AlignCenter);
    y_sensitivity->setSuffix(QStringLiteral("%"));
    y_sensitivity->setMinimum(1);
    y_sensitivity->setMaximum(100);
    sens_layout->addWidget(y_sensitivity, 1, 1);

    groups_layout->addWidget(sensitivity_group);

    // Deadzone counterweight group
    auto* deadzone_group = new QGroupBox(tr("Deadzone counterweight"), this);
    deadzone_group->setToolTip(tr("Counteracts a game's built-in deadzone"));
    auto* dz_layout = new QGridLayout(deadzone_group);

    dz_layout->addWidget(new QLabel(tr("Deadzone"), this), 0, 0);
    deadzone_counterweight = new QSpinBox(this);
    deadzone_counterweight->setAlignment(Qt::AlignCenter);
    deadzone_counterweight->setSuffix(QStringLiteral("%"));
    deadzone_counterweight->setMinimum(0);
    deadzone_counterweight->setMaximum(100);
    dz_layout->addWidget(deadzone_counterweight, 0, 1);

    groups_layout->addWidget(deadzone_group);

    // Stick decay group
    auto* decay_group = new QGroupBox(tr("Stick decay"), this);
    auto* decay_layout = new QGridLayout(decay_group);

    decay_layout->addWidget(new QLabel(tr("Strength"), this), 0, 0);
    decay_strength = new QSpinBox(this);
    decay_strength->setAlignment(Qt::AlignCenter);
    decay_strength->setSuffix(QStringLiteral("%"));
    decay_strength->setMinimum(0);
    decay_strength->setMaximum(100);
    decay_layout->addWidget(decay_strength, 0, 1);

    decay_layout->addWidget(new QLabel(tr("Minimum"), this), 1, 0);
    min_decay = new QSpinBox(this);
    min_decay->setAlignment(Qt::AlignCenter);
    min_decay->setSuffix(QStringLiteral("%"));
    min_decay->setMinimum(0);
    min_decay->setMaximum(100);
    decay_layout->addWidget(min_decay, 1, 1);

    groups_layout->addWidget(decay_group);

    main_layout->addLayout(groups_layout);

    // Warning label
    warning_label = new QLabel(this);
    main_layout->addWidget(warning_label);

    // Bottom: Default + OK/Cancel
    auto* bottom_layout = new QHBoxLayout;

    auto* default_button = new QPushButton(tr("Default"), this);
    connect(default_button, &QPushButton::clicked, this,
            &ConfigureMousePanning::SetDefaultConfiguration);
    bottom_layout->addWidget(default_button);

    auto* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this,
            &ConfigureMousePanning::ApplyConfiguration);
    connect(button_box, &QDialogButtonBox::rejected, this, [this] { reject(); });
    bottom_layout->addWidget(button_box);

    main_layout->addLayout(bottom_layout);

    SetConfiguration(right_stick_deadzone, right_stick_range);
}

ConfigureMousePanning::~ConfigureMousePanning() = default;

void ConfigureMousePanning::closeEvent(QCloseEvent* event) {
    event->accept();
}

void ConfigureMousePanning::SetConfiguration(float right_stick_deadzone, float right_stick_range) {
    enable->setChecked(Settings::values.mouse_panning.GetValue());
    x_sensitivity->setValue(Settings::values.mouse_panning_x_sensitivity.GetValue());
    y_sensitivity->setValue(Settings::values.mouse_panning_y_sensitivity.GetValue());
    deadzone_counterweight->setValue(
        Settings::values.mouse_panning_deadzone_counterweight.GetValue());
    decay_strength->setValue(Settings::values.mouse_panning_decay_strength.GetValue());
    min_decay->setValue(Settings::values.mouse_panning_min_decay.GetValue());

    if (right_stick_deadzone > 0.0f || right_stick_range != 1.0f) {
        const QString right_stick_deadzone_str =
            QString::fromStdString(std::to_string(static_cast<int>(right_stick_deadzone * 100.0f)));
        const QString right_stick_range_str =
            QString::fromStdString(std::to_string(static_cast<int>(right_stick_range * 100.0f)));

        warning_label->setText(
            tr("Mouse panning works better with a deadzone of 0% and a range of 100%.\nCurrent "
               "values are %1% and %2% respectively.")
                .arg(right_stick_deadzone_str, right_stick_range_str));
    }

    if (Settings::values.mouse_enabled) {
        warning_label->setText(
            tr("Emulated mouse is enabled. This is incompatible with mouse panning."));
    }
}

void ConfigureMousePanning::SetDefaultConfiguration() {
    x_sensitivity->setValue(Settings::values.mouse_panning_x_sensitivity.GetDefault());
    y_sensitivity->setValue(Settings::values.mouse_panning_y_sensitivity.GetDefault());
    deadzone_counterweight->setValue(
        Settings::values.mouse_panning_deadzone_counterweight.GetDefault());
    decay_strength->setValue(Settings::values.mouse_panning_decay_strength.GetDefault());
    min_decay->setValue(Settings::values.mouse_panning_min_decay.GetDefault());
}

void ConfigureMousePanning::ApplyConfiguration() {
    Settings::values.mouse_panning = enable->isChecked();
    Settings::values.mouse_panning_x_sensitivity = static_cast<float>(x_sensitivity->value());
    Settings::values.mouse_panning_y_sensitivity = static_cast<float>(y_sensitivity->value());
    Settings::values.mouse_panning_deadzone_counterweight =
        static_cast<float>(deadzone_counterweight->value());
    Settings::values.mouse_panning_decay_strength = static_cast<float>(decay_strength->value());
    Settings::values.mouse_panning_min_decay = static_cast<float>(min_decay->value());

    if (Settings::values.mouse_enabled && Settings::values.mouse_panning) {
        Settings::values.mouse_panning = false;
        QMessageBox::critical(
            this, tr("Emulated mouse is enabled"),
            tr("Real mouse input and mouse panning are incompatible. Please disable the "
               "emulated mouse in input advanced settings to allow mouse panning."));
        return;
    }

    accept();
}
