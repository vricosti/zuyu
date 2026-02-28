// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QString>

class CompatDBModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(bool nextEnabled READ nextEnabled WRITE setNextEnabled NOTIFY nextEnabledChanged)
    Q_PROPERTY(bool isSubmitting READ isSubmitting WRITE setIsSubmitting NOTIFY isSubmittingChanged)
    Q_PROPERTY(bool isFinished READ isFinished WRITE setIsFinished NOTIFY isFinishedChanged)
    Q_PROPERTY(QString nextButtonText READ nextButtonText NOTIFY nextButtonTextChanged)
    Q_PROPERTY(bool cancelVisible READ cancelVisible NOTIFY cancelVisibleChanged)
    Q_PROPERTY(bool backVisible READ backVisible NOTIFY backVisibleChanged)

    // Radio button selections (-1 = none selected)
    Q_PROPERTY(int gameBootSelection READ gameBootSelection WRITE setGameBootSelection NOTIFY
                   gameBootSelectionChanged)
    Q_PROPERTY(int gameplaySelection READ gameplaySelection WRITE setGameplaySelection NOTIFY
                   gameplaySelectionChanged)
    Q_PROPERTY(int freezeSelection READ freezeSelection WRITE setFreezeSelection NOTIFY
                   freezeSelectionChanged)
    Q_PROPERTY(int completionSelection READ completionSelection WRITE setCompletionSelection NOTIFY
                   completionSelectionChanged)
    Q_PROPERTY(int graphicalSelection READ graphicalSelection WRITE setGraphicalSelection NOTIFY
                   graphicalSelectionChanged)
    Q_PROPERTY(int audioSelection READ audioSelection WRITE setAudioSelection NOTIFY
                   audioSelectionChanged)

public:
    explicit CompatDBModel(QObject* parent = nullptr);

    int currentPage() const { return m_currentPage; }
    bool nextEnabled() const { return m_nextEnabled; }
    bool isSubmitting() const { return m_isSubmitting; }
    bool isFinished() const { return m_isFinished; }
    QString nextButtonText() const;
    bool cancelVisible() const;
    bool backVisible() const;

    int gameBootSelection() const { return m_gameBootSelection; }
    int gameplaySelection() const { return m_gameplaySelection; }
    int freezeSelection() const { return m_freezeSelection; }
    int completionSelection() const { return m_completionSelection; }
    int graphicalSelection() const { return m_graphicalSelection; }
    int audioSelection() const { return m_audioSelection; }

    void setCurrentPage(int page);
    void setNextEnabled(bool enabled);
    void setIsSubmitting(bool submitting);
    void setIsFinished(bool finished);
    void setGameBootSelection(int sel);
    void setGameplaySelection(int sel);
    void setFreezeSelection(int sel);
    void setCompletionSelection(int sel);
    void setGraphicalSelection(int sel);
    void setAudioSelection(int sel);

    Q_INVOKABLE void nextPage();
    Q_INVOKABLE void previousPage();

signals:
    void currentPageChanged();
    void nextEnabledChanged();
    void isSubmittingChanged();
    void isFinishedChanged();
    void nextButtonTextChanged();
    void cancelVisibleChanged();
    void backVisibleChanged();
    void gameBootSelectionChanged();
    void gameplaySelectionChanged();
    void freezeSelectionChanged();
    void completionSelectionChanged();
    void graphicalSelectionChanged();
    void audioSelectionChanged();

    // Emitted when the user reaches the Final page and submission should happen
    void submitRequested();
    // Emitted when cancel is clicked
    void cancelRequested();

private:
    int computeNextPage() const;
    void updateNextEnabled();

    int m_currentPage = 0;
    bool m_nextEnabled = true; // Intro page always has next enabled
    bool m_isSubmitting = false;
    bool m_isFinished = false;

    int m_gameBootSelection = -1;
    int m_gameplaySelection = -1;
    int m_freezeSelection = -1;
    int m_completionSelection = -1;
    int m_graphicalSelection = -1;
    int m_audioSelection = -1;

    // Track page history for back navigation
    QList<int> m_pageHistory;
};
