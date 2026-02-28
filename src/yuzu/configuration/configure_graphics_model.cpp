// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QColorDialog>
#include <QWidget>

#include "common/settings.h"
#include "yuzu/configuration/configure_graphics_model.h"
#include "yuzu/configuration/shared_translation.h"
#include "yuzu/uisettings.h"
#include "yuzu/vk_device_info.h"

static const std::vector<VkPresentModeKHR> default_present_modes{VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                                  VK_PRESENT_MODE_FIFO_KHR};

static constexpr VkPresentModeKHR VSyncSettingToMode(Settings::VSyncMode mode) {
    switch (mode) {
    case Settings::VSyncMode::Immediate:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case Settings::VSyncMode::Mailbox:
        return VK_PRESENT_MODE_MAILBOX_KHR;
    case Settings::VSyncMode::Fifo:
        return VK_PRESENT_MODE_FIFO_KHR;
    case Settings::VSyncMode::FifoRelaxed:
        return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    default:
        return VK_PRESENT_MODE_FIFO_KHR;
    }
}

static constexpr Settings::VSyncMode PresentModeToSetting(VkPresentModeKHR mode) {
    switch (mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        return Settings::VSyncMode::Immediate;
    case VK_PRESENT_MODE_MAILBOX_KHR:
        return Settings::VSyncMode::Mailbox;
    case VK_PRESENT_MODE_FIFO_KHR:
        return Settings::VSyncMode::Fifo;
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        return Settings::VSyncMode::FifoRelaxed;
    default:
        return Settings::VSyncMode::Fifo;
    }
}

GraphicsConfigModel::GraphicsConfigModel(std::vector<VkDeviceInfo::Record>& records,
                                         const std::function<void()>& expose_compute_option,
                                         bool is_powered_on, QObject* parent)
    : QObject(parent), m_runtimeLocked(is_powered_on), m_records(records),
      m_exposeComputeOption(expose_compute_option) {

    // Load enum translations
    auto combobox = ConfigurationShared::ComboboxEnumeration(nullptr);
    if (combobox) {
        // API options
        auto it = combobox->find(Settings::EnumMetadata<Settings::RendererBackend>::Index());
        if (it != combobox->end()) {
            for (const auto& [val, text] : it->second) {
                m_apiEntries.push_back({val, text});
            }
        }

        // Shader backend options
        auto sit = combobox->find(Settings::EnumMetadata<Settings::ShaderBackend>::Index());
        if (sit != combobox->end()) {
            for (const auto& [val, text] : sit->second) {
                m_shaderEntries.push_back({val, text});
            }
        }
    }

    // Retrieve Vulkan devices
    retrieveVulkanDevices();

    // Set initial API index from setting
    const auto backend = Settings::values.renderer_backend.GetValue();
    for (int i = 0; i < static_cast<int>(m_apiEntries.size()); i++) {
        if (m_apiEntries[i].value == static_cast<u32>(backend)) {
            m_apiIndex = i;
            break;
        }
    }

    // Set initial device index
    m_deviceIndex = static_cast<int>(Settings::values.vulkan_device.GetValue());

    // Set initial shader backend index
    const auto shader = Settings::values.shader_backend.GetValue();
    for (int i = 0; i < static_cast<int>(m_shaderEntries.size()); i++) {
        if (m_shaderEntries[i].value == static_cast<u32>(shader)) {
            m_shaderIndex = i;
            break;
        }
    }

    // Set initial background color
    m_bgColor = QColor::fromRgb(Settings::values.bg_red.GetValue(),
                                 Settings::values.bg_green.GetValue(),
                                 Settings::values.bg_blue.GetValue());

    // Populate VSync modes based on initial state
    populateVsyncModes();
}

QStringList GraphicsConfigModel::apiOptions() const {
    QStringList result;
    for (const auto& entry : m_apiEntries) {
        result.append(entry.label);
    }
    return result;
}

