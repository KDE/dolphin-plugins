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

#include <QtCore/QString>
#include <QtCore/QProcess>
#include <QtCore/QMap>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QTableWidget>
#include <QtGui/QProgressBar>
#include <kdialog.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <kcombobox.h>
#include <kpushbutton.h>

//TODO: Save/Load dialog geometry
//TODO: Resize dialog according to visibility of outgoing changes

class HgPushDialog : public KDialog
{
    Q_OBJECT

public:
    HgPushDialog(QWidget *parent = 0);

private:
    void done(int r);
    void setupUI();
    void createOptionGroup();
    void createOutgoingChangesGroup();
    void appendOptionArguments(QStringList &args);
    void parseUpdateOutgoingChanges(const QString &input);

private slots:
    void slotChangeEditUrl(int index);
    void slotGetOutgoingChanges();
    void slotOutChangesProcessComplete(int exitCode, QProcess::ExitStatus status);
    void slotOutChangesProcessError(QProcess::ProcessError error);
    void slotOutSelChanged();

private:
    QMap<QString, QString> m_pathList;
    KComboBox *m_selectPathAlias;
    KLineEdit *m_pushUrlEdit;
    QProgressBar *m_statusProg;
    bool m_haveOutgoingChanges;
    HgWrapper *m_hgw;

    // Options
    QCheckBox *m_optAllowNewBranch;
    QCheckBox *m_optInsecure;
    QCheckBox *m_optForce;
    QGroupBox *m_optionGroup;

    // outgoing Changes
    KPushButton *m_outgoingChangesButton;
    QGroupBox *m_outChangesGroup;
    QTableWidget *m_outChangesList;
    KTextEdit *m_changesetInfo;
    QProcess m_process;
};

#endif // HGPUSHDILAOG_H

