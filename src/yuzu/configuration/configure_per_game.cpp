// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>

#include "common/fs/fs_util.h"
#include "common/settings_enums.h"
#include "common/settings_input.h"
#include "configuration/shared_widget.h"
#include "core/core.h"
#include "core/file_sys/control_metadata.h"
#include "core/file_sys/patch_manager.h"
#include "core/file_sys/xts_archive.h"
#include "core/loader/loader.h"
#include "yuzu/configuration/configuration_shared.h"
#include "yuzu/configuration/configure_audio.h"
#include "yuzu/configuration/configure_cpu.h"
#include "yuzu/configuration/configure_graphics.h"
#include "yuzu/configuration/configure_graphics_advanced.h"
#include "yuzu/configuration/configure_input_per_game.h"
#include "yuzu/configuration/configure_linux_tab.h"
#include "yuzu/configuration/configure_per_game.h"
#include "yuzu/configuration/configure_per_game_addons.h"
#include "yuzu/configuration/configure_system.h"
#include "yuzu/uisettings.h"
#include "yuzu/util/util.h"
#include "yuzu/vk_device_info.h"

ConfigurePerGame::ConfigurePerGame(QWidget* parent, u64 title_id_, const std::string& file_name,
                                   std::vector<VkDeviceInfo::Record>& vk_device_records,
                                   Core::System& system_)
    : QDialog(parent), title_id{title_id_}, system{system_},
      builder{std::make_unique<ConfigurationShared::Builder>(this, !system_.IsPoweredOn())},
      tab_group{std::make_shared<std::vector<ConfigurationShared::Tab*>>()} {

    const auto file_path = std::filesystem::path(Common::FS::ToU8String(file_name));
    const auto config_file_name = title_id == 0 ? Common::FS::PathToUTF8String(file_path.filename())
                                                : fmt::format("{:016X}", title_id);
    game_config = std::make_unique<QtConfig>(config_file_name, Config::ConfigType::PerGameConfig);

    // Create sub-tabs
    addons_tab = std::make_unique<ConfigurePerGameAddons>(system_, this);
    audio_tab = std::make_unique<ConfigureAudio>(system_, tab_group, *builder, this);
    cpu_tab = std::make_unique<ConfigureCpu>(system_, tab_group, *builder, this);
    graphics_advanced_tab =
        std::make_unique<ConfigureGraphicsAdvanced>(system_, tab_group, *builder, this);
    graphics_tab = std::make_unique<ConfigureGraphics>(
        system_, vk_device_records, [&]() { graphics_advanced_tab->ExposeComputeOption(); },
        [](Settings::AspectRatio, Settings::ResolutionSetup) {}, tab_group, *builder, this);
    input_tab = std::make_unique<ConfigureInputPerGame>(system_, game_config.get(), this);
    linux_tab = std::make_unique<ConfigureLinuxTab>(system_, tab_group, *builder, this);
    system_tab = std::make_unique<ConfigureSystem>(system_, tab_group, *builder, this);

    // Build layout manually (replacing .ui file)
    auto* main_layout = new QVBoxLayout(this);

    auto* top_layout = new QHBoxLayout;

    // -- Info group box (left side) --
    auto* info_group = new QGroupBox(tr("Info"), this);
    info_group->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    auto* info_layout = new QVBoxLayout(info_group);

    scene = new QGraphicsScene(this);
    icon_view = new QGraphicsView(scene, this);
    icon_view->setMinimumSize(256, 256);
    icon_view->setMaximumSize(256, 256);
    icon_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    icon_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    icon_view->setInteractive(false);
    info_layout->addWidget(icon_view, 0, Qt::AlignHCenter);

    auto* grid = new QGridLayout;
    auto make_readonly_edit = [this]() {
        auto* edit = new QLineEdit(this);
        edit->setReadOnly(true);
        return edit;
    };

    display_name = make_readonly_edit();
    display_developer = make_readonly_edit();
    display_version = make_readonly_edit();
    display_title_id = make_readonly_edit();
    display_format = make_readonly_edit();
    display_size = make_readonly_edit();
    display_filename = make_readonly_edit();

    grid->addWidget(new QLabel(tr("Name"), this), 0, 0);
    grid->addWidget(display_name, 0, 1);
    grid->addWidget(new QLabel(tr("Developer"), this), 1, 0);
    grid->addWidget(display_developer, 1, 1);
    grid->addWidget(new QLabel(tr("Version"), this), 2, 0);
    grid->addWidget(display_version, 2, 1);
    grid->addWidget(new QLabel(tr("Title ID"), this), 3, 0);
    grid->addWidget(display_title_id, 3, 1);
    grid->addWidget(new QLabel(tr("Format"), this), 4, 0);
    grid->addWidget(display_format, 4, 1);
    grid->addWidget(new QLabel(tr("Size"), this), 5, 0);
    grid->addWidget(display_size, 5, 1);
    grid->addWidget(new QLabel(tr("Filename"), this), 6, 0);
    grid->addWidget(display_filename, 6, 1);
    info_layout->addLayout(grid);
    info_layout->addStretch();

    top_layout->addWidget(info_group);

    // -- Tab widget (right side) --
    auto* right_layout = new QVBoxLayout;
    tab_widget = new QTabWidget(this);

    tab_widget->addTab(addons_tab.get(), tr("Add-Ons"));
    tab_widget->addTab(system_tab.get(), tr("System"));
    tab_widget->addTab(cpu_tab.get(), tr("CPU"));
    tab_widget->addTab(graphics_tab.get(), tr("Graphics"));
    tab_widget->addTab(graphics_advanced_tab.get(), tr("Adv. Graphics"));
    tab_widget->addTab(audio_tab.get(), tr("Audio"));
    tab_widget->addTab(input_tab.get(), tr("Input Profiles"));

    linux_tab->setVisible(false);
#ifdef __unix__
    linux_tab->setVisible(true);
    tab_widget->addTab(linux_tab.get(), tr("Linux"));
#endif

    right_layout->addWidget(tab_widget);
    top_layout->addLayout(right_layout);

    main_layout->addLayout(top_layout);

    // -- Bottom: warning + button box --
    auto* bottom_layout = new QHBoxLayout;
    bottom_layout->addWidget(
        new QLabel(tr("Some settings are only available when a game is not running."), this));

    button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    bottom_layout->addWidget(button_box);

    main_layout->addLayout(bottom_layout);

    setLayout(main_layout);
    setMinimumWidth(900);
    setFocusPolicy(Qt::ClickFocus);
    setWindowTitle(tr("Properties"));

    addons_tab->SetTitleId(title_id);

    if (system.IsPoweredOn()) {
        QPushButton* apply_button = button_box->addButton(QDialogButtonBox::Apply);
        connect(apply_button, &QAbstractButton::clicked, this,
                &ConfigurePerGame::HandleApplyButtonClicked);
    }

    LoadConfiguration();
}

