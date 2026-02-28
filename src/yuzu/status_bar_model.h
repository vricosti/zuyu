// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QString>

/**
 * StatusBarModel exposes all status bar state to QML via Q_PROPERTY.
 * GMainWindow updates this model; the QML StatusBar.qml reads from it.
 */
class StatusBarModel : public QObject {
    Q_OBJECT

    // Performance labels (visible during emulation)
    Q_PROPERTY(QString messageText READ GetMessageText WRITE SetMessageText NOTIFY messageTextChanged)
    Q_PROPERTY(
        QString shaderBuildingText READ GetShaderBuildingText NOTIFY shaderBuildingTextChanged)
    Q_PROPERTY(bool shaderBuildingVisible READ IsShaderBuildingVisible NOTIFY
                   shaderBuildingVisibleChanged)
    Q_PROPERTY(QString resScaleText READ GetResScaleText NOTIFY resScaleTextChanged)
    Q_PROPERTY(QString emuSpeedText READ GetEmuSpeedText NOTIFY emuSpeedTextChanged)
    Q_PROPERTY(bool emuSpeedVisible READ IsEmuSpeedVisible NOTIFY emuSpeedVisibleChanged)
    Q_PROPERTY(QString gameFpsText READ GetGameFpsText NOTIFY gameFpsTextChanged)
    Q_PROPERTY(QString emuFrametimeText READ GetEmuFrametimeText NOTIFY emuFrametimeTextChanged)
    Q_PROPERTY(QString tasText READ GetTasText NOTIFY tasTextChanged)
    Q_PROPERTY(bool perfVisible READ IsPerfVisible NOTIFY perfVisibleChanged)

    // Firmware
    Q_PROPERTY(QString firmwareText READ GetFirmwareText NOTIFY firmwareTextChanged)
    Q_PROPERTY(
        QString firmwareTooltip READ GetFirmwareTooltip NOTIFY firmwareTooltipChanged)
    Q_PROPERTY(bool firmwareVisible READ IsFirmwareVisible NOTIFY firmwareVisibleChanged)

    // Toggle buttons
    Q_PROPERTY(QString rendererText READ GetRendererText NOTIFY rendererTextChanged)
    Q_PROPERTY(bool rendererChecked READ IsRendererChecked NOTIFY rendererCheckedChanged)
    Q_PROPERTY(bool rendererEnabled READ IsRendererEnabled WRITE SetRendererEnabled NOTIFY
                   rendererEnabledChanged)

    Q_PROPERTY(
        QString gpuAccuracyText READ GetGpuAccuracyText NOTIFY gpuAccuracyTextChanged)
    Q_PROPERTY(
        bool gpuAccuracyChecked READ IsGpuAccuracyChecked NOTIFY gpuAccuracyCheckedChanged)

    Q_PROPERTY(QString dockedText READ GetDockedText NOTIFY dockedTextChanged)
    Q_PROPERTY(bool dockedChecked READ IsDockedChecked NOTIFY dockedCheckedChanged)