int GraphicsConfigModel::apiIndex() const {
    return m_apiIndex;
}

void GraphicsConfigModel::setApiIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_apiEntries.size()) || index == m_apiIndex) {
        return;
    }
    m_apiIndex = index;
    emit apiIndexChanged();
    emit layoutChanged();
    populateVsyncModes();
}

bool GraphicsConfigModel::apiEnabled() const {
    return !UISettings::values.has_broken_vulkan && !m_runtimeLocked;
}

QStringList GraphicsConfigModel::deviceOptions() const {
    return m_deviceOptions;
}

int GraphicsConfigModel::deviceIndex() const {
    return m_deviceIndex;
}

void GraphicsConfigModel::setDeviceIndex(int index) {
    if (index < 0 || index >= m_deviceOptions.size() || index == m_deviceIndex) {
        return;
    }
    m_deviceIndex = index;
    emit deviceIndexChanged();
    populateVsyncModes();
}

bool GraphicsConfigModel::deviceVisible() const {
    return currentBackend() == Settings::RendererBackend::Vulkan;
}

QStringList GraphicsConfigModel::shaderOptions() const {
    QStringList result;
    for (const auto& entry : m_shaderEntries) {
        result.append(entry.label);
    }
    return result;
}

int GraphicsConfigModel::shaderIndex() const {
    return m_shaderIndex;
}

void GraphicsConfigModel::setShaderIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_shaderEntries.size()) || index == m_shaderIndex) {
        return;
    }
    m_shaderIndex = index;
    emit shaderIndexChanged();
}

bool GraphicsConfigModel::shaderVisible() const {
    return currentBackend() == Settings::RendererBackend::OpenGL;
}

QStringList GraphicsConfigModel::vsyncOptions() const {
    return m_vsyncOptions;
}

int GraphicsConfigModel::vsyncIndex() const {
    return m_vsyncIndex;
}

void GraphicsConfigModel::setVsyncIndex(int index) {
    if (index < 0 || index >= m_vsyncOptions.size() || index == m_vsyncIndex) {
        return;
    }
    m_vsyncIndex = index;
    emit vsyncIndexChanged();
}

QString GraphicsConfigModel::bgColorHex() const {
    return m_bgColor.name();
}

bool GraphicsConfigModel::runtimeLocked() const {
    return m_runtimeLocked;
}

void GraphicsConfigModel::pickBackgroundColor() {
    auto* widget = qobject_cast<QWidget*>(parent());
    const QColor newColor = QColorDialog::getColor(m_bgColor, widget);
    if (newColor.isValid()) {
        m_bgColor = newColor;
        emit bgColorChanged();
    }
}

void GraphicsConfigModel::applyConfiguration() {
    // Apply API backend
    if (m_apiIndex >= 0 && m_apiIndex < static_cast<int>(m_apiEntries.size())) {
        Settings::values.renderer_backend.SetValue(
            static_cast<Settings::RendererBackend>(m_apiEntries[m_apiIndex].value));
    }

    // Apply Vulkan device or shader backend based on current API
    const auto backend = currentBackend();
    if (backend == Settings::RendererBackend::Vulkan) {
        Settings::values.vulkan_device.SetValue(m_deviceIndex);
    } else if (backend == Settings::RendererBackend::OpenGL) {
        if (m_shaderIndex >= 0 && m_shaderIndex < static_cast<int>(m_shaderEntries.size())) {
            Settings::values.shader_backend.SetValue(
                static_cast<Settings::ShaderBackend>(m_shaderEntries[m_shaderIndex].value));
        }
    }

    // Apply VSync mode
    if (m_vsyncIndex >= 0 && m_vsyncIndex < static_cast<int>(m_vsyncModeMap.size()) &&
        backend != Settings::RendererBackend::Null) {
        const auto vsync_mode = PresentModeToSetting(m_vsyncModeMap[m_vsyncIndex]);
        Settings::values.vsync_mode.SetValue(vsync_mode);
    }

    // Apply background color
    Settings::values.bg_red.SetValue(static_cast<u8>(m_bgColor.red()));
    Settings::values.bg_green.SetValue(static_cast<u8>(m_bgColor.green()));
    Settings::values.bg_blue.SetValue(static_cast<u8>(m_bgColor.blue()));
}

