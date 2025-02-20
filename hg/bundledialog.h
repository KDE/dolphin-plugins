/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGBUNDLEDIALOG_H
#define HGBUNDLEDIALOG_H

#include "dialogbase.h"

class QGroupBox;
class QCheckBox;
class QLineEdit;
class HgCommitInfoWidget;
class HgPathSelector;

/**
 * Dialog which implements bundle feature of Mercurial. Bundle enables
 * user to creates a file containing all the selected/desired changesets
 * in mercurial's internal format rather than patches.
 *
 * Changesets can either be selected by user or after being compared by
 * remote repository selected.
 *
 */
class HgBundleDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgBundleDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void done(int r) override;

private Q_SLOTS:
    void saveGeometry();

    /**
     * Opens a dialog listing all changeset from which user will select a
     * changeset for base revision.
     */
    void slotSelectChangeset();
    void slotAllChangesCheckToggled(Qt::CheckState state);

private:
    void setupUI();

    /**
     * Creates bundle file.
     *
     * @param fileName Path to file where bundle will be created.
     */
    void createBundle(const QString &fileName);

    /**
     * Find all changesets in repository and show them in Commit Selector in
     * Base Changeset selector.
     */
    void loadCommits();

private:
    QGroupBox *m_mainGroup;
    HgPathSelector *m_pathSelect;
    HgCommitInfoWidget *m_commitInfo;
    QPushButton *m_selectCommitButton;
    QLineEdit *m_baseRevision;
    QCheckBox *m_allChangesets;

    // options
    QGroupBox *m_optionGroup;
    QCheckBox *m_optForce;
    QCheckBox *m_optInsecure;
};

#endif /* HGBUNDLEDIALOG_H */
