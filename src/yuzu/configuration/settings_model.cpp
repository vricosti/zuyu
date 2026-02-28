// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>

#include "common/settings.h"
#include "common/settings_common.h"
#include "yuzu/configuration/settings_model.h"
#include "yuzu/configuration/shared_translation.h"

SettingsModel::SettingsModel(QObject* parent) : QAbstractListModel(parent) {
    auto translations = ConfigurationShared::InitializeTranslations(nullptr);
    if (translations) {
        m_translations = *translations;
    }
    auto combobox = ConfigurationShared::ComboboxEnumeration(nullptr);
    if (combobox) {
        m_comboboxTranslations = *combobox;
    }
}

void SettingsModel::populate(Settings::Category category) {
    beginResetModel();
    m_entries.clear();

    auto& linkage = Settings::values.linkage;
    auto it = linkage.by_category.find(category);
    if (it != linkage.by_category.end()) {
        for (auto* setting : it->second) {
            if (!setting->Save()) {
                continue;
            }

            Entry entry;
            entry.setting = setting;

            auto trans_it = m_translations.find(setting->Id());
            if (trans_it != m_translations.end()) {
                entry.name = trans_it->second.first;
                entry.tooltip = trans_it->second.second;
            } else {
                entry.name = QString::fromStdString(setting->GetLabel());
            }

            m_entries.append(entry);
        }
    }

    endResetModel();
}

void SettingsModel::populate(const std::vector<Settings::BasicSetting*>& settings) {
    beginResetModel();
    m_entries.clear();

    for (auto* setting : settings) {
        Entry entry;
        entry.setting = setting;

        auto trans_it = m_translations.find(setting->Id());
        if (trans_it != m_translations.end()) {
            entry.name = trans_it->second.first;
            entry.tooltip = trans_it->second.second;
        } else {
            entry.name = QString::fromStdString(setting->GetLabel());
        }

        m_entries.append(entry);
    }

    endResetModel();
}

void SettingsModel::setConfigureGlobal(bool is_global) {
    m_configureGlobal = is_global;
}

void SettingsModel::setRuntimeLock(bool locked) {
    m_runtimeLock = locked;
}

int SettingsModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.size();
}

QVariant SettingsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return {};
    }

    const auto& entry = m_entries[index.row()];
    auto* setting = entry.setting;

    switch (role) {
    case NameRole:
        return entry.name;
    case TooltipRole:
        return entry.tooltip;
    case SettingTypeRole:
        return settingTypeString(setting);
    case ValueRole:
        return QString::fromStdString(setting->ToString());
    case MinValueRole:
        return QString::fromStdString(setting->MinVal());
    case MaxValueRole:
        return QString::fromStdString(setting->MaxVal());
    case OptionsRole:
        return enumOptions(setting);
    case OptionValuesRole:
        return enumValues(setting);
    case IsGlobalRole:
        return setting->UsingGlobal();
    case IsSwitchableRole:
        return setting->Switchable();
    case IsRuntimeLockedRole:
        return m_runtimeLock && !setting->RuntimeModifiable();
    case IsPercentageRole: {
        const u32 spec = setting->Specialization();
        return (spec & Settings::SpecializationAttributeMask) != 0;
    }
    case HasPairedRole:
        return setting->PairedSetting() != nullptr;
    case PairedValueRole:
        if (setting->PairedSetting()) {
            return QString::fromStdString(setting->PairedSetting()->ToString());
        }
        return {};
    case SettingIdRole:
        return setting->Id();
    default:
        return {};
    }
}

bool SettingsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return false;
    }

    if (role == ValueRole) {
        auto* setting = m_entries[index.row()].setting;
        setting->LoadString(value.toString().toStdString());
        emit dataChanged(index, index, {ValueRole});
        return true;
    }
    return false;
}

