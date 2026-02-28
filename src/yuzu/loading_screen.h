// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <chrono>
#include <memory>
#include <QString>
#include <QWidget>

namespace Loader {
class AppLoader;
}

namespace VideoCore {
enum class LoadCallbackStage;
}

class LoadingScreenImageProvider;
class LoadingScreenModel;
class QQuickWidget;

class LoadingScreen : public QWidget {
    Q_OBJECT

public:
    explicit LoadingScreen(QWidget* parent = nullptr);
    ~LoadingScreen();

    /// Call before showing the loading screen to load the widgets with the logo and banner for the
    /// currently loaded application.
    void Prepare(Loader::AppLoader& loader);

    /// After the loading screen is hidden, the owner of this class can call this to clean up any
    /// used resources such as the logo and banner.
    void Clear();

    /// Slot used to update the status of the progress bar
    void OnLoadProgress(VideoCore::LoadCallbackStage stage, std::size_t value, std::size_t total);

    /// Hides the LoadingScreen with a fade out effect
    void OnLoadComplete();

signals:
    void LoadProgress(VideoCore::LoadCallbackStage stage, std::size_t value, std::size_t total);
    /// Signals that this widget is completely hidden now and should be replaced with the other
    /// widget
    void Hidden();

private:
    LoadingScreenModel* model = nullptr;
    LoadingScreenImageProvider* image_provider = nullptr;
    QQuickWidget* quick_widget = nullptr;

    std::size_t previous_total = 0;
    VideoCore::LoadCallbackStage previous_stage;

    std::unordered_map<VideoCore::LoadCallbackStage, QString> stage_translations;

    bool slow_shader_compile_start = false;
    std::chrono::steady_clock::time_point slow_shader_start;
    std::chrono::steady_clock::time_point previous_time;
    std::size_t slow_shader_first_value = 0;

    // Fade animation
    QTimer* fade_timer = nullptr;
    qreal fade_opacity = 1.0;
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_DECLARE_METATYPE(VideoCore::LoadCallbackStage);
#endif
