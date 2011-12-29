/******************************************************************************
 *   Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                     *
 *   Copyright (C) 2010 by Sebastian Doerner <sebastian@sebastian-doerner.de> *
 *   Copyright (C) 2010 by Johannes Steffen <jsteffen@st.ovgu.de>             *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program; if not, write to the                            *
 *   Free Software Foundation, Inc.,                                          *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA               *
 ******************************************************************************/

#include "fileviewgitplugin.h"
#include "checkoutdialog.h"
#include "commitdialog.h"
#include "tagdialog.h"
#include "pushdialog.h"
#include "gitwrapper.h"
#include "pulldialog.h"

#include <kaction.h>
#include <kdemacros.h>
#include <kfileitem.h>
#include <kicon.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextCodec>

#include <KPluginFactory>
#include <KPluginLoader>
K_PLUGIN_FACTORY(FileViewGitPluginFactory, registerPlugin<FileViewGitPlugin>();)
K_EXPORT_PLUGIN(FileViewGitPluginFactory("fileviewgitplugin"))

FileViewGitPlugin::FileViewGitPlugin(QObject* parent, const QList<QVariant>& args) :
    KVersionControlPlugin(parent),
    m_pendingOperation(false),
    m_addAction(0),
    m_removeAction(0),
    m_checkoutAction(0),
    m_commitAction(0),
    m_tagAction(0),
    m_pushAction(0),
    m_pullAction(0)
{
    Q_UNUSED(args);

    m_addAction = new KAction(this);
    m_addAction->setIcon(KIcon("list-add"));
    m_addAction->setText(i18nc("@action:inmenu", "<application>Git</application> Add"));
    connect(m_addAction, SIGNAL(triggered()),
            this, SLOT(addFiles()));

    m_removeAction = new KAction(this);
    m_removeAction->setIcon(KIcon("list-remove"));
    m_removeAction->setText(i18nc("@action:inmenu", "<application>Git</application> Remove"));
    connect(m_removeAction, SIGNAL(triggered()),
            this, SLOT(removeFiles()));

    m_checkoutAction = new KAction(this);
//     m_checkoutAction->setIcon(KIcon("svn_switch")); does not exist in normal kde SC
    m_checkoutAction->setText(i18nc("@action:inmenu", "<application>Git</application> Checkout..."));
    connect(m_checkoutAction, SIGNAL(triggered()),
            this, SLOT(checkout()));

    m_commitAction = new KAction(this);
    m_commitAction->setIcon(KIcon("svn-commit"));
    m_commitAction->setText(i18nc("@action:inmenu", "<application>Git</application> Commit..."));
    connect(m_commitAction, SIGNAL(triggered()),
            this, SLOT(commit()));

    m_tagAction = new KAction(this);
//     m_tagAction->setIcon(KIcon("svn-commit"));
    m_tagAction->setText(i18nc("@action:inmenu", "<application>Git</application> Create Tag..."));
    connect(m_tagAction, SIGNAL(triggered()),
            this, SLOT(createTag()));
    m_pushAction = new KAction(this);
//     m_pushAction->setIcon(KIcon("svn-commit"));
    m_pushAction->setText(i18nc("@action:inmenu", "<application>Git</application> Push..."));
    connect(m_pushAction, SIGNAL(triggered()),
            this, SLOT(push()));
    m_pullAction = new KAction(this);
    m_pullAction->setText(i18nc("@action:inmenu", "<application>Git</application> Pull..."));
    connect(m_pullAction, SIGNAL(triggered()),
            this, SLOT(pull()));

    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotOperationCompleted(int, QProcess::ExitStatus)));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(slotOperationError()));
}

FileViewGitPlugin::~FileViewGitPlugin()
{
    GitWrapper::freeInstance();
}

QString FileViewGitPlugin::fileName() const
{
    return QLatin1String(".git");
}

int FileViewGitPlugin::readUntilZeroChar(QIODevice* device, char* buffer, const int maxChars) {
    if (buffer == 0) { // discard until next \0
        char c;
        while (device->getChar(&c) && c != '\0')
            ;
        return 0;
    }
    int index = -1;
    while (++index < maxChars) {
        if (!device->getChar(&buffer[index])) {
            buffer[index] = '\0';
            return index == 0 ? 0 : index + 1;
        }
        if (buffer[index] == '\0') {  // line end or we put it there (see above)
            return index + 1;
        }
    }
    return maxChars;
}

