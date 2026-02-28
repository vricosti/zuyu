// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QString>

class FilesystemConfigModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString nandDir READ nandDir NOTIFY nandDirChanged)
    Q_PROPERTY(QString sdmcDir READ sdmcDir NOTIFY sdmcDirChanged)
    Q_PROPERTY(QString gamecardPath READ gamecardPath NOTIFY gamecardPathChanged)
    Q_PROPERTY(QString dumpDir READ dumpDir NOTIFY dumpDirChanged)
    Q_PROPERTY(QString loadDir READ loadDir NOTIFY loadDirChanged)
    Q_PROPERTY(bool gamecardInserted READ gamecardInserted WRITE setGamecardInserted NOTIFY
                   gamecardInsertedChanged)
    Q_PROPERTY(bool gamecardCurrentGame READ gamecardCurrentGame WRITE setGamecardCurrentGame NOTIFY
                   gamecardCurrentGameChanged)
    Q_PROPERTY(bool dumpExefs READ dumpExefs WRITE setDumpExefs NOTIFY dumpExefsChanged)
    Q_PROPERTY(bool dumpNso READ dumpNso WRITE setDumpNso NOTIFY dumpNsoChanged)
    Q_PROPERTY(bool cacheGameList READ cacheGameList WRITE setCacheGameList NOTIFY
                   cacheGameListChanged)

public:
    explicit FilesystemConfigModel(QObject* parent = nullptr);

    void initialize();

    QString nandDir() const { return m_nandDir; }
    QString sdmcDir() const { return m_sdmcDir; }
    QString gamecardPath() const { return m_gamecardPath; }
    QString dumpDir() const { return m_dumpDir; }
    QString loadDir() const { return m_loadDir; }
    bool gamecardInserted() const { return m_gamecardInserted; }
    bool gamecardCurrentGame() const { return m_gamecardCurrentGame; }
    bool dumpExefs() const { return m_dumpExefs; }
    bool dumpNso() const { return m_dumpNso; }
    bool cacheGameList() const { return m_cacheGameList; }

    void setNandDir(const QString& dir);
    void setSdmcDir(const QString& dir);
    void setGamecardPath(const QString& path);
    void setDumpDir(const QString& dir);
    void setLoadDir(const QString& dir);
    void setGamecardInserted(bool val);
    void setGamecardCurrentGame(bool val);
    void setDumpExefs(bool val);
    void setDumpNso(bool val);
    void setCacheGameList(bool val);

    void apply();

signals:
    void nandDirChanged();
    void sdmcDirChanged();
    void gamecardPathChanged();
    void dumpDirChanged();
    void loadDirChanged();
    void gamecardInsertedChanged();
    void gamecardCurrentGameChanged();
    void dumpExefsChanged();
    void dumpNsoChanged();
    void cacheGameListChanged();

private:
    QString m_nandDir;
    QString m_sdmcDir;
    QString m_gamecardPath;
    QString m_dumpDir;
    QString m_loadDir;
    bool m_gamecardInserted = false;
    bool m_gamecardCurrentGame = false;
    bool m_dumpExefs = false;
    bool m_dumpNso = false;
    bool m_cacheGameList = false;
};