ConfigurePerGame::~ConfigurePerGame() = default;

void ConfigurePerGame::ApplyConfiguration() {
    for (const auto tab : *tab_group) {
        tab->ApplyConfiguration();
    }
    addons_tab->ApplyConfiguration();
    input_tab->ApplyConfiguration();

    if (Settings::IsDockedMode() && Settings::values.players.GetValue()[0].controller_type ==
                                        Settings::ControllerType::Handheld) {
        Settings::values.use_docked_mode.SetValue(Settings::ConsoleMode::Handheld);
        Settings::values.use_docked_mode.SetGlobal(true);
    }

    system.ApplySettings();
    Settings::LogSettings();

    game_config->SaveAllValues();
}

void ConfigurePerGame::HandleApplyButtonClicked() {
    UISettings::values.configuration_applied = true;
    ApplyConfiguration();
}

void ConfigurePerGame::LoadFromFile(FileSys::VirtualFile file_) {
    file = std::move(file_);
    LoadConfiguration();
}

void ConfigurePerGame::LoadConfiguration() {
    if (file == nullptr) {
        return;
    }

    addons_tab->LoadFromFile(file);

    display_title_id->setText(
        QStringLiteral("%1").arg(title_id, 16, 16, QLatin1Char{'0'}).toUpper());

    const FileSys::PatchManager pm{title_id, system.GetFileSystemController(),
                                   system.GetContentProvider()};
    const auto control = pm.GetControlMetadata();
    const auto loader = Loader::GetLoader(system, file);

    if (control.first != nullptr) {
        display_version->setText(QString::fromStdString(control.first->GetVersionString()));
        display_name->setText(QString::fromStdString(control.first->GetApplicationName()));
        display_developer->setText(QString::fromStdString(control.first->GetDeveloperName()));
    } else {
        std::string title;
        if (loader->ReadTitle(title) == Loader::ResultStatus::Success)
            display_name->setText(QString::fromStdString(title));

        FileSys::NACP nacp;
        if (loader->ReadControlData(nacp) == Loader::ResultStatus::Success)
            display_developer->setText(QString::fromStdString(nacp.GetDeveloperName()));

        display_version->setText(QStringLiteral("1.0.0"));
    }

    if (control.second != nullptr) {
        scene->clear();

        QPixmap map;
        const auto bytes = control.second->ReadAllBytes();
        map.loadFromData(bytes.data(), static_cast<u32>(bytes.size()));

        scene->addPixmap(map.scaled(icon_view->width(), icon_view->height(),
                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        std::vector<u8> bytes;
        if (loader->ReadIcon(bytes) == Loader::ResultStatus::Success) {
            scene->clear();

            QPixmap map;
            map.loadFromData(bytes.data(), static_cast<u32>(bytes.size()));

            scene->addPixmap(map.scaled(icon_view->width(), icon_view->height(),
                                        Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    }

    display_filename->setText(QString::fromStdString(file->GetName()));

    display_format->setText(
        QString::fromStdString(Loader::GetFileTypeString(loader->GetFileType())));

    const auto valueText = ReadableByteSize(file->GetSize());
    display_size->setText(valueText);
}