bool FileViewGitPlugin::beginRetrieval(const QString& directory)
{
    Q_ASSERT(directory.endsWith('/'));

    GitWrapper::instance()->setWorkingDirectory(directory);

    // ----- find path below git base dir -----
    QProcess process;
    process.setWorkingDirectory(directory);
    process.start(QLatin1String("git rev-parse --show-prefix"));
    QString dirBelowBaseDir = "";
    while (process.waitForReadyRead()) {
        char buffer[512];
        while (process.readLine(buffer, sizeof(buffer)) > 0)  {
            dirBelowBaseDir = QString(buffer).trimmed(); // ends in "/" or is empty
        }
    }

    m_versionInfoHash.clear();
    
    // ----- find files with special status -----
    process.start(QLatin1String("git status --porcelain -z --ignored"));
    while (process.waitForReadyRead()) {
        char buffer[1024];
        while (readUntilZeroChar(&process, buffer, sizeof(buffer)) > 0 ) {
            QString line = QTextCodec::codecForLocale()->toUnicode(buffer);
            // ----- recognize file status -----
            char X = line[0].toAscii();  // X and Y from the table in `man git-status`
            char Y = line[1].toAscii();
            const QString fileName= line.mid(3);
            VersionState state = NormalVersion;
            switch (X) {
                case '!': // handle ignored as unversioned
                case '?':
                    state = UnversionedVersion;
                    break;
                case 'C': // handle copied as added version
                case 'A':
                    state = AddedVersion;
                    break;
                case 'D':
                    state = RemovedVersion;
                    break;
                case 'M':
                    state = LocallyModifiedVersion;
                    break;
                case 'R':
                    state = LocallyModifiedVersion;
                    // Renames list the old file name directly afterwards, separated by \0.
                    readUntilZeroChar(&process, 0, 0); // discard old file name
                    break;
            }
            // overwrite status depending on the working tree
            switch (Y) {
                case 'D': // handle "deleted in working tree" as "modified in working tree"
                case 'M':
                    state = LocallyModifiedUnstagedVersion;
                    break;
            }
            // overwrite status in case of conflicts (lower part of the table in `man git-status`)
            if (X == 'U' ||
                Y == 'U' ||
                (X == 'A' && Y == 'A') ||
                (X == 'D' && Y == 'D')) {
                state = ConflictingVersion;
            }
            
            // ----- decide what to record about that file -----
            if (state == NormalVersion || !fileName.startsWith(dirBelowBaseDir)) {
                continue;
            }
            /// File name relative to the current working directory.
            const QString relativeFileName = fileName.mid(dirBelowBaseDir.length());
            //if file is part of a sub-directory, record the directory
            if (relativeFileName.contains('/')) {
                if (state == AddedVersion || state == RemovedVersion) {
                    state = LocallyModifiedVersion;
                }
                const QString absoluteDirName = directory + relativeFileName.left(relativeFileName.indexOf('/'));
                if (m_versionInfoHash.contains(absoluteDirName)) {
                    VersionState oldState = m_versionInfoHash.value(absoluteDirName);
                    //only keep the most important state for a directory
                    if (oldState == ConflictingVersion)
                        continue;
                    if (oldState == LocallyModifiedUnstagedVersion && state != ConflictingVersion)
                        continue;
                    if (oldState == LocallyModifiedVersion &&
                        state != LocallyModifiedUnstagedVersion && state != ConflictingVersion)
                        continue;
                    m_versionInfoHash.insert(absoluteDirName, state);
                } else {
                    m_versionInfoHash.insert(absoluteDirName, state);
                }
            } else { //normal file, no directory
                    m_versionInfoHash.insert(directory + relativeFileName, state);
            }
        }
    }
    return true;
}

void FileViewGitPlugin::endRetrieval()
{
}

KVersionControlPlugin::VersionState FileViewGitPlugin::versionState(const KFileItem& item)
{
    const QString itemUrl = item.localPath();
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
    } else {
        // files that are not in our map are normal, tracked files by definition
        return NormalVersion;
    }
}

