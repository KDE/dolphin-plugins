/*
    SPDX-FileCopyrightText: 2009-2011 Peter Penz <peter.penz19@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fileviewsvnplugin.h"

#include "fileviewsvnpluginsettings.h"

#include <KLocalizedString>
#include <KShell>
#include <KPluginFactory>

#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVBoxLayout>
#include <QListWidget>
#include <KConfigGroup>
#include <KWindowConfig>
#include <QWindow>
#include <QTableWidget>
#include <QHeaderView>

#include "svncommitdialog.h"
#include "svnlogdialog.h"
#include "svncheckoutdialog.h"
#include "svnprogressdialog.h"
#include "svncleanupdialog.h"

#include "svncommands.h"

K_PLUGIN_CLASS_WITH_JSON(FileViewSvnPlugin, "fileviewsvnplugin.json")

FileViewSvnPlugin::FileViewSvnPlugin(QObject* parent, const QList<QVariant>& args) :
    KVersionControlPlugin(parent),
    m_pendingOperation(false),
    m_versionInfoHash(),
    m_updateAction(nullptr),
    m_showLocalChangesAction(nullptr),
    m_commitAction(nullptr),
    m_addAction(nullptr),
    m_removeAction(nullptr),
    m_showUpdatesAction(nullptr),
    m_logAction(nullptr),
    m_checkoutAction(nullptr),
    m_cleanupAction(nullptr),
    m_command(),
    m_arguments(),
    m_errorMsg(),
    m_operationCompletedMsg(),
    m_contextDir(),
    m_contextItems(),
    m_process(),
    m_tempFile()
{
    Q_UNUSED(args);

    m_parentWidget = qobject_cast<QWidget*>(parent);

    m_updateAction = new QAction(this);
    m_updateAction->setIcon(QIcon::fromTheme("view-refresh"));
    m_updateAction->setText(i18nc("@item:inmenu", "SVN Update"));
    connect(m_updateAction, &QAction::triggered,
            this, &FileViewSvnPlugin::updateFiles);

    m_showLocalChangesAction = new QAction(this);
    m_showLocalChangesAction->setIcon(QIcon::fromTheme("view-split-left-right"));
    m_showLocalChangesAction->setText(i18nc("@item:inmenu", "Show Local SVN Changes"));
    connect(m_showLocalChangesAction, &QAction::triggered,
            this, &FileViewSvnPlugin::showLocalChanges);

    m_commitAction = new QAction(this);
    m_commitAction->setIcon(QIcon::fromTheme("vcs-commit"));
    m_commitAction->setText(i18nc("@item:inmenu", "SVN Commit..."));
    connect(m_commitAction, &QAction::triggered,
            this, &FileViewSvnPlugin::commitDialog);

    m_addAction = new QAction(this);
    m_addAction->setIcon(QIcon::fromTheme("list-add"));
    m_addAction->setText(i18nc("@item:inmenu", "SVN Add"));
    connect(m_addAction, &QAction::triggered,
            this, QOverload<>::of(&FileViewSvnPlugin::addFiles));

    m_removeAction = new QAction(this);
    m_removeAction->setIcon(QIcon::fromTheme("list-remove"));
    m_removeAction->setText(i18nc("@item:inmenu", "SVN Delete"));
    connect(m_removeAction, &QAction::triggered,
            this, &FileViewSvnPlugin::removeFiles);

    m_revertAction = new QAction(this);
    m_revertAction->setIcon(QIcon::fromTheme("document-revert"));
    m_revertAction->setText(i18nc("@item:inmenu", "SVN Revert"));
    connect(m_revertAction, &QAction::triggered,
            this, QOverload<>::of(&FileViewSvnPlugin::revertFiles));

    m_showUpdatesAction = new QAction(this);
    m_showUpdatesAction->setCheckable(true);
    m_showUpdatesAction->setText(i18nc("@item:inmenu", "Show SVN Updates"));
    m_showUpdatesAction->setChecked(FileViewSvnPluginSettings::showUpdates());
    connect(m_showUpdatesAction, &QAction::toggled,
            this, &FileViewSvnPlugin::slotShowUpdatesToggled);
    connect(this, &FileViewSvnPlugin::setShowUpdatesChecked,
            m_showUpdatesAction, &QAction::setChecked);

    m_logAction = new QAction(this);
    m_logAction->setText(i18nc("@action:inmenu", "SVN Log..."));
    connect(m_logAction, &QAction::triggered,
            this, &FileViewSvnPlugin::logDialog);

    m_checkoutAction = new QAction(this);
    m_checkoutAction->setText(i18nc("@action:inmenu", "SVN Checkout..."));
    connect(m_checkoutAction, &QAction::triggered,
            this, &FileViewSvnPlugin::checkoutDialog);

    m_cleanupAction = new QAction(this);
    m_cleanupAction->setText(i18nc("@action:inmenu", "SVN Cleanup..."));
    connect(m_cleanupAction, &QAction::triggered,
            this, &FileViewSvnPlugin::cleanupDialog);

    connect(&m_process, &QProcess::finished,
            this, &FileViewSvnPlugin::slotOperationCompleted);
    connect(&m_process, &QProcess::errorOccurred,
            this, &FileViewSvnPlugin::slotOperationError);
}

FileViewSvnPlugin::~FileViewSvnPlugin()
{
}

QString FileViewSvnPlugin::fileName() const
{
    return QLatin1String(".svn");
}

QString FileViewSvnPlugin::localRepositoryRoot(const QString& directory) const
{
    QProcess process;
    process.setWorkingDirectory(directory);
    process.start("svn", {"info", "--show-item", "wc-root"});
    if (process.waitForReadyRead(100) && process.exitCode() == 0) {
        return QString::fromUtf8(process.readAll().chopped(1));
    }
    return QString();
}

bool FileViewSvnPlugin::beginRetrieval(const QString& directory)
{
    Q_ASSERT(directory.endsWith(QLatin1Char('/')));

    // Clear all entries for this directory including the entries
    // for sub directories
    QMutableHashIterator<QString, ItemVersion> it(m_versionInfoHash);
    while (it.hasNext()) {
        it.next();
        // 'svn status' return dirs without trailing slash, so without it we can't remove current
        // directory from hash.
        if ((it.key() + QLatin1Char('/')).startsWith(directory)) {
            it.remove();
        }
    }

    QStringList arguments;
    arguments << QLatin1String("status");
    if (FileViewSvnPluginSettings::showUpdates()) {
        arguments << QLatin1String("--show-updates");
    }
    arguments << QLatin1String("--no-ignore") << directory;

    QProcess process;
    process.start(QLatin1String("svn"), arguments);
    while (process.waitForReadyRead()) {
        char buffer[1024];
        while (process.readLine(buffer, sizeof(buffer)) > 0)  {
            ItemVersion version = NormalVersion;
            QString filePath(buffer);

            switch (buffer[0]) {
            case 'I':
            case '?': version = UnversionedVersion; break;
            case 'M': version = LocallyModifiedVersion; break;
            case 'A': version = AddedVersion; break;
            case 'D': version = RemovedVersion; break;
            case 'C': version = ConflictingVersion; break;
            case '!': version = MissingVersion; break;
            default:
                if (filePath.contains('*')) {
                    version = UpdateRequiredVersion;
                } else if (filePath.contains("W155010")) {
                    version = UnversionedVersion;
                }
                break;
            }

            // Only values with a different version as 'NormalVersion'
            // are added to the hash table. If a value is not in the
            // hash table, it is automatically defined as 'NormalVersion'
            // (see FileViewSvnPlugin::itemVersion()).
            if (version != NormalVersion) {
                int pos = filePath.indexOf('/');
                const int length = filePath.length() - pos - 1;
                filePath = filePath.mid(pos, length);
                if (!filePath.isEmpty()) {
                    m_versionInfoHash.insert(filePath, version);
                }
            }
        }
    }
    if ((process.exitCode() != 0 || process.exitStatus() != QProcess::NormalExit)) {
        if (FileViewSvnPluginSettings::showUpdates()) {
            // Network update failed. Unset ShowUpdates option, which triggers a refresh
            Q_EMIT infoMessage(i18nc("@info:status", "SVN status update failed. Disabling Option "
                                   "\"Show SVN Updates\"."));
            Q_EMIT setShowUpdatesChecked(false);
            // this is no fail, we just try again differently
            // furthermore returning false shows an error message that would override our info
            return true;
        } else {
            return false;
        }
    }

    return true;
}

void FileViewSvnPlugin::endRetrieval()
{
    Q_EMIT versionInfoUpdated();
}

KVersionControlPlugin::ItemVersion FileViewSvnPlugin::itemVersion(const KFileItem& item) const
{
    const QString itemUrl = item.localPath();
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
    }

    // If parent directory is unversioned item itself is unversioned.
    if (isInUnversionedDir(item)) {
        return UnversionedVersion;
    }

    if (!item.isDir()) {
        // files that have not been listed by 'svn status' (= m_versionInfoHash)
        // are under version control per definition
        // NOTE: svn status does not report files in unversioned paths
        const QString path = QFileInfo(itemUrl).path();
        return m_versionInfoHash.value(path, NormalVersion);
    }

    // The item is a directory. Check whether an item listed by 'svn status' (= m_versionInfoHash)
    // is part of this directory. In this case a local modification should be indicated in the
    // directory already.
    const QString itemDir = itemUrl + QDir::separator();
    QHash<QString, ItemVersion>::const_iterator it = m_versionInfoHash.constBegin();
    while (it != m_versionInfoHash.constEnd()) {
        if (it.key().startsWith(itemDir)) {
            const ItemVersion version = m_versionInfoHash.value(it.key());
            if (version == LocallyModifiedVersion || version == AddedVersion || version == RemovedVersion) {
                return LocallyModifiedVersion;
            }
        }
        ++it;
    }

    return NormalVersion;
}

QList<QAction*> FileViewSvnPlugin::versionControlActions(const KFileItemList& items) const
{
    // Special case: if any item is in unversioned directory we shouldn't add any actions because
    // we can do nothing with this item.
    for (const auto &i : items) {
        if (isInUnversionedDir(i)) {
            return {};
        }
    }

    if (items.count() == 1 && items.first().isDir()) {
        return directoryActions(items.first());
    }

    for (const KFileItem& item : items) {
        m_contextItems.append(item);
    }
    m_contextDir.clear();

    const bool noPendingOperation = !m_pendingOperation;
    if (noPendingOperation) {
        // iterate all items and check the version version to know which
        // actions can be enabled
        const int itemsCount = items.count();
        int versionedCount = 0;
        int editingCount = 0;
        for (const KFileItem& item : items) {
            const ItemVersion version = itemVersion(item);
            if (version != UnversionedVersion) {
                ++versionedCount;
            }

            switch (version) {
                case LocallyModifiedVersion:
                case ConflictingVersion:
                case AddedVersion:
                case RemovedVersion:
                    ++editingCount;
                    break;
                default:
                    break;
            }
        }
        m_commitAction->setEnabled(editingCount > 0);
        m_addAction->setEnabled(versionedCount == 0);
        m_revertAction->setEnabled(editingCount == itemsCount);
        m_removeAction->setEnabled(versionedCount == itemsCount);
    } else {
        m_commitAction->setEnabled(false);
        m_addAction->setEnabled(false);
        m_revertAction->setEnabled(false);
        m_removeAction->setEnabled(false);
    }
    m_updateAction->setEnabled(noPendingOperation);

    QList<QAction*> actions;
    actions.append(m_updateAction);
    actions.append(m_commitAction);
    actions.append(m_addAction);
    actions.append(m_removeAction);
    actions.append(m_revertAction);
    actions.append(m_showUpdatesAction);
    return actions;
}

QList<QAction*> FileViewSvnPlugin::outOfVersionControlActions(const KFileItemList& items) const
{
    // Only for a single directory.
    if (items.count() != 1 || !items.first().isDir()) {
        return {};
    }

    m_contextDir = items.first().localPath();

    return QList<QAction*>{} << m_checkoutAction;
}

void FileViewSvnPlugin::updateFiles()
{
    SvnProgressDialog *progressDialog = new SvnProgressDialog(i18nc("@title:window", "SVN Update"), m_contextDir, m_parentWidget);
    progressDialog->connectToProcess(&m_process);

    execSvnCommand(QLatin1String("update"), QStringList(),
                   i18nc("@info:status", "Updating SVN repository..."),
                   i18nc("@info:status", "Update of SVN repository failed."),
                   i18nc("@info:status", "Updated SVN repository."));
}

void FileViewSvnPlugin::showLocalChanges()
{
    Q_ASSERT(!m_contextDir.isEmpty());
    Q_ASSERT(m_contextItems.isEmpty());

    // This temporary file will be deleted on Dolphin close. We make an assumption:
    // when the file gets deleted kompare has already loaded it and no longer needs it.
    const QString tmpFileNameTemplate = QString("%1/%2.XXXXXX").arg(QDir::tempPath(), QDir(m_contextDir).dirName());
    QTemporaryFile *file = new QTemporaryFile(tmpFileNameTemplate, this);

    if (!file->open()) {
        Q_EMIT errorMessage(i18nc("@info:status", "Could not show local SVN changes."));
        return;
    }

    QProcess process;
    process.setStandardOutputFile(file->fileName());
    process.start(
        QLatin1String("svn"),
        QStringList {
            QLatin1String("diff"),
            QLatin1String("--git"),
            m_contextDir
        }
    );
    if (!process.waitForFinished() || process.exitCode() != 0) {
        Q_EMIT errorMessage(i18nc("@info:status", "Could not show local SVN changes: svn diff failed."));
        file->deleteLater();
        return;
    }

    const bool started = QProcess::startDetached(
        QLatin1String("kompare"),
        QStringList {
            file->fileName()
        }
    );
    if (!started) {
        Q_EMIT errorMessage(i18nc("@info:status", "Could not show local SVN changes: could not start kompare."));
        file->deleteLater();
    }
}

void FileViewSvnPlugin::commitDialog()
{
    QStringList context;
    if (!m_contextDir.isEmpty()) {
        context << m_contextDir;
    } else {
        for (const auto &i : qAsConst(m_contextItems)) {
            context << i.localPath();
        }
    }

    SvnCommitDialog *svnCommitDialog = new SvnCommitDialog(&m_versionInfoHash, context, m_parentWidget);

    connect(this, &FileViewSvnPlugin::versionInfoUpdated, svnCommitDialog, &SvnCommitDialog::refreshChangesList);

    connect(svnCommitDialog, &SvnCommitDialog::revertFiles, this, QOverload<const QStringList&>::of(&FileViewSvnPlugin::revertFiles));
    connect(svnCommitDialog, &SvnCommitDialog::diffFile, this, QOverload<const QString&>::of(&FileViewSvnPlugin::diffFile));
    connect(svnCommitDialog, &SvnCommitDialog::addFiles, this, QOverload<const QStringList&>::of(&FileViewSvnPlugin::addFiles));
    connect(svnCommitDialog, &SvnCommitDialog::commit, this, &FileViewSvnPlugin::commitFiles);

    svnCommitDialog->setAttribute(Qt::WA_DeleteOnClose);
    svnCommitDialog->show();
}

void FileViewSvnPlugin::addFiles()
{
    execSvnCommand(QLatin1String("add"), QStringList(),
                   i18nc("@info:status", "Adding files to SVN repository..."),
                   i18nc("@info:status", "Adding of files to SVN repository failed."),
                   i18nc("@info:status", "Added files to SVN repository."));
}

void FileViewSvnPlugin::removeFiles()
{
    execSvnCommand(QLatin1String("remove"), QStringList(),
                   i18nc("@info:status", "Removing files from SVN repository..."),
                   i18nc("@info:status", "Removing of files from SVN repository failed."),
                   i18nc("@info:status", "Removed files from SVN repository."));
}

void FileViewSvnPlugin::revertFiles()
{
    if (m_contextDir.isEmpty() && m_contextItems.empty()) {
        return;
    }

    QStringList arguments;
    QString root;

    // If we are reverting a directory let's revert everything in it.
    if (!m_contextDir.isEmpty()) {
        arguments << QLatin1String("--depth") << QLatin1String("infinity");
        root = m_contextDir;
    } else {
        root = SvnCommands::localRoot( m_contextItems.last().localPath() );
    }

    SvnProgressDialog *progressDialog = new SvnProgressDialog(i18nc("@title:window", "SVN Revert"), root, m_parentWidget);
    progressDialog->connectToProcess(&m_process);

    execSvnCommand(QStringLiteral("revert"), arguments,
                   i18nc("@info:status", "Reverting files from SVN repository..."),
                   i18nc("@info:status", "Reverting of files from SVN repository failed."),
                   i18nc("@info:status", "Reverted files from SVN repository."));
}

void FileViewSvnPlugin::logDialog()
{
    SvnLogDialog *svnLogDialog = new SvnLogDialog(m_contextDir, m_parentWidget);

    connect(svnLogDialog, &SvnLogDialog::errorMessage, this, &FileViewSvnPlugin::errorMessage);
    connect(svnLogDialog, &SvnLogDialog::operationCompletedMessage, this, &FileViewSvnPlugin::operationCompletedMessage);
    connect(svnLogDialog, &SvnLogDialog::diffAgainstWorkingCopy, this, &FileViewSvnPlugin::diffAgainstWorkingCopy);
    connect(svnLogDialog, &SvnLogDialog::diffBetweenRevs, this, &FileViewSvnPlugin::diffBetweenRevs);

    svnLogDialog->setAttribute(Qt::WA_DeleteOnClose);
    svnLogDialog->show();
}

void FileViewSvnPlugin::checkoutDialog()
{
    SvnCheckoutDialog *svnCheckoutDialog = new SvnCheckoutDialog(m_contextDir, m_parentWidget);

    connect(svnCheckoutDialog, &SvnCheckoutDialog::infoMessage, this, &FileViewSvnPlugin::infoMessage);
    connect(svnCheckoutDialog, &SvnCheckoutDialog::errorMessage, this, &FileViewSvnPlugin::errorMessage);
    connect(svnCheckoutDialog, &SvnCheckoutDialog::operationCompletedMessage, this, &FileViewSvnPlugin::operationCompletedMessage);

    svnCheckoutDialog->setAttribute(Qt::WA_DeleteOnClose);
    svnCheckoutDialog->show();
}

void FileViewSvnPlugin::cleanupDialog()
{
    SvnCleanupDialog *svnCleanupDialog = new SvnCleanupDialog(m_contextDir, m_parentWidget);

    connect(svnCleanupDialog, &SvnCleanupDialog::errorMessage, this, &FileViewSvnPlugin::errorMessage);
    connect(svnCleanupDialog, &SvnCleanupDialog::operationCompletedMessage, this, &FileViewSvnPlugin::operationCompletedMessage);
}

void FileViewSvnPlugin::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_pendingOperation = false;

    if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
        Q_EMIT errorMessage(m_errorMsg);
    } else if (m_contextItems.isEmpty()) {
        Q_EMIT operationCompletedMessage(m_operationCompletedMsg);
        Q_EMIT itemVersionsChanged();
    } else {
        startSvnCommandProcess();
    }
}

void FileViewSvnPlugin::slotOperationError()
{
    // don't do any operation on other items anymore
    m_contextItems.clear();
    m_pendingOperation = false;

    Q_EMIT errorMessage(m_errorMsg);
}

void FileViewSvnPlugin::slotShowUpdatesToggled(bool checked)
{
    FileViewSvnPluginSettings* settings = FileViewSvnPluginSettings::self();
    Q_ASSERT(settings != nullptr);
    settings->setShowUpdates(checked);
    settings->save();

    Q_EMIT itemVersionsChanged();
}

void FileViewSvnPlugin::revertFiles(const QStringList& filesPath)
{
    if (filesPath.empty()) {
        return;
    }

    for (const auto &i : qAsConst(filesPath)) {
        m_contextItems.append(KFileItem(QUrl::fromLocalFile(i)));
    }
    m_contextDir.clear();

    SvnProgressDialog *progressDialog = new SvnProgressDialog(i18nc("@title:window", "SVN Revert"), SvnCommands::localRoot(filesPath.first()), m_parentWidget);
    progressDialog->connectToProcess(&m_process);

    execSvnCommand(QLatin1String("revert"), QStringList() << filesPath,
                   i18nc("@info:status", "Reverting changes to file..."),
                   i18nc("@info:status", "Revert file failed."),
                   i18nc("@info:status", "File reverted."));
}

void FileViewSvnPlugin::diffFile(const QString& filePath)
{
    // For a diff we will export last known file local revision from a remote and compare. We will
    // not use basic SVN action 'svn diff --extensions -U<lines> <fileName>' because we should count
    // lines or set maximum number for this.
    // With a maximum number (2147483647) 'svn diff' starts to work slowly.

    diffAgainstWorkingCopy(filePath, SvnCommands::localRevision(filePath));
}

void FileViewSvnPlugin::diffAgainstWorkingCopy(const QString& localFilePath, ulong rev)
{
    QTemporaryFile *file = new QTemporaryFile(this);
    if (!SvnCommands::exportFile(QUrl::fromLocalFile(localFilePath), rev, file)) {
        Q_EMIT errorMessage(i18nc("@info:status", "Could not show local SVN changes for a file: could not get file."));
        file->deleteLater();
        return;
    }

    const bool started = QProcess::startDetached(
        QLatin1String("kompare"),
        QStringList {
            file->fileName(),
            localFilePath
        }
    );
    if (!started) {
        Q_EMIT errorMessage(i18nc("@info:status", "Could not show local SVN changes: could not start kompare."));
        file->deleteLater();
    }
}

void FileViewSvnPlugin::diffBetweenRevs(const QString& remoteFilePath, ulong rev1, ulong rev2)
{
    QTemporaryFile *file1 = new QTemporaryFile(this);
    QTemporaryFile *file2 = new QTemporaryFile(this);
    if (!SvnCommands::exportFile(QUrl::fromLocalFile(remoteFilePath), rev1, file1)) {
        Q_EMIT errorMessage(i18nc("@info:status", "Could not show local SVN changes for a file: could not get file."));
        file1->deleteLater();
        return;
    }
    if (!SvnCommands::exportFile(QUrl::fromLocalFile(remoteFilePath), rev2, file2)) {
        Q_EMIT errorMessage(i18nc("@info:status", "Could not show local SVN changes for a file: could not get file."));
        file1->deleteLater();
        file2->deleteLater();
        return;
    }

    const bool started = QProcess::startDetached(
        QLatin1String("kompare"),
        QStringList {
            file2->fileName(),
            file1->fileName()
        }
    );
    if (!started) {
        Q_EMIT errorMessage(i18nc("@info:status", "Could not show local SVN changes: could not start kompare."));
        file1->deleteLater();
        file2->deleteLater();
    }
}

void FileViewSvnPlugin::addFiles(const QStringList& filesPath)
{
    for (const auto &i : qAsConst(filesPath)) {
        m_contextItems.append(KFileItem(QUrl::fromLocalFile(i)));
    }
    m_contextDir.clear();

    addFiles();
}

void FileViewSvnPlugin::commitFiles(const QStringList& context, const QString& msg)
{
    if (context.empty()) {
        return;
    }

    // Write the commit description into a temporary file, so
    // that it can be read by the command "svn commit -F". The temporary
    // file must stay alive until slotOperationCompleted() is invoked and will
    // be destroyed when the version plugin is destructed.
    if (!m_tempFile.open())  {
        Q_EMIT errorMessage(i18nc("@info:status", "Commit of SVN changes failed."));
        return;
    }

    QTextStream out(&m_tempFile);
    const QString fileName = m_tempFile.fileName();
    out << msg;
    m_tempFile.close();

    QStringList arguments;
    arguments << context << "-F" << fileName;

    // Lets clear m_contextDir and m_contextItems variables: we will pass everything in arguments.
    // This is needed because startSvnCommandProcess() uses only one QString for svn transaction at
    // a time but we want to commit everything.
    m_contextDir.clear();
    m_contextItems.clear();

    SvnProgressDialog *progressDialog = new SvnProgressDialog(i18nc("@title:window", "SVN Commit"), SvnCommands::localRoot(context.first()), m_parentWidget);
    progressDialog->connectToProcess(&m_process);

    execSvnCommand(QLatin1String("commit"), arguments,
                   i18nc("@info:status", "Committing SVN changes..."),
                   i18nc("@info:status", "Commit of SVN changes failed."),
                   i18nc("@info:status", "Committed SVN changes."));
}

void FileViewSvnPlugin::execSvnCommand(const QString& svnCommand,
                                       const QStringList& arguments,
                                       const QString& infoMsg,
                                       const QString& errorMsg,
                                       const QString& operationCompletedMsg)
{
    Q_EMIT infoMessage(infoMsg);

    m_command = svnCommand;
    m_arguments = arguments;
    m_errorMsg = errorMsg;
    m_operationCompletedMsg = operationCompletedMsg;

    startSvnCommandProcess();
}

void FileViewSvnPlugin::startSvnCommandProcess()
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);
    m_pendingOperation = true;

    const QString program(QLatin1String("svn"));
    QStringList arguments;
    arguments << m_command << m_arguments;
    if (!m_contextDir.isEmpty()) {
        arguments << m_contextDir;
        m_contextDir.clear();
    } else {
        // If m_contextDir is empty and m_contextItems is empty then all svn arguments are in
        // m_arguments (for example see commitFiles()).
        if (!m_contextItems.isEmpty()) {
            const KFileItem item = m_contextItems.takeLast();
            arguments << item.localPath();
            // the remaining items of m_contextItems will be executed
            // after the process has finished (see slotOperationFinished())
        }
    }
    m_process.start(program, arguments);
}

QList<QAction*> FileViewSvnPlugin::directoryActions(const KFileItem& directory) const
{
    m_contextDir = directory.localPath();
    if (!m_contextDir.endsWith(QLatin1Char('/'))) {
        m_contextDir += QLatin1Char('/');
    }
    m_contextItems.clear();

    // Only enable the SVN actions if no SVN commands are
    // executed currently (see slotOperationCompleted() and
    // startSvnCommandProcess()).
    const bool enabled = !m_pendingOperation;
    m_updateAction->setEnabled(enabled);

    const ItemVersion version = itemVersion(directory);
    m_showLocalChangesAction->setEnabled(enabled && (version != NormalVersion));
    m_addAction->setEnabled(enabled && (version == UnversionedVersion));
    m_removeAction->setEnabled(enabled && (version == NormalVersion));
    if (version == LocallyModifiedVersion || version == AddedVersion || version == RemovedVersion) {
        m_commitAction->setEnabled(enabled);
        m_revertAction->setEnabled(enabled);
    } else {
        m_commitAction->setEnabled(false);
        m_revertAction->setEnabled(false);
    }

    QList<QAction*> actions;
    actions.append(m_updateAction);
    actions.append(m_showLocalChangesAction);
    actions.append(m_commitAction);
    actions.append(m_showUpdatesAction);
    actions.append(m_addAction);
    actions.append(m_removeAction);
    actions.append(m_revertAction);
    actions.append(m_logAction);
    actions.append(m_cleanupAction);
    return actions;
}

bool FileViewSvnPlugin::isInUnversionedDir(const KFileItem& item) const
{
    const QString itemPath = item.localPath();

    for (auto it = m_versionInfoHash.cbegin(); it != m_versionInfoHash.cend(); ++it) {
        // Add QDir::separator() to m_versionInfoHash entry to ensure this is a directory.
        if (it.value() == UnversionedVersion && itemPath.startsWith(it.key() + QDir::separator())) {
            return true;
        }
    }

    return false;
}

#include "fileviewsvnplugin.moc"

#include "moc_fileviewsvnplugin.cpp"
