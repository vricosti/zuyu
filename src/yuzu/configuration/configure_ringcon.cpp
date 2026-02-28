// SPDX-FileCopyrightText: Copyright 2022 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

#include <fmt/format.h>

#include "configuration/qt_config.h"
#include "hid_core/frontend/emulated_controller.h"
#include "hid_core/hid_core.h"
#include "input_common/drivers/keyboard.h"
#include "input_common/drivers/mouse.h"
#include "input_common/main.h"
#include "yuzu/bootmanager.h"
#include "yuzu/configuration/configure_ringcon.h"

const std::array<std::string, ConfigureRingController::ANALOG_SUB_BUTTONS_NUM>
    ConfigureRingController::analog_sub_buttons{{
        "left",
        "right",
    }};

namespace {

QString GetKeyName(int key_code) {
    switch (key_code) {
    case Qt::Key_Shift:
        return QObject::tr("Shift");
    case Qt::Key_Control:
        return QObject::tr("Ctrl");
    case Qt::Key_Alt:
        return QObject::tr("Alt");
    case Qt::Key_Meta:
        return {};
    default:
        return QKeySequence(key_code).toString();
    }
}

QString GetButtonName(Common::Input::ButtonNames button_name) {
    switch (button_name) {
    case Common::Input::ButtonNames::ButtonLeft:
        return QObject::tr("Left");
    case Common::Input::ButtonNames::ButtonRight:
        return QObject::tr("Right");
    case Common::Input::ButtonNames::ButtonDown:
        return QObject::tr("Down");
    case Common::Input::ButtonNames::ButtonUp:
        return QObject::tr("Up");
    case Common::Input::ButtonNames::TriggerZ:
        return QObject::tr("Z");
    case Common::Input::ButtonNames::TriggerR:
        return QObject::tr("R");
    case Common::Input::ButtonNames::TriggerL:
        return QObject::tr("L");
    case Common::Input::ButtonNames::ButtonA:
        return QObject::tr("A");
    case Common::Input::ButtonNames::ButtonB:
        return QObject::tr("B");
    case Common::Input::ButtonNames::ButtonX:
        return QObject::tr("X");
    case Common::Input::ButtonNames::ButtonY:
        return QObject::tr("Y");
    case Common::Input::ButtonNames::ButtonStart:
        return QObject::tr("Start");
    case Common::Input::ButtonNames::L1:
        return QObject::tr("L1");
    case Common::Input::ButtonNames::L2:
        return QObject::tr("L2");
    case Common::Input::ButtonNames::L3:
        return QObject::tr("L3");
    case Common::Input::ButtonNames::R1:
        return QObject::tr("R1");
    case Common::Input::ButtonNames::R2:
        return QObject::tr("R2");
    case Common::Input::ButtonNames::R3:
        return QObject::tr("R3");
    case Common::Input::ButtonNames::Circle:
        return QObject::tr("Circle");
    case Common::Input::ButtonNames::Cross:
        return QObject::tr("Cross");
    case Common::Input::ButtonNames::Square:
        return QObject::tr("Square");
    case Common::Input::ButtonNames::Triangle:
        return QObject::tr("Triangle");
    case Common::Input::ButtonNames::Share:
        return QObject::tr("Share");
    case Common::Input::ButtonNames::Options:
        return QObject::tr("Options");
    default:
        return QObject::tr("[undefined]");
    }
}

void SetAnalogParam(const Common::ParamPackage& input_param, Common::ParamPackage& analog_param,
                    const std::string& button_name) {
    if (input_param.Has("axis_x") && input_param.Has("axis_y")) {
        analog_param = input_param;
        return;
    }
    if (!analog_param.Has("engine") || analog_param.Has("axis_x") || analog_param.Has("axis_y")) {
        analog_param = {
            {"engine", "analog_from_button"},
        };
    }
    analog_param.Set(button_name, input_param.Serialize());
}
} // namespace

