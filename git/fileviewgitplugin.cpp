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

#include <KLocalizedString>
#include <KUrl>
#include <KRun>
#include <KShell>
#include <KPluginFactory>

#include <QTemporaryFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QDir>
#include <QTextBrowser>

K_PLUGIN_FACTORY(FileViewGitPluginFactory, registerPlugin<FileViewGitPlugin>();)

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

    m_revertAction = new QAction(this);
    m_revertAction->setIcon(QIcon::fromTheme("document-revert"));
    m_revertAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Revert"));
    connect(m_revertAction, SIGNAL(triggered()),
            this, SLOT(revertFiles()));

    m_addAction = new QAction(this);
    m_addAction->setIcon(QIcon::fromTheme("list-add"));
    m_addAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Add"));
    connect(m_addAction, SIGNAL(triggered()),
            this, SLOT(addFiles()));

    m_showLocalChangesAction = new QAction(this);
    m_showLocalChangesAction->setIcon(QIcon::fromTheme("view-split-left-right"));
    m_showLocalChangesAction->setText(xi18nd("@item:inmenu", "Show Local <application>Git</application> Changes"));
    connect(m_showLocalChangesAction, SIGNAL(triggered()),
            this, SLOT(showLocalChanges()));

    m_removeAction = new QAction(this);
    m_removeAction->setIcon(QIcon::fromTheme("list-remove"));
    m_removeAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Remove"));
    connect(m_removeAction, SIGNAL(triggered()),
            this, SLOT(removeFiles()));

    m_checkoutAction = new QAction(this);
//     m_checkoutAction->setIcon(QIcon::fromTheme("svn_switch")); does not exist in normal kde SC
    m_checkoutAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Checkout..."));
    connect(m_checkoutAction, SIGNAL(triggered()),
            this, SLOT(checkout()));

    m_commitAction = new QAction(this);
    m_commitAction->setIcon(QIcon::fromTheme("svn-commit"));
    m_commitAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Commit..."));
    connect(m_commitAction, SIGNAL(triggered()),
            this, SLOT(commit()));

    m_tagAction = new QAction(this);
//     m_tagAction->setIcon(QIcon::fromTheme("svn-commit"));
    m_tagAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Create Tag..."));
    connect(m_tagAction, SIGNAL(triggered()),
            this, SLOT(createTag()));
    m_pushAction = new QAction(this);
