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

#include "hgwrapper.h"

#include <QtGui/QApplication>
#include <kdebug.h>
#include <kurl.h>

//TODO: Replace start() with executeCommand functions wherever possible.

HgWrapper *HgWrapper::m_instance = 0;

HgWrapper::HgWrapper(QObject *parent) :
    QObject(parent)
{
    m_localCodec = QTextCodec::codecForLocale();

    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotOperationCompleted(int, QProcess::ExitStatus)));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(slotOperationError()));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
            this, SIGNAL(error(QProcess::ProcessError))); 
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SIGNAL(finished(int, QProcess::ExitStatus))),
    connect(&m_process, SIGNAL(started()),
            this, SIGNAL(started()));
}

HgWrapper *HgWrapper::instance()
{
    if (!m_instance) {
        m_instance = new HgWrapper;
    }
    return m_instance;
}

void HgWrapper::freeInstance()
{
    delete m_instance;
    m_instance = 0;
}

void HgWrapper::slotOperationCompleted(int exitCode, 
                                       QProcess::ExitStatus exitStatus)
{
    kDebug() << "Done executing successfully: 'hg' with arguments "
        << m_arguments << " Exit Code: " << exitCode;
    m_arguments.clear();
}

void HgWrapper::slotOperationError()
{
    kDebug() << "Error occurred while executing 'hg' with arguments "
        << m_arguments;
    m_arguments.clear();
}

bool HgWrapper::executeCommand(const QString &hgCommand,
                               const QStringList &arguments,
                               QString &output)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    executeCommand(hgCommand, arguments);
    m_process.waitForFinished();
    output = QTextCodec::codecForLocale()->toUnicode(m_process.readAllStandardOutput());

    return (m_process.exitStatus() == QProcess::NormalExit &&
            m_process.exitCode() == 0);
}

void HgWrapper::executeCommand(const QString &hgCommand,
                               const QStringList &arguments)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    m_arguments << hgCommand;
    m_arguments << arguments;
    m_process.setWorkingDirectory(m_currentDir);
    m_process.start(QLatin1String("hg"), m_arguments);
}

bool HgWrapper::executeCommand(const QString &hgCommand,
                               const QStringList &arguments,
                               KTextEdit *textEdit)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    m_arguments << hgCommand;
    m_arguments << arguments;
    connect(&m_process, SIGNAL(readyRead()),
            this, SLOT(slotOutputToTextEdit()));
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotCleanTextEditAfterFinish(int, QProcess::ExitStatus)));
    m_outTextEdit = textEdit;
    m_process.setWorkingDirectory(m_currentDir);
    kDebug() << "Current directory: " << m_currentDir;
    m_process.start(QLatin1String("hg"), m_arguments, QProcess::Unbuffered|QProcess::ReadWrite);

    return (m_process.exitStatus() == QProcess::NormalExit &&
            m_process.exitCode() == 0);
}

bool HgWrapper::executeCommandTillFinished(const QString &hgCommand,
                               const QStringList &arguments)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    m_arguments << hgCommand;
    m_arguments << arguments;
    m_process.setWorkingDirectory(m_currentDir);
    m_process.start(QLatin1String("hg"), m_arguments);
    m_process.waitForFinished();

    return (m_process.exitStatus() == QProcess::NormalExit &&
            m_process.exitCode() == 0);
}

QString HgWrapper::getBaseDir() const
{
    return m_hgBaseDir;
}

QString HgWrapper::getCurrentDir() const
{
    return m_currentDir;
}

void HgWrapper::updateBaseDir()
{
    m_process.setWorkingDirectory(m_currentDir);
    m_process.start(QLatin1String("hg root"));
    m_process.waitForFinished();
    m_hgBaseDir = QString(m_process.readAll()).trimmed();
}

void HgWrapper::setCurrentDir(const QString &directory)
{
    m_currentDir = directory;
    updateBaseDir();
}

void  HgWrapper::setBaseAsWorkingDir()
{
    m_process.setWorkingDirectory(getBaseDir());
}

void HgWrapper::addFiles(const KFileItemList &fileList)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    m_arguments << QLatin1String("add");
    foreach (const KFileItem &item, fileList) {
        m_arguments << item.localPath();
    }
    m_process.start(QLatin1String("hg"), m_arguments);
}

bool HgWrapper::renameFile(const QString &source, const QString &destination)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    QStringList args;
    args <<  source << destination;
    executeCommand(QLatin1String("rename"), args);

    m_process.waitForFinished();
    return (m_process.exitStatus() == QProcess::NormalExit
            && m_process.exitCode() == 0);
}

void HgWrapper::removeFiles(const KFileItemList &fileList)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    m_arguments << QLatin1String("remove");
    foreach (const KFileItem &item, fileList) {
        m_arguments << item.localPath();
    }
    m_process.start(QLatin1String("hg"), m_arguments);
}

bool HgWrapper::commit(const QString &message, const QStringList &files,
                       bool closeCurrentBranch)
{
    QStringList args;
    args << files;
    args << QLatin1String("-m") << message;
    if (closeCurrentBranch) {
        args << "--close-branch";
    }
    executeCommand(QLatin1String("commit"), args);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0 
            && m_process.exitStatus() == QProcess::NormalExit);
}