ConfigureRingController::ConfigureRingController(QWidget* parent,
                                                 InputCommon::InputSubsystem* input_subsystem_,
                                                 Core::HID::HIDCore& hid_core_)
    : QDialog(parent), timeout_timer(std::make_unique<QTimer>()),
      poll_timer(std::make_unique<QTimer>()), input_subsystem{input_subsystem_} {
    setWindowTitle(tr("Configure Ring Controller"));

    auto* main_layout = new QVBoxLayout(this);

    // Info label
    auto* info_label = new QLabel(
        tr("To use Ring-Con, configure player 1 as right Joy-Con (both physical and emulated), "
           "and player 2 as left Joy-Con (left physical and dual emulated) before starting the "
           "game."),
        this);
    info_label->setWordWrap(true);
    info_label->setMinimumWidth(280);
    main_layout->addWidget(info_label);

    main_layout->addSpacing(10);

    // Virtual Ring Sensor Parameters group
    auto* ring_group = new QGroupBox(tr("Virtual Ring Sensor Parameters"), this);
    auto* ring_layout = new QVBoxLayout(ring_group);
    ring_layout->setSpacing(0);
    ring_layout->setContentsMargins(3, 6, 3, 0);

    // Pull/Push buttons
    auto* buttons_layout = new QHBoxLayout;
    buttons_layout->setSpacing(3);

    auto* pull_group = new QGroupBox(tr("Pull"), this);
    pull_group->setAlignment(Qt::AlignCenter);
    auto* pull_layout = new QVBoxLayout(pull_group);
    pull_layout->setSpacing(3);
    pull_layout->setContentsMargins(3, 3, 3, 3);
    auto* pull_button = new QPushButton(tr("Pull"), this);
    pull_button->setMinimumWidth(70);
    pull_button->setMaximumWidth(68);
    pull_button->setStyleSheet(QStringLiteral("min-width: 68px;"));
    pull_layout->addWidget(pull_button);
    buttons_layout->addWidget(pull_group, 0, Qt::AlignHCenter);

    auto* push_group = new QGroupBox(tr("Push"), this);
    push_group->setAlignment(Qt::AlignCenter);
    auto* push_layout = new QVBoxLayout(push_group);
    push_layout->setSpacing(3);
    push_layout->setContentsMargins(3, 3, 3, 3);
    auto* push_button = new QPushButton(tr("Push"), this);
    push_button->setMinimumWidth(70);
    push_button->setMaximumWidth(68);
    push_button->setStyleSheet(QStringLiteral("min-width: 68px;"));
    push_layout->addWidget(push_button);
    buttons_layout->addWidget(push_group, 0, Qt::AlignHCenter);

    ring_layout->addLayout(buttons_layout);

    analog_map_buttons = {pull_button, push_button};

    // Deadzone slider
    auto* deadzone_layout = new QVBoxLayout;
    deadzone_layout->setSpacing(3);
    deadzone_layout->setContentsMargins(0, 10, 0, 3);

    deadzone_label = new QLabel(tr("Deadzone: 0%"), this);
    deadzone_label->setAlignment(Qt::AlignHCenter);
    deadzone_layout->addWidget(deadzone_label);

    deadzone_slider = new QSlider(Qt::Horizontal, this);
    deadzone_slider->setMaximum(100);
    deadzone_layout->addWidget(deadzone_slider);

    ring_layout->addLayout(deadzone_layout);
    main_layout->addWidget(ring_group);

    // Direct Joycon Driver group
    auto* driver_group = new QGroupBox(tr("Direct Joycon Driver"), this);
    auto* driver_layout = new QGridLayout(driver_group);
    driver_layout->setContentsMargins(10, 6, 10, 10);
    driver_layout->setVerticalSpacing(10);

    driver_layout->addWidget(new QLabel(tr("Enable Ring Input"), this), 0, 0);
    enable_ring_button = new QPushButton(tr("Enable"), this);
    driver_layout->addWidget(enable_ring_button, 0, 2);

    driver_layout->addWidget(new QLabel(tr("Ring Sensor Value"), this), 1, 0);
    sensor_value_label = new QLabel(tr("Not connected"), this);
    sensor_value_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    driver_layout->addWidget(sensor_value_label, 1, 2);

    main_layout->addWidget(driver_group);
    main_layout->addStretch();

    // Bottom: Restore Defaults + OK/Cancel
    auto* bottom_layout = new QHBoxLayout;
    auto* restore_button = new QPushButton(tr("Restore Defaults"), this);
    connect(restore_button, &QPushButton::clicked, this,
            &ConfigureRingController::RestoreDefaults);
    bottom_layout->addWidget(restore_button);

    auto* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    bottom_layout->addWidget(button_box);

    main_layout->addLayout(bottom_layout);

    // Controller setup
    emulated_controller = hid_core_.GetEmulatedController(Core::HID::NpadIdType::Player1);
    emulated_controller->SaveCurrentConfig();
    emulated_controller->EnableConfiguration();

    Core::HID::ControllerUpdateCallback engine_callback{
        .on_change = [this](Core::HID::ControllerTriggerType type) { ControllerUpdate(type); },
        .is_npad_service = false,
    };
    callback_key = emulated_controller->SetCallback(engine_callback);
    is_controller_set = true;

    LoadConfiguration();

    for (int sub_button_id = 0; sub_button_id < ANALOG_SUB_BUTTONS_NUM; ++sub_button_id) {
        auto* const analog_button = analog_map_buttons[sub_button_id];

        if (analog_button == nullptr) {
            continue;
        }

        connect(analog_button, &QPushButton::clicked, [=, this] {
            HandleClick(
                analog_map_buttons[sub_button_id],
                [=, this](const Common::ParamPackage& params) {
                    Common::ParamPackage param = emulated_controller->GetRingParam();
                    SetAnalogParam(params, param, analog_sub_buttons[sub_button_id]);
                    emulated_controller->SetRingParam(param);
                },
                InputCommon::Polling::InputType::Stick);
        });

        analog_button->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(analog_button, &QPushButton::customContextMenuRequested,
                [=, this](const QPoint& menu_location) {
                    QMenu context_menu;
                    Common::ParamPackage param = emulated_controller->GetRingParam();
                    context_menu.addAction(tr("Clear"), [&] {
                        emulated_controller->SetRingParam(param);
                        analog_map_buttons[sub_button_id]->setText(tr("[not set]"));
                    });
                    context_menu.addAction(tr("Invert axis"), [&] {
                        const bool invert_value = param.Get("invert_x", "+") == "-";
                        const std::string invert_str = invert_value ? "+" : "-";
                        param.Set("invert_x", invert_str);
                        emulated_controller->SetRingParam(param);
                        for (int sub_button_id2 = 0; sub_button_id2 < ANALOG_SUB_BUTTONS_NUM;
                             ++sub_button_id2) {
                            analog_map_buttons[sub_button_id2]->setText(
                                AnalogToText(param, analog_sub_buttons[sub_button_id2]));
                        }
                    });
                    context_menu.exec(
                        analog_map_buttons[sub_button_id]->mapToGlobal(menu_location));
                });
    }

    connect(deadzone_slider, &QSlider::valueChanged, [this] {
        Common::ParamPackage param = emulated_controller->GetRingParam();
        const auto slider_value = deadzone_slider->value();
        deadzone_label->setText(tr("Deadzone: %1%").arg(slider_value));
        param.Set("deadzone", slider_value / 100.0f);
        emulated_controller->SetRingParam(param);
    });

    connect(enable_ring_button, &QPushButton::clicked, this,
            &ConfigureRingController::EnableRingController);

    timeout_timer->setSingleShot(true);
    connect(timeout_timer.get(), &QTimer::timeout, [this] { SetPollingResult({}, true); });

    connect(poll_timer.get(), &QTimer::timeout, [this] {
        const auto& params = input_subsystem->GetNextInput();
        if (params.Has("engine") && IsInputAcceptable(params)) {
            SetPollingResult(params, false);
            return;
        }
    });

    resize(0, 0);
}

