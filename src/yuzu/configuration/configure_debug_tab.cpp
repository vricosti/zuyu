// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <memory>
#include <QTabWidget>
#include <QVBoxLayout>

#include "yuzu/configuration/configure_cpu_debug.h"
#include "yuzu/configuration/configure_debug.h"
#include "yuzu/configuration/configure_debug_tab.h"

ConfigureDebugTab::ConfigureDebugTab(const Core::System& system_, QWidget* parent)
    : QWidget(parent), debug_tab{std::make_unique<ConfigureDebug>(system_, this)},
      cpu_debug_tab{std::make_unique<ConfigureCpuDebug>(system_, this)} {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    tab_widget = new QTabWidget(this);
    tab_widget->addTab(debug_tab.get(), tr("Debug"));
    tab_widget->addTab(cpu_debug_tab.get(), tr("CPU"));

    layout->addWidget(tab_widget);
    setLayout(layout);
}

ConfigureDebugTab::~ConfigureDebugTab() = default;

void ConfigureDebugTab::ApplyConfiguration() {
    debug_tab->ApplyConfiguration();
    cpu_debug_tab->ApplyConfiguration();
}

void ConfigureDebugTab::SetCurrentIndex(int index) {
    tab_widget->setCurrentIndex(index);
}
