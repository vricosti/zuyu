// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <array>
#include <chrono>

#include "common/settings.h"
#include "core/core.h"
#include "yuzu/configuration/configure_system_model.h"

constexpr std::array<u32, 7> LOCALE_BLOCKLIST{
    0b0100011100001100000, // Japan
    0b0000001101001100100, // Americas
    0b0100110100001000010, // Europe
    0b0100110100001000010, // Australia
    0b0000000000000000000, // China
    0b0100111100001000000, // Korea
    0b0100111100001000000, // Taiwan
};

SystemConfigModel::SystemConfigModel(Core::System& system, QObject* parent)
    : QObject(parent), m_system(system) {}

void SystemConfigModel::initialize() {
    m_rtcEnabled = Settings::values.custom_rtc_enabled.GetValue();

    const auto posix_time = std::chrono::system_clock::now().time_since_epoch();
    m_previousTime = std::chrono::duration_cast<std::chrono::seconds>(posix_time).count();
    m_rtcOffset = static_cast<int>(Settings::values.custom_rtc_offset.GetValue());

    updateRtcTime();

    emit rtcEnabledChanged();
    emit rtcOffsetChanged();
    emit rtcDateTimeChanged();
}

void SystemConfigModel::setRtcEnabled(bool val) {
    if (m_rtcEnabled != val) {
        m_rtcEnabled = val;
        emit rtcEnabledChanged();
        updateRtcTime();
    }
}

void SystemConfigModel::setRtcDateTime(qint64 secs_since_epoch) {
    if (!m_rtcEnabled) {
        return;
    }
    // When the datetime changes, update the offset to match
    int new_offset = m_rtcOffset + static_cast<int>(secs_since_epoch - m_previousTime);
    m_previousTime = secs_since_epoch;
    m_rtcDateTime = secs_since_epoch;
    m_rtcOffset = new_offset;
    emit rtcDateTimeChanged();
    emit rtcOffsetChanged();
}

void SystemConfigModel::setRtcOffset(int offset) {
    if (m_rtcOffset != offset) {
        m_rtcOffset = offset;
        emit rtcOffsetChanged();
        updateRtcTime();
    }
}

void SystemConfigModel::onRegionOrLanguageChanged(int regionIndex, int languageIndex) {
    if (!isValidLocale(static_cast<u32>(regionIndex), static_cast<u32>(languageIndex))) {
        m_localeWarning = tr("Warning: The selected language may not be valid for the "
                             "selected region.");
    } else {
        m_localeWarning = QString();
    }
    emit localeWarningChanged();
}

void SystemConfigModel::apply(bool powered_on) {
    Settings::values.custom_rtc_enabled.SetValue(m_rtcEnabled);
    Settings::values.custom_rtc_offset.SetValue(m_rtcOffset);
}

void SystemConfigModel::updateRtcTime() {
    const auto posix_time = std::chrono::system_clock::now().time_since_epoch();
    m_previousTime = std::chrono::duration_cast<std::chrono::seconds>(posix_time).count();

    if (m_rtcEnabled) {
        m_previousTime += m_rtcOffset;
    }

    m_rtcDateTime = m_previousTime;
    emit rtcDateTimeChanged();
}

bool SystemConfigModel::isValidLocale(u32 region_index, u32 language_index) {
    if (region_index >= LOCALE_BLOCKLIST.size()) {
        return false;
    }
    return ((LOCALE_BLOCKLIST.at(region_index) >> language_index) & 1) == 0;
}