ConfigureRingController::~ConfigureRingController() {
    emulated_controller->SetPollingMode(Core::HID::EmulatedDeviceIndex::RightIndex,
                                        Common::Input::PollingMode::Active);
    emulated_controller->DisableConfiguration();

    if (is_controller_set) {
        emulated_controller->DeleteCallback(callback_key);
        is_controller_set = false;
    }
};

void ConfigureRingController::UpdateUI() {
    const Common::ParamPackage param = emulated_controller->GetRingParam();

    for (int sub_button_id = 0; sub_button_id < ANALOG_SUB_BUTTONS_NUM; ++sub_button_id) {
        auto* const analog_button = analog_map_buttons[sub_button_id];

        if (analog_button == nullptr) {
            continue;
        }

        analog_button->setText(AnalogToText(param, analog_sub_buttons[sub_button_id]));
    }

    int slider_value = static_cast<int>(param.Get("deadzone", 0.15f) * 100);
    deadzone_label->setText(tr("Deadzone: %1%").arg(slider_value));
    deadzone_slider->setValue(slider_value);
}

void ConfigureRingController::ApplyConfiguration() {
    emulated_controller->DisableConfiguration();
    emulated_controller->SaveCurrentConfig();
    emulated_controller->EnableConfiguration();
}

void ConfigureRingController::LoadConfiguration() {
    UpdateUI();
}

