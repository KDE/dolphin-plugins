/***************************************************************************
 *   Copyright (C) 2019-2020                                               *
 *                  by Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>  *
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

#ifndef SVNPROGRESSDIALOG_H
#define SVNPROGRESSDIALOG_H

#include <QDialog>

#include "ui_svnprogressdialog.h"

class QProcess;

/**
 * \brief Dialog for showing SVN operation process.
 *
 * This dialog connects to the Subversion process (by \p connectToProcess()) and shows its output.
 * User has possibility to terminate the process by pressing cancel button. Normally do not need to
 * call \p disconnectFromProcess() as it calls automaticaly on connected process finished() signal.
 *
 * \note This class can call 'svn cleanup' on a Subversion process dir in case of terminating it if
 *       a working directory were passed to the constructor.
 */
class SvnProgressDialog : public QDialog {
    Q_OBJECT
public:
    /**
     * \param[in] title Dialog title.
     * \param[in] workingDir Directory to call 'svn cleanup' on. Empty for no cleanup.
     * \param[in,out] parent Parent widget.
     */
    SvnProgressDialog(const QString& title, const QString& workingDir = QString(), QWidget *parent = nullptr);
    virtual ~SvnProgressDialog() override;

    /**
     * Connects to the process signals, stdout and stderr.
     */
    void connectToProcess(QProcess *process);

    /**
     * Disconnects from previously connected process, nothing happens either. This function is
     * automaticaly called on connected process finished() signal.
     */
    void disconnectFromProcess();

public slots:
    void appendInfoText(const QString& text);
    void appendErrorText(const QString& text);
    void operationCompeleted();

    virtual void reject() override;

private:
    Ui::SvnProgressDialog m_ui;

    QMetaObject::Connection m_conCancel;
    QMetaObject::Connection m_conCompeted;
    QMetaObject::Connection m_conProcessError;
    QMetaObject::Connection m_conStdOut;
    QMetaObject::Connection m_conStrErr;

    bool m_svnTerminated;
    const QString m_workingDir;
};

#endif  // SVNPROGRESSDIALOG_H
