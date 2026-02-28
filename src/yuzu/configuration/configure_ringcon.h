// SPDX-FileCopyrightText: Copyright 2022 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <functional>
#include <QDialog>

class QLabel;
class QPushButton;
class QSlider;

namespace InputCommon {
class InputSubsystem;
} // namespace InputCommon

namespace Core::HID {
class HIDCore;
class EmulatedController;
} // namespace Core::HID

class ConfigureRingController : public QDialog {
    Q_OBJECT

public:
    explicit ConfigureRingController(QWidget* parent, InputCommon::InputSubsystem* input_subsystem_,
                                     Core::HID::HIDCore& hid_core_);
    ~ConfigureRingController() override;

    void ApplyConfiguration();

private:
    void UpdateUI();
    void LoadConfiguration();
    void RestoreDefaults();
    void EnableRingController();

    // Handles emulated controller events
    void ControllerUpdate(Core::HID::ControllerTriggerType type);

    void HandleClick(QPushButton* button,
                     std::function<void(const Common::ParamPackage&)> new_input_setter,
                     InputCommon::Polling::InputType type);

    void SetPollingResult(const Common::ParamPackage& params, bool abort);

    bool IsInputAcceptable(const Common::ParamPackage& params) const;

    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    QString ButtonToText(const Common::ParamPackage& param);
    QString AnalogToText(const Common::ParamPackage& param, const std::string& dir);

    static constexpr int ANALOG_SUB_BUTTONS_NUM = 2;

    std::array<QPushButton*, ANALOG_SUB_BUTTONS_NUM> analog_map_buttons;
    static const std::array<std::string, ANALOG_SUB_BUTTONS_NUM> analog_sub_buttons;

    std::unique_ptr<QTimer> timeout_timer;
    std::unique_ptr<QTimer> poll_timer;

    std::optional<std::function<void(const Common::ParamPackage&)>> input_setter;

    InputCommon::InputSubsystem* input_subsystem;
    Core::HID::EmulatedController* emulated_controller;

    QLabel* deadzone_label;
    QSlider* deadzone_slider;
    QPushButton* enable_ring_button;
    QLabel* sensor_value_label;

    bool is_ring_enabled{};
    bool is_controller_set{};
    int callback_key;
};
