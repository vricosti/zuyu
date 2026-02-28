// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <array>
#include <set>

#include <QCoreApplication>
#include <QDirIterator>
#include <QLocale>

#include "common/fs/path_util.h"
#include "common/settings.h"
#include "common/settings_enums.h"
#include "core/frontend/framebuffer_layout.h"
#include "yuzu/configuration/configure_ui_model.h"
#include "yuzu/uisettings.h"

namespace {
constexpr std::array default_game_icon_sizes{
    std::make_pair(0, QT_TRANSLATE_NOOP("ConfigureUI", "None")),
    std::make_pair(32, QT_TRANSLATE_NOOP("ConfigureUI", "Small (32x32)")),
    std::make_pair(64, QT_TRANSLATE_NOOP("ConfigureUI", "Standard (64x64)")),
    std::make_pair(128, QT_TRANSLATE_NOOP("ConfigureUI", "Large (128x128)")),
    std::make_pair(256, QT_TRANSLATE_NOOP("ConfigureUI", "Full Size (256x256)")),
};

constexpr std::array default_folder_icon_sizes{
    std::make_pair(0, QT_TRANSLATE_NOOP("ConfigureUI", "None")),
    std::make_pair(24, QT_TRANSLATE_NOOP("ConfigureUI", "Small (24x24)")),
    std::make_pair(48, QT_TRANSLATE_NOOP("ConfigureUI", "Standard (48x48)")),
    std::make_pair(72, QT_TRANSLATE_NOOP("ConfigureUI", "Large (72x72)")),
};

constexpr std::array row_text_names{
    QT_TRANSLATE_NOOP("ConfigureUI", "Filename"),
    QT_TRANSLATE_NOOP("ConfigureUI", "Filetype"),
    QT_TRANSLATE_NOOP("ConfigureUI", "Title ID"),
    QT_TRANSLATE_NOOP("ConfigureUI", "Title Name"),
    QT_TRANSLATE_NOOP("ConfigureUI", "None"),
};

float GetUpFactor(Settings::ResolutionSetup res_setup) {
    Settings::ResolutionScalingInfo info{};
    Settings::TranslateResolutionInfo(res_setup, info);
    return info.up_factor;
}
} // namespace

UiConfigModel::UiConfigModel(QObject* parent) : QObject(parent) {}

