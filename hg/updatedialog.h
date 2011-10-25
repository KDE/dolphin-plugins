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

#ifndef HGUPDATEDIALOG_H
#define HGUPDATEDIALOG_H

#include <QtCore/QString>
#include <kdialog.h>

class KComboBox;
class QLabel;
class QCheckBox;

/**
 * Dialog to update working directory to specific revision/changeset/branch/tag.
 * Also shows working directory summary.
 */
class HgUpdateDialog : public KDialog
{
    Q_OBJECT

public:
    HgUpdateDialog(QWidget *parent = 0);

public slots:
    void slotUpdateDialog(int index);

private:
    void done(int r);

private:
    enum {ToBranch, ToTag, ToRevision} m_updateTo;
    KComboBox *m_selectType;
    KComboBox *m_selectFinal;
    QLabel *m_currentInfo;
    QStringList m_selectList;
    QCheckBox *m_discardChanges;
};

#endif // HGUPDATEDIALOG_H

