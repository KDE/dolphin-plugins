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

#ifndef HGMERGE_H
#define HGMERGE_H

#include <QtCore/QString>
#include <kdialog.h>

class KComboBox;
class KPushButton;
class QLabel;
class HgCommitInfoWidget;

/**
 * Implements dialog to perform merge operations
 */
class HgMergeDialog : public KDialog
{
    Q_OBJECT

public:
    HgMergeDialog(QWidget *parent = 0);
    void done(int r);

private slots:
    void saveGeometry();

private:
    void updateInitialDialog();

private:
    QLabel *m_currentChangeset;
    HgCommitInfoWidget *m_commitInfoWidget;
};

#endif // HGMERGE_H

