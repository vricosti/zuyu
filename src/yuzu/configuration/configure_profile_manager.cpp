// SPDX-FileCopyrightText: 2016 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <functional>

#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "common/assert.h"
#include "common/fs/path_util.h"
#include "common/logging/log.h"
#include "common/settings.h"
#include "common/string_util.h"
#include "core/core.h"
#include "core/hle/service/acc/profile_manager.h"
#include "yuzu/configuration/configure_profile_manager.h"
#include "yuzu/qml_bridge.h"
#include "yuzu/util/limitable_input_dialog.h"

namespace {
constexpr std::array<u8, 107> backup_jpeg{
    0xff, 0xd8, 0xff, 0xdb, 0x00, 0x43, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03,
    0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x06, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x08, 0x06, 0x06, 0x05, 0x06, 0x09, 0x08, 0x0a, 0x0a, 0x09, 0x08, 0x09, 0x09, 0x0a,
    0x0c, 0x0f, 0x0c, 0x0a, 0x0b, 0x0e, 0x0b, 0x09, 0x09, 0x0d, 0x11, 0x0d, 0x0e, 0x0f,
    0x10, 0x10, 0x11, 0x10, 0x0a, 0x0c, 0x12, 0x13, 0x12, 0x10, 0x13, 0x0f, 0x10, 0x10,
    0x10, 0xff, 0xc9, 0x00, 0x0b, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x11, 0x00,
    0xff, 0xcc, 0x00, 0x06, 0x00, 0x10, 0x10, 0x05, 0xff, 0xda, 0x00, 0x08, 0x01, 0x01,
    0x00, 0x00, 0x3f, 0x00, 0xd2, 0xcf, 0x20, 0xff, 0xd9,
};

QString GetImagePath(const Common::UUID& uuid) {
    const auto path =
        Common::FS::GetYuzuPath(Common::FS::YuzuPath::NANDDir) /
        fmt::format("system/save/8000000000000010/su/avators/{}.jpg", uuid.FormattedString());
    return QString::fromStdString(Common::FS::PathToUTF8String(path));
}

QString GetAccountUsername(const Service::Account::ProfileManager& manager, Common::UUID uuid) {
    Service::Account::ProfileBase profile{};
    if (!manager.GetProfileBase(uuid, profile)) {
        return {};
    }
    const auto text = Common::StringFromFixedZeroTerminatedBuffer(
        reinterpret_cast<const char*>(profile.username.data()), profile.username.size());
    return QString::fromStdString(text);
}

QString FormatUserEntryText(const QString& username, Common::UUID uuid) {
    return ConfigureProfileManager::tr(
               "%1\n%2",
               "%1 is the profile username, %2 is the formatted UUID")
        .arg(username, QString::fromStdString(uuid.FormattedString()));
}

QPixmap GetIcon(const Common::UUID& uuid) {
    QPixmap icon{GetImagePath(uuid)};
    if (!icon) {
        icon.fill(Qt::black);
        icon.loadFromData(backup_jpeg.data(), static_cast<u32>(backup_jpeg.size()));
    }
    return icon.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QString GetProfileUsernameFromUser(QWidget* parent, const QString& description_text) {
    return LimitableInputDialog::GetText(parent, ConfigureProfileManager::tr("Enter Username"),
                                         description_text, 1,
                                         static_cast<int>(Service::Account::profile_username_size));
}
} // Anonymous namespace

ConfigureProfileManager::ConfigureProfileManager(Core::System& system_, QWidget* parent)
    : QWidget(parent), profile_manager{system_.GetProfileManager()}, system{system_} {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    user_model = new QStandardItemModel(this);
    user_model->setColumnCount(1);

    enabled = !system.IsPoweredOn();

    quick_widget = new QQuickWidget(this);
    quick_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* ctx = quick_widget->rootContext();
    QmlBridge::SetupContext(ctx);
    ctx->setContextProperty(QStringLiteral("profileModel"), user_model);
    ctx->setContextProperty(QStringLiteral("maxUsers"),
                            static_cast<int>(Service::Account::MAX_USERS));

    quick_widget->setSource(
        QUrl(QStringLiteral("qrc:/qml/qml/ConfigureProfileManager.qml")));

    if (quick_widget->status() == QQuickWidget::Error) {
        for (const auto& error : quick_widget->errors()) {
            LOG_ERROR(Frontend, "ConfigureProfileManager QML Error: {}",
                      error.toString().toStdString());
        }
    }

    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        connect(root, SIGNAL(selectUser(int)), this, SLOT(onSelectUser(int)));
        connect(root, SIGNAL(addUser()), this, SLOT(onAddUser()));
        connect(root, SIGNAL(renameUser()), this, SLOT(onRenameUser()));
        connect(root, SIGNAL(deleteUser()), this, SLOT(onDeleteUser()));
        connect(root, SIGNAL(setUserImage()), this, SLOT(onSetUserImage()));
    }

