// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include "common/settings.h"
#include "hid_core/frontend/emulated_controller.h"
#include "hid_core/hid_core.h"
#include "hid_core/hid_types.h"
#include "yuzu/configuration/configure_vibration.h"

ConfigureVibration::ConfigureVibration(QWidget* parent, Core::HID::HIDCore& hid_core_)
    : QDialog(parent), hid_core{hid_core_} {
    setWindowTitle(tr("Configure Vibration"));

    auto* main_layout = new QVBoxLayout(this);

    // Info label
    main_layout->addWidget(
        new QLabel(tr("Press any controller button to vibrate the controller."), this));

    // Vibration group
    auto* vibration_group = new QGroupBox(tr("Vibration"), this);
    auto* vibration_layout = new QVBoxLayout(vibration_group);
    vibration_layout->setContentsMargins(9, 9, 9, 9);

    // Players 1-4
    auto* row1_layout = new QHBoxLayout;
    row1_layout->setContentsMargins(0, 0, 0, 0);

    // Players 5-8
    auto* row2_layout = new QHBoxLayout;
    row2_layout->setContentsMargins(0, 0, 0, 0);

    const auto& players = Settings::values.players.GetValue();

    for (std::size_t i = 0; i < NUM_PLAYERS; ++i) {
        auto* group = new QGroupBox(tr("Player %1").arg(i + 1), this);
        group->setCheckable(true);

        auto* group_layout = new QHBoxLayout(group);
        group_layout->setContentsMargins(3, 3, 3, 3);

        auto* spinbox = new QSpinBox(this);
        spinbox->setMinimumSize(68, 21);
        spinbox->setMaximumWidth(68);
        spinbox->setSuffix(QStringLiteral("%"));
        spinbox->setMinimum(1);
        spinbox->setMaximum(150);
        spinbox->setValue(100);
        group_layout->addWidget(spinbox);

        vibration_groupboxes[i] = group;
        vibration_spinboxes[i] = spinbox;

        if (i < 4) {
            row1_layout->addWidget(group);
        } else {
            row2_layout->addWidget(group);
        }

        auto controller = hid_core.GetEmulatedControllerByIndex(i);
        Core::HID::ControllerUpdateCallback engine_callback{
            .on_change = [this,
                          i](Core::HID::ControllerTriggerType type) { VibrateController(type, i); },
            .is_npad_service = false,
        };
        controller_callback_key[i] = controller->SetCallback(engine_callback);
        vibration_groupboxes[i]->setChecked(players[i].vibration_enabled);
        vibration_spinboxes[i]->setValue(players[i].vibration_strength);
    }

    vibration_layout->addLayout(row1_layout);
    vibration_layout->addLayout(row2_layout);
    main_layout->addWidget(vibration_group);

    // Settings group
    auto* settings_group = new QGroupBox(tr("Settings"), this);
    auto* settings_layout = new QVBoxLayout(settings_group);

    accurate_vibration_checkbox = new QCheckBox(tr("Enable Accurate Vibration"), this);
    accurate_vibration_checkbox->setChecked(
        Settings::values.enable_accurate_vibrations.GetValue());
    if (!Settings::IsConfiguringGlobal()) {
        accurate_vibration_checkbox->setDisabled(true);
    }
    settings_layout->addWidget(accurate_vibration_checkbox);

    main_layout->addWidget(settings_group);
    main_layout->addStretch();

    // Button box
    auto* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    main_layout->addWidget(button_box);
}

ConfigureVibration::~ConfigureVibration() {
    StopVibrations();

    for (std::size_t i = 0; i < NUM_PLAYERS; ++i) {
        auto controller = hid_core.GetEmulatedControllerByIndex(i);
        controller->DeleteCallback(controller_callback_key[i]);
    }
};

void ConfigureVibration::ApplyConfiguration() {
    auto& players = Settings::values.players.GetValue();

    for (std::size_t i = 0; i < NUM_PLAYERS; ++i) {
        players[i].vibration_enabled = vibration_groupboxes[i]->isChecked();
        players[i].vibration_strength = vibration_spinboxes[i]->value();
    }

    Settings::values.enable_accurate_vibrations.SetValue(
        accurate_vibration_checkbox->isChecked());
}

void ConfigureVibration::VibrateController(Core::HID::ControllerTriggerType type,
                                           std::size_t player_index) {
    if (type != Core::HID::ControllerTriggerType::Button) {
        return;
    }

    auto& player = Settings::values.players.GetValue()[player_index];
    auto controller = hid_core.GetEmulatedControllerByIndex(player_index);
    const int vibration_strength = vibration_spinboxes[player_index]->value();
    const auto& buttons = controller->GetButtonsValues();

    bool button_is_pressed = false;
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        if (buttons[i].value) {
            button_is_pressed = true;
            break;
        }
    }

    if (!button_is_pressed) {
        StopVibrations();
        return;
    }

    const bool old_vibration_enabled = player.vibration_enabled;
    const int old_vibration_strength = player.vibration_strength;
    player.vibration_enabled = true;
    player.vibration_strength = vibration_strength;

    const Core::HID::VibrationValue vibration{
        .low_amplitude = 1.0f,
        .low_frequency = 160.0f,
        .high_amplitude = 1.0f,
        .high_frequency = 320.0f,
    };
    controller->SetVibration(Core::HID::DeviceIndex::Left, vibration);
    controller->SetVibration(Core::HID::DeviceIndex::Right, vibration);

    // Restore previous values
    player.vibration_enabled = old_vibration_enabled;
    player.vibration_strength = old_vibration_strength;
}

void ConfigureVibration::StopVibrations() {
    for (std::size_t i = 0; i < NUM_PLAYERS; ++i) {
        auto controller = hid_core.GetEmulatedControllerByIndex(i);
        controller->SetVibration(Core::HID::DeviceIndex::Left, Core::HID::DEFAULT_VIBRATION_VALUE);
        controller->SetVibration(Core::HID::DeviceIndex::Right, Core::HID::DEFAULT_VIBRATION_VALUE);
    }
}
