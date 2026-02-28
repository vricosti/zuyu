// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QVBoxLayout>

#include "common/fs/fs.h"
#include "common/fs/path_util.h"
#include "common/settings.h"
#include "yuzu/configuration/configure_tas.h"
#include "yuzu/uisettings.h"

ConfigureTasDialog::ConfigureTasDialog(QWidget* parent) : QDialog(parent) {
    setFocusPolicy(Qt::ClickFocus);
    setWindowTitle(tr("TAS Configuration"));

    auto* main_layout = new QVBoxLayout(this);

    // Info group
    auto* info_group = new QGroupBox(tr("TAS"), this);
    auto* info_layout = new QVBoxLayout(info_group);
    auto* info_label = new QLabel(
        tr("<html><head/><body><p>Reads controller input from scripts in the same format as TAS-nx "
           "scripts.<br/>For a more detailed explanation, please consult the <a "
           "href=\"https://yuzu-emu.org/help/feature/tas/\"><span style=\" text-decoration: "
           "underline; color:#039be5;\">help page</span></a> on the yuzu "
           "website.</p></body></html>"),
        this);
    info_label->setOpenExternalLinks(true);
    info_layout->addWidget(info_label);

    auto* hotkey_label = new QLabel(
        tr("To check which hotkeys control the playback/recording, please refer to the Hotkey "
           "settings (Configure -> General -> Hotkeys)."),
        this);
    hotkey_label->setWordWrap(true);
    info_layout->addWidget(hotkey_label);

    auto* warning_label = new QLabel(
        tr("WARNING: This is an experimental feature.<br/>It will not play back scripts frame "
           "perfectly with the current, imperfect syncing method."),
        this);
    warning_label->setWordWrap(true);
    info_layout->addWidget(warning_label);

    main_layout->addWidget(info_group);

    // Settings group
    auto* settings_group = new QGroupBox(tr("Settings"), this);
    auto* settings_layout = new QVBoxLayout(settings_group);

    tas_enable = new QCheckBox(tr("Enable TAS features"), this);
    settings_layout->addWidget(tas_enable);

    tas_loop_script = new QCheckBox(tr("Loop script"), this);
    settings_layout->addWidget(tas_loop_script);

    tas_pause_on_load = new QCheckBox(tr("Pause execution during loads"), this);
    tas_pause_on_load->setEnabled(false);
    settings_layout->addWidget(tas_pause_on_load);

    main_layout->addWidget(settings_group);

    // Script Directory group
    auto* dir_group = new QGroupBox(tr("Script Directory"), this);
    auto* dir_layout = new QHBoxLayout(dir_group);

    dir_layout->addWidget(new QLabel(tr("Path"), this));

    tas_path_edit = new QLineEdit(this);
    dir_layout->addWidget(tas_path_edit);

    auto* browse_button = new QToolButton(this);
    browse_button->setText(QStringLiteral("..."));
    connect(browse_button, &QToolButton::pressed, this,
            [this] { SetDirectory(tas_path_edit); });
    dir_layout->addWidget(browse_button);

    main_layout->addWidget(dir_group);

    main_layout->addStretch();

    // Button box
    auto* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    main_layout->addWidget(button_box);

    LoadConfiguration();
}

ConfigureTasDialog::~ConfigureTasDialog() = default;

void ConfigureTasDialog::LoadConfiguration() {
    tas_path_edit->setText(
        QString::fromStdString(Common::FS::GetYuzuPathString(Common::FS::YuzuPath::TASDir)));
    tas_enable->setChecked(Settings::values.tas_enable.GetValue());
    tas_loop_script->setChecked(Settings::values.tas_loop.GetValue());
    tas_pause_on_load->setChecked(Settings::values.pause_tas_on_load.GetValue());
}

void ConfigureTasDialog::ApplyConfiguration() {
    Common::FS::SetYuzuPath(Common::FS::YuzuPath::TASDir,
                            tas_path_edit->text().toStdString());
    Settings::values.tas_enable.SetValue(tas_enable->isChecked());
    Settings::values.tas_loop.SetValue(tas_loop_script->isChecked());
    Settings::values.pause_tas_on_load.SetValue(tas_pause_on_load->isChecked());
}

void ConfigureTasDialog::SetDirectory(QLineEdit* edit) {
    QString str =
        QFileDialog::getExistingDirectory(this, tr("Select TAS Load Directory..."), edit->text());

    if (str.isEmpty()) {
        return;
    }

    if (str.back() != QChar::fromLatin1('/')) {
        str.append(QChar::fromLatin1('/'));
    }

    edit->setText(str);
}

void ConfigureTasDialog::HandleApplyButtonClicked() {
    UISettings::values.configuration_applied = true;
    ApplyConfiguration();
}
