// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <QDialog>

#include "core/file_sys/vfs/vfs_types.h"
#include "frontend_common/config.h"
#include "vk_device_info.h"
#include "yuzu/configuration/configuration_shared.h"
#include "yuzu/configuration/qt_config.h"

namespace Core {
class System;
}

namespace ConfigurationShared {
class Builder;
}

class ConfigurePerGameAddons;
class ConfigureAudio;
class ConfigureCpu;
class ConfigureGraphics;
class ConfigureGraphicsAdvanced;
class ConfigureInputPerGame;
class ConfigureLinuxTab;
class ConfigureSystem;

class QDialogButtonBox;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QLineEdit;
class QTabWidget;

class ConfigurePerGame : public QDialog {
    Q_OBJECT

public:
    explicit ConfigurePerGame(QWidget* parent, u64 title_id_, const std::string& file_name,
                              std::vector<VkDeviceInfo::Record>& vk_device_records,
                              Core::System& system_);
    ~ConfigurePerGame() override;

    void ApplyConfiguration();

    void LoadFromFile(FileSys::VirtualFile file_);

private:
    void HandleApplyButtonClicked();
    void LoadConfiguration();

    FileSys::VirtualFile file;
    u64 title_id;

    QGraphicsScene* scene;
    QGraphicsView* icon_view;
    QLineEdit* display_name;
    QLineEdit* display_developer;
    QLineEdit* display_version;
    QLineEdit* display_title_id;
    QLineEdit* display_format;
    QLineEdit* display_size;
    QLineEdit* display_filename;
    QTabWidget* tab_widget;
    QDialogButtonBox* button_box;

    std::unique_ptr<QtConfig> game_config;

    Core::System& system;
    std::unique_ptr<ConfigurationShared::Builder> builder;
    std::shared_ptr<std::vector<ConfigurationShared::Tab*>> tab_group;

    std::unique_ptr<ConfigurePerGameAddons> addons_tab;
    std::unique_ptr<ConfigureAudio> audio_tab;
    std::unique_ptr<ConfigureCpu> cpu_tab;
    std::unique_ptr<ConfigureGraphicsAdvanced> graphics_advanced_tab;
    std::unique_ptr<ConfigureGraphics> graphics_tab;
    std::unique_ptr<ConfigureInputPerGame> input_tab;
    std::unique_ptr<ConfigureLinuxTab> linux_tab;
    std::unique_ptr<ConfigureSystem> system_tab;
};
