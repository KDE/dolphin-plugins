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

#include <kdebug.h>
#include <kurl.h>

HgWrapper *HgWrapper::m_instance = 0;

HgWrapper::HgWrapper(QObject *parent) :
    QProcess(parent)
{
    m_localCodec = QTextCodec::codecForLocale();

    connect(this, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotOperationCompleted(int, QProcess::ExitStatus)));
    connect(this, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(slotOperationError()));
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

void HgWrapper::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Done executing successfully: 'hg' with arguments "
        << m_arguments;
    m_arguments.clear();
}

void HgWrapper::slotOperationError()
{
    qDebug() << "Error occurred while executing 'hg' with arguments "
        << m_arguments;
    m_arguments.clear();
}

bool HgWrapper::executeCommand(const QString &hgCommand,
                               const QStringList &arguments,
                               QString &output)
{
    Q_ASSERT(this->state() == QProcess::NotRunning);

    executeCommand(hgCommand, arguments);
    waitForFinished();
    output = readAll();

    return (exitStatus() == QProcess::NormalExit &&
            exitCode() == 0);
}
void HgWrapper::executeCommand(const QString &hgCommand,
                               const QStringList &arguments)
{
    Q_ASSERT(this->state() == QProcess::NotRunning);

    m_arguments << hgCommand;
    m_arguments << arguments;
    start(QLatin1String("hg"), m_arguments);
}

QString HgWrapper::getBaseDir() const
{
    return m_hgBaseDir;
}

void HgWrapper::updateBaseDir()
{
    setWorkingDirectory(m_currentDir);
    start(QLatin1String("hg root"));
    waitForFinished();
    m_hgBaseDir = QString(this->readAll()).trimmed();
}

void HgWrapper::setCurrentDir(const QString &directory)
{
    m_currentDir = directory;
    updateBaseDir();
}

void  HgWrapper::setBaseAsWorkingDir()
{
    setWorkingDirectory(getBaseDir());
}

void HgWrapper::addFiles(const KFileItemList &fileList)
{
    Q_ASSERT(this->state() == QProcess::NotRunning);

    m_arguments << QLatin1String("add");
    foreach (const KFileItem &item, fileList) {
        m_arguments << item.localPath();
    }
    start(QLatin1String("hg"), m_arguments);
}

bool HgWrapper::renameFile(const QString &source, const QString &destination)
{
    Q_ASSERT(this->state() == QProcess::NotRunning);

    QStringList args;
    args <<  source << destination;
    executeCommand(QLatin1String("rename"), args);

    waitForFinished();
    return (exitStatus() == QProcess::NormalExit
            && exitCode() == 0);
}

void HgWrapper::removeFiles(const KFileItemList &fileList)
{
    Q_ASSERT(this->state() == QProcess::NotRunning);

    m_arguments << QLatin1String("remove");
    foreach (const KFileItem &item, fileList) {
        m_arguments << item.localPath();
    }
    start(QLatin1String("hg"), m_arguments);
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
    waitForFinished();
    return (exitCode() == 0 && exitStatus() == QProcess::NormalExit);
}

//TODO: Make it return QStringList.
QString HgWrapper::getParentsOfHead()
{
    Q_ASSERT(this->state() == QProcess::NotRunning);

    QString line;
    start(QLatin1String("hg parents"));
    while (waitForReadyRead()) {
        char buffer[1024];
        while (readLine(buffer, sizeof(buffer))) {
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

QStringList HgWrapper::getBranches()
{
    QStringList result;
    executeCommand(QLatin1String("branches"));
    while (waitForReadyRead()) {
        char buffer[1048];
        while (readLine(buffer, sizeof(buffer)) > 0) {
            result << QString(buffer).remove(QRegExp("[\\s]+[\\d:a-zA-Z\\(\\)]*"));
        }
    }
    return result;
}

bool HgWrapper::createBranch(const QString &branchName)
{
    executeCommand("branch " + branchName);
    waitForFinished();
    return (exitStatus() == QProcess::NormalExit) && (exitCode() == 0);
}

QHash<QString, HgVersionState>& HgWrapper::getVersionStates(bool ignoreParents)
{
    int nTrimOutLeft = m_hgBaseDir.length();
    QString relativePrefix = m_currentDir.right(m_currentDir.length() -
                             nTrimOutLeft - 1);
    m_versionStateResult.clear();

    // Get status of files
    setWorkingDirectory(m_currentDir);
    start(QLatin1String("hg status"));
    while (waitForReadyRead()) {
        char buffer[1024];
        while (readLine(buffer, sizeof(buffer)) > 0)  {
            const QString currentLine(QTextCodec::codecForLocale()->toUnicode(buffer).trimmed());
            char currentStatus = buffer[0];
            QString currentFile = currentLine.mid(2);
            if (currentFile.startsWith(relativePrefix)) {
                HgVersionState vs = HgCleanVersion;
                switch (currentStatus) {
                case 'A':
                    vs = HgAddedVersion;
                    break;
                case 'M':
                    vs = HgModifiedVersion;
                    break;
                case '?':
                    vs = HgUntrackedVersion;
                    break;
                case 'R':
                    vs = HgRemovedVersion;
                    break;
                case 'I':
                    vs = HgIgnoredVersion;
                    break;
                case 'C':
                    vs = HgCleanVersion;
                    break;
                case '!':
                    vs = HgMissingVersion;
                    break;
                }
                if (vs != HgCleanVersion) {
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

#include "hgwrapper.moc"

