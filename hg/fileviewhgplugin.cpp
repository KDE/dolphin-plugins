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

#include "fileviewhgplugin.h"
#include "renamedialog.h"

#include <QDebug>
#include <kaction.h>
#include <kicon.h>
#include <klocale.h>

#include <KPluginFactory>
#include <KPluginLoader>
K_PLUGIN_FACTORY(FileViewHgPluginFactory, registerPlugin<FileViewHgPlugin>();)
K_EXPORT_PLUGIN(FileViewHgPluginFactory("fileviewhgplugin"))

FileViewHgPlugin::FileViewHgPlugin(QObject* parent, const QList<QVariant>& args):
    KVersionControlPlugin(parent),
    m_pendingOperation(false),
    m_addAction(0),
    m_removeAction(0),
    m_renameAction(0)
{
    Q_UNUSED(args);
    
    m_addAction = new KAction(this);
    m_addAction->setIcon(KIcon("list-add"));
    m_addAction->setText(i18nc("@action:inmenu", 
                "<application>Hg</application> Add"));
    connect(m_addAction, SIGNAL(triggered()),
            this, SLOT(addFiles()));



    m_removeAction = new KAction(this);
    m_removeAction->setIcon(KIcon("list-remove"));
    m_removeAction->setText(i18nc("@action:inmenu", 
                "<application>Hg</application> Remove"));
    connect(m_removeAction, SIGNAL(triggered()),
            this, SLOT(removeFiles()));


    m_renameAction = new KAction(this);
    m_renameAction->setIcon(KIcon("list-rename"));
    m_renameAction->setText(i18nc("@action:inmenu", 
                "<application>Hg</application> Rename"));
    connect(m_renameAction, SIGNAL(triggered()),
            this, SLOT(renameFile()));

    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotOperationCompleted(int, QProcess::ExitStatus)));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(slotOperationError()));
}

FileViewHgPlugin::~FileViewHgPlugin()
{
}

QString FileViewHgPlugin::fileName() const
{
    return QLatin1String(".hg");
}

bool FileViewHgPlugin::beginRetrieval(const QString& directory)
{
    int nTrimOutLeft = 0;
    QProcess process;
    process.setWorkingDirectory(directory);

    // Get repo root directory
    process.start(QLatin1String("hg root"));
    while (process.waitForReadyRead()) {
        char buffer[512];
        while (process.readLine(buffer, sizeof(buffer)) > 0)  {
            nTrimOutLeft = QString(buffer).trimmed().length(); 
        }
    }

    QString relativePrefix = directory.right(directory.length() - 
            nTrimOutLeft - 1);
    QString &hgBaseDir = m_hgBaseDir;
    hgBaseDir = directory.left(directory.length() -relativePrefix.length());
    
    // Clear all entries for this directory including the entries
    QMutableHashIterator<QString, VersionState> it(m_versionInfoHash);
    while (it.hasNext()) {
        it.next();
        if (it.key().startsWith(directory)) {
            it.remove();
        }
    }

    // Get status of files
    process.start(QLatin1String("hg status"));
    while (process.waitForReadyRead()) {
        char buffer[1024];
        while (process.readLine(buffer, sizeof(buffer)) > 0)  {
            const QString currentLine(buffer);
            char currentStatus = buffer[0];
            QString currentFile = currentLine.mid(2);
            if (currentFile.startsWith(relativePrefix)) {
                VersionState vs = NormalVersion;
                switch (currentStatus) {
                case 'A':  vs = AddedVersion; break;
                case 'M': vs = LocallyModifiedVersion; break;
                case '?': vs = UnversionedVersion; break;
                case 'R': vs = RemovedVersion; break;
                case 'I': vs = UnversionedVersion; break;
                case '!': continue;
                default: vs = NormalVersion; break;
                }
                if (vs != NormalVersion) {
                    QString filePath = hgBaseDir + currentFile;
                    filePath.remove(QChar('\n'));
                    m_versionInfoHash.insert(filePath, vs);
                }
            }
        }
    }
    return true;
}

void FileViewHgPlugin::endRetrieval()
{
}

KVersionControlPlugin::VersionState FileViewHgPlugin::versionState(const KFileItem& item)
{
    const QString itemUrl = item.localPath();
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
    } 
    if (item.isDir()) {
        QHash<QString, VersionState>::const_iterator it = m_versionInfoHash.constBegin();
        while (it != m_versionInfoHash.constEnd()) {
            if (it.key().startsWith(itemUrl)) {
                const VersionState state = m_versionInfoHash.value(it.key());
                if (state == LocallyModifiedVersion) {
                    return LocallyModifiedVersion;
                }
            }
            ++it;
        }
        return NormalVersion;
    }
    return NormalVersion;
}

