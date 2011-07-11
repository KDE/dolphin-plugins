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

#ifndef HGPULLDILAOG_H
#define HGPULLDILAOG_H

#include "hgwrapper.h"
#include "syncdialogbase.h"

#include <QtCore/QString>
#include <QtGui/QCheckBox>
#include <QtGui/QTableWidget>
#include <ktextedit.h>
#include <kcombobox.h>

//TODO: Save/Load dialog geometry
//TODO: Resize dialog according to visibility of incoming changes
//TODO: HTTPS login
//TODO: Cancel current operation

class HgPullDialog : public HgSyncBaseDialog
{
    Q_OBJECT

public:
    HgPullDialog(QWidget *parent = 0);

protected:
    void setOptions();
    void parseUpdateChanges(const QString &input); 
    void appendOptionArguments(QStringList &args); 
    void createChangesGroup();
    void getHgChangesArguments(QStringList &args);

private slots:
    void slotUpdateChangesGeometry();
    void readBigSize();
    void writeBigSize();

private:
    // Options
    QCheckBox *m_optUpdate;
    QCheckBox *m_optInsecure;
    QCheckBox *m_optForce;
    QGroupBox *m_optionGroup;

    // incoming Changes
    QTableWidget *m_changesList;
};

#endif // HGPULLDILAOG_H

