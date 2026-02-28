// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "yuzu/status_bar_model.h"

StatusBarModel::StatusBarModel(QObject* parent) : QObject(parent) {}

void StatusBarModel::SetMessageText(const QString& text) {
    if (m_messageText != text) {
        m_messageText = text;
        emit messageTextChanged();
    }
}

void StatusBarModel::SetShaderBuildingText(const QString& text) {
    if (m_shaderBuildingText != text) {
        m_shaderBuildingText = text;
        emit shaderBuildingTextChanged();
    }
}

void StatusBarModel::SetShaderBuildingVisible(bool visible) {
    if (m_shaderBuildingVisible != visible) {
        m_shaderBuildingVisible = visible;
        emit shaderBuildingVisibleChanged();
    }
}

void StatusBarModel::SetResScaleText(const QString& text) {
    if (m_resScaleText != text) {
        m_resScaleText = text;
        emit resScaleTextChanged();
    }
}

void StatusBarModel::SetEmuSpeedText(const QString& text) {
    if (m_emuSpeedText != text) {
        m_emuSpeedText = text;
        emit emuSpeedTextChanged();
    }
}

void StatusBarModel::SetEmuSpeedVisible(bool visible) {
    if (m_emuSpeedVisible != visible) {
        m_emuSpeedVisible = visible;
        emit emuSpeedVisibleChanged();
    }
}

void StatusBarModel::SetGameFpsText(const QString& text) {
    if (m_gameFpsText != text) {
        m_gameFpsText = text;
        emit gameFpsTextChanged();
    }
}

void StatusBarModel::SetEmuFrametimeText(const QString& text) {
    if (m_emuFrametimeText != text) {
        m_emuFrametimeText = text;
        emit emuFrametimeTextChanged();
    }
}

void StatusBarModel::SetTasText(const QString& text) {
    if (m_tasText != text) {
        m_tasText = text;
        emit tasTextChanged();
    }
}

void StatusBarModel::SetPerfVisible(bool visible) {
    if (m_perfVisible != visible) {
        m_perfVisible = visible;
        emit perfVisibleChanged();
    }
}

void StatusBarModel::SetFirmwareText(const QString& text) {
    if (m_firmwareText != text) {
        m_firmwareText = text;
        emit firmwareTextChanged();
    }
}

void StatusBarModel::SetFirmwareTooltip(const QString& text) {
    if (m_firmwareTooltip != text) {
        m_firmwareTooltip = text;
        emit firmwareTooltipChanged();
    }
}

void StatusBarModel::SetFirmwareVisible(bool visible) {
    if (m_firmwareVisible != visible) {
        m_firmwareVisible = visible;
        emit firmwareVisibleChanged();
    }
}

void StatusBarModel::SetRendererText(const QString& text) {
    if (m_rendererText != text) {
        m_rendererText = text;
        emit rendererTextChanged();
    }
}

void StatusBarModel::SetRendererChecked(bool checked) {
    if (m_rendererChecked != checked) {
        m_rendererChecked = checked;
        emit rendererCheckedChanged();
    }
}

void StatusBarModel::SetRendererEnabled(bool enabled) {
    if (m_rendererEnabled != enabled) {
        m_rendererEnabled = enabled;
        emit rendererEnabledChanged();
    }
}

void StatusBarModel::SetGpuAccuracyText(const QString& text) {
    if (m_gpuAccuracyText != text) {
        m_gpuAccuracyText = text;
        emit gpuAccuracyTextChanged();
    }
}

void StatusBarModel::SetGpuAccuracyChecked(bool checked) {
    if (m_gpuAccuracyChecked != checked) {
        m_gpuAccuracyChecked = checked;
        emit gpuAccuracyCheckedChanged();
    }
}

void StatusBarModel::SetDockedText(const QString& text) {
    if (m_dockedText != text) {
        m_dockedText = text;
        emit dockedTextChanged();
    }
}

void StatusBarModel::SetDockedChecked(bool checked) {
    if (m_dockedChecked != checked) {
        m_dockedChecked = checked;
        emit dockedCheckedChanged();
    }
}

void StatusBarModel::SetFilterText(const QString& text) {
    if (m_filterText != text) {
        m_filterText = text;
        emit filterTextChanged();
    }
}

void StatusBarModel::SetAAText(const QString& text) {
    if (m_aaText != text) {
        m_aaText = text;
        emit aaTextChanged();
    }
}

void StatusBarModel::SetVolume(int volume) {
    if (m_volume != volume) {
        m_volume = volume;
        emit volumeChanged();
    }
}

void StatusBarModel::SetMuted(bool muted) {
    if (m_isMuted != muted) {
        m_isMuted = muted;
        emit isMutedChanged();
    }
}

void StatusBarModel::SetVolumeText(const QString& text) {
    if (m_volumeText != text) {
        m_volumeText = text;
        emit volumeTextChanged();
    }
}