QList<QAction*> FileViewHgPlugin::contextMenuActions(const KFileItemList& items)
{
    Q_ASSERT(!items.isEmpty());

    if (!m_pendingOperation){
        m_contextDir.clear();
        m_contextItems.clear();
        foreach (const KFileItem& item, items){
            m_contextItems.append(item);
        }

        //see which actions should be enabled
        int versionedCount = 0;
        int addableCount = 0;
        foreach (const KFileItem& item, items){
            const VersionState state = versionState(item);
            if (state != UnversionedVersion && state != RemovedVersion) {
                ++versionedCount;
            }
            if (state == UnversionedVersion || 
                    state == LocallyModifiedUnstagedVersion) {
                ++addableCount;
            }
        }

        m_addAction->setEnabled(addableCount == items.count());
        m_removeAction->setEnabled(versionedCount == items.count());
        m_renameAction->setEnabled(items.size() == 1 && 
            versionState(items.first()) != UnversionedVersion);
    }
    else {
        m_addAction->setEnabled(false);
        m_removeAction->setEnabled(false);
    }

    QList<QAction*> actions;
    actions.append(m_addAction);
    actions.append(m_removeAction);
    actions.append(m_renameAction);

    return actions;
}

QList<QAction*> FileViewHgPlugin::contextMenuActions(const QString& directory)
{
    if (!m_pendingOperation)
        m_contextDir = directory;
    return QList<QAction*>();
}

void FileViewHgPlugin::addFiles()
{
    execHgCommand(QLatin1String("add"), QStringList(),
            i18nc("@info:status", "Adding files to <application>Hg</application> repository..."),
            i18nc("@info:status", "Adding files to <application>Hg</application> repository failed."),
            i18nc("@info:status", "Added files to <application>Hg</application> repository."));
}

void FileViewHgPlugin::removeFiles()
{
    QStringList arguments;
    arguments << "--force"; //also remove files that have not been committed yet
    execHgCommand(QLatin1String("remove"), arguments,
            i18nc("@info:status", "Removing files from <application>Hg</application> repository..."),
            i18nc("@info:status", "Removing files from <application>Hg</application> repository failed."),
            i18nc("@info:status", "Removed files from <application>Hg</application> repository."));

}

void FileViewHgPlugin::renameFile()
{
    //QString source = KUrl::relativeUrl(KUrl(m_hgBaseDir), m_contextItems.first().url());
    QString source = m_contextItems.first().name();
    RenameDialog dialog(source);
    if (dialog.exec() == QDialog::Accepted) {
        m_process.setWorkingDirectory(m_contextItems.first().url().directory());
        qDebug() << m_contextItems.first().url().directory();

        m_errorMsg = i18nc("@info:status", 
              "Renaming of file in <application>Hg</application> repository failed.");
        m_operationCompletedMsg = i18nc("@info:status", 
              "Renamed file in <application>Hg</application> repository successfully.");
        emit infoMessage(i18nc("@info:status", 
                    "Renaming file in <application>Hg</application> repository."));

        m_pendingOperation = true;
        m_process.start(QString("hg rename %1 %2")
                .arg(source).arg(dialog.destination()));
    }
}

void FileViewHgPlugin::execHgCommand(const QString& hgCommand,
                                       const QStringList& arguments,
                                       const QString& infoMsg,
                                       const QString& errorMsg,
                                       const QString& operationCompletedMsg)
{
    emit infoMessage(infoMsg);

    m_command = hgCommand;
    m_arguments = arguments;
    m_errorMsg = errorMsg;
    m_operationCompletedMsg = operationCompletedMsg;

    startHgCommandProcess();
}


void FileViewHgPlugin::startHgCommandProcess()
{
    Q_ASSERT(!m_contextItems.isEmpty());
    Q_ASSERT(m_process.state() == QProcess::NotRunning);
    m_pendingOperation = true;

    const KFileItem item = m_contextItems.takeLast();
    m_process.setWorkingDirectory(item.url().directory());
    QStringList arguments;
    arguments << m_command;
    arguments << m_arguments;
    arguments << item.url().fileName();
    m_process.start(QLatin1String("hg"), arguments);
}

void FileViewHgPlugin::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_pendingOperation = false;

    QString message;

    if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
        emit errorMessage(message.isNull() ? m_errorMsg : message);
    } else if (m_contextItems.isEmpty()) {
        emit operationCompletedMessage(message.isNull() ? m_operationCompletedMsg : message);
        emit versionStatesChanged();
    } else {
        startHgCommandProcess();
    }
}

void FileViewHgPlugin::slotOperationError()
{
    // don't do any operation on other items anymore
    m_contextItems.clear();
    m_pendingOperation = false;

    emit errorMessage(m_errorMsg);
}

#include "fileviewhgplugin.moc"

