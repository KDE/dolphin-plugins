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

#include "svnprogressdialog.h"

#include <QProcess>
#include <QDebug>

#include "svncommands.h"

SvnProgressDialog::SvnProgressDialog(const QString& title, const QString& workingDir, QWidget *parent) :
    QDialog(parent),
    m_svnTerminated(false),
    m_workingDir(workingDir)
{
    m_ui.setupUi(this);

    /*
     * Add actions, establish connections.
     */
    QObject::connect(m_ui.buttonOk, &QPushButton::clicked, this, &QDialog::close);

    /*
     * Additional setup.
     */
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(title);
    show();
    activateWindow();
}

SvnProgressDialog::~SvnProgressDialog()
{
    disconnectFromProcess();
}

void SvnProgressDialog::connectToProcess(QProcess *process)
{
    disconnectFromProcess();

    m_svnTerminated = false;

    m_conCancel = connect(m_ui.buttonCancel, &QPushButton::clicked, [this, process] () {
        process->terminate();
        m_svnTerminated = true;
    } );
    m_conCompeted = connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &SvnProgressDialog::operationCompeleted);
    m_conProcessError = connect(process, &QProcess::errorOccurred, [this, process] (QProcess::ProcessError) {
        const QString commandLine = process->program() + process->arguments().join(' ');
        appendErrorText(i18nc("@info:status", "Error starting: %1", commandLine));
        operationCompeleted();
    } );
    m_conStdOut = connect(process, &QProcess::readyReadStandardOutput, [this, process] () {
        appendInfoText( process->readAllStandardOutput() );
    } );
    m_conStrErr = connect(process, &QProcess::readyReadStandardError, [this, process] () {
        appendErrorText( process->readAllStandardError() );
    } );
}

void SvnProgressDialog::disconnectFromProcess()
{
    QObject::disconnect(m_conCancel);
    QObject::disconnect(m_conCompeted);
    QObject::disconnect(m_conProcessError);
    QObject::disconnect(m_conStdOut);
    QObject::disconnect(m_conStrErr);
}

void SvnProgressDialog::appendInfoText(const QString& text)
{
    const QTextCursor pos = m_ui.texteditMessage->textCursor();

    m_ui.texteditMessage->moveCursor(QTextCursor::End);
    m_ui.texteditMessage->insertPlainText(text);
    m_ui.texteditMessage->setTextCursor(pos);
}

void SvnProgressDialog::appendErrorText(const QString& text)
{
    static const QString htmlBegin = "<font color=\"Red\">";
    static const QString htmlEnd = "</font><br>";

    QString message = QString(text).replace('\n', QLatin1String("<br>"));
    // Remove last <br> as it will be in htmlEnd.
    if (message.endsWith(QLatin1String("<br>"))) {
        message.chop(4);
    }

    m_ui.texteditMessage->appendHtml(htmlBegin + message + htmlEnd);
}

void SvnProgressDialog::operationCompeleted()
{
    disconnectFromProcess();

    if (m_svnTerminated && !m_workingDir.isEmpty()) {
        const CommandResult result = SvnCommands::cleanup(m_workingDir);
        if (!result.success) {
            qWarning() << QString("'svn cleanup' failed for %1").arg(m_workingDir);
            qWarning() << result.stdErr;
        }
        m_svnTerminated = false;
    }

    m_ui.buttonOk->setEnabled(true);
    m_ui.buttonCancel->setEnabled(false);
}

void SvnProgressDialog::reject()
{
    if (m_ui.buttonOk->isEnabled()) {
        QDialog::reject();
    } else {
        emit m_ui.buttonCancel->clicked();
    }
}
