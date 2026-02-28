// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QString>

class WebConfigModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool enableTelemetry READ enableTelemetry WRITE setEnableTelemetry NOTIFY
                   enableTelemetryChanged)
    Q_PROPERTY(QString telemetryId READ telemetryId NOTIFY telemetryIdChanged)
    Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)
    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
    Q_PROPERTY(bool enableDiscordRpc READ enableDiscordRpc WRITE setEnableDiscordRpc NOTIFY
                   enableDiscordRpcChanged)
    Q_PROPERTY(bool discordVisible READ discordVisible CONSTANT)
    Q_PROPERTY(bool webServiceEnabled READ webServiceEnabled WRITE setWebServiceEnabled NOTIFY
                   webServiceEnabledChanged)
    Q_PROPERTY(QString verifyStatus READ verifyStatus NOTIFY verifyStatusChanged)

public:
    explicit WebConfigModel(QObject* parent = nullptr);

    void initialize();
    void apply();

    bool enableTelemetry() const { return m_enableTelemetry; }
    QString telemetryId() const { return m_telemetryId; }
    QString token() const { return m_token; }
    QString username() const { return m_username; }
    bool enableDiscordRpc() const { return m_enableDiscordRpc; }
    bool discordVisible() const;
    bool webServiceEnabled() const { return m_webServiceEnabled; }
    QString verifyStatus() const { return m_verifyStatus; }

    void setEnableTelemetry(bool val);
    void setToken(const QString& val);
    void setEnableDiscordRpc(bool val);
    void setWebServiceEnabled(bool val);

    Q_INVOKABLE void verifyLogin();
    Q_INVOKABLE void regenerateTelemetryId();

signals:
    void enableTelemetryChanged();
    void telemetryIdChanged();
    void tokenChanged();
    void usernameChanged();
    void enableDiscordRpcChanged();
    void webServiceEnabledChanged();
    void verifyStatusChanged();
    void verifyFailed(const QString& message);

private:
    void onLoginVerified();

    static std::string generateDisplayToken(const std::string& username,
                                            const std::string& token);
    static std::string usernameFromDisplayToken(const std::string& display_token);
    static std::string tokenFromDisplayToken(const std::string& display_token);

    bool m_enableTelemetry = false;
    QString m_telemetryId;
    QString m_token;
    QString m_username;
    bool m_enableDiscordRpc = false;
    bool m_webServiceEnabled = true;
    QString m_verifyStatus;
    bool m_userVerified = true;

    QFutureWatcher<bool> verify_watcher;
};
