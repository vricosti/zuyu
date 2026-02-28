// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "yuzu/compatdb_model.h"

// Page indices match original QWizard page IDs
enum class CompatDBPage {
    Intro = 0,
    GameBoot = 1,
    GamePlay = 2,
    Freeze = 3,
    Completion = 4,
    Graphical = 5,
    Audio = 6,
    Final = 7,
};

CompatDBModel::CompatDBModel(QObject* parent) : QObject(parent) {}

QString CompatDBModel::nextButtonText() const {
    if (m_isSubmitting) {
        return tr("Submitting");
    }
    if (m_currentPage == static_cast<int>(CompatDBPage::Final)) {
        return tr("Finish");
    }
    return tr("Next");
}

bool CompatDBModel::cancelVisible() const {
    return !m_isSubmitting && !m_isFinished;
}

bool CompatDBModel::backVisible() const {
    return m_currentPage > 0 && !m_isSubmitting && !m_isFinished;
}

void CompatDBModel::setCurrentPage(int page) {
    if (m_currentPage != page) {
        m_currentPage = page;
        emit currentPageChanged();
        emit nextButtonTextChanged();
        emit backVisibleChanged();
        updateNextEnabled();
    }
}

void CompatDBModel::setNextEnabled(bool enabled) {
    if (m_nextEnabled != enabled) {
        m_nextEnabled = enabled;
        emit nextEnabledChanged();
    }
}

void CompatDBModel::setIsSubmitting(bool submitting) {
    if (m_isSubmitting != submitting) {
        m_isSubmitting = submitting;
        emit isSubmittingChanged();
        emit nextButtonTextChanged();
        emit cancelVisibleChanged();
        emit backVisibleChanged();
        setNextEnabled(!submitting);
    }
}

void CompatDBModel::setIsFinished(bool finished) {
    if (m_isFinished != finished) {
        m_isFinished = finished;
        emit isFinishedChanged();
        emit cancelVisibleChanged();
        emit backVisibleChanged();
    }
}

void CompatDBModel::setGameBootSelection(int sel) {
    if (m_gameBootSelection != sel) {
        m_gameBootSelection = sel;
        emit gameBootSelectionChanged();
        updateNextEnabled();
    }
}

void CompatDBModel::setGameplaySelection(int sel) {
    if (m_gameplaySelection != sel) {
        m_gameplaySelection = sel;
        emit gameplaySelectionChanged();
        updateNextEnabled();
    }
}

void CompatDBModel::setFreezeSelection(int sel) {
    if (m_freezeSelection != sel) {
        m_freezeSelection = sel;
        emit freezeSelectionChanged();
        updateNextEnabled();
    }
}

void CompatDBModel::setCompletionSelection(int sel) {
    if (m_completionSelection != sel) {
        m_completionSelection = sel;
        emit completionSelectionChanged();
        updateNextEnabled();
    }
}

void CompatDBModel::setGraphicalSelection(int sel) {
    if (m_graphicalSelection != sel) {
        m_graphicalSelection = sel;
        emit graphicalSelectionChanged();
        updateNextEnabled();
    }
}

void CompatDBModel::setAudioSelection(int sel) {
    if (m_audioSelection != sel) {
        m_audioSelection = sel;
        emit audioSelectionChanged();
        updateNextEnabled();
    }
}

int CompatDBModel::computeNextPage() const {
    switch (static_cast<CompatDBPage>(m_currentPage)) {
    case CompatDBPage::Intro:
        return static_cast<int>(CompatDBPage::GameBoot);
    case CompatDBPage::GameBoot:
        if (m_gameBootSelection == 1) { // No
            return static_cast<int>(CompatDBPage::Final);
        }
        return static_cast<int>(CompatDBPage::GamePlay);
    case CompatDBPage::GamePlay:
        if (m_gameplaySelection == 1) { // No
            return static_cast<int>(CompatDBPage::Final);
        }
        return static_cast<int>(CompatDBPage::Freeze);
    case CompatDBPage::Freeze:
        if (m_freezeSelection == 1) { // No
            return static_cast<int>(CompatDBPage::Final);
        }
        return static_cast<int>(CompatDBPage::Completion);
    case CompatDBPage::Completion:
        if (m_completionSelection == 1) { // No
            return static_cast<int>(CompatDBPage::Final);
        }
        return static_cast<int>(CompatDBPage::Graphical);
    case CompatDBPage::Graphical:
        return static_cast<int>(CompatDBPage::Audio);
    case CompatDBPage::Audio:
        return static_cast<int>(CompatDBPage::Final);
    case CompatDBPage::Final:
        return -1;
    default:
        return -1;
    }
}

void CompatDBModel::nextPage() {
    if (m_currentPage == static_cast<int>(CompatDBPage::Final)) {
        // Already on final page, trigger submit or close
        if (m_isFinished) {
            emit cancelRequested(); // Close dialog (accept)
            return;
        }
        emit submitRequested();
        return;
    }

    int next = computeNextPage();
    if (next >= 0) {
        m_pageHistory.append(m_currentPage);
        if (next == static_cast<int>(CompatDBPage::Final)) {
            // Going to final page means submit
            setCurrentPage(next);
            emit submitRequested();
        } else {
            setCurrentPage(next);
        }
    }
}

void CompatDBModel::previousPage() {
    if (!m_pageHistory.isEmpty()) {
        int prev = m_pageHistory.takeLast();
        setCurrentPage(prev);
    }
}

void CompatDBModel::updateNextEnabled() {
    switch (static_cast<CompatDBPage>(m_currentPage)) {
    case CompatDBPage::Intro:
        setNextEnabled(true);
        break;
    case CompatDBPage::GameBoot:
        setNextEnabled(m_gameBootSelection >= 0);
        break;
    case CompatDBPage::GamePlay:
        setNextEnabled(m_gameplaySelection >= 0);
        break;
    case CompatDBPage::Freeze:
        setNextEnabled(m_freezeSelection >= 0);
        break;
    case CompatDBPage::Completion:
        setNextEnabled(m_completionSelection >= 0);
        break;
    case CompatDBPage::Graphical:
        setNextEnabled(m_graphicalSelection >= 0);
        break;
    case CompatDBPage::Audio:
        setNextEnabled(m_audioSelection >= 0);
        break;
    case CompatDBPage::Final:
        setNextEnabled(!m_isSubmitting);
        break;
    }
}
