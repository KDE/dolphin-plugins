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

#ifndef HGBUNDLEDIALOG_H
#define HGBUNDLEDIALOG_H

#include <kdialog.h>

class QGroupBox;
class QCheckBox;
class KLineEdit;
class HgCommitInfoWidget;
class HgPathSelector;
class KPushButton;

/**
 * Dialog which implements bundle feature of Mercurial. Bundle enables
 * user to creates a file containg all the selected/desired changesets
 * in mercurial's internal format rather than patches. 
 * 
 * Changesets can either be selected by user or after being compared by
 * remote repository selected.
 *
 */
class HgBundleDialog : public KDialog
{
    Q_OBJECT

public:
    HgBundleDialog(QWidget *parent=0);

public slots:
    void done(int r);

private slots:
    void saveGeometry();

    /**
     * Opens a dialog listing all changeset from which user will select a 
     * changeset for base revision. 
     */
    void slotSelectChangeset();
    void slotAllChangesCheckToggled(int state);

private:
    void setupUI();

    /**
     * Creates bundle file.
     *
     * @param fileName Path to file where bundle will be created.
     */
    void createBundle(const QString &fileName);

    /**
     * Find all changesets in respository and show them in Commit Selector in 
     * Base Changeset selector.
     */
    void loadCommits();

private:
    QGroupBox *m_mainGroup;
    HgPathSelector *m_pathSelect;
    HgCommitInfoWidget *m_commitInfo;
    KPushButton *m_selectCommitButton;
    KLineEdit *m_baseRevision;
    QCheckBox *m_allChangesets;

    //options
    QGroupBox *m_optionGroup;
    QCheckBox *m_optForce;
    QCheckBox *m_optInsecure;
};

#endif /* HGBUNDLEDIALOG_H */

