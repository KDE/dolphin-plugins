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

#ifndef HGEXPORTDIALOG_H
#define HGEXPORTDIALOG_H

#include "dialogbase.h"

class HgCommitInfoWidget;
class QCheckBox;
class QGroupBox;

//TODO: Some helper for writing patterns
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
    explicit HgExportDialog(QWidget *parent=0);

public Q_SLOTS:
    void done(int r) override;

private Q_SLOTS:
    void saveGeometry();

private:
    void setupUI();
    void loadCommits();

private:
    HgCommitInfoWidget *m_commitInfoWidget;

    //options
    QGroupBox *m_optionGroup;
    QCheckBox *m_optText;
    QCheckBox *m_optGit;
    QCheckBox *m_optNoDates;

};

#endif /* HGEXPORTDIALOG_H */ 