Settings::RendererBackend GraphicsConfigModel::currentBackend() const {
    if (m_apiIndex >= 0 && m_apiIndex < static_cast<int>(m_apiEntries.size())) {
        auto backend = static_cast<Settings::RendererBackend>(m_apiEntries[m_apiIndex].value);
        if (backend == Settings::RendererBackend::Vulkan &&
            UISettings::values.has_broken_vulkan) {
            return Settings::RendererBackend::OpenGL;
        }
        return backend;
    }
    return Settings::RendererBackend::OpenGL;
}

void GraphicsConfigModel::populateVsyncModes() {
    const auto backend = currentBackend();
    if (backend == Settings::RendererBackend::Null) {
        m_vsyncOptions.clear();
        m_vsyncModeMap.clear();
        m_vsyncIndex = 0;
        emit vsyncOptionsChanged();
        emit vsyncIndexChanged();
        return;
    }

    const auto currentMode = m_vsyncModeMap.empty()
                                 ? VSyncSettingToMode(Settings::values.vsync_mode.GetValue())
                                 : m_vsyncModeMap[qBound(0, m_vsyncIndex,
                                                         static_cast<int>(m_vsyncModeMap.size()) -
                                                             1)];

    const auto& present_modes =
        (backend == Settings::RendererBackend::Vulkan && m_deviceIndex >= 0 &&
         m_deviceIndex < static_cast<int>(m_devicePresentModes.size()))
            ? m_devicePresentModes[m_deviceIndex]
            : default_present_modes;

    m_vsyncOptions.clear();
    m_vsyncModeMap.clear();
    int new_index = 0;
    int idx = 0;
    for (const auto present_mode : present_modes) {
        const auto name = translateVSyncMode(present_mode, backend);
        if (name.isEmpty()) {
            continue;
        }
        m_vsyncOptions.append(name);
        m_vsyncModeMap.push_back(present_mode);
        if (present_mode == currentMode) {
            new_index = idx;
        }
        idx++;
    }

    m_vsyncIndex = new_index;
    emit vsyncOptionsChanged();
    emit vsyncIndexChanged();
}

void GraphicsConfigModel::retrieveVulkanDevices() {
    m_deviceOptions.clear();
    m_devicePresentModes.clear();
    for (const auto& record : m_records) {
        m_deviceOptions.append(QString::fromStdString(record.name));
        m_devicePresentModes.push_back(record.vsync_support);
        if (record.has_broken_compute) {
            m_exposeComputeOption();
        }
    }
}

QString GraphicsConfigModel::translateVSyncMode(VkPresentModeKHR mode,
                                                 Settings::RendererBackend backend) const {
    switch (mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        if (backend == Settings::RendererBackend::OpenGL) {
            return tr("Off");
        } else {
            QString result = QStringLiteral("Immediate (");
            result.append(tr("VSync Off"));
            result.append(QStringLiteral(")"));
            return result;
        }
    case VK_PRESENT_MODE_MAILBOX_KHR: {
        QString result = QStringLiteral("Mailbox (");
        result.append(tr("Recommended"));
        result.append(QStringLiteral(")"));
        return result;
    }
    case VK_PRESENT_MODE_FIFO_KHR:
        if (backend == Settings::RendererBackend::OpenGL) {
            return tr("On");
        } else {
            QString result = QStringLiteral("FIFO (");
            result.append(tr("VSync On"));
            result.append(QStringLiteral(")"));
            return result;
        }
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        return QStringLiteral("FIFO Relaxed");
    default:
        return {};
    }
}