    Q_PROPERTY(QString filterText READ GetFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(QString aaText READ GetAAText NOTIFY aaTextChanged)

    // Volume
    Q_PROPERTY(int volume READ GetVolume WRITE SetVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool isMuted READ IsMuted WRITE SetMuted NOTIFY isMutedChanged)
    Q_PROPERTY(QString volumeText READ GetVolumeText NOTIFY volumeTextChanged)

public:
    explicit StatusBarModel(QObject* parent = nullptr);

    // Getters
    QString GetMessageText() const { return m_messageText; }
    QString GetShaderBuildingText() const { return m_shaderBuildingText; }
    bool IsShaderBuildingVisible() const { return m_shaderBuildingVisible; }
    QString GetResScaleText() const { return m_resScaleText; }
    QString GetEmuSpeedText() const { return m_emuSpeedText; }
    bool IsEmuSpeedVisible() const { return m_emuSpeedVisible; }
    QString GetGameFpsText() const { return m_gameFpsText; }
    QString GetEmuFrametimeText() const { return m_emuFrametimeText; }
    QString GetTasText() const { return m_tasText; }
    bool IsPerfVisible() const { return m_perfVisible; }
    QString GetFirmwareText() const { return m_firmwareText; }
    QString GetFirmwareTooltip() const { return m_firmwareTooltip; }
    bool IsFirmwareVisible() const { return m_firmwareVisible; }
    QString GetRendererText() const { return m_rendererText; }
    bool IsRendererChecked() const { return m_rendererChecked; }
    bool IsRendererEnabled() const { return m_rendererEnabled; }
    QString GetGpuAccuracyText() const { return m_gpuAccuracyText; }
    bool IsGpuAccuracyChecked() const { return m_gpuAccuracyChecked; }
    QString GetDockedText() const { return m_dockedText; }
    bool IsDockedChecked() const { return m_dockedChecked; }
    QString GetFilterText() const { return m_filterText; }
    QString GetAAText() const { return m_aaText; }
    int GetVolume() const { return m_volume; }
    bool IsMuted() const { return m_isMuted; }
    QString GetVolumeText() const { return m_volumeText; }

    // Setters called from GMainWindow
    void SetMessageText(const QString& text);
    void SetShaderBuildingText(const QString& text);
    void SetShaderBuildingVisible(bool visible);
    void SetResScaleText(const QString& text);
    void SetEmuSpeedText(const QString& text);
    void SetEmuSpeedVisible(bool visible);
    void SetGameFpsText(const QString& text);
    void SetEmuFrametimeText(const QString& text);
    void SetTasText(const QString& text);
    void SetPerfVisible(bool visible);
    void SetFirmwareText(const QString& text);
    void SetFirmwareTooltip(const QString& text);
    void SetFirmwareVisible(bool visible);
    void SetRendererText(const QString& text);
    void SetRendererChecked(bool checked);
    void SetRendererEnabled(bool enabled);
    void SetGpuAccuracyText(const QString& text);
    void SetGpuAccuracyChecked(bool checked);
    void SetDockedText(const QString& text);
    void SetDockedChecked(bool checked);
    void SetFilterText(const QString& text);
    void SetAAText(const QString& text);
    void SetVolume(int volume);
    void SetMuted(bool muted);
    void SetVolumeText(const QString& text);

signals:
    void messageTextChanged();
    void shaderBuildingTextChanged();
    void shaderBuildingVisibleChanged();
    void resScaleTextChanged();
    void emuSpeedTextChanged();
    void emuSpeedVisibleChanged();
    void gameFpsTextChanged();
    void emuFrametimeTextChanged();
    void tasTextChanged();
    void perfVisibleChanged();
    void firmwareTextChanged();
    void firmwareTooltipChanged();
    void firmwareVisibleChanged();
    void rendererTextChanged();
    void rendererCheckedChanged();
    void rendererEnabledChanged();
    void gpuAccuracyTextChanged();
    void gpuAccuracyCheckedChanged();
    void dockedTextChanged();
    void dockedCheckedChanged();
    void filterTextChanged();
    void aaTextChanged();
    void volumeChanged();
    void isMutedChanged();
    void volumeTextChanged();

    // Action signals emitted from QML, handled by GMainWindow
    void rendererClicked();
    void gpuAccuracyClicked();
    void dockedClicked();
    void filterClicked();
    void aaClicked();
    void volumeToggled();
    void muteRequested();
    void resetVolumeRequested();

private:
    QString m_messageText;
    QString m_shaderBuildingText;
    bool m_shaderBuildingVisible = false;
    QString m_resScaleText;
    QString m_emuSpeedText;
    bool m_emuSpeedVisible = false;
    QString m_gameFpsText;
    QString m_emuFrametimeText;
    QString m_tasText;
    bool m_perfVisible = false;
    QString m_firmwareText;
    QString m_firmwareTooltip;
    bool m_firmwareVisible = false;
    QString m_rendererText;
    bool m_rendererChecked = false;
    bool m_rendererEnabled = true;
    QString m_gpuAccuracyText;
    bool m_gpuAccuracyChecked = false;
    QString m_dockedText;
    bool m_dockedChecked = false;
    QString m_filterText;
    QString m_aaText;
    int m_volume = 100;
    bool m_isMuted = false;
    QString m_volumeText;
};
