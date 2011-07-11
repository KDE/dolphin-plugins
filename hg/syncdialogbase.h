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

#ifndef HGSYNCBASEDILAOG_H
#define HGSYNCBASEDILAOG_H

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

class HgSyncBaseDialog : public KDialog
{
    Q_OBJECT

public:
    enum DialogType {PushDialog, PullDialog};

    HgSyncBaseDialog(DialogType dialogType, QWidget *parent = 0);

protected:
    QString remoteUrl() const;
    void done(int r);
    void setupUI();
    void createOptionGroup();
    void setup();
    virtual void setOptions() = 0;
    virtual void createChangesGroup() = 0;
    virtual void parseUpdateChanges(const QString &input) = 0;
    virtual void appendOptionArguments(QStringList &args) = 0;
    virtual void getHgChangesArguments(QStringList &args) = 0;

protected slots:
    void slotGetChanges();
    void slotChangeEditUrl(int index);
    void slotChangesProcessComplete(int exitCode, QProcess::ExitStatus status);
    void slotChangesProcessError();
    void slotOperationComplete(int exitCode, QProcess::ExitStatus status);
    void slotOperationError();
    void slotUpdateBusy(QProcess::ProcessState state);

protected:
    QMap<QString, QString> m_pathList;
    KComboBox *m_selectPathAlias;
    KLineEdit *m_urlEdit;
    QProgressBar *m_statusProg;
    bool m_haveChanges;
    HgWrapper *m_hgw;
    DialogType m_dialogType;

    // Options
    QList<QCheckBox*> m_options;
    QGroupBox *m_optionGroup;

    // changes
    KPushButton *m_changesButton;
    QGroupBox *m_changesGroup;
    QProcess m_process;
    QProcess m_main_process; //should I use another process?
};

#endif // HGSYNCBASEDILAOG_H

