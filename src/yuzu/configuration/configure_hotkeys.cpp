// SPDX-FileCopyrightText: 2017 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QMessageBox>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>

#include "common/logging/log.h"
#include "hid_core/frontend/emulated_controller.h"
#include "hid_core/hid_core.h"

#include "frontend_common/config.h"
#include "yuzu/configuration/configure_hotkeys.h"
#include "yuzu/hotkeys.h"
#include "yuzu/qml_bridge.h"
#include "yuzu/uisettings.h"
#include "yuzu/util/sequence_dialog/sequence_dialog.h"

constexpr int name_column = 0;
constexpr int hotkey_column = 1;
constexpr int controller_column = 2;

ConfigureHotkeys::ConfigureHotkeys(Core::HID::HIDCore& hid_core, QWidget* parent)
    : QWidget(parent), timeout_timer(std::make_unique<QTimer>()),
      poll_timer(std::make_unique<QTimer>()) {
    setFocusPolicy(Qt::ClickFocus);

    model = new QStandardItemModel(this);
    model->setColumnCount(3);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("hotkeyModel"), model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureHotkeys.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureHotkeys QML Error: {}",
                      error.toString().toStdString());
        }
    }

    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        connect(root, SIGNAL(configureHotkey(int, int, int)), this,
                SLOT(onConfigureHotkey(int, int, int)));
        connect(root, SIGNAL(restoreDefaults()), this, SLOT(onRestoreDefaults()));
        connect(root, SIGNAL(clearAll()), this, SLOT(onClearAll()));
        connect(root, SIGNAL(restoreDefault(int, int, int)), this,
                SLOT(onRestoreDefault(int, int, int)));
        connect(root, SIGNAL(clearHotkey(int, int, int)), this,
                SLOT(onClearHotkey(int, int, int)));
    }

    controller = hid_core.GetEmulatedController(Core::HID::NpadIdType::Player1);

    connect(timeout_timer.get(), &QTimer::timeout, [this] {
        const bool is_button_pressed = pressed_buttons != Core::HID::NpadButton::None ||
                                       pressed_home_button || pressed_capture_button;
        SetPollingResult(!is_button_pressed);
    });

    connect(poll_timer.get(), &QTimer::timeout, [this] {
        pressed_buttons |= controller->GetNpadButtons().raw;
        pressed_home_button |= this->controller->GetHomeButtons().home != 0;
        pressed_capture_button |= this->controller->GetCaptureButtons().capture != 0;
        if (pressed_buttons != Core::HID::NpadButton::None || pressed_home_button ||
            pressed_capture_button) {
            const QString button_name =
                GetButtonCombinationName(pressed_buttons, pressed_home_button,
                                         pressed_capture_button) +
                QStringLiteral("...");
            // Update the model during polling
            if (target_group_index >= 0 && target_action_index >= 0) {
                auto* parent_item = model->item(target_group_index, 0);
                if (parent_item && target_action_index < parent_item->rowCount()) {
                    parent_item->child(target_action_index, controller_column)
                        ->setText(button_name);
                }
            }
        }
    });

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureHotkeys::~ConfigureHotkeys() = default;

void ConfigureHotkeys::Populate(const HotkeyRegistry& registry) {
    model->removeRows(0, model->rowCount());
    model->setHorizontalHeaderLabels({tr("Action"), tr("Hotkey"), tr("Controller Hotkey")});

    for (const auto& group : registry.hotkey_groups) {
        QString parent_item_data = QString::fromStdString(group.first);
        auto* parent_item = new QStandardItem(
            QCoreApplication::translate("Hotkeys", qPrintable(parent_item_data)));
        parent_item->setEditable(false);
        parent_item->setData(parent_item_data);
        for (const auto& hotkey : group.second) {
            QString hotkey_action_data = QString::fromStdString(hotkey.first);
            auto* action = new QStandardItem(
                QCoreApplication::translate("Hotkeys", qPrintable(hotkey_action_data)));
            auto* keyseq =
                new QStandardItem(hotkey.second.keyseq.toString(QKeySequence::NativeText));
            auto* controller_keyseq =
                new QStandardItem(QString::fromStdString(hotkey.second.controller_keyseq));
            action->setEditable(false);
            action->setData(hotkey_action_data);
            keyseq->setEditable(false);
            controller_keyseq->setEditable(false);
            parent_item->appendRow({action, keyseq, controller_keyseq});
        }
        model->appendRow(parent_item);
    }
}

void ConfigureHotkeys::onConfigureHotkey(int groupIndex, int actionIndex, int column) {
    if (column == controller_column) {
        ConfigureController(groupIndex, actionIndex);
    } else {
        Configure(groupIndex, actionIndex);
    }
}

