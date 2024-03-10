/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGMERGEDIALOG_H
#define HGMERGEDIALOG_H

#include "dialogbase.h"
#include <QString>

class QLabel;
class HgCommitInfoWidget;

/**
 * Implements dialog to perform merge operations
 */
class HgMergeDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgMergeDialog(QWidget *parent = nullptr);
    void done(int r) override;

private Q_SLOTS:
    void saveGeometry();

private:
    void updateInitialDialog();

private:
    QLabel *m_currentChangeset;
    HgCommitInfoWidget *m_commitInfoWidget;
};

#endif // HGMERGEDIALOG_H
