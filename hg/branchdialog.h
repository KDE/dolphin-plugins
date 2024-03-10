/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGBRANCHDIALOG_H
#define HGBRANCHDIALOG_H

#include "dialogbase.h"
#include <QString>

class KComboBox;
class QLabel;

/**
 * Implements dialog to list & create branches and update/switch working
 * directory to different branch.
 */
class HgBranchDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgBranchDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void slotUpdateDialog(const QString &text);
    void slotCreateBranch();
    void slotSwitch();

private:
    void updateInitialDialog();

private:
    KComboBox *m_branchComboBox;
    QPushButton *m_createBranch;
    QPushButton *m_updateBranch;
    QLabel *m_currentBranchLabel;

    QStringList m_branchList;
};

#endif // HGBRANCHDIALOG_H
