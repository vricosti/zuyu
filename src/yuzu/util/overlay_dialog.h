// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <atomic>
#include <thread>

#include <QDialog>

#include "common/common_types.h"

class InputInterpreter;
class QQuickWidget;

namespace Core {
class System;
}

namespace Core::HID {
enum class NpadButton : u64;
}

/**
 * An OverlayDialog is an interactive dialog that accepts controller input (while a game is running)
 * This dialog attempts to replicate the look and feel of the Nintendo Switch's overlay dialogs and
 * provide some extra features such as embedding HTML/Rich Text content.
 */
class OverlayDialog final : public QDialog {
    Q_OBJECT

public:
    explicit OverlayDialog(QWidget* parent, Core::System& system, const QString& title_text,
                           const QString& body_text, const QString& left_button_text,
                           const QString& right_button_text,
                           Qt::Alignment alignment = Qt::AlignCenter, bool use_rich_text_ = false);
    ~OverlayDialog() override;

private:
    /// Moves and resizes the dialog to be fully overlaid on top of the parent window.
    void MoveAndResizeWindow();

    /**
     * Handles button presses and converts them into keyboard input.
     *
     * @tparam HIDButton The list of buttons that can be converted into keyboard input.
     */
    template <Core::HID::NpadButton... T>
    void HandleButtonPressedOnce();

    /**
     * Translates a button press to focus or click either the left or right buttons.
     *
     * @param button The button press to process.
     */
    void TranslateButtonPress(Core::HID::NpadButton button);

    void StartInputThread();
    void StopInputThread();

    /// The thread where input is being polled and processed.
    void InputThread();
    void keyPressEvent(QKeyEvent* e) override;

    QQuickWidget* quick_widget = nullptr;

    bool use_rich_text;
    bool has_buttons = false;

    // Track which button has focus for controller input
    int focused_button = 1; // 0 = left, 1 = right

    std::unique_ptr<InputInterpreter> input_interpreter;

    std::thread input_thread;

    std::atomic<bool> input_thread_running{};
};
