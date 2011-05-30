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

#ifndef STATUSLIST_H
#define STATUSLIST_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QTableWidget>
#include <QtGui/QGroupBox>
#include <klineedit.h>

class HgStatusList : public QGroupBox
{
    Q_OBJECT

public:
    HgStatusList(QWidget *parent = 0);
    bool getSelectionForCommit(QStringList &files);

private slots:
    void reloadStatusTable();

private slots:
    void itemSelectionChangedSlot();

signals:
    void itemSelectionChanged(const char status, const QString &fileName);

private:
    QString m_hgBaseDir;

    QTableWidget *m_statusTable;
    KLineEdit *m_filter;
};

#endif // STATUSLIST_H

