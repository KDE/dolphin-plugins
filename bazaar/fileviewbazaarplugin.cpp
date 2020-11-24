/***************************************************************************
 *   Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>             *
 *   Copyright (C) 2011 Canonical Ltd.                                     *
 *        By Jonathan Riddell <jriddell@ubuntu.com>                        *
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

#include "fileviewbazaarplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <QAction>
#include <QDir>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextStream>

K_PLUGIN_CLASS_WITH_JSON(FileViewBazaarPlugin, "fileviewbazaarplugin.json")
K_EXPORT_PLUGIN_VERSION(2)

FileViewBazaarPlugin::FileViewBazaarPlugin(QObject* parent, const QList<QVariant>& args) :
    KVersionControlPlugin(parent),
    m_pendingOperation(false),
    m_versionInfoHash(),
    m_updateAction(0),
    m_pullAction(0),
    m_pushAction(0),
    m_showLocalChangesAction(0),
    m_commitAction(0),
    m_addAction(0),
    m_removeAction(0),
    m_logAction(0),
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
    m_updateAction->setIcon(QIcon::fromTheme("go-down"));
    m_updateAction->setText(i18nc("@item:inmenu", "Bazaar Update"));
    connect(m_updateAction, &QAction::triggered,
            this, &FileViewBazaarPlugin::updateFiles);

    m_pullAction = new QAction(this);
    m_pullAction->setIcon(QIcon::fromTheme("go-bottom"));
    m_pullAction->setText(i18nc("@item:inmenu", "Bazaar Pull"));
    connect(m_pullAction, &QAction::triggered,
            this, &FileViewBazaarPlugin::pullFiles);

    m_pushAction = new QAction(this);
    m_pushAction->setIcon(QIcon::fromTheme("go-top"));
    m_pushAction->setText(i18nc("@item:inmenu", "Bazaar Push"));
    connect(m_pushAction, &QAction::triggered,
            this, &FileViewBazaarPlugin::pushFiles);

    m_showLocalChangesAction = new QAction(this);
    m_showLocalChangesAction->setIcon(QIcon::fromTheme("view-split-left-right"));
    m_showLocalChangesAction->setText(i18nc("@item:inmenu", "Show Local Bazaar Changes"));
    connect(m_showLocalChangesAction, &QAction::triggered,
            this, &FileViewBazaarPlugin::showLocalChanges);

    m_commitAction = new QAction(this);
    m_commitAction->setIcon(QIcon::fromTheme("svn-commit"));
    m_commitAction->setText(i18nc("@item:inmenu", "Bazaar Commit..."));
    connect(m_commitAction, &QAction::triggered,
            this, &FileViewBazaarPlugin::commitFiles);

    m_addAction = new QAction(this);
    m_addAction->setIcon(QIcon::fromTheme("list-add"));
    m_addAction->setText(i18nc("@item:inmenu", "Bazaar Add..."));
    connect(m_addAction, &QAction::triggered,
            this, &FileViewBazaarPlugin::addFiles);

    m_removeAction = new QAction(this);
    m_removeAction->setIcon(QIcon::fromTheme("list-remove"));
    m_removeAction->setText(i18nc("@item:inmenu", "Bazaar Delete"));
    connect(m_removeAction, &QAction::triggered,
            this, &FileViewBazaarPlugin::removeFiles);

    m_logAction = new QAction(this);
    m_logAction->setIcon(QIcon::fromTheme("format-list-ordered"));
    m_logAction->setText(i18nc("@item:inmenu", "Bazaar Log"));
    connect(m_logAction, &QAction::triggered,
            this, &FileViewBazaarPlugin::log);

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FileViewBazaarPlugin::slotOperationCompleted);
    connect(&m_process, &QProcess::errorOccurred,
            this, &FileViewBazaarPlugin::slotOperationError);
}

FileViewBazaarPlugin::~FileViewBazaarPlugin()
{
}

QString FileViewBazaarPlugin::fileName() const
{
    return QLatin1String(".bzr");
}

bool FileViewBazaarPlugin::beginRetrieval(const QString& directory)
{
    Q_ASSERT(directory.endsWith(QLatin1Char('/')));

    QString baseDir;
    QProcess process1;
    process1.setWorkingDirectory(directory);
    process1.start(QLatin1String("bzr"), {"root"});
    while (process1.waitForReadyRead()) {
        char buffer[512];
        while (process1.readLine(buffer, sizeof(buffer)) > 0)  {
            baseDir = QString(buffer).trimmed();
        }
    }
    // if bzr is not installed
    if (baseDir.isEmpty()) {
            return false;
    }

    // Clear all entries for this directory including the entries
    // for sub directories
    QMutableHashIterator<QString, ItemVersion> it(m_versionInfoHash);
    while (it.hasNext()) {
        it.next();
        if (it.key().startsWith(directory) || !it.key().startsWith(baseDir)) {
            it.remove();
        }
    }

    QProcess process2;
    process2.setWorkingDirectory(directory);
    process2.start(QLatin1String("bzr"), {"ignored"});
    while (process2.waitForReadyRead()) {
        char buffer[512];
        while (process2.readLine(buffer, sizeof(buffer)) > 0)  {
            QString line = QString(buffer).trimmed();
            QStringList list = line.split(' ');
            QString file = baseDir + '/' + list[0];
            m_versionInfoHash.insert(file, UnversionedVersion);
        }
    }

    QStringList arguments;
    arguments << QLatin1String("status") << QLatin1String("-S");
    arguments << baseDir;

    QProcess process;
    process.start(QLatin1String("bzr"), arguments);
    while (process.waitForReadyRead()) {
        char buffer[1024];
        while (process.readLine(buffer, sizeof(buffer)) > 0)  {
            ItemVersion state = NormalVersion;
            QString filePath = QString::fromUtf8(buffer);

            // This could probably do with being more consistent
            switch (buffer[0]) {
            case '?': state = UnversionedVersion; break;
            case ' ': if (buffer[1] == 'M') {state = LocallyModifiedVersion;} break;
            case '+': state = AddedVersion; break;
            case '-': state = RemovedVersion; break;
            case 'C': state = ConflictingVersion; break;
            default:
                if (filePath.contains('*')) {
                    state = UpdateRequiredVersion;
                }
                break;
            }

            // Only values with a different state as 'NormalVersion'
            // are added to the hash table. If a value is not in the
            // hash table, it is automatically defined as 'NormalVersion'
            // (see FileViewBazaarPlugin::versionState()).
            if (state != NormalVersion) {
                int pos = 4;
                const int length = filePath.length() - pos - 1;
                //conflicts annoyingly have a human readable text before the filename
                //TODO cover other conflict types
                if (filePath.startsWith(QLatin1String("C   Text conflict"))) {
                    filePath = filePath.mid(17, length);
                }
                filePath = baseDir + '/' + filePath.mid(pos, length);
                //remove type symbols from directories, links and executables
                if (filePath.endsWith('/') || filePath.endsWith('@') || filePath.endsWith('*')) {
                    filePath = filePath.left(filePath.length() - 1);
                }
                if (!filePath.isEmpty()) {
                    m_versionInfoHash.insert(filePath, state);
                }
            }
        }
    }
    if ((process.exitCode() != 0 || process.exitStatus() != QProcess::NormalExit)) {
            return false;
    }

    return true;
}

void FileViewBazaarPlugin::endRetrieval()
{
}

KVersionControlPlugin::ItemVersion FileViewBazaarPlugin::itemVersion(const KFileItem& item) const
{
    const QString itemUrl = item.localPath();
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
    }

    if (!item.isDir()) {
        // files that have not been listed by 'bzr status' or 'bzr ignored' (= m_versionInfoHash)
        // are under version control per definition
        return NormalVersion;
    }

    // The item is a directory. Check whether an item listed by 'bzr status' (= m_versionInfoHash)
    // is part of this directory. In this case a local modification should be indicated in the
    // directory already.
    const QString itemDir = itemUrl + QDir::separator();
    QHash<QString, ItemVersion>::const_iterator it = m_versionInfoHash.constBegin();
    while (it != m_versionInfoHash.constEnd()) {
        if (it.key().startsWith(itemDir)) {
            const ItemVersion state = m_versionInfoHash.value(it.key());
            if (state == LocallyModifiedVersion) {
                return LocallyModifiedVersion;
            }
        }
        ++it;
    }

    return NormalVersion;
}

QList<QAction*> FileViewBazaarPlugin::versionControlActions(const KFileItemList &items) const
{
    if (items.count() == 1 && items.first().isDir()) {
        QString directory = items.first().localPath();
        if (!directory.endsWith(QLatin1Char('/'))) {
            directory += QLatin1Char('/');
        }

        if (directory == m_contextDir) {
            return contextMenuDirectoryActions(directory);
        } else {
            return contextMenuFilesActions(items);
        }
    } else {
        return contextMenuFilesActions(items);
    }
}

QList<QAction*> FileViewBazaarPlugin::outOfVersionControlActions(const KFileItemList& items) const
{
    Q_UNUSED(items)

    return {};
}

QList<QAction*> FileViewBazaarPlugin::contextMenuFilesActions(const KFileItemList& items) const
{
    Q_ASSERT(!items.isEmpty());
    for (const KFileItem& item : items) {
        m_contextItems.append(item);
    }
    m_contextDir.clear();

    const bool noPendingOperation = !m_pendingOperation;
    if (noPendingOperation) {
        // iterate all items and check the version state to know which
        // actions can be enabled
        const int itemsCount = items.count();
        int versionedCount = 0;
        int editingCount = 0;
        for (const KFileItem& item : items) {
            const ItemVersion state = itemVersion(item);
            if (state != UnversionedVersion) {
                ++versionedCount;
            }

            switch (state) {
                case LocallyModifiedVersion:
                case ConflictingVersion:
                    ++editingCount;
                    break;
                default:
                    break;
            }
        }
        m_commitAction->setEnabled(editingCount > 0);
        m_addAction->setEnabled(versionedCount == 0);
        m_removeAction->setEnabled(versionedCount == itemsCount);
    } else {
        m_commitAction->setEnabled(false);
        m_addAction->setEnabled(false);
        m_removeAction->setEnabled(false);
    }
    m_updateAction->setEnabled(noPendingOperation);
    m_pullAction->setEnabled(noPendingOperation);
    m_pushAction->setEnabled(noPendingOperation);
    m_showLocalChangesAction->setEnabled(noPendingOperation);
    m_logAction->setEnabled(noPendingOperation);

    QList<QAction*> actions;
    actions.append(m_updateAction);
    actions.append(m_pullAction);
    actions.append(m_pushAction);
    actions.append(m_commitAction);
    actions.append(m_addAction);
    actions.append(m_removeAction);
    actions.append(m_showLocalChangesAction);
    actions.append(m_logAction);
    return actions;
}

QList<QAction*> FileViewBazaarPlugin::contextMenuDirectoryActions(const QString& directory) const
{
    m_contextDir = directory;
    m_contextItems.clear();

    // Only enable the actions if no commands are
    // executed currently (see slotOperationCompleted() and
    // startBazaarCommandProcess()).
    const bool enabled = !m_pendingOperation;
    m_updateAction->setEnabled(enabled);
    m_pullAction->setEnabled(enabled);
    m_pushAction->setEnabled(enabled);
    m_commitAction->setEnabled(enabled);
    m_addAction->setEnabled(enabled);
    m_showLocalChangesAction->setEnabled(enabled);
    m_logAction->setEnabled(enabled);

    QList<QAction*> actions;
    actions.append(m_updateAction);
    actions.append(m_pullAction);
    actions.append(m_pushAction);
    actions.append(m_commitAction);
    actions.append(m_addAction);
    actions.append(m_showLocalChangesAction);
    actions.append(m_logAction);
    return actions;
}

void FileViewBazaarPlugin::updateFiles()
{
    execBazaarCommand("qupdate", QStringList(),
                   i18nc("@info:status", "Updating Bazaar repository..."),
                   i18nc("@info:status", "Update of Bazaar repository failed."),
                   i18nc("@info:status", "Updated Bazaar repository."));
}

void FileViewBazaarPlugin::pullFiles()
{
    QStringList arguments = QStringList();
    arguments << "-d";
    execBazaarCommand("qpull", arguments,
                   i18nc("@info:status", "Pulling Bazaar repository..."),
                   i18nc("@info:status", "Pull of Bazaar repository failed."),
                   i18nc("@info:status", "Pulled Bazaar repository."));
}

void FileViewBazaarPlugin::pushFiles()
{
    QStringList arguments = QStringList();
    arguments << "-d";
    execBazaarCommand("qpush", arguments,
                   i18nc("@info:status", "Pushing Bazaar repository..."),
                   i18nc("@info:status", "Push of Bazaar repository failed."),
                   i18nc("@info:status", "Pushed Bazaar repository."));
}

void FileViewBazaarPlugin::showLocalChanges()
{
    execBazaarCommand("qdiff", QStringList(),
                   i18nc("@info:status", "Reviewing Changes..."),
                   i18nc("@info:status", "Review Changes failed."),
                   i18nc("@info:status", "Reviewed Changes."));
}

void FileViewBazaarPlugin::commitFiles()
{
    execBazaarCommand("qcommit", QStringList(),
                   i18nc("@info:status", "Committing Bazaar changes..."),
                   i18nc("@info:status", "Commit of Bazaar changes failed."),
                   i18nc("@info:status", "Committed Bazaar changes."));
}

void FileViewBazaarPlugin::addFiles()
{
    execBazaarCommand(QLatin1String("qadd"), QStringList(),
                   i18nc("@info:status", "Adding files to Bazaar repository..."),
                   i18nc("@info:status", "Adding of files to Bazaar repository failed."),
                   i18nc("@info:status", "Added files to Bazaar repository."));
}

void FileViewBazaarPlugin::removeFiles()
{
    execBazaarCommand(QLatin1String("remove"), QStringList(),
                   i18nc("@info:status", "Removing files from Bazaar repository..."),
                   i18nc("@info:status", "Removing of files from Bazaar repository failed."),
                   i18nc("@info:status", "Removed files from Bazaar repository."));
}

void FileViewBazaarPlugin::log()
{
    execBazaarCommand(QLatin1String("qlog"), QStringList(),
                   i18nc("@info:status", "Running Bazaar Log..."),
                   i18nc("@info:status", "Running Bazaar Log failed."),
                   i18nc("@info:status", "Bazaar Log closed."));
}

void FileViewBazaarPlugin::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_pendingOperation = false;

    if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
        Q_EMIT errorMessage(m_errorMsg);
    } else if (m_contextItems.isEmpty()) {
        Q_EMIT operationCompletedMessage(m_operationCompletedMsg);
        Q_EMIT itemVersionsChanged();
    } else {
        startBazaarCommandProcess();
    }
}

void FileViewBazaarPlugin::slotOperationError()
{
    // don't do any operation on other items anymore
    m_contextItems.clear();
    m_pendingOperation = false;

    Q_EMIT errorMessage(m_errorMsg);
}

void FileViewBazaarPlugin::execBazaarCommand(const QString& command,
                                       const QStringList& arguments,
                                       const QString& infoMsg,
                                       const QString& errorMsg,
                                       const QString& operationCompletedMsg)
{
    Q_EMIT infoMessage(infoMsg);

    QProcess process;
    process.start(QLatin1String("bzr"), {"plugins"});
    bool foundQbzr = false;
    while (process.waitForReadyRead()) {
        char buffer[512];
        while (process.readLine(buffer, sizeof(buffer)) > 0)  {
            QString output = QString(buffer).trimmed();
            if (output.startsWith(QLatin1String("qbzr"))) {
                foundQbzr = true;
                break;
            }
        }
    }

    if (!foundQbzr) {
        Q_EMIT infoMessage("Please Install QBzr");
        return;
    }
    
    m_command = command;
    m_arguments = arguments;
    m_errorMsg = errorMsg;
    m_operationCompletedMsg = operationCompletedMsg;

    startBazaarCommandProcess();
}

void FileViewBazaarPlugin::startBazaarCommandProcess()
{
    Q_ASSERT(m_process.state() == QProcess::NotRunning);
    m_pendingOperation = true;

    const QString program(QLatin1String("bzr"));
    QStringList arguments;
    arguments << m_command << m_arguments;
    if (!m_contextDir.isEmpty()) {
        arguments << m_contextDir;
        m_contextDir.clear();
    } else {
        const KFileItem item = m_contextItems.takeLast();
        arguments << item.localPath();
        // the remaining items of m_contextItems will be executed
        // after the process has finished (see slotOperationFinished())
    }
    m_process.start(program, arguments);
}

#include "fileviewbazaarplugin.moc"