//     m_pushAction->setIcon(QIcon::fromTheme("svn-commit"));
    m_pushAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Push..."));
    connect(m_pushAction, SIGNAL(triggered()),
            this, SLOT(push()));
    m_pullAction = new QAction(this);
    m_pullAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Pull..."));
    connect(m_pullAction, SIGNAL(triggered()),
            this, SLOT(pull()));
    m_mergeAction = new QAction(this);
    m_mergeAction->setIcon(QIcon::fromTheme("merge"));
    m_mergeAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Merge..."));
    connect(m_mergeAction, &QAction::triggered, this, &FileViewGitPlugin::merge);

    m_logAction = new QAction(this);
    m_logAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Log..."));
    connect(m_logAction, &QAction::triggered, this, &FileViewGitPlugin::log);

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
    m_currentDir = directory;

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
    process.start(QLatin1String("git status --porcelain -z -u --ignored"));
    while (process.waitForReadyRead()) {
        char buffer[1024];
        while (readUntilZeroChar(&process, buffer, sizeof(buffer)) > 0 ) {
            QString line = QTextCodec::codecForLocale()->toUnicode(buffer);
            // ----- recognize file status -----
            char X = line[0].toAscii();  // X and Y from the table in `man git-status`
            char Y = line[1].toAscii();
            const QString fileName= line.mid(3);
            ItemVersion state = NormalVersion;
            switch (X) {
                case '!':
                    state = IgnoredVersion;
                    break;
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
                if (state == IgnoredVersion)
                    continue;
                if (state == AddedVersion || state == RemovedVersion) {
                    state = LocallyModifiedVersion;
                }
                const QString absoluteDirName = directory + relativeFileName.left(relativeFileName.indexOf('/'));
                if (m_versionInfoHash.contains(absoluteDirName)) {
                    ItemVersion oldState = m_versionInfoHash.value(absoluteDirName);
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

KVersionControlPlugin::ItemVersion FileViewGitPlugin::itemVersion(const KFileItem& item) const
{
    const QString itemUrl = item.localPath();
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
    } else {
        // files that are not in our map are normal, tracked files by definition
        return NormalVersion;
    }
}

QList<QAction*> FileViewGitPlugin::actions(const KFileItemList &items) const
{
    if (items.count() == 1 && items.first().isDir()) {
        QString directory = items.first().localPath();
        if (!directory.endsWith(QLatin1Char('/'))) {
            directory += QLatin1Char('/');
        }

        if (directory == m_currentDir) {
            return contextMenuDirectoryActions(directory);
        } else {
            return contextMenuFilesActions(items);
        }
    } else {
        return contextMenuFilesActions(items);
    }
}

QList<QAction*> FileViewGitPlugin::contextMenuFilesActions(const KFileItemList& items) const
{
    Q_ASSERT(!items.isEmpty());

    if (!m_pendingOperation){
        m_contextDir = QFileInfo(items.first().localPath()).canonicalPath();
        m_contextItems.clear();
        foreach(const KFileItem& item, items){
            m_contextItems.append(item);
        }

        //see which actions should be enabled
        int versionedCount = 0;
        int addableCount = 0;
        int revertCount = 0;
        foreach(const KFileItem& item, items){
            const ItemVersion state = itemVersion(item);
            if (state != UnversionedVersion && state != RemovedVersion &&
                state != IgnoredVersion) {
                ++versionedCount;
            }
            if (state == UnversionedVersion || state == LocallyModifiedUnstagedVersion ||
                state == IgnoredVersion) {
                ++addableCount;
            }
            if (state == LocallyModifiedVersion || state == LocallyModifiedUnstagedVersion ||
                state == ConflictingVersion) {
                ++revertCount;
            }
        }

        m_addAction->setEnabled(addableCount == items.count());
        m_revertAction->setEnabled(revertCount == items.count());
        m_removeAction->setEnabled(versionedCount == items.count());
    }
    else{
        m_addAction->setEnabled(false);
        m_revertAction->setEnabled(false);
        m_removeAction->setEnabled(false);
    }

    QList<QAction*> actions;
    actions.append(m_addAction);
    actions.append(m_removeAction);
    actions.append(m_revertAction);
    return actions;
}

QList<QAction*> FileViewGitPlugin::contextMenuDirectoryActions(const QString& directory) const
{
    QList<QAction*> actions;
    if (!m_pendingOperation){
        m_contextDir = directory;
    }
    m_checkoutAction->setEnabled(!m_pendingOperation);
    actions.append(m_checkoutAction);

    bool canCommit = false;
    bool showChanges = false;
    bool shouldMerge = false;
    QHash<QString, ItemVersion>::const_iterator it = m_versionInfoHash.constBegin();
    while (it != m_versionInfoHash.constEnd()) {
        const ItemVersion state = it.value();
        if (state == LocallyModifiedVersion || state == AddedVersion || state == RemovedVersion) {
            canCommit = true;
        }
        if (state == LocallyModifiedUnstagedVersion || state == LocallyModifiedVersion) {
            showChanges = true;
        }
        if (state == ConflictingVersion) {
            canCommit = false;
            showChanges = true;
            shouldMerge = true;
            break;
        }
        ++it;
    }

    m_logAction->setEnabled(!m_pendingOperation);
    actions.append(m_logAction);

    m_showLocalChangesAction->setEnabled(!m_pendingOperation && showChanges);
    actions.append(m_showLocalChangesAction);

    if (!shouldMerge) {
        m_commitAction->setEnabled(!m_pendingOperation && canCommit);
        actions.append(m_commitAction);
    } else {
        m_mergeAction->setEnabled(!m_pendingOperation);
        actions.append(m_mergeAction);
    }

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
                   xi18nd("@info:status", "Adding files to <application>Git</application> repository..."),
                   xi18nd("@info:status", "Adding files to <application>Git</application> repository failed."),
                   xi18nd("@info:status", "Added files to <application>Git</application> repository."));
}

void FileViewGitPlugin::removeFiles()
{
    QStringList arguments;
    arguments << "-r"; //recurse through directories
    arguments << "--force"; //also remove files that have not been committed yet
    execGitCommand(QLatin1String("rm"), arguments,
                   xi18nd("@info:status", "Removing files from <application>Git</application> repository..."),
                   xi18nd("@info:status", "Removing files from <application>Git</application> repository failed."),
                   xi18nd("@info:status", "Removed files from <application>Git</application> repository."));
}

void FileViewGitPlugin::revertFiles()
{
    execGitCommand(QLatin1String("checkout"), { "--" },
                   xi18nd("@info:status", "Reverting files from <application>Git</application> repository..."),
                   xi18nd("@info:status", "Reverting files from <application>Git</application> repository failed."),
                   xi18nd("@info:status", "Reverted files from <application>Git</application> repository."));
}

void FileViewGitPlugin::showLocalChanges()
{
    Q_ASSERT(!m_contextDir.isEmpty());

    KRun::runCommand(QLatin1String("git difftool --dir-diff ."), nullptr, m_contextDir);
}

void FileViewGitPlugin::showDiff(const QUrl &link)
{
    if (link.scheme() != QLatin1String("rev")) {
        return;
    }
    KRun::runCommand(QStringLiteral("git difftool --dir-diff %1^ %1").arg(link.path()), nullptr, m_contextDir);
}

void FileViewGitPlugin::log()
{
    QProcess process;
    process.setWorkingDirectory(m_contextDir);
    process.start(
        QLatin1String("git"),
        QStringList {
            QStringLiteral("log"),
            QStringLiteral("--relative=."),
            QStringLiteral("--date=format:%d-%m-%Y"),
            QStringLiteral("-n 100"),
            QStringLiteral("--pretty=format:<tr> <td><a href=\"rev:%h\">%h</a></td> <td>%ad</td> <td>%s</td> <td>%an</td> </tr>")
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        emit errorMessage(xi18nd("@info:status", "<application>Git</application> Log failed."));
        return;
    }

    const QString gitOutput = process.readAllStandardOutput();

    QPalette palette;
    const QString styleSheet = QStringLiteral(
        "body { background: %1; color: %2; }" \
        "table.logtable td { padding: 9px 8px 9px; }" \
        "a { color: %3; }" \
        "a:visited { color: %4; } "
    ).arg(palette.background().color().name(),
          palette.text().color().name(),
          palette.link().color().name(),
          palette.linkVisited().color().name());

    auto view = new QTextBrowser();
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->setWindowTitle(xi18nd("@title:window", "<application>Git</application> Log"));
    view->setOpenLinks(false);
    view->setOpenExternalLinks(false);
    connect(view, &QTextBrowser::anchorClicked, this, &FileViewGitPlugin::showDiff);
    view->setHtml(QStringLiteral(
        "<html>" \
        "<style> %1 </style>" \
        "<table class=\"logtable\">" \
        "<tr bgcolor=\"%2\">" \
        "<td> %3 </td> <td> %4 </td> <td> %5 </p> </td> <td> %6 </td>" \
        "</tr>" \
        "%7" \
        "</table>" \
        "</html>"
    ).arg(styleSheet,
          palette.highlight().color().name(),
          i18nc("Git commit hash", "Commit"),
          i18nc("Git commit date", "Date"),
          i18nc("Git commit message", "Message"),
          i18nc("Git commit author", "Author"),
          gitOutput));

    view->resize(QSize(720, 560));
    view->show();
}

void FileViewGitPlugin::merge()
{
    Q_ASSERT(!m_contextDir.isEmpty());

    KRun::runCommand(QStringLiteral("git mergetool"), nullptr, m_contextDir);
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
                    completedMessage = xi18nd("@info:status", "Switched to branch '%1'", currentBranchName);
                }
                if (currentLine.startsWith(QLatin1String("HEAD is now at"))) {
                    const QString headIdentifier = currentLine.
                        mid(QString("HEAD is now at ").length()).trimmed();
                    completedMessage = xi18nd("@info:status Git HEAD pointer, parameter includes "
                    "short SHA-1 & commit message ", "HEAD is now at %1", headIdentifier);
                }
                //special output for checkout -b
                if (currentLine.startsWith(QLatin1String("Switched to a new branch"))) {
                    completedMessage = xi18nd("@info:status", "Switched to a new branch '%1'", currentBranchName);
                }
            }
        }
        if (process.exitCode() == 0 && process.exitStatus() == QProcess::NormalExit) {
            if (!completedMessage.isEmpty()) {
                emit operationCompletedMessage(completedMessage);
                emit itemVersionsChanged();
            }
        }
        else {
            emit errorMessage(xi18nd("@info:status", "<application>Git</application> Checkout failed."
            " Maybe your working directory is dirty."));
        }
    }
}

