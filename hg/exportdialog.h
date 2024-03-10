/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGEXPORTDIALOG_H
#define HGEXPORTDIALOG_H

#include "dialogbase.h"

class HgCommitInfoWidget;
class QCheckBox;
class QGroupBox;

// TODO: Some helper for writing patterns
//
/**
 * Dialog to implement mercurial export feature. Dialogs presents list of
 * changesets from which the user will select entries and export a series of
 * patch files for each changeset.
 */
class HgExportDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgExportDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void done(int r) override;

private Q_SLOTS:
    void saveGeometry();

private:
    void setupUI();
    void loadCommits();

private:
    HgCommitInfoWidget *m_commitInfoWidget;

    // options
    QGroupBox *m_optionGroup;
    QCheckBox *m_optText;
    QCheckBox *m_optGit;
    QCheckBox *m_optNoDates;
};

#endif /* HGEXPORTDIALOG_H */