QHash<int, QByteArray> SettingsModel::roleNames() const {
    return {
        {NameRole, "settingName"},
        {TooltipRole, "settingTooltip"},
        {SettingTypeRole, "settingType"},
        {ValueRole, "settingValue"},
        {MinValueRole, "settingMin"},
        {MaxValueRole, "settingMax"},
        {OptionsRole, "settingOptions"},
        {OptionValuesRole, "settingOptionValues"},
        {IsGlobalRole, "settingIsGlobal"},
        {IsSwitchableRole, "settingIsSwitchable"},
        {IsRuntimeLockedRole, "settingIsLocked"},
        {IsPercentageRole, "settingIsPercentage"},
        {HasPairedRole, "settingHasPaired"},
        {PairedValueRole, "settingPairedValue"},
        {SettingIdRole, "settingId"},
    };
}

void SettingsModel::setValue(int row, const QString& value) {
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    auto* setting = m_entries[row].setting;
    setting->LoadString(value.toStdString());
    const auto idx = index(row);
    emit dataChanged(idx, idx, {ValueRole});
}

void SettingsModel::setGlobal(int row, bool is_global) {
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    auto* setting = m_entries[row].setting;
    if (setting->Switchable()) {
        setting->SetGlobal(is_global);
        const auto idx = index(row);
        emit dataChanged(idx, idx, {IsGlobalRole, ValueRole});
    }
}

void SettingsModel::setPairedValue(int row, bool enabled) {
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    auto* setting = m_entries[row].setting;
    auto* paired = setting->PairedSetting();
    if (paired) {
        paired->LoadString(enabled ? "true" : "false");
        const auto idx = index(row);
        emit dataChanged(idx, idx, {PairedValueRole});
    }
}

void SettingsModel::resetToDefault(int row) {
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    auto* setting = m_entries[row].setting;
    setting->LoadString(setting->DefaultToString());
    const auto idx = index(row);
    emit dataChanged(idx, idx, {ValueRole});
}

void SettingsModel::applyAll() {
    for (auto& func : m_applyFuncs) {
        func(m_configureGlobal);
    }
}

void SettingsModel::addApplyFunc(std::function<void(bool)> func) {
    m_applyFuncs.push_back(std::move(func));
}

QString SettingsModel::settingTypeString(Settings::BasicSetting* setting) const {
    const u32 spec = setting->Specialization() & Settings::SpecializationTypeMask;

    // Check for boolean type first
    if (setting->TypeId() == typeid(bool)) {
        return QStringLiteral("checkbox");
    }

    switch (spec) {
    case Settings::Specialization::Time:
        return QStringLiteral("datetime");
    case Settings::Specialization::Hex:
        return QStringLiteral("hexedit");
    case Settings::Specialization::List:
    case Settings::Specialization::RuntimeList:
        return QStringLiteral("combobox");
    case Settings::Specialization::Scalar:
        return QStringLiteral("slider");
    case Settings::Specialization::Countable:
        return QStringLiteral("spinbox");
    case Settings::Specialization::Radio:
        return QStringLiteral("radio");
    case Settings::Specialization::Paired:
        // Paired settings act as checkboxes (enable/disable)
        return QStringLiteral("checkbox");
    default:
        // Auto-detect from type
        if (setting->IsEnum()) {
            return QStringLiteral("combobox");
        }
        if (setting->Ranged()) {
            if (setting->IsFloatingPoint()) {
                return QStringLiteral("slider");
            }
            return QStringLiteral("spinbox");
        }
        if (setting->IsIntegral()) {
            return QStringLiteral("spinbox");
        }
        return QStringLiteral("lineedit");
    }
}

QStringList SettingsModel::enumOptions(Settings::BasicSetting* setting) const {
    QStringList result;
    if (!setting->IsEnum()) {
        return result;
    }

    const u32 enum_index = setting->EnumIndex();
    auto it = m_comboboxTranslations.find(enum_index);
    if (it != m_comboboxTranslations.end()) {
        for (const auto& [val, text] : it->second) {
            result.append(text);
        }
    }
    return result;
}

QVariantList SettingsModel::enumValues(Settings::BasicSetting* setting) const {
    QVariantList result;
    if (!setting->IsEnum()) {
        return result;
    }

    const u32 enum_index = setting->EnumIndex();
    auto it = m_comboboxTranslations.find(enum_index);
    if (it != m_comboboxTranslations.end()) {
        for (const auto& [val, text] : it->second) {
            result.append(val);
        }
    }
    return result;
}