void UiConfigModel::initialize(Settings::AspectRatio ratio,
                                Settings::ResolutionSetup resolution) {
    m_ratio = ratio;
    m_resolutionSetting = resolution;

    // Languages
    m_languageList.clear();
    m_languageValues.clear();
    m_languageList.append(tr("<System>"));
    m_languageValues.append(QString{});
    m_languageList.append(tr("English"));
    m_languageValues.append(QStringLiteral("en"));

    QDirIterator it(QStringLiteral(":/languages"), QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        QString locale = it.next();
        locale.truncate(locale.lastIndexOf(QLatin1Char{'.'}));
        locale.remove(0, locale.lastIndexOf(QLatin1Char{'/'}) + 1);
        const QString lang = QLocale::languageToString(QLocale(locale).language());
        const QString country = QLocale::territoryToString(QLocale(locale).territory());
        m_languageList.append(QStringLiteral("%1 (%2)").arg(lang, country));
        m_languageValues.append(locale);
    }

    const auto current_lang = QString::fromStdString(UISettings::values.language.GetValue());
    m_languageIndex = m_languageValues.indexOf(current_lang);
    if (m_languageIndex < 0) {
        m_languageIndex = 0;
    }

    // Themes
    m_themeList.clear();
    m_themeValues.clear();
    for (const auto& theme : UISettings::themes) {
        m_themeList.append(QString::fromUtf8(theme.first));
        m_themeValues.append(QString::fromUtf8(theme.second));
    }
    const auto current_theme = QString::fromStdString(UISettings::values.theme);
    m_themeIndex = m_themeValues.indexOf(current_theme);
    if (m_themeIndex < 0) {
        m_themeIndex = 0;
    }

    // Checkboxes
    m_showAddOns = UISettings::values.show_add_ons.GetValue();
    m_showCompat = UISettings::values.show_compat.GetValue();
    m_showSize = UISettings::values.show_size.GetValue();
    m_showTypes = UISettings::values.show_types.GetValue();
    m_showPlayTime = UISettings::values.show_play_time.GetValue();

    // Icon sizes
    m_gameIconSizeList.clear();
    m_gameIconSizeValues.clear();
    for (size_t i = 0; i < default_game_icon_sizes.size(); ++i) {
        m_gameIconSizeList.append(
            QCoreApplication::translate("ConfigureUI", default_game_icon_sizes[i].second));
        m_gameIconSizeValues.append(default_game_icon_sizes[i].first);
    }
    const int current_game_icon = UISettings::values.game_icon_size.GetValue();
    m_gameIconSizeIndex = m_gameIconSizeValues.indexOf(current_game_icon);
    if (m_gameIconSizeIndex < 0) {
        m_gameIconSizeIndex = 2; // Standard
    }

    m_folderIconSizeList.clear();
    m_folderIconSizeValues.clear();
    for (size_t i = 0; i < default_folder_icon_sizes.size(); ++i) {
        m_folderIconSizeList.append(
            QCoreApplication::translate("ConfigureUI", default_folder_icon_sizes[i].second));
        m_folderIconSizeValues.append(default_folder_icon_sizes[i].first);
    }
    const int current_folder_icon = UISettings::values.folder_icon_size.GetValue();
    m_folderIconSizeIndex = m_folderIconSizeValues.indexOf(current_folder_icon);
    if (m_folderIconSizeIndex < 0) {
        m_folderIconSizeIndex = 2; // Standard
    }

    // Row text names
    m_rowTextNames.clear();
    for (size_t i = 0; i < row_text_names.size(); ++i) {
        m_rowTextNames.append(QCoreApplication::translate("ConfigureUI", row_text_names[i]));
    }
    m_row1TextIndex = static_cast<int>(UISettings::values.row_1_text_id.GetValue());
    m_row2TextIndex = static_cast<int>(UISettings::values.row_2_text_id.GetValue());
    updateRowLists();

    // Screenshots
    m_enableScreenshotSaveAs = UISettings::values.enable_screenshot_save_as.GetValue();
    m_screenshotPath = QString::fromStdString(
        Common::FS::GetYuzuPathString(Common::FS::YuzuPath::ScreenshotsDir));

    // Screenshot heights
    m_screenshotHeightList.clear();
    m_screenshotHeightValues.clear();
    m_screenshotHeightList.append(tr("Auto"));
    m_screenshotHeightValues.append(0);

    const auto& enumeration =
        Settings::EnumMetadata<Settings::ResolutionSetup>::Canonicalizations();
    std::set<u32> resolutions;
    for (const auto& [name, value] : enumeration) {
        const float up_factor = GetUpFactor(value);
        resolutions.emplace(static_cast<u32>(Layout::ScreenUndocked::Height * up_factor));
        resolutions.emplace(static_cast<u32>(Layout::ScreenDocked::Height * up_factor));
    }
    for (const auto res : resolutions) {
        m_screenshotHeightList.append(QString::number(res));
        m_screenshotHeightValues.append(res);
    }

    const u32 current_height = UISettings::values.screenshot_height.GetValue();
    m_screenshotHeightIndex = m_screenshotHeightValues.indexOf(current_height);
    if (m_screenshotHeightIndex < 0) {
        m_screenshotHeightIndex = 0;
    }

    updateWidthText();

    // Emit all signals
    emit languageIndexChanged();
    emit themeIndexChanged();
    emit showAddOnsChanged();
    emit showCompatChanged();
    emit showSizeChanged();
    emit showTypesChanged();
    emit showPlayTimeChanged();
    emit gameIconSizeIndexChanged();
    emit folderIconSizeIndexChanged();
    emit row1TextIndexChanged();
    emit row2TextIndexChanged();
    emit enableScreenshotSaveAsChanged();
    emit screenshotPathChanged();
    emit screenshotHeightIndexChanged();
}

void UiConfigModel::apply() {
    UISettings::values.theme = m_themeValues.value(m_themeIndex).toStdString();
    UISettings::values.show_add_ons = m_showAddOns;
    UISettings::values.show_compat = m_showCompat;
    UISettings::values.show_size = m_showSize;
    UISettings::values.show_types = m_showTypes;
    UISettings::values.show_play_time = m_showPlayTime;
    UISettings::values.game_icon_size = m_gameIconSizeValues.value(m_gameIconSizeIndex);
    UISettings::values.folder_icon_size = m_folderIconSizeValues.value(m_folderIconSizeIndex);
    UISettings::values.row_1_text_id = m_row1TextIndex;
    UISettings::values.row_2_text_id = m_row2TextIndex;
    UISettings::values.enable_screenshot_save_as = m_enableScreenshotSaveAs;

    Common::FS::SetYuzuPath(Common::FS::YuzuPath::ScreenshotsDir,
                            m_screenshotPath.toStdString());

    const u32 height = m_screenshotHeightValues.value(m_screenshotHeightIndex, 0);
    UISettings::values.screenshot_height.SetValue(height);

    UISettings::values.is_game_list_reload_pending.exchange(true);
}

void UiConfigModel::setLanguageIndex(int idx) {
    if (m_languageIndex != idx) {
        m_languageIndex = idx;
        emit languageIndexChanged();
        emit languageChanged(currentLanguageCode());
    }
}

