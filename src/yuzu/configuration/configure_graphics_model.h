// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <functional>
#include <vector>
#include <QColor>
#include <QObject>
#include <QStringList>
#include <vulkan/vulkan_core.h>
#include "common/common_types.h"
#include "common/settings_enums.h"

namespace VkDeviceInfo {
class Record;
}

/**
 * Model for the ConfigureGraphics tab — manages the interdependent API,
 * Vulkan device, shader backend, and VSync dropdowns plus background color.
 */
class GraphicsConfigModel : public QObject {
    Q_OBJECT

    // API selector
    Q_PROPERTY(QStringList apiOptions READ apiOptions CONSTANT)
    Q_PROPERTY(int apiIndex READ apiIndex WRITE setApiIndex NOTIFY apiIndexChanged)
    Q_PROPERTY(bool apiEnabled READ apiEnabled CONSTANT)

    // Vulkan device
    Q_PROPERTY(QStringList deviceOptions READ deviceOptions CONSTANT)
    Q_PROPERTY(int deviceIndex READ deviceIndex WRITE setDeviceIndex NOTIFY deviceIndexChanged)
    Q_PROPERTY(bool deviceVisible READ deviceVisible NOTIFY layoutChanged)

    // Shader backend (OpenGL)
    Q_PROPERTY(QStringList shaderOptions READ shaderOptions CONSTANT)
    Q_PROPERTY(int shaderIndex READ shaderIndex WRITE setShaderIndex NOTIFY shaderIndexChanged)
    Q_PROPERTY(bool shaderVisible READ shaderVisible NOTIFY layoutChanged)

    // VSync
    Q_PROPERTY(QStringList vsyncOptions READ vsyncOptions NOTIFY vsyncOptionsChanged)
    Q_PROPERTY(int vsyncIndex READ vsyncIndex WRITE setVsyncIndex NOTIFY vsyncIndexChanged)

    // Background color
    Q_PROPERTY(QString bgColorHex READ bgColorHex NOTIFY bgColorChanged)

    // Runtime lock
    Q_PROPERTY(bool runtimeLocked READ runtimeLocked CONSTANT)

public:
    explicit GraphicsConfigModel(std::vector<VkDeviceInfo::Record>& records,
                                 const std::function<void()>& expose_compute_option,
                                 bool is_powered_on, QObject* parent = nullptr);

    QStringList apiOptions() const;
    int apiIndex() const;
    void setApiIndex(int index);
    bool apiEnabled() const;

    QStringList deviceOptions() const;
    int deviceIndex() const;
    void setDeviceIndex(int index);
    bool deviceVisible() const;

    QStringList shaderOptions() const;
    int shaderIndex() const;
    void setShaderIndex(int index);
    bool shaderVisible() const;

    QStringList vsyncOptions() const;
    int vsyncIndex() const;
    void setVsyncIndex(int index);

    QString bgColorHex() const;
    bool runtimeLocked() const;

    Q_INVOKABLE void pickBackgroundColor();

    void applyConfiguration();

signals:
    void apiIndexChanged();
    void deviceIndexChanged();
    void shaderIndexChanged();
    void vsyncOptionsChanged();
    void vsyncIndexChanged();
    void layoutChanged();
    void bgColorChanged();
    void aspectRatioChanged();

private:
    Settings::RendererBackend currentBackend() const;
    void populateVsyncModes();
    void retrieveVulkanDevices();
    QString translateVSyncMode(VkPresentModeKHR mode, Settings::RendererBackend backend) const;

    // API
    struct EnumEntry {
        u32 value;
        QString label;
    };
    std::vector<EnumEntry> m_apiEntries;
    int m_apiIndex = 0;

    // Vulkan devices
    QStringList m_deviceOptions;
    int m_deviceIndex = 0;
    std::vector<std::vector<VkPresentModeKHR>> m_devicePresentModes;

    // Shader backend
    std::vector<EnumEntry> m_shaderEntries;
    int m_shaderIndex = 0;

    // VSync
    QStringList m_vsyncOptions;
    int m_vsyncIndex = 0;
    std::vector<VkPresentModeKHR> m_vsyncModeMap;

    // Background color
    QColor m_bgColor;

    bool m_runtimeLocked;
    std::vector<VkDeviceInfo::Record>& m_records;
    const std::function<void()>& m_exposeComputeOption;
};