QList<QAction*> FileViewGitPlugin::contextMenuActions(const KFileItemList& items)
{
    Q_ASSERT(!items.isEmpty());

    if (!m_pendingOperation){
        m_contextDir.clear();
        m_contextItems.clear();
        foreach(const KFileItem& item, items){
            m_contextItems.append(item);
        }

        //see which actions should be enabled
        int versionedCount = 0;
        int addableCount = 0;
        foreach(const KFileItem& item, items){
            const VersionState state = versionState(item);
            if (state != UnversionedVersion && state != RemovedVersion){
                ++versionedCount;
            }
            if (state == UnversionedVersion || state == LocallyModifiedUnstagedVersion) {
                ++addableCount;
            }
        }

        m_addAction->setEnabled(addableCount == items.count());
        m_removeAction->setEnabled(versionedCount == items.count());
    }
    else{
        m_addAction->setEnabled(false);
        m_removeAction->setEnabled(false);
    }

    QList<QAction*> actions;
    actions.append(m_addAction);
    actions.append(m_removeAction);
    return actions;
}

QList<QAction*> FileViewGitPlugin::contextMenuActions(const QString& directory)
{
    QList<QAction*> actions;
    if (!m_pendingOperation){
        m_contextDir = directory;
    }
    m_checkoutAction->setEnabled(!m_pendingOperation);
    actions.append(m_checkoutAction);

    bool canCommit = false;
    QHash<QString, VersionState>::const_iterator it = m_versionInfoHash.constBegin();
    while (it != m_versionInfoHash.constEnd()) {
        const VersionState state = it.value();
        if (state == LocallyModifiedVersion || state == AddedVersion || state == RemovedVersion) {
            canCommit = true;
        }
        if (state == ConflictingVersion) {
            canCommit = false;
            break;
        }
        ++it;
    }

    m_commitAction->setEnabled(!m_pendingOperation && canCommit);
    actions.append(m_commitAction);

    m_tagAction->setEnabled(!m_pendingOperation);
    actions.append(m_tagAction);
    m_pushAction->setEnabled(!m_pendingOperation);
    actions.append(m_pushAction);
    m_pullAction->setEnabled(!m_pendingOperation);
    actions.append(m_pullAction);

    return actions;
}

void FileViewGitPlugin::addFiles()
{
    execGitCommand(QLatin1String("add"), QStringList(),
                   i18nc("@info:status", "Adding files to <application>Git</application> repository..."),
                   i18nc("@info:status", "Adding files to <application>Git</application> repository failed."),
                   i18nc("@info:status", "Added files to <application>Git</application> repository."));
}

void FileViewGitPlugin::removeFiles()
{
    QStringList arguments;
    arguments << "-r"; //recurse through directories
    arguments << "--force"; //also remove files that have not been committed yet
    execGitCommand(QLatin1String("rm"), arguments,
                   i18nc("@info:status", "Removing files from <application>Git</application> repository..."),
                   i18nc("@info:status", "Removing files from <application>Git</application> repository failed."),
                   i18nc("@info:status", "Removed files from <application>Git</application> repository."));

}

void FileViewGitPlugin::checkout()
{
    CheckoutDialog dialog;
    if (dialog.exec() == QDialog::Accepted){
        QProcess process;
        process.setWorkingDirectory(m_contextDir);
        QStringList arguments;
        arguments << "checkout";
        if (dialog.force()) {
            arguments << "-f";
        }
        const QString newBranchName = dialog.newBranchName();
        if (!newBranchName.isEmpty()) {
            arguments << "-b";
            arguments << newBranchName;
        }
        const QString checkoutIdentifier = dialog.checkoutIdentifier();
        if (!checkoutIdentifier.isEmpty()) {
            arguments << checkoutIdentifier;
        }
        //to appear in messages
        const QString currentBranchName = newBranchName.isEmpty() ? checkoutIdentifier : newBranchName;
        process.start(QLatin1String("git"), arguments);
        process.setReadChannel(QProcess::StandardError); //git writes info messages to stderr as well
        QString completedMessage;
        while (process.waitForReadyRead()) {
            char buffer[512];
            while (process.readLine(buffer, sizeof(buffer)) > 0){
                const QString currentLine(buffer);
                if (currentLine.startsWith(QLatin1String("Switched to branch"))) {
                    completedMessage = i18nc("@info:status", "Switched to branch '%1'", currentBranchName);
                }
                if (currentLine.startsWith(QLatin1String("HEAD is now at"))) {
                    const QString headIdentifier = currentLine.
                        mid(QString("HEAD is now at ").length()).trimmed();
                    completedMessage = i18nc("@info:status Git HEAD pointer, parameter includes "
                    "short SHA-1 & commit message ", "HEAD is now at %1", headIdentifier);
                }
                //special output for checkout -b
                if (currentLine.startsWith(QLatin1String("Switched to a new branch"))) {
                    completedMessage = i18nc("@info:status", "Switched to a new branch '%1'", currentBranchName);
                }
            }
        }
        if (process.exitCode() == 0 && process.exitStatus() == QProcess::NormalExit) {
            if (!completedMessage.isEmpty()) {
                emit operationCompletedMessage(completedMessage);
                emit versionStatesChanged();
            }
        }
        else {
            emit errorMessage(i18nc("@info:status", "<application>Git</application> Checkout failed."
            " Maybe your working directory is dirty."));
        }
    }
}

