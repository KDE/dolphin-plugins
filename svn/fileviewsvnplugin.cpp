/***************************************************************************
 *   Copyright (C) 2009-2011 by Peter Penz <peter.penz19@gmail.com>        *
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

#include "fileviewsvnplugin.h"

#include "fileviewsvnpluginsettings.h"

#include <KLocalizedString>
#include <KRun>
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
#include "svncommands.h"

K_PLUGIN_FACTORY(FileViewSvnPluginFactory, registerPlugin<FileViewSvnPlugin>();)

FileViewSvnPlugin::FileViewSvnPlugin(QObject* parent, const QList<QVariant>& args) :
    KVersionControlPlugin(parent),
    m_pendingOperation(false),
    m_versionInfoHash(),
    m_updateAction(0),
    m_showLocalChangesAction(0),
    m_commitAction(0),
    m_addAction(0),
    m_removeAction(0),
    m_showUpdatesAction(0),
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

    m_updateAction = new QAction(this);
    m_updateAction->setIcon(QIcon::fromTheme("view-refresh"));
    m_updateAction->setText(i18nc("@item:inmenu", "SVN Update"));
    connect(m_updateAction, SIGNAL(triggered()),
            this, SLOT(updateFiles()));

    m_showLocalChangesAction = new QAction(this);
    m_showLocalChangesAction->setIcon(QIcon::fromTheme("view-split-left-right"));
    m_showLocalChangesAction->setText(i18nc("@item:inmenu", "Show Local SVN Changes"));
    connect(m_showLocalChangesAction, SIGNAL(triggered()),
            this, SLOT(showLocalChanges()));

    m_commitAction = new QAction(this);
    m_commitAction->setIcon(QIcon::fromTheme("svn-commit"));
    m_commitAction->setText(i18nc("@item:inmenu", "SVN Commit..."));
    connect(m_commitAction, SIGNAL(triggered()),
            this, SLOT(commitDialog()));

    m_addAction = new QAction(this);
    m_addAction->setIcon(QIcon::fromTheme("list-add"));
    m_addAction->setText(i18nc("@item:inmenu", "SVN Add"));
    connect(m_addAction, SIGNAL(triggered()),
            this, SLOT(addFiles()));

    m_removeAction = new QAction(this);
    m_removeAction->setIcon(QIcon::fromTheme("list-remove"));
    m_removeAction->setText(i18nc("@item:inmenu", "SVN Delete"));
    connect(m_removeAction, SIGNAL(triggered()),
            this, SLOT(removeFiles()));

    m_revertAction = new QAction(this);
    m_revertAction->setIcon(QIcon::fromTheme("document-revert"));
    m_revertAction->setText(i18nc("@item:inmenu", "SVN Revert"));
    connect(m_revertAction, SIGNAL(triggered()),
            this, SLOT(revertFiles()));

    m_showUpdatesAction = new QAction(this);
    m_showUpdatesAction->setCheckable(true);
    m_showUpdatesAction->setText(i18nc("@item:inmenu", "Show SVN Updates"));
    m_showUpdatesAction->setChecked(FileViewSvnPluginSettings::showUpdates());
    connect(m_showUpdatesAction, SIGNAL(toggled(bool)),
            this, SLOT(slotShowUpdatesToggled(bool)));
    connect(this, SIGNAL(setShowUpdatesChecked(bool)),
            m_showUpdatesAction, SLOT(setChecked(bool)));

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
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

bool FileViewSvnPlugin::beginRetrieval(const QString& directory)
{
    Q_ASSERT(directory.endsWith(QLatin1Char('/')));

    // Clear all entries for this directory including the entries
    // for sub directories
    QMutableHashIterator<QString, ItemVersion> it(m_versionInfoHash);
    while (it.hasNext()) {
        it.next();
        if (it.key().startsWith(directory)) {
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
            emit infoMessage(i18nc("@info:status", "SVN status update failed. Disabling Option "
                                   "\"Show SVN Updates\"."));
            emit setShowUpdatesChecked(false);
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
    emit versionInfoUpdated();
}

KVersionControlPlugin::ItemVersion FileViewSvnPlugin::itemVersion(const KFileItem& item) const
{
    const QString itemUrl = item.localPath();
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
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

QList<QAction*> FileViewSvnPlugin::actions(const KFileItemList& items) const
{
    if (items.count() == 1 && items.first().isDir()) {
        return directoryActions(items.first());
    }

    foreach (const KFileItem& item, items) {
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
        foreach (const KFileItem& item, items) {
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


void FileViewSvnPlugin::updateFiles()
{
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
        emit errorMessage(i18nc("@info:status", "Could not show local SVN changes."));
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
        emit errorMessage(i18nc("@info:status", "Could not show local SVN changes: svn diff failed."));
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
        emit errorMessage(i18nc("@info:status", "Could not show local SVN changes: could not start kompare."));
        file->deleteLater();
    }
}

void FileViewSvnPlugin::commitDialog()
{
    QStringList context;
    if (!m_contextDir.isEmpty()) {
        context << m_contextDir;
    } else {
        for (const auto &i : m_contextItems) {
            context << i.localPath();
        }
    }

    SvnCommitDialog *svnCommitDialog = new SvnCommitDialog(&m_versionInfoHash, context);

    connect(this, &FileViewSvnPlugin::versionInfoUpdated, svnCommitDialog, &SvnCommitDialog::refreshChangesList);

    connect(svnCommitDialog, &SvnCommitDialog::revertFiles, this, QOverload<const QStringList&>::of(&FileViewSvnPlugin::revertFiles));
    connect(svnCommitDialog, &SvnCommitDialog::diffFile, this, &FileViewSvnPlugin::diffFile);
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
    execSvnCommand(QStringLiteral("revert"), QStringList(),
                   i18nc("@info:status", "Reverting files from SVN repository..."),
                   i18nc("@info:status", "Reverting of files from SVN repository failed."),
                   i18nc("@info:status", "Reverted files from SVN repository."));
}

void FileViewSvnPlugin::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_pendingOperation = false;

    if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
        emit errorMessage(m_errorMsg);
    } else if (m_contextItems.isEmpty()) {
        emit operationCompletedMessage(m_operationCompletedMsg);
        emit itemVersionsChanged();
    } else {
        startSvnCommandProcess();
    }
}

void FileViewSvnPlugin::slotOperationError()
{
    // don't do any operation on other items anymore
    m_contextItems.clear();
    m_pendingOperation = false;

    emit errorMessage(m_errorMsg);
}

void FileViewSvnPlugin::slotShowUpdatesToggled(bool checked)
{
    FileViewSvnPluginSettings* settings = FileViewSvnPluginSettings::self();
    Q_ASSERT(settings != 0);
    settings->setShowUpdates(checked);
    settings->save();

    emit itemVersionsChanged();
}

void FileViewSvnPlugin::revertFiles(const QStringList& filesPath)
{
    for (const auto &i : qAsConst(filesPath)) {
        m_contextItems.append( QUrl::fromLocalFile(i) );
    }
    m_contextDir.clear();

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

    QTemporaryFile *file = new QTemporaryFile(this);
    // TODO: Calling a blocking operation: with a slow connection this might take some time. Work
    //       should be done in a separate thread or process.
    if (!SVNCommands::exportLocalFile(filePath, SVNCommands::localRevision(filePath), file)) {
        emit errorMessage(i18nc("@info:status", "Could not show local SVN changes for a file: could not get file."));
        file->deleteLater();
    }

    const bool started = QProcess::startDetached(
        QLatin1String("kompare"),
        QStringList {
            file->fileName(),
            filePath
        }
    );
    if (!started) {
        emit errorMessage(i18nc("@info:status", "Could not show local SVN changes: could not start kompare."));
        file->deleteLater();
    }
}

void FileViewSvnPlugin::addFiles(const QStringList& filesPath)
{
    for (const auto &i : qAsConst(filesPath)) {
        m_contextItems.append( QUrl::fromLocalFile(i) );
    }
    m_contextDir.clear();

    addFiles();
}

void FileViewSvnPlugin::commitFiles(const QStringList& context, const QString& msg)
{
    // Write the commit description into a temporary file, so
    // that it can be read by the command "svn commit -F". The temporary
    // file must stay alive until slotOperationCompleted() is invoked and will
    // be destroyed when the version plugin is destructed.
    if (!m_tempFile.open())  {
        emit errorMessage(i18nc("@info:status", "Commit of SVN changes failed."));
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
    emit infoMessage(infoMsg);

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
    if (version == LocallyModifiedVersion || version == AddedVersion || version == RemovedVersion) {
        m_commitAction->setEnabled(enabled);
    } else {
        m_commitAction->setEnabled(false);
    }

    QList<QAction*> actions;
    actions.append(m_updateAction);
    actions.append(m_showLocalChangesAction);
    actions.append(m_commitAction);
    actions.append(m_showUpdatesAction);
    return actions;
}

#include "fileviewsvnplugin.moc"
