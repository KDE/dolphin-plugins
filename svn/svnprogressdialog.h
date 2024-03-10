/*
    SPDX-FileCopyrightText: 2019-2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
class SvnProgressDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * \param[in] title Dialog title.
     * \param[in] workingDir Directory to call 'svn cleanup' on. Empty for no cleanup.
     * \param[in,out] parent Parent widget.
     */
    SvnProgressDialog(const QString &title, const QString &workingDir = QString(), QWidget *parent = nullptr);
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

public Q_SLOTS:
    void appendInfoText(const QString &text);
    void appendErrorText(const QString &text);
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

#endif // SVNPROGRESSDIALOG_H
