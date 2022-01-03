/*
    SPDX-FileCopyrightText: 2019-2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

    m_conCancel = connect(m_ui.buttonCancel, &QPushButton::clicked, this, [this, process] () {
        process->terminate();
        m_svnTerminated = true;
    } );
    m_conCompeted = connect(process, &QProcess::finished, this, &SvnProgressDialog::operationCompeleted);
    m_conProcessError = connect(process, &QProcess::errorOccurred, this, [this, process] (QProcess::ProcessError) {
        const QString commandLine = process->program() + process->arguments().join(' ');
        appendErrorText(i18nc("@info:status", "Error starting: %1", commandLine));
        operationCompeleted();
    } );
    m_conStdOut = connect(process, &QProcess::readyReadStandardOutput, this, [this, process] () {
        appendInfoText( process->readAllStandardOutput() );
    } );
    m_conStrErr = connect(process, &QProcess::readyReadStandardError, this, [this, process] () {
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
        Q_EMIT m_ui.buttonCancel->clicked();
    }
}
