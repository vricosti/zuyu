// SPDX-FileCopyrightText: 2017 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QWidget>

class QQuickWidget;
class WebConfigModel;

class ConfigureWeb : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureWeb(QWidget* parent = nullptr);
    ~ConfigureWeb() override;

    void ApplyConfiguration();
    void SetWebServiceConfigEnabled(bool enabled);

private slots:
    void onVerifyFailed(const QString& message);

private:
    QQuickWidget* quick_widget = nullptr;
    WebConfigModel* web_model = nullptr;
};