void ConfigureHotkeys::Configure(int groupIndex, int actionIndex) {
    auto* parent_item = model->item(groupIndex, 0);
    if (!parent_item || actionIndex >= parent_item->rowCount()) {
        return;
    }
    auto* keyseq_item = parent_item->child(actionIndex, hotkey_column);
    const auto previous_key = keyseq_item->text();

    SequenceDialog hotkey_dialog{this};
    const int return_code = hotkey_dialog.exec();
    const auto key_sequence = hotkey_dialog.GetSequence();
    if (return_code == QDialog::Rejected || key_sequence.isEmpty()) {
        return;
    }

    const auto [key_sequence_used, used_action] = IsUsedKey(key_sequence);
    if (key_sequence_used && key_sequence != QKeySequence(previous_key)) {
        QMessageBox::warning(
            this, tr("Conflicting Key Sequence"),
            tr("The entered key sequence is already assigned to: %1").arg(used_action));
    } else {
        keyseq_item->setText(key_sequence.toString(QKeySequence::NativeText));
    }
}

void ConfigureHotkeys::ConfigureController(int groupIndex, int actionIndex) {
    if (timeout_timer->isActive()) {
        return;
    }

    auto* parent_item = model->item(groupIndex, 0);
    if (!parent_item || actionIndex >= parent_item->rowCount()) {
        return;
    }
    auto* ctrl_item = parent_item->child(actionIndex, controller_column);
    const auto previous_key = ctrl_item->text();

    target_group_index = groupIndex;
    target_action_index = actionIndex;

    input_setter = [this, ctrl_item, previous_key](const bool cancel) {
        if (cancel) {
            ctrl_item->setText(previous_key);
            return;
        }

        const QString button_string =
            GetButtonCombinationName(pressed_buttons, pressed_home_button, pressed_capture_button);

        const auto [key_sequence_used, used_action] = IsUsedControllerKey(button_string);
        if (key_sequence_used) {
            QMessageBox::warning(
                this, tr("Conflicting Key Sequence"),
                tr("The entered key sequence is already assigned to: %1").arg(used_action));
            ctrl_item->setText(previous_key);
        } else {
            ctrl_item->setText(button_string);
        }
    };

    pressed_buttons = Core::HID::NpadButton::None;
    pressed_home_button = false;
    pressed_capture_button = false;

    ctrl_item->setText(tr("[waiting]"));
    timeout_timer->start(2500);
    poll_timer->start(100);
    controller->DisableConfiguration();
}

void ConfigureHotkeys::SetPollingResult(const bool cancel) {
    timeout_timer->stop();
    poll_timer->stop();
    (*input_setter)(cancel);
    controller->EnableConfiguration();
    input_setter = std::nullopt;
    target_group_index = -1;
    target_action_index = -1;
}

void ConfigureHotkeys::onRestoreDefaults() {
    for (int r = 0; r < model->rowCount(); ++r) {
        const QStandardItem* parent = model->item(r, 0);
        const int hotkey_size = static_cast<int>(UISettings::default_hotkeys.size());

        if (hotkey_size != parent->rowCount()) {
            QMessageBox::warning(this, tr("Invalid hotkey settings"),
                                 tr("An error occurred. Please report this issue on github."));
            return;
        }

        for (int r2 = 0; r2 < parent->rowCount(); ++r2) {
            model->item(r, 0)
                ->child(r2, hotkey_column)
                ->setText(QString::fromStdString(UISettings::default_hotkeys[r2].shortcut.keyseq));
            model->item(r, 0)
                ->child(r2, controller_column)
                ->setText(QString::fromStdString(
                    UISettings::default_hotkeys[r2].shortcut.controller_keyseq));
        }
    }
}

void ConfigureHotkeys::onClearAll() {
    for (int r = 0; r < model->rowCount(); ++r) {
        const QStandardItem* parent = model->item(r, 0);
        for (int r2 = 0; r2 < parent->rowCount(); ++r2) {
            model->item(r, 0)->child(r2, hotkey_column)->setText(QString{});
            model->item(r, 0)->child(r2, controller_column)->setText(QString{});
        }
    }
}

void ConfigureHotkeys::onRestoreDefault(int groupIndex, int actionIndex, int column) {
    if (actionIndex < 0 ||
        actionIndex >= static_cast<int>(UISettings::default_hotkeys.size())) {
        return;
    }

    if (column == controller_column) {
        const QString& default_key = QString::fromStdString(
            UISettings::default_hotkeys[actionIndex].shortcut.controller_keyseq);
        const auto [used, action] = IsUsedControllerKey(default_key);
        auto* item = model->item(groupIndex, 0)->child(actionIndex, controller_column);
        if (used && default_key != item->text()) {
            QMessageBox::warning(
                this, tr("Conflicting Button Sequence"),
                tr("The default button sequence is already assigned to: %1").arg(action));
        } else {
            item->setText(default_key);
        }
    } else {
        const QKeySequence default_key = QKeySequence::fromString(
            QString::fromStdString(UISettings::default_hotkeys[actionIndex].shortcut.keyseq),
            QKeySequence::NativeText);
        const auto [used, action] = IsUsedKey(default_key);
        auto* item = model->item(groupIndex, 0)->child(actionIndex, hotkey_column);
        if (used && default_key != QKeySequence(item->text())) {
            QMessageBox::warning(
                this, tr("Conflicting Key Sequence"),
                tr("The default key sequence is already assigned to: %1").arg(action));
        } else {
            item->setText(default_key.toString(QKeySequence::NativeText));
        }
    }
}

