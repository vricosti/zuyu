// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "common/fs/fs.h"
#include "common/fs/path_util.h"
#include "common/settings.h"
#include "yuzu/configuration/configure_filesystem_model.h"
#include "yuzu/uisettings.h"

FilesystemConfigModel::FilesystemConfigModel(QObject* parent) : QObject(parent) {}

void FilesystemConfigModel::initialize() {
    m_nandDir = QString::fromStdString(
        Common::FS::GetYuzuPathString(Common::FS::YuzuPath::NANDDir));
    m_sdmcDir = QString::fromStdString(
        Common::FS::GetYuzuPathString(Common::FS::YuzuPath::SDMCDir));
    m_gamecardPath =
        QString::fromStdString(Settings::values.gamecard_path.GetValue());
    m_dumpDir = QString::fromStdString(
        Common::FS::GetYuzuPathString(Common::FS::YuzuPath::DumpDir));
    m_loadDir = QString::fromStdString(
        Common::FS::GetYuzuPathString(Common::FS::YuzuPath::LoadDir));

    m_gamecardInserted = Settings::values.gamecard_inserted.GetValue();
    m_gamecardCurrentGame = Settings::values.gamecard_current_game.GetValue();
    m_dumpExefs = Settings::values.dump_exefs.GetValue();
    m_dumpNso = Settings::values.dump_nso.GetValue();
    m_cacheGameList = UISettings::values.cache_game_list.GetValue();

    emit nandDirChanged();
    emit sdmcDirChanged();
    emit gamecardPathChanged();
    emit dumpDirChanged();
    emit loadDirChanged();
    emit gamecardInsertedChanged();
    emit gamecardCurrentGameChanged();
    emit dumpExefsChanged();
    emit dumpNsoChanged();
    emit cacheGameListChanged();
}

void FilesystemConfigModel::setNandDir(const QString& dir) {
    if (m_nandDir != dir) { m_nandDir = dir; emit nandDirChanged(); }
}
void FilesystemConfigModel::setSdmcDir(const QString& dir) {
    if (m_sdmcDir != dir) { m_sdmcDir = dir; emit sdmcDirChanged(); }
}
void FilesystemConfigModel::setGamecardPath(const QString& path) {
    if (m_gamecardPath != path) { m_gamecardPath = path; emit gamecardPathChanged(); }
}
void FilesystemConfigModel::setDumpDir(const QString& dir) {
    if (m_dumpDir != dir) { m_dumpDir = dir; emit dumpDirChanged(); }
}
void FilesystemConfigModel::setLoadDir(const QString& dir) {
    if (m_loadDir != dir) { m_loadDir = dir; emit loadDirChanged(); }
}
void FilesystemConfigModel::setGamecardInserted(bool val) {
    if (m_gamecardInserted != val) { m_gamecardInserted = val; emit gamecardInsertedChanged(); }
}
void FilesystemConfigModel::setGamecardCurrentGame(bool val) {
    if (m_gamecardCurrentGame != val) {
        m_gamecardCurrentGame = val; emit gamecardCurrentGameChanged();
    }
}
void FilesystemConfigModel::setDumpExefs(bool val) {
    if (m_dumpExefs != val) { m_dumpExefs = val; emit dumpExefsChanged(); }
}
void FilesystemConfigModel::setDumpNso(bool val) {
    if (m_dumpNso != val) { m_dumpNso = val; emit dumpNsoChanged(); }
}
void FilesystemConfigModel::setCacheGameList(bool val) {
    if (m_cacheGameList != val) { m_cacheGameList = val; emit cacheGameListChanged(); }
}

void FilesystemConfigModel::apply() {
    Common::FS::SetYuzuPath(Common::FS::YuzuPath::NANDDir, m_nandDir.toStdString());
    Common::FS::SetYuzuPath(Common::FS::YuzuPath::SDMCDir, m_sdmcDir.toStdString());
    Common::FS::SetYuzuPath(Common::FS::YuzuPath::DumpDir, m_dumpDir.toStdString());
    Common::FS::SetYuzuPath(Common::FS::YuzuPath::LoadDir, m_loadDir.toStdString());

    Settings::values.gamecard_inserted = m_gamecardInserted;
    Settings::values.gamecard_current_game = m_gamecardCurrentGame;
    Settings::values.dump_exefs = m_dumpExefs;
    Settings::values.dump_nso = m_dumpNso;

    UISettings::values.cache_game_list = m_cacheGameList;
}