void FileViewGitPlugin::commit()
{
    CommitDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        QTemporaryFile tmpCommitMessageFile;
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
            emit itemVersionsChanged();
        }
    }
}

void FileViewGitPlugin::createTag()
{
   TagDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        QTemporaryFile tempTagMessageFile;
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
            completedMessage = xi18nd("@info:status","Successfully created tag '%1'", dialog.tagName());
            emit operationCompletedMessage(completedMessage);
        } else {
            //I don't know any other error, but in case one occurs, the user doesn't get FALSE error messages
            emit errorMessage(gotTagAlreadyExistsMessage ?
                xi18nd("@info:status", "<application>Git</application> tag creation failed."
                                      " A tag with the name '%1' already exists.", dialog.tagName()) :
                xi18nd("@info:status", "<application>Git</application> tag creation failed.")
            );
        }
    }
}

void FileViewGitPlugin::push()
{
    PushDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        m_process.setWorkingDirectory(m_contextDir);

        m_errorMsg = xi18nd("@info:status", "Pushing branch %1 to %2:%3 failed.",
                        dialog.localBranch(), dialog.destination(), dialog.remoteBranch());
        m_operationCompletedMsg = xi18nd("@info:status", "Pushed branch %1 to %2:%3.",
                        dialog.localBranch(), dialog.destination(), dialog.remoteBranch());
        emit infoMessage(xi18nd("@info:status", "Pushing branch %1 to %2:%3...",
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

        m_errorMsg = xi18nd("@info:status", "Pulling branch %1 from %2 failed.",
                        dialog.remoteBranch(), dialog.source());
        m_operationCompletedMsg = xi18nd("@info:status", "Pulled branch %1 from %2 successfully.",
                        dialog.remoteBranch(), dialog.source());
        emit infoMessage(xi18nd("@info:status", "Pulling branch %1 from %2...", dialog.remoteBranch(),
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
        emit itemVersionsChanged();
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
            message = xi18nd("@info:status", "Branch is already up-to-date.");
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
            return xi18nd("@info:status", "Branch is already up-to-date.");
        }
        if (line.contains("CONFLICT")) {
            emit itemVersionsChanged();
            return xi18nd("@info:status", "Merge conflicts occurred. Fix them and commit the result.");
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
    m_process.setWorkingDirectory(m_contextDir);
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
