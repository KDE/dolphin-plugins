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

#include <QtGui/QGroupBox>
#include <QtCore/QString>

class KLineEdit;
class QTableWidget;

/**
 * Shows a list of files and their corresponding version states in a table.
 * Used in commit dialog.
 */
class HgStatusList : public QGroupBox
{
    Q_OBJECT

public:
    HgStatusList(QWidget *parent = 0);

    /**
     * Appends the list of selected files whose changes should be 
     * committed. If all files are selected, nothing is appended and true 
     * is returned. If no files are selected, false is returned.
     *
     * @param files Append all the selected files to this. If all files are
     *              selected, nothing is appended
     * @return If at least one file is selected, true is returned; otherwise
     *          false.
     */
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
    //KLineEdit *m_filter;
};

#endif // STATUSLIST_H

