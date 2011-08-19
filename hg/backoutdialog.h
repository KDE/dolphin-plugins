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

#include <kdialog.h>

class QGroupBox;
class QCheckBox;
class KLineEdit;
class HgCommitInfoWidget;
class KPushButton;

/**
 * Implements dialog for Mercurial's backout feature.
 */
class HgBackoutDialog : public KDialog
{
    Q_OBJECT

public:
    HgBackoutDialog(QWidget *parent = 0);

public slots:
    void done(int r);

private slots:
    void saveGeometry();
    void slotSelectBaseChangeset();
    void slotSelectParentChangeset();
    void slotUpdateOkButton(const QString &text);

private:
    void setupUI();

    /**
     * Find appropiate changesets in repository and show them in 
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

    KPushButton *m_selectBaseCommitButton;
    KLineEdit *m_baseRevision;

    KPushButton *m_selectParentCommitButton;
    KLineEdit *m_parentRevision;

    QCheckBox *m_optMerge;
};

#endif /* HG_BACKOUT_DIALOG */