void ConfigureHotkeys::onClearHotkey(int groupIndex, int actionIndex, int column) {
    auto* parent_item = model->item(groupIndex, 0);
    if (parent_item && actionIndex < parent_item->rowCount()) {
        parent_item->child(actionIndex, column)->setText(QString{});
    }
}

QString ConfigureHotkeys::GetButtonCombinationName(Core::HID::NpadButton button,
                                                   const bool home,
                                                   const bool capture) const {
    Core::HID::NpadButtonState state{button};
    QString button_combination;
    if (home) button_combination.append(QStringLiteral("Home+"));
    if (capture) button_combination.append(QStringLiteral("Screenshot+"));
    if (state.a) button_combination.append(QStringLiteral("A+"));
    if (state.b) button_combination.append(QStringLiteral("B+"));
    if (state.x) button_combination.append(QStringLiteral("X+"));
    if (state.y) button_combination.append(QStringLiteral("Y+"));
    if (state.l || state.right_sl || state.left_sl)
        button_combination.append(QStringLiteral("L+"));
    if (state.r || state.right_sr || state.left_sr)
        button_combination.append(QStringLiteral("R+"));
    if (state.zl) button_combination.append(QStringLiteral("ZL+"));
    if (state.zr) button_combination.append(QStringLiteral("ZR+"));
    if (state.left) button_combination.append(QStringLiteral("Dpad_Left+"));
    if (state.right) button_combination.append(QStringLiteral("Dpad_Right+"));
    if (state.up) button_combination.append(QStringLiteral("Dpad_Up+"));
    if (state.down) button_combination.append(QStringLiteral("Dpad_Down+"));
    if (state.stick_l) button_combination.append(QStringLiteral("Left_Stick+"));
    if (state.stick_r) button_combination.append(QStringLiteral("Right_Stick+"));
    if (state.minus) button_combination.append(QStringLiteral("Minus+"));
    if (state.plus) button_combination.append(QStringLiteral("Plus+"));
    if (button_combination.isEmpty()) {
        return tr("Invalid");
    } else {
        button_combination.chop(1);
        return button_combination;
    }
}

std::pair<bool, QString> ConfigureHotkeys::IsUsedKey(QKeySequence key_sequence) const {
    for (int r = 0; r < model->rowCount(); ++r) {
        const QStandardItem* const parent = model->item(r, 0);
        for (int r2 = 0; r2 < parent->rowCount(); ++r2) {
            const QStandardItem* const key_seq_item = parent->child(r2, hotkey_column);
            const auto key_seq = QKeySequence::fromString(key_seq_item->text(),
                                                          QKeySequence::NativeText);
            if (key_sequence == key_seq) {
                return std::make_pair(true, parent->child(r2, 0)->text());
            }
        }
    }
    return std::make_pair(false, QString());
}

std::pair<bool, QString> ConfigureHotkeys::IsUsedControllerKey(
    const QString& key_sequence) const {
    for (int r = 0; r < model->rowCount(); ++r) {
        const QStandardItem* const parent = model->item(r, 0);
        for (int r2 = 0; r2 < parent->rowCount(); ++r2) {
            const QStandardItem* const key_seq_item = parent->child(r2, controller_column);
            if (key_sequence == key_seq_item->text()) {
                return std::make_pair(true, parent->child(r2, 0)->text());
            }
        }
    }
    return std::make_pair(false, QString());
}

void ConfigureHotkeys::ApplyConfiguration(HotkeyRegistry& registry) {
    for (int key_id = 0; key_id < model->rowCount(); key_id++) {
        const QStandardItem* parent = model->item(key_id, 0);
        for (int key_column_id = 0; key_column_id < parent->rowCount(); key_column_id++) {
            const QStandardItem* action = parent->child(key_column_id, name_column);
            const QStandardItem* keyseq = parent->child(key_column_id, hotkey_column);
            const QStandardItem* controller_keyseq =
                parent->child(key_column_id, controller_column);
            for (auto& [group, sub_actions] : registry.hotkey_groups) {
                if (group != parent->data().toString().toStdString())
                    continue;
                for (auto& [action_name, hotkey] : sub_actions) {
                    if (action_name != action->data().toString().toStdString())
                        continue;
                    hotkey.keyseq = QKeySequence(keyseq->text());
                    hotkey.controller_keyseq = controller_keyseq->text().toStdString();
                }
            }
        }
    }
    registry.SaveHotkeys();
}