void UiConfigModel::setThemeIndex(int idx) {
    if (m_themeIndex != idx) {
        m_themeIndex = idx;
        emit themeIndexChanged();
    }
}

void UiConfigModel::setShowAddOns(bool val) {
    if (m_showAddOns != val) { m_showAddOns = val; emit showAddOnsChanged(); }
}
void UiConfigModel::setShowCompat(bool val) {
    if (m_showCompat != val) { m_showCompat = val; emit showCompatChanged(); }
}
void UiConfigModel::setShowSize(bool val) {
    if (m_showSize != val) { m_showSize = val; emit showSizeChanged(); }
}
void UiConfigModel::setShowTypes(bool val) {
    if (m_showTypes != val) { m_showTypes = val; emit showTypesChanged(); }
}
void UiConfigModel::setShowPlayTime(bool val) {
    if (m_showPlayTime != val) { m_showPlayTime = val; emit showPlayTimeChanged(); }
}

void UiConfigModel::setGameIconSizeIndex(int idx) {
    if (m_gameIconSizeIndex != idx) {
        m_gameIconSizeIndex = idx;
        emit gameIconSizeIndexChanged();
    }
}

void UiConfigModel::setFolderIconSizeIndex(int idx) {
    if (m_folderIconSizeIndex != idx) {
        m_folderIconSizeIndex = idx;
        emit folderIconSizeIndexChanged();
    }
}

void UiConfigModel::setRow1TextIndex(int idx) {
    if (m_row1TextIndex != idx) {
        m_row1TextIndex = idx;
        emit row1TextIndexChanged();
        updateRowLists();
    }
}

void UiConfigModel::setRow2TextIndex(int idx) {
    if (m_row2TextIndex != idx) {
        m_row2TextIndex = idx;
        emit row2TextIndexChanged();
        updateRowLists();
    }
}

void UiConfigModel::setEnableScreenshotSaveAs(bool val) {
    if (m_enableScreenshotSaveAs != val) {
        m_enableScreenshotSaveAs = val;
        emit enableScreenshotSaveAsChanged();
    }
}

void UiConfigModel::setScreenshotPath(const QString& path) {
    if (m_screenshotPath != path) {
        m_screenshotPath = path;
        emit screenshotPathChanged();
    }
}

void UiConfigModel::setScreenshotHeightIndex(int idx) {
    if (m_screenshotHeightIndex != idx) {
        m_screenshotHeightIndex = idx;
        emit screenshotHeightIndexChanged();
        updateWidthText();
    }
}

void UiConfigModel::updateScreenshotInfo(Settings::AspectRatio ratio,
                                          Settings::ResolutionSetup resolution) {
    m_ratio = ratio;
    m_resolutionSetting = resolution;
    updateWidthText();
}

QString UiConfigModel::currentLanguageCode() const {
    return m_languageValues.value(m_languageIndex);
}

void UiConfigModel::updateWidthText() {
    const u32 height = m_screenshotHeightValues.value(m_screenshotHeightIndex, 0);
    const u32 width = UISettings::CalculateWidth(height, m_ratio);

    if (height == 0) {
        const auto up_factor = GetUpFactor(m_resolutionSetting);
        const u32 height_docked = Layout::ScreenDocked::Height * up_factor;
        const u32 width_docked = UISettings::CalculateWidth(height_docked, m_ratio);
        const u32 height_undocked = Layout::ScreenUndocked::Height * up_factor;
        const u32 width_undocked = UISettings::CalculateWidth(height_undocked, m_ratio);
        m_screenshotWidthText = tr("Auto (%1 x %2, %3 x %4)")
                                    .arg(width_undocked)
                                    .arg(height_undocked)
                                    .arg(width_docked)
                                    .arg(height_docked);
    } else {
        m_screenshotWidthText = QStringLiteral("%1 x").arg(width);
    }
    emit screenshotWidthTextChanged();
}

void UiConfigModel::updateRowLists() {
    // Row 1 list: all except "None" and whatever row 2 has selected
    m_row1TextList.clear();
    for (int i = 0; i < m_rowTextNames.size(); ++i) {
        if (i == 4) continue; // "None" not allowed in row 1
        if (i == m_row2TextIndex) continue;
        m_row1TextList.append(m_rowTextNames[i]);
    }

    // Row 2 list: all except whatever row 1 has selected
    m_row2TextList.clear();
    for (int i = 0; i < m_rowTextNames.size(); ++i) {
        if (i == m_row1TextIndex) continue;
        m_row2TextList.append(m_rowTextNames[i]);
    }

    emit row1TextListChanged();
    emit row2TextListChanged();
}
