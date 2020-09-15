/***************************************************************************
 *   Copyright (C) 2011 by Vishesh Yadav <vishesh3y@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

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
    explicit HgBackoutDialog(QWidget *parent = 0);

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

