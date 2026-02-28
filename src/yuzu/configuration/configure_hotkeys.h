// SPDX-FileCopyrightText: 2017 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>
#include <optional>
#include <QStandardItemModel>
#include <QWidget>

namespace Common {
class ParamPackage;
}

namespace Core::HID {
class HIDCore;
class EmulatedController;
enum class NpadButton : u64;
} // namespace Core::HID

class HotkeyRegistry;
class QQuickWidget;
class QTimer;

class ConfigureHotkeys : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureHotkeys(Core::HID::HIDCore& hid_core_, QWidget* parent = nullptr);
    ~ConfigureHotkeys() override;

    void ApplyConfiguration(HotkeyRegistry& registry);
    void Populate(const HotkeyRegistry& registry);

public slots:
    void onConfigureHotkey(int groupIndex, int actionIndex, int column);
    void onRestoreDefaults();
    void onClearAll();
    void onRestoreDefault(int groupIndex, int actionIndex, int column);
    void onClearHotkey(int groupIndex, int actionIndex, int column);

private:
    void Configure(int groupIndex, int actionIndex);
    void ConfigureController(int groupIndex, int actionIndex);
    std::pair<bool, QString> IsUsedKey(QKeySequence key_sequence) const;
    std::pair<bool, QString> IsUsedControllerKey(const QString& key_sequence) const;

    void SetPollingResult(bool cancel);
    QString GetButtonCombinationName(Core::HID::NpadButton button, bool home,
                                     bool capture) const;

    QQuickWidget* quick_widget = nullptr;
    QStandardItemModel* model;

    bool pressed_home_button;
    bool pressed_capture_button;
    int target_group_index = -1;
    int target_action_index = -1;
    Core::HID::NpadButton pressed_buttons;

    Core::HID::EmulatedController* controller;
    std::unique_ptr<QTimer> timeout_timer;
    std::unique_ptr<QTimer> poll_timer;
    std::optional<std::function<void(bool)>> input_setter;
};
