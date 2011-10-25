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

#ifndef HGPUSHDILAOG_H
#define HGPUSHDILAOG_H

#include "hgwrapper.h"
#include "syncdialogbase.h"

class QCheckBox;
class QTableWidget;
class KTextEdit;
class QGroupBox;

/**
 * Dialog to implement Push operation
 */
class HgPushDialog : public HgSyncBaseDialog 
{
    Q_OBJECT

public:
    HgPushDialog(QWidget *parent = 0);

private:
    void setOptions();
    void createChangesGroup();
    void parseUpdateChanges(const QString &input);
    void appendOptionArguments(QStringList &args);
    void getHgChangesArguments(QStringList &args);
    void noChangesMessage();

private slots:
    void slotOutSelChanged();
    void slotUpdateChangesGeometry();
    void readBigSize();
    void writeBigSize();

private:
    // Options
    QCheckBox *m_optAllowNewBranch;
    QCheckBox *m_optInsecure;
    QCheckBox *m_optForce;
    QGroupBox *m_optionGroup;

    // outgoing Changes
    QTableWidget *m_outChangesList;
    KTextEdit *m_changesetInfo;
};

#endif // HGPUSHDILAOG_H

