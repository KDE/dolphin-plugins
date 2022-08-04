/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "hgwrapper.h"

#include <QApplication>
#include <QTextCodec>
#include <QUrl>
#include <QDebug>
#include <QRegularExpression>

//TODO: Replace start() with executeCommand functions wherever possible.
//FIXME: Add/Remove/Revert argument length limit. Divide the list.
//FIXME: Cannot create thread for parent that is in different thread.

HgWrapper *HgWrapper::m_instance = nullptr;

HgWrapper::HgWrapper(QObject *parent) :
    QObject(parent)
{
    m_localCodec = QTextCodec::codecForLocale();

    // re-emit QProcess signals
    connect(&m_process, &QProcess::errorOccurred,
            this, &HgWrapper::errorOccurred);
    connect(&m_process, &QProcess::finished,
            this, &HgWrapper::finished);
    connect(&m_process, &QProcess::stateChanged,
            this, &HgWrapper::stateChanged);
    connect(&m_process, &QProcess::started,
            this, &HgWrapper::started);

    connect(&m_process, &QProcess::finished,
            this, &HgWrapper::slotOperationCompleted);
    connect(&m_process, &QProcess::errorOccurred,
            this, &HgWrapper::slotOperationError);

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
    m_instance = nullptr;
}

void HgWrapper::slotOperationCompleted(int exitCode, 
                                       QProcess::ExitStatus exitStatus)
{
    qDebug() << "'hg' Exit Code: " << exitCode << "  Exit Status: "
        << exitStatus;
    if (m_primaryOperation) {
        Q_EMIT primaryOperationFinished(exitCode, exitStatus);
    }
}

void HgWrapper::slotOperationError(QProcess::ProcessError error)
{
    qDebug() << "Error occurred while executing 'hg' with arguments ";
    if (m_primaryOperation) {
        Q_EMIT primaryOperationError(error);
    }
}

bool HgWrapper::executeCommand(const QString &hgCommand,
                               const QStringList &arguments,
                               QString &output,
                               bool primaryOperation)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    executeCommand(hgCommand, arguments, primaryOperation);
    m_process.waitForFinished();
    output = QTextCodec::codecForLocale()->toUnicode(m_process.readAllStandardOutput());

    return (m_process.exitStatus() == QProcess::NormalExit &&
            m_process.exitCode() == 0);
}

void HgWrapper::executeCommand(const QString &hgCommand,
                               const QStringList &arguments,
                               bool primaryOperation)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    m_primaryOperation = primaryOperation;
    if (m_primaryOperation) {
        qDebug() << "Primary operation";
    }

    QStringList args;
    args << hgCommand;
    args << arguments;
    m_process.setWorkingDirectory(m_currentDir);
    m_process.start(QLatin1String("hg"), args);
}

bool HgWrapper::executeCommandTillFinished(const QString &hgCommand,
                               const QStringList &arguments,
                               bool primaryOperation)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    m_primaryOperation = primaryOperation;

    QStringList args;
    args << hgCommand;
    args << arguments;
    m_process.setWorkingDirectory(m_currentDir);
    m_process.start(QLatin1String("hg"), args);
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
    m_process.start(QStringLiteral("hg"), QStringList{QStringLiteral("root")});
    m_process.waitForFinished();
    m_hgBaseDir = QString(m_process.readAllStandardOutput()).trimmed();
}

void HgWrapper::setCurrentDir(const QString &directory)
{
    m_currentDir = directory;
    updateBaseDir(); //now get root directory of repository
}

void  HgWrapper::setBaseAsWorkingDir()
{
    m_process.setWorkingDirectory(getBaseDir());
}

void HgWrapper::addFiles(const KFileItemList &fileList)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    QStringList args;
    args << QLatin1String("add");
    for (const KFileItem &item : fileList) {
        args << item.localPath();
    }
    m_process.start(QLatin1String("hg"), args);
}

bool HgWrapper::renameFile(const QString &source, const QString &destination)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    QStringList args;
    args <<  source << destination;
    executeCommand(QLatin1String("rename"), args, true);

    m_process.waitForFinished();
    return (m_process.exitStatus() == QProcess::NormalExit &&
            m_process.exitCode() == 0);
}

