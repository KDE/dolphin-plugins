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

// TODO: Copy last commit messages
// TODO: Filter in HgStatusList.

class HgCommitDialog : public KDialog
{
    Q_OBJECT

public:
    HgCommitDialog(QWidget *parent = 0);

private slots:
    void slotItemSelectionChanged(const char status, const QString &fileName);
    void slotMessageChanged();
    void saveGeometry();
    void slotBranchActions(QAction *action);
    void slotInitDiffOutput();

private:
    QString getParentForLabel();
    void done(int r);

private:
    QString m_hgBaseDir;

    QPlainTextEdit *m_commitMessage;
    HgStatusList *m_statusList;

    KTextEditor::View *m_fileDiffView;
    KTextEditor::Document *m_fileDiffDoc;

    KPushButton *m_branchButton;

    KAction *m_closeBranch;
    KAction *m_newBranch;
    KAction *m_useCurrentBranch;
    KMenu *m_branchMenu;

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

//TODO: Design and implement commit options dialog
class CommitOptionsDialog : public KDialog
{
    Q_OBJECT
public:
    CommitOptionsDialog(QWidget *parent=0);
};

#endif // HGCOMMITDIALOG_H