void ConfigureRingController::RestoreDefaults() {
    const std::string default_ring_string = InputCommon::GenerateAnalogParamFromKeys(
        0, 0, QtConfig::default_ringcon_analogs[0], QtConfig::default_ringcon_analogs[1], 0, 0.05f);
    emulated_controller->SetRingParam(Common::ParamPackage(default_ring_string));
    UpdateUI();
}

void ConfigureRingController::EnableRingController() {
    const auto dialog_title = tr("Error enabling ring input");

    is_ring_enabled = false;
    sensor_value_label->setText(tr("Not connected"));

    if (!Settings::values.enable_joycon_driver) {
        QMessageBox::warning(this, dialog_title, tr("Direct Joycon driver is not enabled"));
        return;
    }

    enable_ring_button->setEnabled(false);
    enable_ring_button->setText(tr("Configuring"));
    repaint();

    const auto result = emulated_controller->SetPollingMode(
        Core::HID::EmulatedDeviceIndex::RightIndex, Common::Input::PollingMode::Ring);
    switch (result) {
    case Common::Input::DriverResult::Success:
        is_ring_enabled = true;
        break;
    case Common::Input::DriverResult::NotSupported:
        QMessageBox::warning(this, dialog_title,
                             tr("The current mapped device doesn't support the ring controller"));
        break;
    case Common::Input::DriverResult::NoDeviceDetected:
        QMessageBox::warning(this, dialog_title,
                             tr("The current mapped device doesn't have a ring attached"));
        break;
    case Common::Input::DriverResult::InvalidHandle:
        QMessageBox::warning(this, dialog_title, tr("The current mapped device is not connected"));
        break;
    default:
        QMessageBox::warning(this, dialog_title,
                             tr("Unexpected driver result %1").arg(static_cast<int>(result)));
        break;
    }
    enable_ring_button->setEnabled(true);
    enable_ring_button->setText(tr("Enable"));
}

void ConfigureRingController::ControllerUpdate(Core::HID::ControllerTriggerType type) {
    if (!is_ring_enabled) {
        return;
    }
    if (type != Core::HID::ControllerTriggerType::RingController) {
        return;
    }

    const auto value = emulated_controller->GetRingSensorValues();
    const auto tex_value = QString::fromStdString(fmt::format("{:.3f}", value.raw_value));
    sensor_value_label->setText(tex_value);
}

void ConfigureRingController::HandleClick(
    QPushButton* button, std::function<void(const Common::ParamPackage&)> new_input_setter,
    InputCommon::Polling::InputType type) {
    button->setText(tr("[waiting]"));
    button->setFocus();

    input_setter = new_input_setter;

    input_subsystem->BeginMapping(type);

    QWidget::grabMouse();
    QWidget::grabKeyboard();

    timeout_timer->start(2500);
    poll_timer->start(25);
}

void ConfigureRingController::SetPollingResult(const Common::ParamPackage& params, bool abort) {
    timeout_timer->stop();
    poll_timer->stop();
    input_subsystem->StopMapping();

    QWidget::releaseMouse();
    QWidget::releaseKeyboard();

    if (!abort) {
        (*input_setter)(params);
    }

    UpdateUI();

    input_setter = std::nullopt;
}

bool ConfigureRingController::IsInputAcceptable(const Common::ParamPackage& params) const {
    return true;
}

void ConfigureRingController::mousePressEvent(QMouseEvent* event) {
    if (!input_setter || !event) {
        return;
    }

    const auto button = GRenderWindow::QtButtonToMouseButton(event->button());
    input_subsystem->GetMouse()->PressButton(0, 0, button);
}

void ConfigureRingController::keyPressEvent(QKeyEvent* event) {
    if (!input_setter || !event) {
        return;
    }
    event->ignore();
    if (event->key() != Qt::Key_Escape) {
        input_subsystem->GetKeyboard()->PressKey(event->key());
    }
}

