// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QWidget>
#include "common/settings_enums.h"

namespace Core {
class System;
}

class QQuickWidget;
class UiConfigModel;

class ConfigureUi : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureUi(Core::System& system_, QWidget* parent = nullptr);
    ~ConfigureUi() override;

    void ApplyConfiguration();
    void UpdateScreenshotInfo(Settings::AspectRatio ratio,
                              Settings::ResolutionSetup resolution_info);

signals:
    void LanguageChanged(const QString& locale);

private slots:
    void onBrowseScreenshotPath();

private:
    QQuickWidget* quick_widget = nullptr;
    UiConfigModel* ui_model = nullptr;
    Core::System& system;
};
