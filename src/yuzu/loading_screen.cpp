// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <unordered_map>
#include <QPixmap>
#include <QTime>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QTimer>
#include <QVBoxLayout>

#include "common/logging/log.h"
#include "core/frontend/framebuffer_layout.h"
#include "core/loader/loader.h"
#include "video_core/rasterizer_interface.h"
#include "yuzu/loading_screen.h"
#include "yuzu/loading_screen_model.h"
#include "yuzu/qml_bridge.h"

LoadingScreen::LoadingScreen(QWidget* parent)
    : QWidget(parent), previous_stage(VideoCore::LoadCallbackStage::Complete) {

    setMinimumSize(Layout::MinimumSize::Width, Layout::MinimumSize::Height);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    model = new LoadingScreenModel(this);
    image_provider = new LoadingScreenImageProvider();

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // Register image provider (ownership transferred to QML engine)
    quick_widget->engine()->addImageProvider(QStringLiteral("loadingscreen"), image_provider);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("loadingModel"), model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/LoadingScreen.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "LoadingScreen QML Error: {}", error.toString().toStdString());
        }
    }

    layout->addWidget(quick_widget);
    setLayout(layout);

    // Set up fade timer for smooth animation
    fade_timer = new QTimer(this);
    fade_timer->setInterval(16); // ~60fps
    connect(fade_timer, &QTimer::timeout, this, [this] {
        fade_opacity -= 0.04; // ~500ms total fade
        if (fade_opacity <= 0.0) {
            fade_opacity = 0.0;
            fade_timer->stop();
            hide();
            model->SetFadeOpacity(1.0);
            fade_opacity = 1.0;
            emit Hidden();
        } else {
            model->SetFadeOpacity(fade_opacity);
        }
    });

    connect(this, &LoadingScreen::LoadProgress, this, &LoadingScreen::OnLoadProgress,
            Qt::QueuedConnection);
    qRegisterMetaType<VideoCore::LoadCallbackStage>();

    stage_translations = {
        {VideoCore::LoadCallbackStage::Prepare, tr("Loading...")},
        {VideoCore::LoadCallbackStage::Build, tr("Loading Shaders %1 / %2")},
        {VideoCore::LoadCallbackStage::Complete, tr("Launching...")},
    };
}

LoadingScreen::~LoadingScreen() = default;

void LoadingScreen::Prepare(Loader::AppLoader& loader) {
    std::vector<u8> buffer;
    if (loader.ReadBanner(buffer) == Loader::ResultStatus::Success) {
        QPixmap map;
        map.loadFromData(buffer.data(), static_cast<uint>(buffer.size()));
        image_provider->SetBanner(map);
        model->SetBannerAvailable(true);
        buffer.clear();
    }
    if (loader.ReadLogo(buffer) == Loader::ResultStatus::Success) {
        QPixmap map;
        map.loadFromData(buffer.data(), static_cast<uint>(buffer.size()));
        image_provider->SetLogo(map);
        model->SetLogoAvailable(true);
    }

    slow_shader_compile_start = false;
    fade_opacity = 1.0;
    model->SetFadeOpacity(1.0);
    OnLoadProgress(VideoCore::LoadCallbackStage::Prepare, 0, 0);
}

void LoadingScreen::OnLoadComplete() {
    fade_timer->start();
}

void LoadingScreen::OnLoadProgress(VideoCore::LoadCallbackStage stage, std::size_t value,
                                   std::size_t total) {
    using namespace std::chrono;
    const auto now = steady_clock::now();

    // Reset the timer if the stage changes
    if (stage != previous_stage) {
        if (stage == VideoCore::LoadCallbackStage::Prepare) {
            model->SetProgressVisible(false);
        } else {
            model->SetProgressVisible(true);
        }
        model->SetStage(static_cast<int>(stage));
        previous_stage = stage;
        slow_shader_compile_start = false;
    }

    // Update max if changed
    if (total != previous_total) {
        model->SetProgressMax(static_cast<int>(total));
        previous_total = total;
    }

    // Reset ranges for Complete stage
    if (stage == VideoCore::LoadCallbackStage::Complete) {
        model->SetProgressMax(0);
    }

    QString estimate;
    // ETA estimation for slow shader compilation
    if (now - previous_time > milliseconds{50} || slow_shader_compile_start) {
        if (!slow_shader_compile_start) {
            slow_shader_start = steady_clock::now();
            slow_shader_compile_start = true;
            slow_shader_first_value = value;
        }
        const auto diff = duration_cast<milliseconds>(now - slow_shader_start);
        if (diff > seconds{1}) {
            const auto eta_mseconds =
                static_cast<long>(static_cast<double>(total - slow_shader_first_value) /
                                  (value - slow_shader_first_value) * diff.count());
            estimate =
                tr("Estimated Time %1")
                    .arg(QTime(0, 0, 0, 0)
                             .addMSecs(std::max<long>(eta_mseconds - diff.count() + 1000, 1000))
                             .toString(QStringLiteral("mm:ss")));
        }
    }

    // Update text
    if (stage == VideoCore::LoadCallbackStage::Build) {
        model->SetStageText(stage_translations[stage].arg(value).arg(total));
    } else {
        model->SetStageText(stage_translations[stage]);
    }
    model->SetEstimateText(estimate);
    model->SetProgressValue(static_cast<int>(value));
    previous_time = now;
}

void LoadingScreen::Clear() {
    image_provider->SetLogo(QPixmap());
    image_provider->SetBanner(QPixmap());
    model->SetLogoAvailable(false);
    model->SetBannerAvailable(false);
}
