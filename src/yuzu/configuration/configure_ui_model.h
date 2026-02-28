// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include "common/common_types.h"
#include "common/settings_enums.h"

class UiConfigModel : public QObject {
    Q_OBJECT

    // Language & Theme
    Q_PROPERTY(int languageIndex READ languageIndex WRITE setLanguageIndex NOTIFY
                   languageIndexChanged)
    Q_PROPERTY(QStringList languageList READ languageList CONSTANT)
    Q_PROPERTY(int themeIndex READ themeIndex WRITE setThemeIndex NOTIFY themeIndexChanged)
    Q_PROPERTY(QStringList themeList READ themeList CONSTANT)

    // Game List checkboxes
    Q_PROPERTY(bool showAddOns READ showAddOns WRITE setShowAddOns NOTIFY showAddOnsChanged)
    Q_PROPERTY(bool showCompat READ showCompat WRITE setShowCompat NOTIFY showCompatChanged)
    Q_PROPERTY(bool showSize READ showSize WRITE setShowSize NOTIFY showSizeChanged)
    Q_PROPERTY(bool showTypes READ showTypes WRITE setShowTypes NOTIFY showTypesChanged)
    Q_PROPERTY(bool showPlayTime READ showPlayTime WRITE setShowPlayTime NOTIFY showPlayTimeChanged)

    // Icon sizes
    Q_PROPERTY(int gameIconSizeIndex READ gameIconSizeIndex WRITE setGameIconSizeIndex NOTIFY
                   gameIconSizeIndexChanged)
    Q_PROPERTY(QStringList gameIconSizeList READ gameIconSizeList CONSTANT)
    Q_PROPERTY(int folderIconSizeIndex READ folderIconSizeIndex WRITE setFolderIconSizeIndex NOTIFY
                   folderIconSizeIndexChanged)
    Q_PROPERTY(QStringList folderIconSizeList READ folderIconSizeList CONSTANT)

    // Row text
    Q_PROPERTY(int row1TextIndex READ row1TextIndex WRITE setRow1TextIndex NOTIFY
                   row1TextIndexChanged)
    Q_PROPERTY(QStringList row1TextList READ row1TextList NOTIFY row1TextListChanged)
    Q_PROPERTY(int row2TextIndex READ row2TextIndex WRITE setRow2TextIndex NOTIFY
                   row2TextIndexChanged)
    Q_PROPERTY(QStringList row2TextList READ row2TextList NOTIFY row2TextListChanged)

    // Screenshots
    Q_PROPERTY(bool enableScreenshotSaveAs READ enableScreenshotSaveAs WRITE
                   setEnableScreenshotSaveAs NOTIFY enableScreenshotSaveAsChanged)
    Q_PROPERTY(QString screenshotPath READ screenshotPath WRITE setScreenshotPath NOTIFY
                   screenshotPathChanged)
    Q_PROPERTY(int screenshotHeightIndex READ screenshotHeightIndex WRITE setScreenshotHeightIndex
                   NOTIFY screenshotHeightIndexChanged)
    Q_PROPERTY(QStringList screenshotHeightList READ screenshotHeightList CONSTANT)
    Q_PROPERTY(QString screenshotWidthText READ screenshotWidthText NOTIFY
                   screenshotWidthTextChanged)

public:
    explicit UiConfigModel(QObject* parent = nullptr);

    void initialize(Settings::AspectRatio ratio, Settings::ResolutionSetup resolution);
    void apply();

    int languageIndex() const { return m_languageIndex; }
    QStringList languageList() const { return m_languageList; }
    int themeIndex() const { return m_themeIndex; }
    QStringList themeList() const { return m_themeList; }

    bool showAddOns() const { return m_showAddOns; }
    bool showCompat() const { return m_showCompat; }
    bool showSize() const { return m_showSize; }
    bool showTypes() const { return m_showTypes; }
    bool showPlayTime() const { return m_showPlayTime; }

    int gameIconSizeIndex() const { return m_gameIconSizeIndex; }
    QStringList gameIconSizeList() const { return m_gameIconSizeList; }
    int folderIconSizeIndex() const { return m_folderIconSizeIndex; }
    QStringList folderIconSizeList() const { return m_folderIconSizeList; }

    int row1TextIndex() const { return m_row1TextIndex; }
    QStringList row1TextList() const { return m_row1TextList; }
    int row2TextIndex() const { return m_row2TextIndex; }
    QStringList row2TextList() const { return m_row2TextList; }

    bool enableScreenshotSaveAs() const { return m_enableScreenshotSaveAs; }
    QString screenshotPath() const { return m_screenshotPath; }
    int screenshotHeightIndex() const { return m_screenshotHeightIndex; }
    QStringList screenshotHeightList() const { return m_screenshotHeightList; }
    QString screenshotWidthText() const { return m_screenshotWidthText; }

    void setLanguageIndex(int idx);
    void setThemeIndex(int idx);
    void setShowAddOns(bool val);
    void setShowCompat(bool val);
    void setShowSize(bool val);
    void setShowTypes(bool val);
    void setShowPlayTime(bool val);
    void setGameIconSizeIndex(int idx);
    void setFolderIconSizeIndex(int idx);
    void setRow1TextIndex(int idx);
    void setRow2TextIndex(int idx);
    void setEnableScreenshotSaveAs(bool val);
    void setScreenshotPath(const QString& path);
    void setScreenshotHeightIndex(int idx);

    void updateScreenshotInfo(Settings::AspectRatio ratio, Settings::ResolutionSetup resolution);

    QString currentLanguageCode() const;

signals:
    void languageIndexChanged();
    void themeIndexChanged();
    void showAddOnsChanged();
    void showCompatChanged();
    void showSizeChanged();
    void showTypesChanged();
    void showPlayTimeChanged();
    void gameIconSizeIndexChanged();
    void folderIconSizeIndexChanged();
    void row1TextIndexChanged();
    void row1TextListChanged();
    void row2TextIndexChanged();
    void row2TextListChanged();
    void enableScreenshotSaveAsChanged();
    void screenshotPathChanged();
    void screenshotHeightIndexChanged();
    void screenshotWidthTextChanged();
    void languageChanged(const QString& locale);

private:
    void updateWidthText();
    void updateRowLists();

    int m_languageIndex = 0;
    QStringList m_languageList;
    QStringList m_languageValues;

    int m_themeIndex = 0;
    QStringList m_themeList;
    QStringList m_themeValues;

    bool m_showAddOns = false;
    bool m_showCompat = false;
    bool m_showSize = false;
    bool m_showTypes = false;
    bool m_showPlayTime = false;

    int m_gameIconSizeIndex = 0;
    QStringList m_gameIconSizeList;
    QList<int> m_gameIconSizeValues;

    int m_folderIconSizeIndex = 0;
    QStringList m_folderIconSizeList;
    QList<int> m_folderIconSizeValues;

    int m_row1TextIndex = 0;
    int m_row2TextIndex = 0;
    QStringList m_row1TextList;
    QStringList m_row2TextList;
    QStringList m_rowTextNames;

    bool m_enableScreenshotSaveAs = false;
    QString m_screenshotPath;
    int m_screenshotHeightIndex = 0;
    QStringList m_screenshotHeightList;
    QList<u32> m_screenshotHeightValues;
    QString m_screenshotWidthText;

    Settings::AspectRatio m_ratio;
    Settings::ResolutionSetup m_resolutionSetting;
};
