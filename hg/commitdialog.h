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

#ifndef HGCOMMITDIALOG_H
#define HGCOMMITDIALOG_H

#include "statuslist.h"

#include <QtCore/QString>
#include <kdialog.h>


class QPlainTextEdit;
class KAction;
class KMenu;
class QLabel;

namespace KTextEditor {
    class View;
    class Document;
};

// TODO: Filter in HgStatusList.

/**
 * Implements dialog for Commit changes. User is presented with a list of
 * changes/added/removed files, their diffs, a TextEdit to enter 
 * commit message, options to change/create/close branch and last 5 commit
 * messages.
 */
class HgCommitDialog : public KDialog
{
    Q_OBJECT

public:
    HgCommitDialog(QWidget *parent = 0);

private slots:
    /**
     * Shows diff of selected file in a KTextEditor widget when user selects
     * one of the entry in HgStatusList widget.
     */
    void slotItemSelectionChanged(const char status, const QString &fileName);

    /**
     * Will enable 'Ok' button of dialog if some message text is available or 
     * disables it.
     */
    void slotMessageChanged();
    void saveGeometry();
    void slotBranchActions(QAction *action);

    /**
     * Shows diff of whole working directory together in KTextEditor widget.
     * Equivalent to plain 'hg diff'
     */
    void slotInitDiffOutput();
    void slotInsertCopyMessage(QAction *action);

private:
    QString getParentForLabel();
    void createCopyMessageMenu();
    void done(int r);

private:
    QString m_hgBaseDir;

    QPlainTextEdit *m_commitMessage;
    HgStatusList *m_statusList;

    KTextEditor::View *m_fileDiffView;
    KTextEditor::Document *m_fileDiffDoc;

    KPushButton *m_branchButton;
    KPushButton *m_copyMessageButton;

    KAction *m_closeBranch;
    KAction *m_newBranch;
    KAction *m_useCurrentBranch;
    KMenu *m_branchMenu;
    KMenu *m_copyMessageMenu;

    /** What will commit do with branch. 
     *
     * CloseBranch: Close the current branch
     * NewBranch  : Creates new branch for this commit
     * NoChanges  : No changes to branch are made. 
     */
    enum {CloseBranch, NewBranch, NoChanges} m_branchAction;
    QString m_newBranchName;
};

/**
 * Dialog which asks for the new branch name in commit dialog
 */
class NewBranchDialog : public KDialog 
{
        Q_OBJECT

    public:
        NewBranchDialog(QWidget *parent = 0);
        QString getBranchName() const;

    private slots:
        void slotTextChanged(const QString &text);

    private:
        QLabel *m_errorLabel;
        KLineEdit *m_branchNameInput;
        QStringList m_branchList;

};

#endif // HGCOMMITDIALOG_H