void FileViewGitPlugin::commit()
{
    CommitDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        KTemporaryFile tmpCommitMessageFile;
        tmpCommitMessageFile.open();
        tmpCommitMessageFile.write(dialog.commitMessage());
        tmpCommitMessageFile.close();
        QProcess process;
        process.setWorkingDirectory(m_contextDir);
        process.start(QString("git commit") + (dialog.amend() ? " --amend" : "")+ " -F " + tmpCommitMessageFile.fileName());
        QString completedMessage;
        while (process.waitForReadyRead()){
            char buffer[512];
            while (process.readLine(buffer, sizeof(buffer)) > 0) {
                if (strlen(buffer) > 0 && buffer[0] == '[') {
                    completedMessage = QTextCodec::codecForLocale()->toUnicode(buffer).trimmed();
                    break;
                }
            }
        }
        if (!completedMessage.isEmpty()) {
            emit operationCompletedMessage(completedMessage);
            emit versionStatesChanged();
        }
    }
}

void FileViewGitPlugin::createTag()
{
   TagDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        KTemporaryFile tempTagMessageFile;
        tempTagMessageFile.open();
        tempTagMessageFile.write(dialog.tagMessage());
        tempTagMessageFile.close();
        QProcess process;
        process.setWorkingDirectory(m_contextDir);
        process.setReadChannel(QProcess::StandardError);
        process.start(QString("git tag -a -F %1 %2 %3").arg(tempTagMessageFile.fileName()).
            arg(dialog.tagName()).arg(dialog.baseBranch()));
        QString completedMessage;
        bool gotTagAlreadyExistsMessage = false;
        while (process.waitForReadyRead()) {
            char buffer[512];
            while (process.readLine(buffer, sizeof(buffer)) > 0) {
                QString line(buffer);
                if (line.contains("already exists")) {
                    gotTagAlreadyExistsMessage = true;
                }
            }
        }
        if (process.exitCode() == 0 && process.exitStatus() == QProcess::NormalExit) {
            completedMessage = i18nc("@info:status","Successfully created tag '%1'", dialog.tagName());
            emit operationCompletedMessage(completedMessage);
        } else {
            //I don't know any other error, but in case one occurs, the user doesn't get FALSE error messages
            emit errorMessage(gotTagAlreadyExistsMessage ?
                i18nc("@info:status", "<application>Git</application> tag creation failed."
                                      " A tag with the name '%1' already exists.", dialog.tagName()) :
                i18nc("@info:status", "<application>Git</application> tag creation failed.")
            );
        }
    }
}

void FileViewGitPlugin::push()
{
    PushDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        m_process.setWorkingDirectory(m_contextDir);

        m_errorMsg = i18nc("@info:status", "Pushing branch %1 to %2:%3 failed.",
                        dialog.localBranch(), dialog.destination(), dialog.remoteBranch());
        m_operationCompletedMsg = i18nc("@info:status", "Pushed branch %1 to %2:%3.",
                        dialog.localBranch(), dialog.destination(), dialog.remoteBranch());
        emit infoMessage(i18nc("@info:status", "Pushing branch %1 to %2:%3...",
                 dialog.localBranch(), dialog.destination(), dialog.remoteBranch()));

        m_command = "push";
        m_pendingOperation = true;
        m_process.start(QString("git push%4 %1 %2:%3").arg(dialog.destination()).
            arg(dialog.localBranch()).arg(dialog.remoteBranch()).
            arg(dialog.force() ? QLatin1String(" --force") : QLatin1String("")));
    }
}

