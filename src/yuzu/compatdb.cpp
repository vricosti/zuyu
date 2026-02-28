// SPDX-FileCopyrightText: 2017 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QMessageBox>
#include <QQmlContext>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QtConcurrent/qtconcurrentrun.h>

#include "common/logging/log.h"
#include "common/telemetry.h"
#include "core/telemetry_session.h"
#include "yuzu/compatdb.h"
#include "yuzu/compatdb_model.h"
#include "yuzu/qml_bridge.h"

CompatDB::CompatDB(Core::TelemetrySession& telemetry_session_, QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint),
      telemetry_session{telemetry_session_} {

    setWindowTitle(tr("Report Compatibility"));
    setMinimumSize(500, 410);
    resize(600, 482);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    model = new CompatDBModel(this);

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("compatModel"), model);

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/CompatDBWizard.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "CompatDB QML Error: {}", error.toString().toStdString());
        }
    }

    layout->addWidget(quick_widget);
    setLayout(layout);

    connect(model, &CompatDBModel::submitRequested, this, &CompatDB::Submit);
    connect(model, &CompatDBModel::cancelRequested, this, &CompatDB::reject);
    connect(&testcase_watcher, &QFutureWatcher<bool>::finished, this,
            &CompatDB::OnTestcaseSubmitted);
}

CompatDB::~CompatDB() = default;

void CompatDB::Submit() {
    const int compatibility = static_cast<int>(CalculateCompatibility());

    LOG_INFO(Frontend, "Compatibility Rating: {}", compatibility);
    telemetry_session.AddField(Common::Telemetry::FieldType::UserFeedback, "Compatibility",
                               compatibility);

    model->setIsSubmitting(true);

    testcase_watcher.setFuture(
        QtConcurrent::run([this] { return telemetry_session.SubmitTestcase(); }));
}

CompatibilityStatus CompatDB::CalculateCompatibility() const {
    // gameBootSelection: 0 = Yes, 1 = No
    if (model->gameBootSelection() == 1) {
        return CompatibilityStatus::WontBoot;
    }

    if (model->gameplaySelection() == 1) {
        return CompatibilityStatus::IntroMenu;
    }

    if (model->freezeSelection() == 1 || model->completionSelection() == 1) {
        return CompatibilityStatus::Ingame;
    }

    // graphicalSelection: 0 = Major, 1 = Minor, 2 = None
    // audioSelection: 0 = Major, 1 = Minor, 2 = None
    if (model->graphicalSelection() == 0 || model->audioSelection() == 0) {
        return CompatibilityStatus::Ingame;
    }

    if (model->graphicalSelection() == 1 || model->audioSelection() == 1) {
        return CompatibilityStatus::Playable;
    }

    return CompatibilityStatus::Perfect;
}

void CompatDB::OnTestcaseSubmitted() {
    if (!testcase_watcher.result()) {
        QMessageBox::critical(this, tr("Communication error"),
                              tr("An error occurred while sending the Testcase"));
        model->setIsSubmitting(false);
    } else {
        model->setIsSubmitting(false);
        model->setIsFinished(true);
    }
}