    PopulateUserList();
    UpdateCurrentUser();

    layout->addWidget(quick_widget);
    setLayout(layout);
}

ConfigureProfileManager::~ConfigureProfileManager() = default;

void ConfigureProfileManager::PopulateUserList() {
    user_model->removeRows(0, user_model->rowCount());

    const auto& profiles = profile_manager.GetAllUsers();
    for (const auto& user : profiles) {
        Service::Account::ProfileBase profile{};
        if (!profile_manager.GetProfileBase(user, profile))
            continue;

        const auto username = Common::StringFromFixedZeroTerminatedBuffer(
            reinterpret_cast<const char*>(profile.username.data()), profile.username.size());

        auto* item = new QStandardItem{
            GetIcon(user), FormatUserEntryText(QString::fromStdString(username), user)};
        user_model->appendRow(item);
    }
}

void ConfigureProfileManager::UpdateCurrentUser() {
    const auto& current_user =
        profile_manager.GetUser(Settings::values.current_user.GetValue());
    ASSERT(current_user);
    const auto username = GetAccountUsername(profile_manager, *current_user);

    QQuickItem* root = quick_widget->rootObject();
    if (root) {
        QMetaObject::invokeMethod(root, "updateCurrentUser",
                                  Q_ARG(QVariant, username),
                                  Q_ARG(QVariant, static_cast<int>(profile_manager.GetUserCount())),
                                  Q_ARG(QVariant, enabled));
    }
}

void ConfigureProfileManager::RefreshModel() {
    PopulateUserList();
    UpdateCurrentUser();
}

void ConfigureProfileManager::ApplyConfiguration() {
    // Profile changes are applied immediately
}

void ConfigureProfileManager::onSelectUser(int index) {
    Settings::values.current_user =
        std::clamp<s32>(index, 0, static_cast<s32>(profile_manager.GetUserCount() - 1));
    UpdateCurrentUser();
}

void ConfigureProfileManager::onAddUser() {
    const auto username =
        GetProfileUsernameFromUser(this, tr("Enter a username for the new user:"));
    if (username.isEmpty()) {
        return;
    }

    const auto uuid = Common::UUID::MakeRandom();
    profile_manager.CreateNewUser(uuid, username.toStdString());
    profile_manager.WriteUserSaveFile();

    user_model->appendRow(
        new QStandardItem{GetIcon(uuid), FormatUserEntryText(username, uuid)});
    UpdateCurrentUser();
}

void ConfigureProfileManager::onRenameUser() {
    QQuickItem* root = quick_widget->rootObject();
    if (!root) return;

    QVariant ret;
    QMetaObject::invokeMethod(root, "getSelectedIndex", Q_RETURN_ARG(QVariant, ret));
    int selectedIndex = ret.toInt();

    if (selectedIndex < 0) return;

    const auto uuid = profile_manager.GetUser(selectedIndex);
    ASSERT(uuid);

    Service::Account::ProfileBase profile{};
    if (!profile_manager.GetProfileBase(*uuid, profile))
        return;

    const auto new_username = GetProfileUsernameFromUser(this, tr("Enter a new username:"));
    if (new_username.isEmpty()) return;

    const auto username_std = new_username.toStdString();
    std::fill(profile.username.begin(), profile.username.end(), '\0');
    std::copy(username_std.begin(), username_std.end(), profile.username.begin());

    profile_manager.SetProfileBase(*uuid, profile);
    profile_manager.WriteUserSaveFile();

    user_model->setItem(
        selectedIndex, 0,
        new QStandardItem{GetIcon(*uuid),
                          FormatUserEntryText(QString::fromStdString(username_std), *uuid)});
    UpdateCurrentUser();
}

