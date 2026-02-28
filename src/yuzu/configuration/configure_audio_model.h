// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QStringList>

class AudioConfigModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QStringList sinkList READ sinkList NOTIFY sinkListChanged)
    Q_PROPERTY(int sinkIndex READ sinkIndex WRITE setSinkIndex NOTIFY sinkIndexChanged)
    Q_PROPERTY(QStringList outputDevices READ outputDevices NOTIFY outputDevicesChanged)
    Q_PROPERTY(int outputDeviceIndex READ outputDeviceIndex WRITE setOutputDeviceIndex NOTIFY
                   outputDeviceIndexChanged)
    Q_PROPERTY(QStringList inputDevices READ inputDevices NOTIFY inputDevicesChanged)
    Q_PROPERTY(int inputDeviceIndex READ inputDeviceIndex WRITE setInputDeviceIndex NOTIFY
                   inputDeviceIndexChanged)

public:
    explicit AudioConfigModel(QObject* parent = nullptr);

    void initialize();

    QStringList sinkList() const { return m_sinkList; }
    int sinkIndex() const { return m_sinkIndex; }
    QStringList outputDevices() const { return m_outputDevices; }
    int outputDeviceIndex() const { return m_outputDeviceIndex; }
    QStringList inputDevices() const { return m_inputDevices; }
    int inputDeviceIndex() const { return m_inputDeviceIndex; }

    void setSinkIndex(int index);
    void setOutputDeviceIndex(int index);
    void setInputDeviceIndex(int index);

    void applySinkAndDevices();

signals:
    void sinkListChanged();
    void sinkIndexChanged();
    void outputDevicesChanged();
    void outputDeviceIndexChanged();
    void inputDevicesChanged();
    void inputDeviceIndexChanged();

private:
    void updateDeviceLists();

    QStringList m_sinkList;
    int m_sinkIndex = 0;
    QStringList m_outputDevices;
    int m_outputDeviceIndex = 0;
    QStringList m_inputDevices;
    int m_inputDeviceIndex = 0;
};
