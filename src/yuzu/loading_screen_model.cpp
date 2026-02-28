// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "yuzu/loading_screen_model.h"

LoadingScreenModel::LoadingScreenModel(QObject* parent) : QObject(parent) {}

void LoadingScreenModel::SetStageText(const QString& text) {
    if (m_stageText != text) {
        m_stageText = text;
        emit stageTextChanged();
    }
}

void LoadingScreenModel::SetEstimateText(const QString& text) {
    if (m_estimateText != text) {
        m_estimateText = text;
        emit estimateTextChanged();
    }
}

void LoadingScreenModel::SetProgressValue(int value) {
    if (m_progressValue != value) {
        m_progressValue = value;
        emit progressValueChanged();
    }
}

void LoadingScreenModel::SetProgressMax(int max) {
    if (m_progressMax != max) {
        m_progressMax = max;
        emit progressMaxChanged();
    }
}

void LoadingScreenModel::SetProgressVisible(bool visible) {
    if (m_progressVisible != visible) {
        m_progressVisible = visible;
        emit progressVisibleChanged();
    }
}

void LoadingScreenModel::SetStage(int stage) {
    if (m_stage != stage) {
        m_stage = stage;
        emit stageChanged();
    }
}

void LoadingScreenModel::SetFadeOpacity(qreal opacity) {
    if (m_fadeOpacity != opacity) {
        m_fadeOpacity = opacity;
        emit fadeOpacityChanged();
    }
}

void LoadingScreenModel::SetLogoAvailable(bool available) {
    if (m_logoAvailable != available) {
        m_logoAvailable = available;
        emit logoAvailableChanged();
    }
}

void LoadingScreenModel::SetBannerAvailable(bool available) {
    if (m_bannerAvailable != available) {
        m_bannerAvailable = available;
        emit bannerAvailableChanged();
    }
}

// Image Provider

LoadingScreenImageProvider::LoadingScreenImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

QPixmap LoadingScreenImageProvider::requestPixmap(const QString& id, QSize* size,
                                                   const QSize& requestedSize) {
    QPixmap result;
    if (id == QStringLiteral("logo")) {
        result = m_logo;
    } else if (id == QStringLiteral("banner")) {
        result = m_banner;
    }

    if (size) {
        *size = result.size();
    }
    if (requestedSize.isValid() && !result.isNull()) {
        return result.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return result;
}

void LoadingScreenImageProvider::SetLogo(const QPixmap& pixmap) {
    m_logo = pixmap;
}

void LoadingScreenImageProvider::SetBanner(const QPixmap& pixmap) {
    m_banner = pixmap;
}
