// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <functional>
#include <map>
#include <utility>
#include <vector>

#include <QAbstractListModel>
#include <QString>
#include <QVariant>

#include "common/common_types.h"

namespace Settings {
class BasicSetting;
enum class Category : u32;
} // namespace Settings

namespace ConfigurationShared {
using TranslationMap = std::map<u32, std::pair<QString, QString>>;
using ComboboxTranslations = std::vector<std::pair<u32, QString>>;
using ComboboxTranslationMap = std::map<u32, ComboboxTranslations>;
} // namespace ConfigurationShared

/**
 * A generic QAbstractListModel that wraps a list of Settings::BasicSetting* pointers,
 * exposing their values and metadata to QML via roles.
 *
 * The model supports:
 * - Auto-detecting the control type from the setting's specialization
 * - Reading/writing values through ToString()/LoadString()
 * - Per-game configuration (global toggle)
 * - Ranged settings with min/max
 * - Enum settings with combobox options
 * - Paired settings (checkbox controlling another setting)
 *
 * Usage:
 *   auto* model = new SettingsModel(this);
 *   model->populate(Settings::Category::Audio);
 *   // or
 *   model->populate({&Settings::values.sink_id, &Settings::values.volume, ...});
 */
class SettingsModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        TooltipRole,
        SettingTypeRole,   // "checkbox", "combobox", "slider", "spinbox", "lineedit",
                           // "hexedit", "datetime", "radio"
        ValueRole,         // Current value as string
        MinValueRole,      // Min value as string (for ranged)
        MaxValueRole,      // Max value as string (for ranged)
        OptionsRole,       // QStringList of enum option display names
        OptionValuesRole,  // QVariantList of u32 enum values
        IsGlobalRole,      // Whether using global value (SwitchableSetting)
        IsSwitchableRole,  // Whether the setting supports global/custom
        IsRuntimeLockedRole, // Disabled when game is running
        IsPercentageRole,  // Display as percentage
        HasPairedRole,     // Has a paired enable/disable setting
        PairedValueRole,   // Value of the paired setting (bool as string)
        SettingIdRole,     // Unique setting ID for identification
    };

    explicit SettingsModel(QObject* parent = nullptr);

    /**
     * Populate the model with settings from a specific category.
     * Only settings with Save()==true and translations are included.
     */
    void populate(Settings::Category category);

    /**
     * Populate the model with a specific list of settings.
     */
    void populate(const std::vector<Settings::BasicSetting*>& settings);

    /**
     * Set whether we're in per-game configuration mode.
     */
    void setConfigureGlobal(bool is_global);

    /**
     * Set whether a game is currently running (for runtime lock).
     */
    void setRuntimeLock(bool locked);

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    // QML-callable methods
    Q_INVOKABLE void setValue(int row, const QString& value);
    Q_INVOKABLE void setGlobal(int row, bool is_global);
    Q_INVOKABLE void setPairedValue(int row, bool enabled);
    Q_INVOKABLE void resetToDefault(int row);

    /**
     * Apply all pending changes. Call this from the tab's ApplyConfiguration().
     */
    void applyAll();

    /**
     * Register an apply function to be called during applyAll().
     */
    void addApplyFunc(std::function<void(bool)> func);

private:
    struct Entry {
        Settings::BasicSetting* setting = nullptr;
        QString name;
        QString tooltip;
    };

    QString settingTypeString(Settings::BasicSetting* setting) const;
    QStringList enumOptions(Settings::BasicSetting* setting) const;
    QVariantList enumValues(Settings::BasicSetting* setting) const;

    QList<Entry> m_entries;
    ConfigurationShared::TranslationMap m_translations;
    ConfigurationShared::ComboboxTranslationMap m_comboboxTranslations;
    std::vector<std::function<void(bool)>> m_applyFuncs;
    bool m_configureGlobal = true;
    bool m_runtimeLock = false;
};
