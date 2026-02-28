// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

pragma Singleton
import QtQuick 2.15

QtObject {
    // Primary surface colors
    readonly property color base: typeof paletteBase !== "undefined" ? paletteBase : "#ffffff"
    readonly property color alternateBase: typeof paletteAlternateBase !== "undefined"
                                           ? paletteAlternateBase : "#f7f7f7"
    readonly property color window: typeof paletteWindow !== "undefined" ? paletteWindow : "#efefef"
    readonly property color windowText: typeof paletteWindowText !== "undefined"
                                        ? paletteWindowText : "#000000"

    // Button colors
    readonly property color button: typeof paletteButton !== "undefined" ? paletteButton : "#e0e0e0"
    readonly property color buttonText: typeof paletteButtonText !== "undefined"
                                        ? paletteButtonText : "#000000"

    // Selection colors
    readonly property color highlight: typeof paletteHighlight !== "undefined"
                                       ? paletteHighlight : "#308cc6"
    readonly property color highlightedText: typeof paletteHighlightedText !== "undefined"
                                             ? paletteHighlightedText : "#ffffff"

    // Mid-tone
    readonly property color mid: typeof paletteMid !== "undefined" ? paletteMid : "#a0a0a0"

    // Additional palette colors (derived from context properties if available)
    readonly property color link: typeof paletteLink !== "undefined" ? paletteLink : "#2a82da"
    readonly property color toolTipBase: typeof paletteToolTipBase !== "undefined"
                                         ? paletteToolTipBase : "#ffffdc"
    readonly property color toolTipText: typeof paletteToolTipText !== "undefined"
                                         ? paletteToolTipText : "#000000"

    // Semantic colors
    readonly property color danger: "#E81123"
    readonly property color success: "#107C10"
    readonly property color warning: "#FF8C00"

    // Standard sizing
    readonly property int borderRadius: 3
    readonly property int borderWidth: 1
    readonly property int spacing: 8
    readonly property int controlHeight: 28
    readonly property int fontSize: 13
    readonly property int fontSizeSmall: 11
    readonly property int fontSizeLarge: 16
}
