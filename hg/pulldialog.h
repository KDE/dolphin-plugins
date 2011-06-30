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
//TODO: Resize dialog according to visibility of incoming changes
//TODO: HTTPS login
//TODO: Cancel current operation

class HgPullDialog : public KDialog
{
    Q_OBJECT

public:
    HgPullDialog(QWidget *parent = 0);

private:
    QString pullPath() const;
    void done(int r);
    void setupUI();
    void createOptionGroup();
    void createIncomingChangesGroup();
    void parseUpdateIncomingChanges(const QString &input);
    void appendOptionArguments(QStringList &args);

private slots:
    void slotChangeEditUrl(int index);
    void slotGetIncomingChanges();
    void slotIncChangesProcessComplete(int exitCode, QProcess::ExitStatus status);
    void slotIncChangesProcessError(QProcess::ProcessError error);
    //void slotIncSelChanged();
    void slotPullComplete(int exitCode, QProcess::ExitStatus status);
    void slotPullError(QProcess::ProcessError);

private:
    QMap<QString, QString> m_pathList;
    KComboBox *m_selectPathAlias;
    KLineEdit *m_pullUrlEdit;
    QProgressBar *m_statusProg;
    bool m_haveIncomingChanges;
    HgWrapper *m_hgw;

    // Options
    QCheckBox *m_optUpdate;
    QCheckBox *m_optInsecure;
    QCheckBox *m_optForce;
    QGroupBox *m_optionGroup;

    // incoming Changes
    KPushButton *m_incomingChangesButton;
    QGroupBox *m_incChangesGroup;
    QTableWidget *m_incChangesList;
    //KTextEdit *m_changesetInfo;
    QProcess m_process;

    // current task
    enum {NoTask, IncomingChanges, PullingChanges} m_currentTask;
};

#endif // HGPULLDILAOG_H

