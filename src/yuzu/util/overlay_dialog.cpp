// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QKeyEvent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>

#include "core/core.h"
#include "hid_core/frontend/input_interpreter.h"
#include "hid_core/hid_types.h"
#include "yuzu/qml_bridge.h"
#include "yuzu/util/overlay_dialog.h"

namespace {

constexpr float BASE_TITLE_FONT_SIZE = 14.0f;
constexpr float BASE_FONT_SIZE = 18.0f;
constexpr float BASE_WIDTH = 1280.0f;
constexpr float BASE_HEIGHT = 720.0f;

} // Anonymous namespace

OverlayDialog::OverlayDialog(QWidget* parent, Core::System& system, const QString& title_text,
                             const QString& body_text, const QString& left_button_text,
                             const QString& right_button_text, Qt::Alignment alignment,
                             bool use_rich_text_)
    : QDialog(parent), use_rich_text{use_rich_text_} {

    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint |
                   Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint);
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_TranslucentBackground);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quick_widget->setAttribute(Qt::WA_AlwaysStackOnTop);
    quick_widget->setClearColor(Qt::transparent);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);

    // Compute DPI-scaled font sizes
    const auto width = static_cast<float>(parentWidget()->width());
    const auto height = static_cast<float>(parentWidget()->height());
    const float dpi_scale = screen()->logicalDotsPerInch() / 96.0f;

    const auto title_font_size = BASE_TITLE_FONT_SIZE * (height / BASE_HEIGHT) / dpi_scale;
    const auto body_font_size =
        BASE_FONT_SIZE * (((width / BASE_WIDTH) + (height / BASE_HEIGHT)) / 2.0f) / dpi_scale;
    const auto button_font_size = BASE_FONT_SIZE * (height / BASE_HEIGHT) / dpi_scale;

    ctx->setContextProperty(QStringLiteral("overlayTitleText"), title_text);
    ctx->setContextProperty(QStringLiteral("overlayBodyText"), body_text);
    ctx->setContextProperty(QStringLiteral("overlayLeftButtonText"), left_button_text);
    ctx->setContextProperty(QStringLiteral("overlayRightButtonText"), right_button_text);
    ctx->setContextProperty(QStringLiteral("overlayUseRichText"), use_rich_text);
    ctx->setContextProperty(QStringLiteral("overlayTitleFontSize"),
                            static_cast<qreal>(title_font_size));
    ctx->setContextProperty(QStringLiteral("overlayBodyFontSize"),
                            static_cast<qreal>(body_font_size));
    ctx->setContextProperty(QStringLiteral("overlayButtonFontSize"),
                            static_cast<qreal>(button_font_size));

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/OverlayDialog.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "OverlayDialog QML Error: {}", error.toString().toStdString());
        }
    }

    // Connect QML signals to dialog accept/reject
    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        connect(root, SIGNAL(leftButtonClicked()), this, SLOT(reject()));
        connect(root, SIGNAL(rightButtonClicked()), this, SLOT(accept()));
    }

    layout->addWidget(quick_widget);
    setLayout(layout);

    has_buttons = !left_button_text.isEmpty() || !right_button_text.isEmpty();

    // Set initial focus to right button if both exist, left if only left
    if (!right_button_text.isEmpty()) {
        focused_button = 1;
    } else if (!left_button_text.isEmpty()) {
        focused_button = 0;
    }

    MoveAndResizeWindow();

    // TODO (Morph): Remove this when InputInterpreter no longer relies on the HID backend
    if (system.IsPoweredOn() && has_buttons) {
        input_interpreter = std::make_unique<InputInterpreter>(system);
        StartInputThread();
    }
}

OverlayDialog::~OverlayDialog() {
    StopInputThread();
}

void OverlayDialog::MoveAndResizeWindow() {
    const auto pos = parentWidget()->mapToGlobal(parentWidget()->rect().topLeft());
    const auto width = parentWidget()->width();
    const auto height = parentWidget()->height();

    QDialog::move(pos);
    QDialog::resize(width, height);
}

template <Core::HID::NpadButton... T>
void OverlayDialog::HandleButtonPressedOnce() {
    const auto f = [this](Core::HID::NpadButton button) {
        if (input_interpreter->IsButtonPressedOnce(button)) {
            TranslateButtonPress(button);
        }
    };

    (f(T), ...);
}

void OverlayDialog::TranslateButtonPress(Core::HID::NpadButton button) {
    switch (button) {
    case Core::HID::NpadButton::A:
        if (focused_button == 0) {
            StopInputThread();
            QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);
        } else {
            StopInputThread();
            QMetaObject::invokeMethod(this, "accept", Qt::QueuedConnection);
        }
        break;
    case Core::HID::NpadButton::B:
        StopInputThread();
        QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);
        break;
    case Core::HID::NpadButton::Left:
    case Core::HID::NpadButton::StickLLeft:
        focused_button = 0;
        break;
    case Core::HID::NpadButton::Right:
    case Core::HID::NpadButton::StickLRight:
        focused_button = 1;
        break;
    default:
        break;
    }
}

void OverlayDialog::StartInputThread() {
    if (input_thread_running) {
        return;
    }

    input_thread_running = true;

    input_thread = std::thread(&OverlayDialog::InputThread, this);
}

void OverlayDialog::StopInputThread() {
    input_thread_running = false;

    if (input_thread.joinable()) {
        input_thread.join();
    }
}

void OverlayDialog::InputThread() {
    while (input_thread_running) {
        input_interpreter->PollInput();

        HandleButtonPressedOnce<Core::HID::NpadButton::A, Core::HID::NpadButton::B,
                                Core::HID::NpadButton::Left, Core::HID::NpadButton::Right,
                                Core::HID::NpadButton::StickLLeft,
                                Core::HID::NpadButton::StickLRight>();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void OverlayDialog::keyPressEvent(QKeyEvent* e) {
    if (has_buttons || e->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(e);
    }
}
