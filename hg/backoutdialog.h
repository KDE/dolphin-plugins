/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HG_BACKOUT_DIALOG
#define HG_BACKOUT_DIALOG

#include "dialogbase.h"

class QGroupBox;
class QCheckBox;
class QLineEdit;
class HgCommitInfoWidget;

/**
 * Implements dialog for Mercurial's backout feature.
 */
class HgBackoutDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgBackoutDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void done(int r) override;

private Q_SLOTS:
    void saveGeometry();
    void slotSelectBaseChangeset();
    void slotSelectParentChangeset();
    void slotUpdateOkButton(const QString &text);

private:
    void setupUI();

    /**
     * Find appropriate changesets in repository and show them in 
     * Commit Selector (CommitInfoWidget)
     */
    void loadCommits();

    /** 
     * Opens a dialog showing all changesets in a list and their respective
     * information when highlighted. 
     */
    QString selectChangeset();

private:
    QGroupBox *m_mainGroup;
    HgCommitInfoWidget *m_commitInfo;

    QPushButton *m_selectBaseCommitButton;
    QLineEdit *m_baseRevision;

    QPushButton *m_selectParentCommitButton;
    QLineEdit *m_parentRevision;

    QCheckBox *m_optMerge;
};

#endif /* HG_BACKOUT_DIALOG */

