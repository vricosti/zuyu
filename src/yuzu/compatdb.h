// SPDX-FileCopyrightText: 2017 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDialog>
#include <QFutureWatcher>
#include "core/telemetry_session.h"

class CompatDBModel;
class QQuickWidget;

enum class CompatibilityStatus {
    Perfect = 0,
    Playable = 1,
    // Unused: Okay = 2,
    Ingame = 3,
    IntroMenu = 4,
    WontBoot = 5,
};

class CompatDB : public QDialog {
    Q_OBJECT

public:
    explicit CompatDB(Core::TelemetrySession& telemetry_session_, QWidget* parent = nullptr);
    ~CompatDB();

private:
    void Submit();
    CompatibilityStatus CalculateCompatibility() const;
    void OnTestcaseSubmitted();

    CompatDBModel* model = nullptr;
    QQuickWidget* quick_widget = nullptr;
    QFutureWatcher<bool> testcase_watcher;

    Core::TelemetrySession& telemetry_session;
};
