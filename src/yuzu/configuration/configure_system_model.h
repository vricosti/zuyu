// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QString>

#include "common/common_types.h"

namespace Core {
class System;
}

class SettingsModel;

class SystemConfigModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString localeWarning READ localeWarning NOTIFY localeWarningChanged)
    Q_PROPERTY(bool rtcEnabled READ rtcEnabled WRITE setRtcEnabled NOTIFY rtcEnabledChanged)
    Q_PROPERTY(qint64 rtcDateTime READ rtcDateTime WRITE setRtcDateTime NOTIFY rtcDateTimeChanged)
    Q_PROPERTY(int rtcOffset READ rtcOffset WRITE setRtcOffset NOTIFY rtcOffsetChanged)

public:
    explicit SystemConfigModel(Core::System& system, QObject* parent = nullptr);

    void initialize();

    QString localeWarning() const { return m_localeWarning; }
    bool rtcEnabled() const { return m_rtcEnabled; }
    qint64 rtcDateTime() const { return m_rtcDateTime; }
    int rtcOffset() const { return m_rtcOffset; }

    void setRtcEnabled(bool val);
    void setRtcDateTime(qint64 secs_since_epoch);
    void setRtcOffset(int offset);

    Q_INVOKABLE void onRegionOrLanguageChanged(int regionIndex, int languageIndex);

    void apply(bool powered_on);

signals:
    void localeWarningChanged();
    void rtcEnabledChanged();
    void rtcDateTimeChanged();
    void rtcOffsetChanged();

private:
    void updateRtcTime();
    static bool isValidLocale(u32 region_index, u32 language_index);

    Core::System& m_system;
    QString m_localeWarning;
    bool m_rtcEnabled = false;
    qint64 m_rtcDateTime = 0;
    int m_rtcOffset = 0;
    qint64 m_previousTime = 0;
};