bool HgWrapper::createBranch(const QString &name)
{
    QStringList args;
    args << name;
    executeCommand(QLatin1String("branch"), args);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0 
            && m_process.exitStatus() == QProcess::NormalExit);
}

bool HgWrapper::switchBranch(const QString &name)
{
    QStringList args;
    args << QLatin1String("-c") << name;
    executeCommand(QLatin1String("update"), args);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0
            && m_process.exitStatus() == QProcess::NormalExit);
}

bool HgWrapper::createTag(const QString &name)
{
    QStringList args;
    args << name;
    executeCommand(QLatin1String("tag"), args);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0 
            && m_process.exitStatus() == QProcess::NormalExit);
}

bool HgWrapper::revertAll()
{
    QStringList args;
    args << "--all";
    return executeCommandTillFinished(QLatin1String("revert"), args);
}


bool HgWrapper::revert(const KFileItemList &fileList)
{
    m_arguments << QLatin1String("revert");
    foreach (const KFileItem &item, fileList) {
        m_arguments << item.localPath();
    }
    return executeCommandTillFinished(QLatin1String("hg"), m_arguments);
}

bool HgWrapper::switchTag(const QString &name)
{
    QStringList args;
    args << QLatin1String("-c") << name;
    executeCommand(QLatin1String("update"), args);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0 
            && m_process.exitStatus() == QProcess::NormalExit);
}

//TODO: Make it return QStringList.
QString HgWrapper::getParentsOfHead()
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    QString line;
    m_process.start(QLatin1String("hg parents"));
    while (m_process.waitForReadyRead()) {
        char buffer[1024];
        while (m_process.readLine(buffer, sizeof(buffer))) {
            QString bufferString = QString(buffer);
            if (bufferString.contains("changeset:")) {
                QStringList parts = bufferString.split(" ", QString::SkipEmptyParts);
                line += parts.takeLast().trimmed();
                line += "   ";
            }
        }
    }
    return line;
}

QStringList HgWrapper::getTags()
{
    QStringList result;
    executeCommand(QLatin1String("tags"));
    while (m_process.waitForReadyRead()) {
        char buffer[1048];
        while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            result << QString(buffer).split(QRegExp("\\s+"),
                    QString::SkipEmptyParts)[0];
        }
    }
    return result;
}

QStringList HgWrapper::getBranches()
{
    QStringList result;
    executeCommand(QLatin1String("branches"));
    while (m_process.waitForReadyRead()) {
        char buffer[1048];
        while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            result << QString(buffer).remove(QRegExp("[\\s]+[\\d:a-zA-Z\\(\\)]*"));
        }
    }
    return result;
}

QHash<QString, KVersionControlPlugin::VersionState>& HgWrapper::getVersionStates(bool ignoreParents)
{
    int nTrimOutLeft = m_hgBaseDir.length();
    QString relativePrefix = m_currentDir.right(m_currentDir.length() -
                             nTrimOutLeft - 1);
    m_versionStateResult.clear();

    // Get status of files
    QStringList args;
    args << QLatin1String("status");
    args << QLatin1String("--modified");
    args << QLatin1String("--added");
    args << QLatin1String("--removed");
    args << QLatin1String("--deleted");
    args << QLatin1String("--unknown");
    args << QLatin1String("--ignored");
    m_process.setWorkingDirectory(m_currentDir);
    m_process.start(QLatin1String("hg"), args);
    while (m_process.waitForReadyRead()) {
        char buffer[1024];
        while (m_process.readLine(buffer, sizeof(buffer)) > 0)  {
            const QString currentLine(QTextCodec::codecForLocale()->toUnicode(buffer).trimmed());
            char currentStatus = buffer[0];
            QString currentFile = currentLine.mid(2);
            if (currentFile.startsWith(relativePrefix)) {
                KVersionControlPlugin::VersionState vs = KVersionControlPlugin::NormalVersion;
                switch (currentStatus) {
                case 'A':
                    vs = KVersionControlPlugin::AddedVersion;
                    break;
                case 'M':
                    vs = KVersionControlPlugin::LocallyModifiedVersion;
                    break;
                case '?':
                    vs = KVersionControlPlugin::UnversionedVersion;
                    break;
                case 'R':
                    vs = KVersionControlPlugin::RemovedVersion;
                    break;
                case 'I':
                    vs = KVersionControlPlugin::IgnoredVersion;
                    break;
                case 'C':
                    vs = KVersionControlPlugin::NormalVersion;
                    break;
                case '!':
                    vs = KVersionControlPlugin::MissingVersion;
                    break;
                }
                if (vs != KVersionControlPlugin::NormalVersion) {
                    KUrl url = KUrl::fromPath(m_hgBaseDir);
                    url.addPath(currentFile);
                    QString filePath = url.path();
                    m_versionStateResult.insert(filePath, vs);
                }
            }
        }
    }
    return m_versionStateResult;
}

void HgWrapper::slotOutputToTextEdit()
{
    kDebug() << "called";
    QString out = m_process.readAllStandardOutput();
    m_outTextEdit->append(out);
}

void HgWrapper::slotCleanTextEditAfterFinish(int exitStatus, QProcess::ExitStatus)
{
    /*disconnect(&m_process, SIGNAL(finished()),
            this, SLOT(slotCleanTextEditAfterFinish()));*/
}


void HgWrapper::terminateCurrentProcess()
{
    m_process.terminate();
}

#include "hgwrapper.moc"

