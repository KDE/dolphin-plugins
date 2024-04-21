/*
    SPDX-FileCopyrightText: 2010 Peter Penz <peter.penz@gmx.at>
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>
    SPDX-FileCopyrightText: 2010 Johannes Steffen <jsteffen@st.ovgu.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fileviewgitplugin.h"
#include "checkoutdialog.h"
#include "clonedialog.h"
#include "commitdialog.h"
#include "gitwrapper.h"
#include "pulldialog.h"
#include "pushdialog.h"
#include "tagdialog.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <KShell>

#include <QDialogButtonBox>
#include <QDir>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextBrowser>
#include <QVBoxLayout>

K_PLUGIN_CLASS_WITH_JSON(FileViewGitPlugin, "fileviewgitplugin.json")

FileViewGitPlugin::FileViewGitPlugin(QObject *parent, const QList<QVariant> &args)
    : KVersionControlPlugin(parent)
    , m_pendingOperation(false)
    , m_addAction(nullptr)
    , m_removeAction(nullptr)
    , m_checkoutAction(nullptr)
    , m_commitAction(nullptr)
    , m_tagAction(nullptr)
    , m_pushAction(nullptr)
    , m_pullAction(nullptr)
    , m_cloneAction(nullptr)
{
    Q_UNUSED(args);

    m_parentWidget = qobject_cast<QWidget *>(parent);

    m_revertAction = new QAction(this);
    m_revertAction->setIcon(QIcon::fromTheme(QStringLiteral("document-revert")));
    m_revertAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Revert"));
    connect(m_revertAction, &QAction::triggered, this, &FileViewGitPlugin::revertFiles);

    m_addAction = new QAction(this);
    m_addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    m_addAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Add"));
    connect(m_addAction, &QAction::triggered, this, &FileViewGitPlugin::addFiles);

    m_showLocalChangesAction = new QAction(this);
    m_showLocalChangesAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-diff")));
    m_showLocalChangesAction->setText(xi18nd("@item:inmenu", "Show Local <application>Git</application> Changes"));
    connect(m_showLocalChangesAction, &QAction::triggered, this, &FileViewGitPlugin::showLocalChanges);

    m_restoreStagedAction = new QAction(this);
    m_restoreStagedAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-undo-symbolic")));
    m_restoreStagedAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Restore staged"));
    connect(m_restoreStagedAction, &QAction::triggered, this, &FileViewGitPlugin::restoreStaged);

    m_removeAction = new QAction(this);
    m_removeAction->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    m_removeAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Remove"));
    connect(m_removeAction, &QAction::triggered, this, &FileViewGitPlugin::removeFiles);

    m_checkoutAction = new QAction(this);
    m_checkoutAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-branch")));
    m_checkoutAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Checkout..."));
    connect(m_checkoutAction, &QAction::triggered, this, &FileViewGitPlugin::checkout);

    m_commitAction = new QAction(this);
    m_commitAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-commit")));
    m_commitAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Commit..."));
    connect(m_commitAction, &QAction::triggered, this, &FileViewGitPlugin::commit);

    m_tagAction = new QAction(this);
    //     m_tagAction->setIcon(QIcon::fromTheme(QStringLiteral("svn-commit")));
    m_tagAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Create Tag..."));
    connect(m_tagAction, &QAction::triggered, this, &FileViewGitPlugin::createTag);
    m_pushAction = new QAction(this);
    m_pushAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-push")));
    m_pushAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Push..."));
    connect(m_pushAction, &QAction::triggered, this, &FileViewGitPlugin::push);
    m_pullAction = new QAction(this);
    m_pullAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-pull")));
    m_pullAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Pull..."));
    connect(m_pullAction, &QAction::triggered, this, &FileViewGitPlugin::pull);
    m_mergeAction = new QAction(this);
    m_mergeAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-merge")));
    m_mergeAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Merge..."));
    connect(m_mergeAction, &QAction::triggered, this, &FileViewGitPlugin::merge);

    m_cloneAction = new QAction(this);
    m_cloneAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Clone..."));
    connect(m_cloneAction, &QAction::triggered, this, &FileViewGitPlugin::clone);

    m_logAction = new QAction(this);
    m_logAction->setText(xi18nd("@action:inmenu", "<application>Git</application> Log..."));
    connect(m_logAction, &QAction::triggered, this, &FileViewGitPlugin::log);

    connect(&m_process, &QProcess::finished, this, &FileViewGitPlugin::slotOperationCompleted);
    connect(&m_process, &QProcess::errorOccurred, this, &FileViewGitPlugin::slotOperationError);
}

FileViewGitPlugin::~FileViewGitPlugin()
{
    GitWrapper::freeInstance();
}

QString FileViewGitPlugin::fileName() const
{
    return QLatin1String(".git");
}

QString FileViewGitPlugin::localRepositoryRoot(const QString &directory) const
{
    QProcess process;
    process.setWorkingDirectory(directory);
    process.start(QStringLiteral("git"), {QStringLiteral("rev-parse"), QStringLiteral("--show-toplevel")});
    if (process.waitForReadyRead(100) && process.exitCode() == 0) {
        return QString::fromUtf8(process.readAll().chopped(1));
    }
    return QString();
}

int FileViewGitPlugin::readUntilZeroChar(QIODevice *device, char *buffer, const int maxChars)
{
    if (buffer == nullptr) { // discard until next \0
        char c;
        while (device->getChar(&c) && c != '\0')
            ;
        return 0;
    }
    int index = -1;
    while (++index < maxChars) {
        if (!device->getChar(&buffer[index])) {
            if (device->waitForReadyRead(30000)) { // 30 seconds to be consistent with QProcess::waitForReadyRead default
                --index;
                continue;
            } else {
                buffer[index] = '\0';
                return index <= 0 ? 0 : index + 1;
            }
        }
        if (buffer[index] == '\0') { // line end or we put it there (see above)
            return index + 1;
        }
    }
    return maxChars;
}

bool FileViewGitPlugin::beginRetrieval(const QString &directory)
{
    Q_ASSERT(directory.endsWith(QLatin1Char('/')));

    GitWrapper::instance()->setWorkingDirectory(directory);
    m_currentDir = directory;

    // ----- find path below git base dir -----
    QProcess process;
    process.setWorkingDirectory(directory);
    process.start(QStringLiteral("git"), {QStringLiteral("rev-parse"), QStringLiteral("--show-prefix")});
    QString dirBelowBaseDir;
    while (process.waitForReadyRead()) {
        char buffer[512];
        while (process.readLine(buffer, sizeof(buffer)) > 0) {
            dirBelowBaseDir = QString::fromLocal8Bit(buffer).trimmed(); // ends in "/" or is empty
        }
    }

    m_versionInfoHash.clear();

    // ----- find files with special status -----
    process.start(QStringLiteral("git"),
                  {QStringLiteral("--no-optional-locks"),
                   QStringLiteral("status"),
                   QStringLiteral("--porcelain"),
                   QStringLiteral("-z"),
                   QStringLiteral("-u"),
                   QStringLiteral("--ignored")});
    while (process.waitForReadyRead()) {
        char buffer[1024];
        while (readUntilZeroChar(&process, buffer, sizeof(buffer)) > 0) {
            QString line = QString::fromLocal8Bit(buffer);
            // ----- recognize file status -----
            char X = line[0].toLatin1(); // X and Y from the table in `man git-status`
            char Y = line[1].toLatin1();
            const QString fileName = line.mid(3);
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
                readUntilZeroChar(&process, nullptr, 0); // discard old file name
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
            if (X == 'U' || Y == 'U' || (X == 'A' && Y == 'A') || (X == 'D' && Y == 'D')) {
                state = ConflictingVersion;
            }

            // ----- decide what to record about that file -----
            if (state == NormalVersion || !fileName.startsWith(dirBelowBaseDir)) {
                continue;
            }
            /// File name relative to the current working directory.
            const QString relativeFileName = fileName.mid(dirBelowBaseDir.length());
            // if file is part of a sub-directory, record the directory
            if (relativeFileName.contains(QLatin1Char('/'))) {
                if (state == IgnoredVersion)
                    continue;
                if (state == AddedVersion || state == RemovedVersion) {
                    state = LocallyModifiedVersion;
                }
                const QString absoluteDirName = directory + relativeFileName.left(relativeFileName.indexOf(QLatin1Char('/')));
                if (m_versionInfoHash.contains(absoluteDirName)) {
                    ItemVersion oldState = m_versionInfoHash.value(absoluteDirName);
                    // only keep the most important state for a directory
                    if (oldState == ConflictingVersion)
                        continue;
                    if (oldState == LocallyModifiedUnstagedVersion && state != ConflictingVersion)
                        continue;
                    if (oldState == LocallyModifiedVersion && state != LocallyModifiedUnstagedVersion && state != ConflictingVersion)
                        continue;
                    m_versionInfoHash.insert(absoluteDirName, state);
                } else {
                    m_versionInfoHash.insert(absoluteDirName, state);
                }
            } else { // normal file, no directory
                m_versionInfoHash.insert(directory + relativeFileName, state);
            }
        }
    }
    return true;
}

void FileViewGitPlugin::endRetrieval()
{
}

KVersionControlPlugin::ItemVersion FileViewGitPlugin::itemVersion(const KFileItem &item) const
{
    const QString itemUrl = item.localPath();
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
    } else {
        // files that are not in our map are normal, tracked files by definition
        return NormalVersion;
    }
}

QList<QAction *> FileViewGitPlugin::versionControlActions(const KFileItemList &items) const
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

QList<QAction *> FileViewGitPlugin::outOfVersionControlActions(const KFileItemList &items) const
{
    // Only for a single directory.
    if (items.count() != 1 || !items.first().isDir()) {
        return {};
    }

    m_contextDir = items.first().localPath();

    return QList<QAction *>{} << m_cloneAction;
}

QList<QAction *> FileViewGitPlugin::contextMenuFilesActions(const KFileItemList &items) const
{
    Q_ASSERT(!items.isEmpty());

    if (!m_pendingOperation) {
        m_contextDir = QFileInfo(items.first().localPath()).canonicalPath();
        m_contextItems.clear();
        for (const KFileItem &item : items) {
            m_contextItems.append(item);
        }

        // see which actions should be enabled
        int versionedCount = 0;
        int addableCount = 0;
        int revertCount = 0;
        int restoreStageCount = 0;
        for (const KFileItem &item : items) {
            const ItemVersion state = itemVersion(item);
            if (state != UnversionedVersion && state != RemovedVersion && state != IgnoredVersion) {
                ++versionedCount;
            }
            if (state == UnversionedVersion || state == LocallyModifiedUnstagedVersion || state == IgnoredVersion || state == ConflictingVersion) {
                ++addableCount;
            }
            if (state == LocallyModifiedVersion || state == LocallyModifiedUnstagedVersion || state == ConflictingVersion) {
                ++revertCount;
            }
            if (state == LocallyModifiedVersion) {
                ++restoreStageCount;
            }
        }

        m_logAction->setEnabled(versionedCount == items.count());
        m_addAction->setEnabled(addableCount == items.count());
        m_revertAction->setEnabled(revertCount == items.count());
        m_removeAction->setEnabled(versionedCount == items.count());
        m_restoreStagedAction->setEnabled(restoreStageCount == items.count());
    } else {
        m_logAction->setEnabled(false);
        m_addAction->setEnabled(false);
        m_revertAction->setEnabled(false);
        m_removeAction->setEnabled(false);
        m_restoreStagedAction->setEnabled(false);
    }

    QList<QAction *> actions;
    actions.append(m_logAction);
    actions.append(m_addAction);
    actions.append(m_restoreStagedAction);
    actions.append(m_revertAction);
    actions.append(m_removeAction);
    return actions;
}

QList<QAction *> FileViewGitPlugin::contextMenuDirectoryActions(const QString &directory) const
{
    QList<QAction *> actions;
    if (!m_pendingOperation) {
        m_contextItems.clear();
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
    execGitCommand(QStringLiteral("add"),
                   QStringList(),
                   xi18nd("@info:status", "Adding files to <application>Git</application> repository..."),
                   xi18nd("@info:status", "Adding files to <application>Git</application> repository failed."),
                   xi18nd("@info:status", "Added files to <application>Git</application> repository."));
}

void FileViewGitPlugin::removeFiles()
{
    const QStringList arguments{
        QStringLiteral("-r"), // recurse through directories
        QStringLiteral("--force"), // also remove files that have not been committed yet
    };
    execGitCommand(QStringLiteral("rm"),
                   arguments,
                   xi18nd("@info:status", "Removing files from <application>Git</application> repository..."),
                   xi18nd("@info:status", "Removing files from <application>Git</application> repository failed."),
                   xi18nd("@info:status", "Removed files from <application>Git</application> repository."));
}

void FileViewGitPlugin::revertFiles()
{
    execGitCommand(QStringLiteral("checkout"),
                   {QStringLiteral("--")},
                   xi18nd("@info:status", "Reverting files from <application>Git</application> repository..."),
                   xi18nd("@info:status", "Reverting files from <application>Git</application> repository failed."),
                   xi18nd("@info:status", "Reverted files from <application>Git</application> repository."));
}

void FileViewGitPlugin::showLocalChanges()
{
    Q_ASSERT(!m_contextDir.isEmpty());

    runCommand(QStringLiteral("git difftool --dir-diff ."));
}

void FileViewGitPlugin::showDiff(const QUrl &link)
{
    if (link.scheme() != QLatin1String("rev")) {
        return;
    }
    runCommand(QStringLiteral("git difftool --dir-diff %1^ %1").arg(link.path()));
}

void FileViewGitPlugin::log()
{
    QStringList items;
    if (m_contextItems.isEmpty()) {
        items << QStringLiteral(".");
    } else {
        for (auto &item : std::as_const(m_contextItems)) {
            items << item.url().fileName();
        }
    }

    QProcess process;
    process.setWorkingDirectory(m_contextDir);
    process.start(QStringLiteral("git"),
                  QStringList{QStringLiteral("log"),
                              QStringLiteral("--date=format:%x"),
                              QStringLiteral("-n 100"),
                              QStringLiteral("--pretty=format:<tr> <td><a href=\"rev:%h\">%h</a></td> <td>%ad</td> <td>%s</td> <td>%an</td> </tr>")}
                      + items);

    if (!process.waitForFinished() || process.exitCode() != 0) {
        Q_EMIT errorMessage(xi18nd("@info:status", "<application>Git</application> Log failed."));
        return;
    }

    const QString gitOutput = QString::fromLocal8Bit(process.readAllStandardOutput());

    QPalette palette;
    const QString styleSheet =
        QStringLiteral(
            "body { background: %1; color: %2; }"
            "table.logtable td { padding: 9px 8px 9px; }"
            "a { color: %3; }"
            "a:visited { color: %4; } ")
            .arg(palette.window().color().name(), palette.text().color().name(), palette.link().color().name(), palette.linkVisited().color().name());

    QDialog *dlg = new QDialog(m_parentWidget);
    QVBoxLayout *layout = new QVBoxLayout;
    auto view = new QTextBrowser(dlg);
    layout->addWidget(view);
    dlg->setLayout(layout);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(xi18nd("@title:window", "<application>Git</application> Log"));
    view->setOpenLinks(false);
    view->setOpenExternalLinks(false);
    connect(view, &QTextBrowser::anchorClicked, this, &FileViewGitPlugin::showDiff);
    view->setHtml(QStringLiteral("<html>"
                                 "<style> %1 </style>"
                                 "<table class=\"logtable\">"
                                 "<tr bgcolor=\"%2\">"
                                 "<td> %3 </td> <td> %4 </td> <td> %5 </p> </td> <td> %6 </td>"
                                 "</tr>"
                                 "%7"
                                 "</table>"
                                 "</html>")
                      .arg(styleSheet,
                           palette.highlight().color().name(),
                           i18nc("Git commit hash", "Commit"),
                           i18nc("Git commit date", "Date"),
                           i18nc("Git commit message", "Message"),
                           i18nc("Git commit author", "Author"),
                           gitOutput));

    dlg->resize(QSize(720, 560));
    dlg->show();
}

void FileViewGitPlugin::merge()
{
    Q_ASSERT(!m_contextDir.isEmpty());

    runCommand(QStringLiteral("git mergetool"));
}

void FileViewGitPlugin::restoreStaged()
{
    const QStringList arguments{
        QStringLiteral("--staged"), // staged, from index
    };
    execGitCommand(QStringLiteral("restore"),
                   arguments,
                   xi18nd("@info:status", "Restoring staged files from <application>Git</application> repository..."),
                   xi18nd("@info:status", "Restoring staged files from <application>Git</application> repository failed."),
                   xi18nd("@info:status", "Restoring staged files from <application>Git</application> repository."));
}

void FileViewGitPlugin::clone()
{
    CloneDialog dialog(m_contextDir, m_parentWidget);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList arguments = {dialog.url(), dialog.directory(), QStringLiteral("clone"), QStringLiteral("--progress")};
        if (dialog.recursive()) {
            arguments << QStringLiteral("--recurse-submodules");
        }
        if (dialog.bare()) {
            arguments << QStringLiteral("--bare");
        }
        if (dialog.noCheckout()) {
            arguments << QStringLiteral("--no-checkout");
        }
        const auto depth = dialog.depth();
        if (depth > 0) {
            arguments << QStringLiteral("--depth") << QString::number(depth);
        }
        const QString branch = dialog.branch();
        if (!branch.isEmpty()) {
            arguments << QStringLiteral("--branch") << branch;
        }

        QProcess *process = new QProcess(m_parentWidget);
        QDialog *progressDialog = new QDialog(m_parentWidget);
        QVBoxLayout *layout = new QVBoxLayout;
        QPlainTextEdit *text = new QPlainTextEdit;
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(text);
        layout->addWidget(buttonBox);
        progressDialog->setLayout(layout);
        connect(buttonBox, &QDialogButtonBox::rejected, process, &QProcess::terminate);
        connect(buttonBox, &QDialogButtonBox::accepted, progressDialog, &QDialog::accept);
        connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError) {
            const QString commandLine = process->program() + process->arguments().join(QLatin1Char(' '));
            Q_EMIT errorMessage(xi18nd("@info:status", "<application>Git</application> error starting: %1", commandLine));
        });
        connect(process, &QProcess::finished, process, [this, process, buttonBox](int exitCode, QProcess::ExitStatus) {
            if (exitCode != EXIT_SUCCESS) {
                Q_EMIT errorMessage(xi18nd("@info:status", "<application>Git</application> clone failed: %1", process->errorString()));
            } else {
                Q_EMIT operationCompletedMessage(xi18nd("@info:status", "<application>Git</application> clone complete."));
            }

            buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
        });
        // git clone outputs only to stderr but we connect stdout anyway.
        connect(process, &QProcess::readyReadStandardOutput, text, [text, process]() {
            auto input = process->readAllStandardOutput();
            auto list = QString::fromLocal8Bit(input).split(QLatin1Char('\r'), Qt::SkipEmptyParts);
            text->moveCursor(QTextCursor::End);
            for (auto &i : std::as_const(list)) {
                text->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
                text->textCursor().removeSelectedText();
                text->insertPlainText(i);
            }
        });
        connect(process, &QProcess::readyReadStandardError, text, [text, process]() {
            auto input = process->readAllStandardError();
            auto list = QString::fromLocal8Bit(input).split(QLatin1Char('\r'), Qt::SkipEmptyParts);
            text->moveCursor(QTextCursor::End);
            for (auto &i : std::as_const(list)) {
                text->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
                text->textCursor().removeSelectedText();
                text->insertPlainText(i);
            }
        });
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        progressDialog->setWindowTitle(dialog.windowTitle());
        progressDialog->setAttribute(Qt::WA_DeleteOnClose);
        progressDialog->resize(progressDialog->sizeHint() + QSize{200, 0});
        progressDialog->show();

        process->setWorkingDirectory(m_contextDir);
        process->start(QStringLiteral("git"), arguments);
        Q_EMIT infoMessage(xi18nd("@info:status", "<application>Git</application> clone repository..."));
    }
}

void FileViewGitPlugin::checkout()
{
    CheckoutDialog dialog(m_parentWidget);
    if (dialog.exec() == QDialog::Accepted) {
        QProcess process;
        process.setWorkingDirectory(m_contextDir);
        QStringList arguments;
        arguments << QStringLiteral("checkout");
        if (dialog.force()) {
            arguments << QStringLiteral("-f");
        }
        const QString newBranchName = dialog.newBranchName();
        if (!newBranchName.isEmpty()) {
            arguments << QStringLiteral("-b");
            arguments << newBranchName;
        }
        const QString checkoutIdentifier = dialog.checkoutIdentifier();
        if (!checkoutIdentifier.isEmpty()) {
            arguments << checkoutIdentifier;
        }
        // to appear in messages
        const QString currentBranchName = newBranchName.isEmpty() ? checkoutIdentifier : newBranchName;
        process.start(QStringLiteral("git"), arguments);
        process.setReadChannel(QProcess::StandardError); // git writes info messages to stderr as well
        QString completedMessage;
        while (process.waitForReadyRead()) {
            char buffer[512];
            while (process.readLine(buffer, sizeof(buffer)) > 0) {
                const QString currentLine = QString::fromLocal8Bit(buffer);
                if (currentLine.startsWith(QLatin1String("Switched to branch"))) {
                    completedMessage = xi18nd("@info:status", "Switched to branch '%1'", currentBranchName);
                }
                if (currentLine.startsWith(QLatin1String("HEAD is now at"))) {
                    const QString headIdentifier = currentLine.mid(QLatin1String("HEAD is now at ").size()).trimmed();
                    completedMessage = xi18nd(
                        "@info:status Git HEAD pointer, parameter includes "
                        "short SHA-1 & commit message ",
                        "HEAD is now at %1",
                        headIdentifier);
                }
                // special output for checkout -b
                if (currentLine.startsWith(QLatin1String("Switched to a new branch"))) {
                    completedMessage = xi18nd("@info:status", "Switched to a new branch '%1'", currentBranchName);
                }
            }
        }
        if (process.exitCode() == 0 && process.exitStatus() == QProcess::NormalExit) {
            if (!completedMessage.isEmpty()) {
                Q_EMIT operationCompletedMessage(completedMessage);
                Q_EMIT itemVersionsChanged();
            }
        } else {
            Q_EMIT errorMessage(xi18nd("@info:status",
                                       "<application>Git</application> Checkout failed."
                                       " Maybe your working directory is dirty."));
        }
    }
}

void FileViewGitPlugin::commit()
{
    CommitDialog dialog(m_parentWidget);
    if (dialog.exec() == QDialog::Accepted) {
        QTemporaryFile tmpCommitMessageFile;
        tmpCommitMessageFile.open();
        tmpCommitMessageFile.write(dialog.commitMessage());
        tmpCommitMessageFile.close();
        QProcess process;
        process.setWorkingDirectory(m_contextDir);
        QStringList args = {QStringLiteral("commit")};
        if (dialog.amend()) {
            args << QStringLiteral("--amend");
        }
        args << QStringLiteral("-F");
        args << tmpCommitMessageFile.fileName();
        process.start(QStringLiteral("git"), args);
        QString completedMessage;
        while (process.waitForReadyRead()) {
            char buffer[512];
            while (process.readLine(buffer, sizeof(buffer)) > 0) {
                if (strlen(buffer) > 0 && buffer[0] == '[') {
                    completedMessage = QString::fromLocal8Bit(buffer).trimmed();
                    break;
                }
            }
        }
        if (!completedMessage.isEmpty()) {
            Q_EMIT operationCompletedMessage(completedMessage);
            Q_EMIT itemVersionsChanged();
        }
    }
}

void FileViewGitPlugin::createTag()
{
    TagDialog dialog(m_parentWidget);
    if (dialog.exec() == QDialog::Accepted) {
        QTemporaryFile tempTagMessageFile;
        tempTagMessageFile.open();
        tempTagMessageFile.write(dialog.tagMessage());
        tempTagMessageFile.close();
        QProcess process;
        process.setWorkingDirectory(m_contextDir);
        process.setReadChannel(QProcess::StandardError);
        process.start(
            QStringLiteral("git"),
            {QStringLiteral("tag"), QStringLiteral("-a"), QStringLiteral("-F"), tempTagMessageFile.fileName(), dialog.tagName(), dialog.baseBranch()});
        QString completedMessage;
        bool gotTagAlreadyExistsMessage = false;
        while (process.waitForReadyRead()) {
            char buffer[512];
            while (process.readLine(buffer, sizeof(buffer)) > 0) {
                const QString line = QString::fromLocal8Bit(buffer);
                if (line.contains(QLatin1String("already exists"))) {
                    gotTagAlreadyExistsMessage = true;
                }
            }
        }
        if (process.exitCode() == 0 && process.exitStatus() == QProcess::NormalExit) {
            completedMessage = xi18nd("@info:status", "Successfully created tag '%1'", dialog.tagName());
            Q_EMIT operationCompletedMessage(completedMessage);
        } else {
            // I don't know any other error, but in case one occurs, the user doesn't get FALSE error messages
            Q_EMIT errorMessage(gotTagAlreadyExistsMessage ? xi18nd("@info:status",
                                                                    "<application>Git</application> tag creation failed."
                                                                    " A tag with the name '%1' already exists.",
                                                                    dialog.tagName())
                                                           : xi18nd("@info:status", "<application>Git</application> tag creation failed."));
        }
    }
}

void FileViewGitPlugin::push()
{
    PushDialog dialog(m_parentWidget);
    if (dialog.exec() == QDialog::Accepted) {
        m_process.setWorkingDirectory(m_contextDir);

        m_errorMsg = xi18nd("@info:status", "Pushing branch %1 to %2:%3 failed.", dialog.localBranch(), dialog.destination(), dialog.remoteBranch());
        m_operationCompletedMsg = xi18nd("@info:status", "Pushed branch %1 to %2:%3.", dialog.localBranch(), dialog.destination(), dialog.remoteBranch());
        Q_EMIT infoMessage(xi18nd("@info:status", "Pushing branch %1 to %2:%3...", dialog.localBranch(), dialog.destination(), dialog.remoteBranch()));

        m_command = QStringLiteral("push");
        m_pendingOperation = true;
        QStringList args;
        args << QStringLiteral("push");
        if (dialog.force()) {
            args << QStringLiteral("--force");
        }
        args << dialog.destination();
        args << QStringLiteral("%1:%2").arg(dialog.localBranch(), dialog.remoteBranch());
        m_process.start(QStringLiteral("git"), args);
    }
}

void FileViewGitPlugin::pull()
{
    PullDialog dialog(m_parentWidget);
    if (dialog.exec() == QDialog::Accepted) {
        m_process.setWorkingDirectory(m_contextDir);

        m_errorMsg = xi18nd("@info:status", "Pulling branch %1 from %2 failed.", dialog.remoteBranch(), dialog.source());
        m_operationCompletedMsg = xi18nd("@info:status", "Pulled branch %1 from %2 successfully.", dialog.remoteBranch(), dialog.source());
        Q_EMIT infoMessage(xi18nd("@info:status", "Pulling branch %1 from %2...", dialog.remoteBranch(), dialog.source()));

        m_command = QStringLiteral("pull");
        m_pendingOperation = true;
        m_process.start(QStringLiteral("git"), {QStringLiteral("pull"), dialog.source(), dialog.remoteBranch()});
    }
}

void FileViewGitPlugin::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_pendingOperation = false;

    QString message;
    if (m_command == QLatin1String("push")) { // output parsing for push
        message = parsePushOutput();
        m_command = QString();
    }
    if (m_command == QLatin1String("pull")) {
        message = parsePullOutput();
        m_command = QString();
    }

    if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
        Q_EMIT errorMessage(message.isNull() ? m_errorMsg : message);
    } else if (m_contextItems.isEmpty()) {
        Q_EMIT operationCompletedMessage(message.isNull() ? m_operationCompletedMsg : message);
        Q_EMIT itemVersionsChanged();
    } else {
        startGitCommandProcess();
    }
}

void FileViewGitPlugin::slotOperationError()
{
    // don't do any operation on other items anymore
    m_contextItems.clear();
    m_pendingOperation = false;

    Q_EMIT errorMessage(m_errorMsg);
}

QString FileViewGitPlugin::parsePushOutput()
{
    m_process.setReadChannel(QProcess::StandardError);
    QString message;
    char buffer[256];
    while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
        const QString line = QString::fromLocal8Bit(buffer);
        if (line.contains(QLatin1String("->")) || (line.contains(QLatin1String("fatal")) && message.isNull())) {
            message = line.trimmed();
        }
        if (line.contains(QLatin1String("Everything up-to-date")) && message.isNull()) {
            message = xi18nd("@info:status", "Branch is already up-to-date.");
        }
    }
    return message;
}

QString FileViewGitPlugin::parsePullOutput()
{
    char buffer[256];
    while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
        const QString line = QString::fromLocal8Bit(buffer);
        if (line.contains(QLatin1String("Already up-to-date"))) {
            return xi18nd("@info:status", "Branch is already up-to-date.");
        }
        if (line.contains(QLatin1String("CONFLICT"))) {
            Q_EMIT itemVersionsChanged();
            return xi18nd("@info:status", "Merge conflicts occurred. Fix them and commit the result.");
        }
    }
    return QString();
}

void FileViewGitPlugin::execGitCommand(const QString &gitCommand,
                                       const QStringList &arguments,
                                       const QString &infoMsg,
                                       const QString &errorMsg,
                                       const QString &operationCompletedMsg)
{
    Q_EMIT infoMessage(infoMsg);

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
    // force explicitly selected files but no files in selected directories
    if (m_command == QLatin1String("add") && !item.isDir()) {
        arguments << QStringLiteral("-f");
    }
    arguments << item.url().fileName();
    m_process.start(QStringLiteral("git"), arguments);
    // the remaining items of m_contextItems will be executed
    // after the process has finished (see slotOperationFinished())
}

#include "fileviewgitplugin.moc"

#include "moc_fileviewgitplugin.cpp"
