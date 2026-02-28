// SPDX-FileCopyrightText: Copyright 2019 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStringList>
#include <QVBoxLayout>

#include "common/settings.h"
#include "core/core.h"
#include "core/internal_network/network_interface.h"
#include "yuzu/configuration/configure_network.h"
#include "yuzu/qml_bridge.h"

ConfigureNetwork::ConfigureNetwork(const Core::System& system_, QWidget* parent)
    : QWidget(parent), system{system_} {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Build interface list
    QStringList interfaces;
    interfaces.append(tr("None"));
    for (const auto& iface : Network::GetAvailableNetworkInterfaces()) {
        interfaces.append(QString::fromStdString(iface.name));
    }

    selected_interface =
        QString::fromStdString(Settings::values.network_interface.GetValue());

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("networkInterfaces"), interfaces);
    ctx->setContextProperty(QStringLiteral("currentInterface"), selected_interface);
    ctx->setContextProperty(QStringLiteral("isLocked"), system.IsPoweredOn());

    quick_widget->setSource(QUrl(QStringLiteral("qrc:/qml/qml/ConfigureNetwork.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureNetwork QML Error: {}",
                      error.toString().toStdString());
        }
    }

    // Connect QML signal
    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        connect(root, SIGNAL(interfaceChanged(QString)), this,
                SLOT(onInterfaceChanged(QString)));
    }

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureNetwork::~ConfigureNetwork() = default;

void ConfigureNetwork::onInterfaceChanged(const QString& name) {
    selected_interface = name;
}

void ConfigureNetwork::ApplyConfiguration() {
    Settings::values.network_interface = selected_interface.toStdString();
}
