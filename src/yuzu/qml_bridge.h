// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QApplication>
#include <QPalette>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QString>

namespace QmlBridge {

/**
 * Sets up common palette context properties on a QQmlContext.
 * This avoids repeating the same 9+ palette color setup for every QQuickWidget.
 *
 * Usage:
 *   QQuickWidget* widget = new QQuickWidget(this);
 *   QmlBridge::SetupPalette(widget->rootContext());
 */
inline void SetupPalette(QQmlContext* ctx) {
    const QPalette pal = QApplication::palette();
    ctx->setContextProperty(QStringLiteral("paletteBase"), pal.color(QPalette::Base));
    ctx->setContextProperty(QStringLiteral("paletteAlternateBase"),
                            pal.color(QPalette::AlternateBase));
    ctx->setContextProperty(QStringLiteral("paletteWindow"), pal.color(QPalette::Window));
    ctx->setContextProperty(QStringLiteral("paletteWindowText"),
                            pal.color(QPalette::WindowText));
    ctx->setContextProperty(QStringLiteral("paletteButton"), pal.color(QPalette::Button));
    ctx->setContextProperty(QStringLiteral("paletteButtonText"),
                            pal.color(QPalette::ButtonText));
    ctx->setContextProperty(QStringLiteral("paletteHighlight"), pal.color(QPalette::Highlight));
    ctx->setContextProperty(QStringLiteral("paletteHighlightedText"),
                            pal.color(QPalette::HighlightedText));
    ctx->setContextProperty(QStringLiteral("paletteMid"), pal.color(QPalette::Mid));
    ctx->setContextProperty(QStringLiteral("paletteLink"), pal.color(QPalette::Link));
    ctx->setContextProperty(QStringLiteral("paletteToolTipBase"),
                            pal.color(QPalette::ToolTipBase));
    ctx->setContextProperty(QStringLiteral("paletteToolTipText"),
                            pal.color(QPalette::ToolTipText));
}

/**
 * Refreshes palette context properties (e.g. after theme change).
 * Same as SetupPalette but can be called on an already-initialized context.
 */
inline void RefreshPalette(QQmlContext* ctx) {
    SetupPalette(ctx);
}

/**
 * Sets up platform info context properties on a QQmlContext.
 */
inline void SetupPlatformInfo(QQmlContext* ctx) {
#ifdef _WIN32
    ctx->setContextProperty(QStringLiteral("platformOS"), QStringLiteral("windows"));
#elif defined(__APPLE__)
    ctx->setContextProperty(QStringLiteral("platformOS"), QStringLiteral("macos"));
#elif defined(__linux__)
    ctx->setContextProperty(QStringLiteral("platformOS"), QStringLiteral("linux"));
#else
    ctx->setContextProperty(QStringLiteral("platformOS"), QStringLiteral("unknown"));
#endif
}

/**
 * Convenience function: sets up all common context properties (palette + platform).
 */
inline void SetupContext(QQmlContext* ctx) {
    SetupPalette(ctx);
    SetupPlatformInfo(ctx);
}

/**
 * Registers the QML components directory with a QQmlEngine so that
 * `import ZComponents 1.0` works in QML files.
 */
inline void RegisterComponents(QQmlEngine* engine) {
    engine->addImportPath(QStringLiteral("qrc:/qml/components"));
}

} // namespace QmlBridge
