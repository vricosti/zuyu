// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>
#include <QDialog>

class QCheckBox;
class QGroupBox;
class QSpinBox;

namespace Core::HID {
enum class ControllerTriggerType;
class HIDCore;
} // namespace Core::HID

class ConfigureVibration : public QDialog {
    Q_OBJECT

public:
    explicit ConfigureVibration(QWidget* parent, Core::HID::HIDCore& hid_core_);
    ~ConfigureVibration() override;

    void ApplyConfiguration();

private:
    void VibrateController(Core::HID::ControllerTriggerType type, std::size_t player_index);
    void StopVibrations();

    static constexpr std::size_t NUM_PLAYERS = 8;

    std::array<QGroupBox*, NUM_PLAYERS> vibration_groupboxes;
    std::array<QSpinBox*, NUM_PLAYERS> vibration_spinboxes;
    std::array<int, NUM_PLAYERS> controller_callback_key;

    QCheckBox* accurate_vibration_checkbox;

    Core::HID::HIDCore& hid_core;
};
