// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QtConcurrent/QtConcurrentRun>

#include "common/settings.h"
#include "core/telemetry_session.h"
#include "yuzu/configuration/configure_web_model.h"
#include "yuzu/uisettings.h"

static constexpr char token_delimiter{':'};

WebConfigModel::WebConfigModel(QObject* parent) : QObject(parent) {
    connect(&verify_watcher, &QFutureWatcher<bool>::finished, this,
            &WebConfigModel::onLoginVerified);
}

void WebConfigModel::initialize() {
    m_enableTelemetry = Settings::values.enable_telemetry.GetValue();
    m_telemetryId =
        QStringLiteral("0x%1").arg(QString::number(Core::GetTelemetryId(), 16).toUpper());

    const auto display_token = generateDisplayToken(Settings::values.yuzu_username.GetValue(),
                                                    Settings::values.yuzu_token.GetValue());
    m_token = QString::fromStdString(display_token);

    if (Settings::values.yuzu_username.GetValue().empty()) {
        m_username = tr("Unspecified");
    } else {
        m_username = QString::fromStdString(Settings::values.yuzu_username.GetValue());
    }

    m_enableDiscordRpc = UISettings::values.enable_discord_presence.GetValue();
    m_userVerified = true;
    m_verifyStatus = QString();

    emit enableTelemetryChanged();
    emit telemetryIdChanged();
    emit tokenChanged();
    emit usernameChanged();
    emit enableDiscordRpcChanged();
    emit verifyStatusChanged();
}

void WebConfigModel::apply() {
    Settings::values.enable_telemetry = m_enableTelemetry;
    UISettings::values.enable_discord_presence = m_enableDiscordRpc;

    if (m_userVerified) {
        Settings::values.yuzu_username =
            usernameFromDisplayToken(m_token.toStdString());
        Settings::values.yuzu_token = tokenFromDisplayToken(m_token.toStdString());
    }
}

void WebConfigModel::setEnableTelemetry(bool val) {
    if (m_enableTelemetry != val) {
        m_enableTelemetry = val;
        emit enableTelemetryChanged();
    }
}

void WebConfigModel::setToken(const QString& val) {
    if (m_token != val) {
        m_token = val;
        emit tokenChanged();

        if (val.isEmpty()) {
            m_userVerified = true;
            m_verifyStatus = QString();
        } else {
            m_userVerified = false;
            m_verifyStatus = QStringLiteral("changed");
        }
        emit verifyStatusChanged();
    }
}

void WebConfigModel::setEnableDiscordRpc(bool val) {
    if (m_enableDiscordRpc != val) {
        m_enableDiscordRpc = val;
        emit enableDiscordRpcChanged();
    }
}

void WebConfigModel::setWebServiceEnabled(bool val) {
    if (m_webServiceEnabled != val) {
        m_webServiceEnabled = val;
        emit webServiceEnabledChanged();
    }
}

void WebConfigModel::verifyLogin() {
    m_verifyStatus = QStringLiteral("verifying");
    emit verifyStatusChanged();

    verify_watcher.setFuture(QtConcurrent::run(
        [username = usernameFromDisplayToken(m_token.toStdString()),
         token = tokenFromDisplayToken(m_token.toStdString())] {
            return Core::VerifyLogin(username, token);
        }));
}

void WebConfigModel::onLoginVerified() {
    if (verify_watcher.result()) {
        m_userVerified = true;
        m_verifyStatus = QStringLiteral("verified");
        m_username =
            QString::fromStdString(usernameFromDisplayToken(m_token.toStdString()));
        emit usernameChanged();
    } else {
        m_verifyStatus = QStringLiteral("failed");
        m_username = tr("Unspecified");
        emit usernameChanged();
        emit verifyFailed(
            tr("Verification failed. Check that you have entered your token "
               "correctly, and that your internet connection is working."));
    }
    emit verifyStatusChanged();
}

void WebConfigModel::regenerateTelemetryId() {
    const u64 new_id = Core::RegenerateTelemetryId();
    m_telemetryId = QStringLiteral("0x%1").arg(QString::number(new_id, 16).toUpper());
    emit telemetryIdChanged();
}

bool WebConfigModel::discordVisible() const {
#ifdef USE_DISCORD_PRESENCE
    return true;
#else
    return false;
#endif
}

std::string WebConfigModel::generateDisplayToken(const std::string& username,
                                                 const std::string& token) {
    if (username.empty() || token.empty()) {
        return {};
    }
    const std::string unencoded{username + token_delimiter + token};
    QByteArray b{unencoded.c_str()};
    return b.toBase64().toStdString();
}

std::string WebConfigModel::usernameFromDisplayToken(const std::string& display_token) {
    const std::string unencoded{
        QByteArray::fromBase64(display_token.c_str()).toStdString()};
    return unencoded.substr(0, unencoded.find(token_delimiter));
}

std::string WebConfigModel::tokenFromDisplayToken(const std::string& display_token) {
    const std::string unencoded{
        QByteArray::fromBase64(display_token.c_str()).toStdString()};
    return unencoded.substr(unencoded.find(token_delimiter) + 1);
}