void ConfigureProfileManager::onDeleteUser() {
    QQuickItem* root = quick_widget->rootObject();
    if (!root) return;

    QVariant ret;
    QMetaObject::invokeMethod(root, "getSelectedIndex", Q_RETURN_ARG(QVariant, ret));
    int selectedIndex = ret.toInt();
    if (selectedIndex < 0) return;

    const auto uuid = profile_manager.GetUser(selectedIndex);
    ASSERT(uuid);
    const auto username = GetAccountUsername(profile_manager, *uuid);

    auto result = QMessageBox::question(
        this, tr("Confirm Delete"),
        tr("Delete this user? All of the user's save data will be deleted.\n\n"
           "Name: %1\nUUID: %2")
            .arg(username, QString::fromStdString(uuid->FormattedString())),
        QMessageBox::Yes | QMessageBox::No);

    if (result != QMessageBox::Yes) return;

    if (Settings::values.current_user.GetValue() == selectedIndex) {
        Settings::values.current_user = 0;
    }

    if (!profile_manager.RemoveUser(*uuid)) return;

    profile_manager.WriteUserSaveFile();
    user_model->removeRows(selectedIndex, 1);
    UpdateCurrentUser();
}

void ConfigureProfileManager::onSetUserImage() {
    QQuickItem* root = quick_widget->rootObject();
    if (!root) return;

    QVariant ret;
    QMetaObject::invokeMethod(root, "getSelectedIndex", Q_RETURN_ARG(QVariant, ret));
    int selectedIndex = ret.toInt();
    if (selectedIndex < 0) return;

    const auto uuid = profile_manager.GetUser(selectedIndex);
    ASSERT(uuid);

    const auto file = QFileDialog::getOpenFileName(this, tr("Select User Image"), QString(),
                                                   tr("JPEG Images (*.jpg *.jpeg)"));
    if (file.isEmpty()) return;

    const auto image_path = GetImagePath(*uuid);
    if (QFile::exists(image_path) && !QFile::remove(image_path)) {
        QMessageBox::warning(
            this, tr("Error deleting image"),
            tr("Error occurred attempting to overwrite previous image at: %1.").arg(image_path));
        return;
    }

    const auto raw_path = QString::fromStdString(Common::FS::PathToUTF8String(
        Common::FS::GetYuzuPath(Common::FS::YuzuPath::NANDDir) /
        "system/save/8000000000000010"));
    const QFileInfo raw_info{raw_path};
    if (raw_info.exists() && !raw_info.isDir() && !QFile::remove(raw_path)) {
        QMessageBox::warning(this, tr("Error deleting file"),
                             tr("Unable to delete existing file: %1.").arg(raw_path));
        return;
    }

    const QString absolute_dst_path = QFileInfo{image_path}.absolutePath();
    if (!QDir{raw_path}.mkpath(absolute_dst_path)) {
        QMessageBox::warning(
            this, tr("Error creating user image directory"),
            tr("Unable to create directory %1 for storing user images.").arg(absolute_dst_path));
        return;
    }

    if (!QFile::copy(file, image_path)) {
        QMessageBox::warning(this, tr("Error copying user image"),
                             tr("Unable to copy image from %1 to %2").arg(file, image_path));
        return;
    }

    QImage image(image_path);
    if (image.width() != 256 || image.height() != 256) {
        image = image.scaled(256, 256, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        if (!image.save(image_path)) {
            QMessageBox::warning(this, tr("Error resizing user image"),
                                 tr("Unable to resize image"));
            return;
        }
    }

    const auto username = GetAccountUsername(profile_manager, *uuid);
    user_model->setItem(selectedIndex, 0,
                        new QStandardItem{GetIcon(*uuid),
                                          FormatUserEntryText(username, *uuid)});
    UpdateCurrentUser();
}