QString ConfigureRingController::ButtonToText(const Common::ParamPackage& param) {
    if (!param.Has("engine")) {
        return QObject::tr("[not set]");
    }

    const QString toggle = QString::fromStdString(param.Get("toggle", false) ? "~" : "");
    const QString inverted = QString::fromStdString(param.Get("inverted", false) ? "!" : "");
    const auto common_button_name = input_subsystem->GetButtonName(param);

    if (param.Get("engine", "") == "keyboard") {
        const QString button_str = GetKeyName(param.Get("code", 0));
        return QObject::tr("%1%2").arg(toggle, button_str);
    }

    if (common_button_name == Common::Input::ButtonNames::Invalid) {
        return QObject::tr("[invalid]");
    }

    if (common_button_name == Common::Input::ButtonNames::Engine) {
        return QString::fromStdString(param.Get("engine", ""));
    }

    if (common_button_name == Common::Input::ButtonNames::Value) {
        if (param.Has("hat")) {
            const QString hat = QString::fromStdString(param.Get("direction", ""));
            return QObject::tr("%1%2Hat %3").arg(toggle, inverted, hat);
        }
        if (param.Has("axis")) {
            const QString axis = QString::fromStdString(param.Get("axis", ""));
            return QObject::tr("%1%2Axis %3").arg(toggle, inverted, axis);
        }
        if (param.Has("axis_x") && param.Has("axis_y") && param.Has("axis_z")) {
            const QString axis_x = QString::fromStdString(param.Get("axis_x", ""));
            const QString axis_y = QString::fromStdString(param.Get("axis_y", ""));
            const QString axis_z = QString::fromStdString(param.Get("axis_z", ""));
            return QObject::tr("%1%2Axis %3,%4,%5").arg(toggle, inverted, axis_x, axis_y, axis_z);
        }
        if (param.Has("motion")) {
            const QString motion = QString::fromStdString(param.Get("motion", ""));
            return QObject::tr("%1%2Motion %3").arg(toggle, inverted, motion);
        }
        if (param.Has("button")) {
            const QString button = QString::fromStdString(param.Get("button", ""));
            return QObject::tr("%1%2Button %3").arg(toggle, inverted, button);
        }
    }

    QString button_name = GetButtonName(common_button_name);
    if (param.Has("hat")) {
        return QObject::tr("%1%2Hat %3").arg(toggle, inverted, button_name);
    }
    if (param.Has("axis")) {
        return QObject::tr("%1%2Axis %3").arg(toggle, inverted, button_name);
    }
    if (param.Has("motion")) {
        return QObject::tr("%1%2Axis %3").arg(toggle, inverted, button_name);
    }
    if (param.Has("button")) {
        return QObject::tr("%1%2Button %3").arg(toggle, inverted, button_name);
    }

    return QObject::tr("[unknown]");
}

QString ConfigureRingController::AnalogToText(const Common::ParamPackage& param,
                                              const std::string& dir) {
    if (!param.Has("engine")) {
        return QObject::tr("[not set]");
    }

    if (param.Get("engine", "") == "analog_from_button") {
        return ButtonToText(Common::ParamPackage{param.Get(dir, "")});
    }

    if (!param.Has("axis_x") || !param.Has("axis_y")) {
        return QObject::tr("[unknown]");
    }

    const auto engine_str = param.Get("engine", "");
    const QString axis_x_str = QString::fromStdString(param.Get("axis_x", ""));
    const QString axis_y_str = QString::fromStdString(param.Get("axis_y", ""));
    const bool invert_x = param.Get("invert_x", "+") == "-";
    const bool invert_y = param.Get("invert_y", "+") == "-";

    if (dir == "modifier") {
        return QObject::tr("[unused]");
    }

    if (dir == "left") {
        const QString invert_x_str = QString::fromStdString(invert_x ? "+" : "-");
        return QObject::tr("Axis %1%2").arg(axis_x_str, invert_x_str);
    }
    if (dir == "right") {
        const QString invert_x_str = QString::fromStdString(invert_x ? "-" : "+");
        return QObject::tr("Axis %1%2").arg(axis_x_str, invert_x_str);
    }
    if (dir == "up") {
        const QString invert_y_str = QString::fromStdString(invert_y ? "-" : "+");
        return QObject::tr("Axis %1%2").arg(axis_y_str, invert_y_str);
    }
    if (dir == "down") {
        const QString invert_y_str = QString::fromStdString(invert_y ? "+" : "-");
        return QObject::tr("Axis %1%2").arg(axis_y_str, invert_y_str);
    }

    return QObject::tr("[unknown]");
}
