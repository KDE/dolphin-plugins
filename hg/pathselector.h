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

#ifndef HGPATHSELECTOR_H
#define HGPATHSELECTOR_H

#include <QtGui/QWidget>
#include <QtCore/QMap>

class KComboBox;
class KLineEdit;

/**
 * A simple widget which presents a ComboBox to select list of Path aliases
 * stored in .hgrc file and show their URL. URL's can be entered manually
 * as well.
 */
class HgPathSelector : public QWidget
{
    Q_OBJECT

public:
    HgPathSelector(QWidget *parent=0);

    /**
     * @return Return QString containing the selected/entered alias/URL
     */
    const QString remote() const;

public slots:
    void reload();

private:
    void setupUI();

private slots:
    void slotChangeEditUrl(int index);

private:
    QMap<QString, QString> m_pathList;
    KComboBox *m_selectPathAlias;
    KLineEdit *m_urlEdit;
};

#endif /* HGPATHSELECTOR_H */