void FileViewGitPlugin::pull()
{
    PullDialog dialog;
    if (dialog.exec()  == QDialog::Accepted) {
        m_process.setWorkingDirectory(m_contextDir);

        m_errorMsg = i18nc("@info:status", "Pulling branch %1 from %2 failed.",
                        dialog.remoteBranch(), dialog.source());
        m_operationCompletedMsg = i18nc("@info:status", "Pulled branch %1 from %2 successfully.",
                        dialog.remoteBranch(), dialog.source());
        emit infoMessage(i18nc("@info:status", "Pulling branch %1 from %2...", dialog.remoteBranch(),
                    dialog.source()));

        m_command = "pull";
        m_pendingOperation = true;
        m_process.start(QString("git pull %1 %2").arg(dialog.source()).arg(dialog.remoteBranch()));
    }
}

void FileViewGitPlugin::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_pendingOperation = false;

    QString message;
    if (m_command == QLatin1String("push")) { //output parsing for push
        message = parsePushOutput();
        m_command = "";
    }
    if (m_command == QLatin1String("pull")) {
        message = parsePullOutput();
        m_command = "";
    }

    if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
        emit errorMessage(message.isNull() ? m_errorMsg : message);
    } else if (m_contextItems.isEmpty()) {
        emit operationCompletedMessage(message.isNull() ? m_operationCompletedMsg : message);
        emit versionStatesChanged();
    } else {
        startGitCommandProcess();
    }
}

void FileViewGitPlugin::slotOperationError()
{
    // don't do any operation on other items anymore
    m_contextItems.clear();
    m_pendingOperation = false;

    emit errorMessage(m_errorMsg);
}

QString FileViewGitPlugin::parsePushOutput()
{
    m_process.setReadChannel(QProcess::StandardError);
    QString message;
    char buffer[256];
    while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
        const QString line(buffer);
        if (line.contains("->") || (line.contains("fatal") && message.isNull())) {
            message = line.trimmed();
        }
        if (line.contains("Everything up-to-date") && message.isNull()) {
            message = i18nc("@info:status", "Branch is already up-to-date.");
        }
    }
    return message;
}

QString FileViewGitPlugin::parsePullOutput()
{
    char buffer[256];
    while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
        const QString line(buffer);
        if (line.contains("Already up-to-date")) {
            return i18nc("@info:status", "Branch is already up-to-date.");
        }
        if (line.contains("CONFLICT")) {
            emit versionStatesChanged();
            return i18nc("@info:status", "Merge conflicts occured. Fix them and commit the result.");
        }
    }
    return QString();
}

void FileViewGitPlugin::execGitCommand(const QString& gitCommand,
                                       const QStringList& arguments,
                                       const QString& infoMsg,
                                       const QString& errorMsg,
                                       const QString& operationCompletedMsg)
{
    emit infoMessage(infoMsg);

    m_command = gitCommand;
    m_arguments = arguments;
    m_errorMsg = errorMsg;
    m_operationCompletedMsg = operationCompletedMsg;

    startGitCommandProcess();
}


void FileViewGitPlugin::startGitCommandProcess()
{
    Q_ASSERT(!m_contextItems.isEmpty());
    Q_ASSERT(m_process.state() == QProcess::NotRunning);
    m_pendingOperation = true;

    const KFileItem item = m_contextItems.takeLast();
    m_process.setWorkingDirectory(item.url().directory());
    QStringList arguments;
    arguments << m_command;
    arguments << m_arguments;
    //force explicitly selected files but no files in selected directories
    if (m_command == "add" && !item.isDir()){
        arguments<< QLatin1String("-f");
    }
    arguments << item.url().fileName();
    m_process.start(QLatin1String("git"), arguments);
    // the remaining items of m_contextItems will be executed
    // after the process has finished (see slotOperationFinished())
}

#include "fileviewgitplugin.moc"
