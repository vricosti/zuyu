// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "common/settings.h"
#include "yuzu/configuration/configure_touchscreen_advanced.h"

ConfigureTouchscreenAdvanced::ConfigureTouchscreenAdvanced(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Configure Touchscreen"));

    auto* main_layout = new QVBoxLayout(this);

    // Warning label
    auto* warning_label = new QLabel(
        tr("Warning: The settings in this page affect the inner workings of yuzu's emulated "
           "touchscreen. Changing them may result in undesirable behavior, such as the touchscreen "
           "partially or not working. You should only use this page if you know what you are "
           "doing."),
        this);
    warning_label->setWordWrap(true);
    warning_label->setMinimumWidth(280);
    main_layout->addWidget(warning_label);

    main_layout->addSpacing(20);

    // Touch Parameters group
    auto* params_group = new QGroupBox(tr("Touch Parameters"), this);
    auto* grid = new QGridLayout(params_group);

    grid->addWidget(new QLabel(tr("Touch Diameter X"), this), 0, 0);
    diameter_x_box = new QSpinBox(this);
    grid->addWidget(diameter_x_box, 0, 1);

    grid->addWidget(new QLabel(tr("Touch Diameter Y"), this), 1, 0);
    diameter_y_box = new QSpinBox(this);
    grid->addWidget(diameter_y_box, 1, 1);

    grid->addWidget(new QLabel(tr("Rotational Angle"), this), 2, 0);
    angle_box = new QSpinBox(this);
    grid->addWidget(angle_box, 2, 1);

    main_layout->addWidget(params_group);
    main_layout->addStretch();

    // Bottom: Restore Defaults + OK/Cancel
    auto* bottom_layout = new QHBoxLayout;
    auto* restore_button = new QPushButton(tr("Restore Defaults"), this);
    connect(restore_button, &QPushButton::clicked, this,
            &ConfigureTouchscreenAdvanced::RestoreDefaults);
    bottom_layout->addWidget(restore_button);

    auto* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    bottom_layout->addWidget(button_box);

    main_layout->addLayout(bottom_layout);

    LoadConfiguration();
    resize(0, 0);
}

ConfigureTouchscreenAdvanced::~ConfigureTouchscreenAdvanced() = default;

void ConfigureTouchscreenAdvanced::ApplyConfiguration() {
    Settings::values.touchscreen.diameter_x = diameter_x_box->value();
    Settings::values.touchscreen.diameter_y = diameter_y_box->value();
    Settings::values.touchscreen.rotation_angle = angle_box->value();
}

void ConfigureTouchscreenAdvanced::LoadConfiguration() {
    diameter_x_box->setValue(Settings::values.touchscreen.diameter_x);
    diameter_y_box->setValue(Settings::values.touchscreen.diameter_y);
    angle_box->setValue(Settings::values.touchscreen.rotation_angle);
}

void ConfigureTouchscreenAdvanced::RestoreDefaults() {
    diameter_x_box->setValue(15);
    diameter_y_box->setValue(15);
    angle_box->setValue(0);
}
