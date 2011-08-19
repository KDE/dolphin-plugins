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

#ifndef HGTAGDIALOG_H
#define HGTAGDIALOG_H

#include <QtCore/QString>
#include <kdialog.h>

class KComboBox;
class KPushButton;

/**
 * Dialog to create/delete/list tags and update working directory to revision
 * represented by a specific tag.
 */
class HgTagDialog : public KDialog
{
    Q_OBJECT

public:
    HgTagDialog(QWidget *parent = 0);

public slots:
    void slotUpdateDialog(const QString &text);
    void slotCreateTag();
    void slotSwitch();
    void slotRemoveTag();

private:
    void updateInitialDialog();

private:
    KComboBox *m_tagComboBox;
    KPushButton *m_createTag;
    KPushButton *m_updateTag;
    KPushButton *m_removeTag;
    QStringList m_tagList;
};

#endif // HGTAGDIALOG_H

