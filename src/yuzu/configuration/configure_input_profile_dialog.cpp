// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/core.h"
#include "yuzu/configuration/configure_input_player.h"
#include "yuzu/configuration/configure_input_profile_dialog.h"

ConfigureInputProfileDialog::ConfigureInputProfileDialog(
    QWidget* parent, InputCommon::InputSubsystem* input_subsystem, InputProfiles* profiles,
    Core::System& system)
    : QDialog(parent),
      profile_widget(new ConfigureInputPlayer(this, 9, nullptr, input_subsystem, profiles,
                                              system.HIDCore(), system.IsPoweredOn(), false)) {
    setWindowTitle(tr("Create Input Profile"));

    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(2);
    main_layout->setContentsMargins(9, 9, 9, 9);

    main_layout->addWidget(profile_widget);

    auto* bottom_layout = new QHBoxLayout;

    auto* clear_button = new QPushButton(tr("Clear"), this);
    connect(clear_button, &QPushButton::clicked, this,
            [this] { profile_widget->ClearAll(); });
    bottom_layout->addWidget(clear_button);

    auto* defaults_button = new QPushButton(tr("Defaults"), this);
    connect(defaults_button, &QPushButton::clicked, this,
            [this] { profile_widget->RestoreDefaults(); });
    bottom_layout->addWidget(defaults_button);

    auto* button_box = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    bottom_layout->addWidget(button_box);

    main_layout->addLayout(bottom_layout);
}

ConfigureInputProfileDialog::~ConfigureInputProfileDialog() = default;
