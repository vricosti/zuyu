// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "audio_core/sink/sink.h"
#include "audio_core/sink/sink_details.h"
#include "common/settings.h"
#include "common/settings_enums.h"
#include "yuzu/configuration/configure_audio_model.h"

AudioConfigModel::AudioConfigModel(QObject* parent) : QObject(parent) {}

void AudioConfigModel::initialize() {
    // Populate sink list
    m_sinkList.clear();
    m_sinkList.append(QString::fromUtf8(AudioCore::Sink::auto_device_name));
    for (const auto& id : AudioCore::Sink::GetSinkIDs()) {
        m_sinkList.append(QString::fromStdString(Settings::CanonicalizeEnum(id)));
    }
    emit sinkListChanged();

    // Set current sink
    const QString current_sink =
        QString::fromStdString(Settings::values.sink_id.ToString());
    m_sinkIndex = 0;
    for (int i = 0; i < m_sinkList.size(); i++) {
        if (m_sinkList[i] == current_sink) {
            m_sinkIndex = i;
            break;
        }
    }
    emit sinkIndexChanged();

    // Populate device lists for current sink
    updateDeviceLists();

    // Set current output device
    const QString output_device =
        QString::fromStdString(Settings::values.audio_output_device_id.GetValue());
    m_outputDeviceIndex = 0;
    for (int i = 0; i < m_outputDevices.size(); i++) {
        if (m_outputDevices[i] == output_device) {
            m_outputDeviceIndex = i;
            break;
        }
    }
    emit outputDeviceIndexChanged();

    // Set current input device
    const QString input_device =
        QString::fromStdString(Settings::values.audio_input_device_id.GetValue());
    m_inputDeviceIndex = 0;
    for (int i = 0; i < m_inputDevices.size(); i++) {
        if (m_inputDevices[i] == input_device) {
            m_inputDeviceIndex = i;
            break;
        }
    }
    emit inputDeviceIndexChanged();
}

void AudioConfigModel::setSinkIndex(int index) {
    if (m_sinkIndex != index && index >= 0 && index < m_sinkList.size()) {
        m_sinkIndex = index;
        emit sinkIndexChanged();
        updateDeviceLists();
    }
}

void AudioConfigModel::setOutputDeviceIndex(int index) {
    if (m_outputDeviceIndex != index && index >= 0 && index < m_outputDevices.size()) {
        m_outputDeviceIndex = index;
        emit outputDeviceIndexChanged();
    }
}

void AudioConfigModel::setInputDeviceIndex(int index) {
    if (m_inputDeviceIndex != index && index >= 0 && index < m_inputDevices.size()) {
        m_inputDeviceIndex = index;
        emit inputDeviceIndexChanged();
    }
}

void AudioConfigModel::updateDeviceLists() {
    const auto sink_text = m_sinkIndex < m_sinkList.size() ? m_sinkList[m_sinkIndex] : QString();
    const auto sink_id =
        Settings::ToEnum<Settings::AudioEngine>(sink_text.toStdString());

    // Output devices
    m_outputDevices.clear();
    m_outputDevices.append(QString::fromUtf8(AudioCore::Sink::auto_device_name));
    for (const auto& device : AudioCore::Sink::GetDeviceListForSink(sink_id, false)) {
        m_outputDevices.append(QString::fromStdString(device));
    }
    m_outputDeviceIndex = 0;
    emit outputDevicesChanged();
    emit outputDeviceIndexChanged();

    // Input devices
    m_inputDevices.clear();
    m_inputDevices.append(QString::fromUtf8(AudioCore::Sink::auto_device_name));
    for (const auto& device : AudioCore::Sink::GetDeviceListForSink(sink_id, true)) {
        m_inputDevices.append(QString::fromStdString(device));
    }
    m_inputDeviceIndex = 0;
    emit inputDevicesChanged();
    emit inputDeviceIndexChanged();
}

void AudioConfigModel::applySinkAndDevices() {
    if (m_sinkIndex >= 0 && m_sinkIndex < m_sinkList.size()) {
        Settings::values.sink_id.LoadString(m_sinkList[m_sinkIndex].toStdString());
    }
    if (m_outputDeviceIndex >= 0 && m_outputDeviceIndex < m_outputDevices.size()) {
        Settings::values.audio_output_device_id.SetValue(
            m_outputDevices[m_outputDeviceIndex].toStdString());
    }
    if (m_inputDeviceIndex >= 0 && m_inputDeviceIndex < m_inputDevices.size()) {
        Settings::values.audio_input_device_id.SetValue(
            m_inputDevices[m_inputDeviceIndex].toStdString());
    }
}