void HgWrapper::removeFiles(const KFileItemList &fileList)
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    QStringList args;
    args << QLatin1String("remove");
    args << QLatin1String("--force");
    for (const KFileItem &item : fileList) {
        args << item.localPath();
    }
    m_process.start(QLatin1String("hg"), args);
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
    executeCommand(QLatin1String("commit"), args, true);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0 &&
            m_process.exitStatus() == QProcess::NormalExit);
}

bool HgWrapper::createBranch(const QString &name)
{
    QStringList args;
    args << name;
    executeCommand(QLatin1String("branch"), args, true);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0 &&
            m_process.exitStatus() == QProcess::NormalExit);
}

bool HgWrapper::switchBranch(const QString &name)
{
    QStringList args;
    args << QLatin1String("-c") << name;
    executeCommand(QLatin1String("update"), args, true);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0 &&
            m_process.exitStatus() == QProcess::NormalExit);
}

bool HgWrapper::createTag(const QString &name)
{
    QStringList args;
    args << name;
    executeCommand(QLatin1String("tag"), args, true);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0 &&
            m_process.exitStatus() == QProcess::NormalExit);
}

bool HgWrapper::revertAll()
{
    QStringList args;
    args << "--all";
    return executeCommandTillFinished(QLatin1String("revert"), args, true);
}


bool HgWrapper::revert(const KFileItemList &fileList)
{
    QStringList arguments;
    for (const KFileItem &item : fileList) {
        arguments << item.localPath();
    }
    return executeCommandTillFinished(QLatin1String("revert"), arguments, true);
}

bool HgWrapper::rollback(bool dryRun)
{
    QStringList args;
    if (dryRun) {
        args << QLatin1String("-n");
    }
    return executeCommandTillFinished(QLatin1String("rollback"), args, true);
}

bool HgWrapper::switchTag(const QString &name)
{
    QStringList args;
    args << QLatin1String("-c") << name;
    executeCommand(QLatin1String("update"), args, true);
    m_process.waitForFinished();
    return (m_process.exitCode() == 0 &&
            m_process.exitStatus() == QProcess::NormalExit);
}

//TODO: Make it return QStringList.
QString HgWrapper::getParentsOfHead()
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);

    QString output;
    QStringList args;
    args << QLatin1String("--template");
    args << QLatin1String("{rev}:{node|short}  ");
    executeCommand(QLatin1String("parents"), args, output);
    return output;
}

QStringList HgWrapper::getTags()
{
    QStringList result;
    executeCommand(QLatin1String("tags"));
    while (m_process.waitForReadyRead()) {
        char buffer[1048];
        while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            result << QString(buffer).split(QRegularExpression("\\s+"),
                                            Qt::SkipEmptyParts).first();
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
            // 'hg branches' command lists the branches in following format
            // <branchname>      <revision:changeset_hash> [(inactive)]
            // Extract just the branchname
            result << QString(buffer).remove(QRegularExpression("[\\s]+[\\d:a-zA-Z\\(\\)]*"));
        }
    }
    return result;
}

void HgWrapper::getItemVersions(QHash<QString, KVersionControlPlugin::ItemVersion> &result)
{
    /*int nTrimOutLeft = m_hgBaseDir.length();
    QString relativePrefix = m_currentDir.right(m_currentDir.length() -
                                                 nTrimOutLeft - 1);
    qDebug() << m_hgBaseDir << "     " << relativePrefix;*/

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
            KVersionControlPlugin::ItemVersion vs = KVersionControlPlugin::NormalVersion;
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
                // Get full path to file and insert it to result
                QUrl url = QUrl::fromLocalFile(m_hgBaseDir);
                url = url.adjusted(QUrl::StripTrailingSlash);
                url.setPath(url.path() + '/' + currentFile);
                QString filePath = url.path();
                result.insert(filePath, vs);
            }
        }
    }
}

void HgWrapper::terminateCurrentProcess()
{
    qDebug() << "terminating";
    m_process.terminate();
}

bool HgWrapper::isWorkingDirectoryClean()
{
    QStringList args;
    args << QLatin1String("--modified");
    args << QLatin1String("--added");
    args << QLatin1String("--removed");
    args << QLatin1String("--deleted");

    QString output;
    executeCommand(QLatin1String("status"), args, output);
    
    return output.trimmed().isEmpty();
}


