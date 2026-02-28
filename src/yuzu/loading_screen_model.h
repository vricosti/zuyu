// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQuickImageProvider>
#include <QString>

/**
 * LoadingScreenModel exposes loading screen state to QML.
 */
class LoadingScreenModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString stageText READ GetStageText NOTIFY stageTextChanged)
    Q_PROPERTY(QString estimateText READ GetEstimateText NOTIFY estimateTextChanged)
    Q_PROPERTY(int progressValue READ GetProgressValue NOTIFY progressValueChanged)
    Q_PROPERTY(int progressMax READ GetProgressMax NOTIFY progressMaxChanged)
    Q_PROPERTY(bool progressVisible READ IsProgressVisible NOTIFY progressVisibleChanged)
    Q_PROPERTY(int stage READ GetStage NOTIFY stageChanged)
    Q_PROPERTY(qreal fadeOpacity READ GetFadeOpacity NOTIFY fadeOpacityChanged)
    Q_PROPERTY(bool logoAvailable READ IsLogoAvailable NOTIFY logoAvailableChanged)
    Q_PROPERTY(bool bannerAvailable READ IsBannerAvailable NOTIFY bannerAvailableChanged)

public:
    explicit LoadingScreenModel(QObject* parent = nullptr);

    QString GetStageText() const { return m_stageText; }
    QString GetEstimateText() const { return m_estimateText; }
    int GetProgressValue() const { return m_progressValue; }
    int GetProgressMax() const { return m_progressMax; }
    bool IsProgressVisible() const { return m_progressVisible; }
    int GetStage() const { return m_stage; }
    qreal GetFadeOpacity() const { return m_fadeOpacity; }
    bool IsLogoAvailable() const { return m_logoAvailable; }
    bool IsBannerAvailable() const { return m_bannerAvailable; }

    void SetStageText(const QString& text);
    void SetEstimateText(const QString& text);
    void SetProgressValue(int value);
    void SetProgressMax(int max);
    void SetProgressVisible(bool visible);
    void SetStage(int stage);
    void SetFadeOpacity(qreal opacity);
    void SetLogoAvailable(bool available);
    void SetBannerAvailable(bool available);

signals:
    void stageTextChanged();
    void estimateTextChanged();
    void progressValueChanged();
    void progressMaxChanged();
    void progressVisibleChanged();
    void stageChanged();
    void fadeOpacityChanged();
    void logoAvailableChanged();
    void bannerAvailableChanged();

private:
    QString m_stageText;
    QString m_estimateText;
    int m_progressValue = 0;
    int m_progressMax = 0;
    bool m_progressVisible = false;
    int m_stage = 0; // 0=Prepare, 1=Build, 2=Complete
    qreal m_fadeOpacity = 1.0;
    bool m_logoAvailable = false;
    bool m_bannerAvailable = false;
};

/**
 * Image provider for loading screen logo and banner.
 */
class LoadingScreenImageProvider : public QQuickImageProvider {
public:
    LoadingScreenImageProvider();

    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) override;

    void SetLogo(const QPixmap& pixmap);
    void SetBanner(const QPixmap& pixmap);

private:
    QPixmap m_logo;
    QPixmap m_banner;
};
