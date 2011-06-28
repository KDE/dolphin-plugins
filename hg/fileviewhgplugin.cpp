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
#include "hgconfig.h"
#include "configdialog.h"
#include "renamedialog.h"
#include "commitdialog.h"
#include "branchdialog.h"
#include "tagdialog.h"
#include "updatedialog.h"
#include "clonedialog.h"
#include "createdialog.h"


#include <QtCore/QTextCodec>
#include <QtCore/QDir>
#include <kaction.h>
#include <kicon.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>

#include <KPluginFactory>
#include <KPluginLoader>
K_PLUGIN_FACTORY(FileViewHgPluginFactory, registerPlugin<FileViewHgPlugin>();)
K_EXPORT_PLUGIN(FileViewHgPluginFactory("fileviewhgplugin"))


//TODO: Emit versionStatesChanged() signals wherever required
//TODO: Build a proper status signal system to sync HgWrapper/Dialgs with this

FileViewHgPlugin::FileViewHgPlugin(QObject *parent, const QList<QVariant> &args):
    KVersionControlPlugin(parent),
    m_addAction(0),
    m_removeAction(0),
    m_renameAction(0),
    m_commitAction(0),
    m_branchAction(0),
    m_tagAction(0)
{
    Q_UNUSED(args);
    m_hgWrapper = HgWrapper::instance();

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

    m_commitAction = new KAction(this);
    m_commitAction->setIcon(KIcon("svn-commit"));
    m_commitAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Commit"));
    connect(m_commitAction, SIGNAL(triggered()),
            this, SLOT(commit()));

    m_tagAction = new KAction(this);
    m_tagAction->setIcon(KIcon("svn-tag"));
    m_tagAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Tag"));
    connect(m_tagAction, SIGNAL(triggered()),
            this, SLOT(tag()));

    m_branchAction = new KAction(this);
    m_branchAction->setIcon(KIcon("svn-branch"));
    m_branchAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Branch"));
    connect(m_branchAction, SIGNAL(triggered()),
            this, SLOT(branch()));

    m_cloneAction = new KAction(this);
    m_cloneAction->setIcon(KIcon("hg-clone"));
    m_cloneAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Clone"));
    connect(m_cloneAction, SIGNAL(triggered()),
            this, SLOT(clone()));

    m_createAction = new KAction(this);
    m_createAction->setIcon(KIcon("hg-create"));
    m_createAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Init"));
    connect(m_createAction, SIGNAL(triggered()),
            this, SLOT(create()));

    m_updateAction = new KAction(this);
    m_updateAction->setIcon(KIcon("hg-update"));
    m_updateAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Update"));
    connect(m_updateAction, SIGNAL(triggered()),
            this, SLOT(update()));

    m_configAction = new KAction(this);
    m_configAction->setIcon(KIcon("hg-config"));
    m_configAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Config"));
    connect(m_configAction, SIGNAL(triggered()),
            this, SLOT(config()));

    connect(m_hgWrapper, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotOperationCompleted(int, QProcess::ExitStatus)));
    connect(m_hgWrapper, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(slotOperationError()));
}

FileViewHgPlugin::~FileViewHgPlugin()
{
}

QString FileViewHgPlugin::fileName() const
{
    return QLatin1String(".hg");
}

bool FileViewHgPlugin::beginRetrieval(const QString &directory)
{
    m_versionInfoHash.clear();
    m_hgWrapper->setCurrentDir(directory);
    QHash<QString, HgVersionState> &hgVsHash 
        = m_hgWrapper->getVersionStates(true);

    QMutableHashIterator<QString, HgVersionState> it(hgVsHash);
    while (it.hasNext()) {
        it.next();
        switch (it.value()) {
        case HgCleanVersion:
            continue;
        case HgRemovedVersion:
            continue;
        case HgAddedVersion:
            m_versionInfoHash.insert(it.key(), AddedVersion);
            break;
        case HgModifiedVersion:
            m_versionInfoHash.insert(it.key(), LocallyModifiedVersion);
            break;
        case HgUntrackedVersion:
            m_versionInfoHash.insert(it.key(), UnversionedVersion);
            break;
        default:
            continue;
        }
    }

    return true;
}

void FileViewHgPlugin::endRetrieval()
{
}

KVersionControlPlugin::VersionState FileViewHgPlugin::versionState(const KFileItem &item)
{
    //FIXME: When folder is empty or all files within untracked.
    const QString itemUrl = item.localPath();
    if (item.isDir()) {
                QHash<QString, VersionState>::const_iterator it 
                                    = m_versionInfoHash.constBegin();
        while (it != m_versionInfoHash.constEnd()) {
            if (it.key().startsWith(itemUrl)) {
                const VersionState state = m_versionInfoHash.value(it.key());
                if (state == LocallyModifiedVersion ||
                        state == AddedVersion ||
                        state == RemovedVersion) {
                    return LocallyModifiedVersion;
                }
            }
            ++it;
        }

        // Making folders with all files within untracked 'Unversioned'
        // will disable the context menu there.
        //
        //QDir dir(item.localPath());
        //QStringList filesInside = dir.entryList();
        //foreach (const QString &fileName, filesInside) {
        //    if (fileName == "." || fileName == ".." ) {
        //        continue;
        //    }
        //    KUrl tempUrl(dir.absoluteFilePath(fileName));
        //    KFileItem tempFileItem(KFileItem::Unknown,
        //            KFileItem::Unknown, tempUrl);
        //    if (versionState(tempFileItem) == NormalVersion) {
        //       return NormalVersion;
        //  }
        //}
        //return UnversionedVersion;
        return NormalVersion;
    }
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
    }
    return NormalVersion;
}

QList<QAction*> FileViewHgPlugin::universalContextMenuActions(const QString &directory) 
{
    QList<QAction*> result;
    kDebug() << "Current directory set to: " << directory;
    m_hgWrapper->setCurrentDir(directory);
    result.append(m_createAction);
    result.append(m_cloneAction);
    return result;
}

QList<QAction*> FileViewHgPlugin::contextMenuActions(const KFileItemList &items)
{
    Q_ASSERT(!items.isEmpty());

    if (!m_hgWrapper->isBusy()) {
        m_contextItems.clear();
        foreach (const KFileItem &item, items) {
            m_contextItems.append(item);
        }

        //see which actions should be enabled
        int versionedCount = 0;
        int addableCount = 0;
        foreach (const KFileItem &item, items) {
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
        m_renameAction->setEnabled(false);
    }

    QList<QAction *> actions;
    actions.append(m_addAction);
    actions.append(m_removeAction);
    actions.append(m_renameAction);

    return actions;
}

QList<QAction*> FileViewHgPlugin::contextMenuActions(const QString &directory)
{
    QList<QAction *> actions;
    if (!m_hgWrapper->isBusy()) {
        actions.append(m_commitAction);
    }
    actions.append(m_updateAction);
    actions.append(m_branchAction);
    actions.append(m_tagAction);
    actions.append(m_configAction);
    return actions;
}

void FileViewHgPlugin::addFiles()
{
    Q_ASSERT(!m_contextItems.isEmpty());
    QString infoMsg = i18nc("@info:status",
         "Adding files to <application>Hg</application> repository...");
    m_errorMsg = i18nc("@info:status",
         "Adding files to <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
         "Added files to <application>Hg</application> repository.");

    emit infoMessage(infoMsg);
    m_hgWrapper->addFiles(m_contextItems);
}

void FileViewHgPlugin::removeFiles()
{
    Q_ASSERT(!m_contextItems.isEmpty());
    QString infoMsg = i18nc("@info:status",
         "Removing files from <application>Hg</application> repository...");
    m_errorMsg = i18nc("@info:status",
         "Removing files from <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
         "Removed files from <application>Hg</application> repository.");

    emit infoMessage(infoMsg);
    m_hgWrapper->removeFiles(m_contextItems);
}


void FileViewHgPlugin::renameFile()
{
    Q_ASSERT(m_contextItems.size() == 1);

    m_errorMsg = i18nc("@info:status",
        "Renaming of file in <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
        "Renamed file in <application>Hg</application> repository successfully.");
    emit infoMessage(i18nc("@info:status",
        "Renaming file in <application>Hg</application> repository."));

    HgRenameDialog dialog(m_contextItems.first());
    dialog.exec();
    m_contextItems.clear();
}


void FileViewHgPlugin::commit()
{
    //FIXME: Disable emitting of status messages when executing sub tasks.
    m_errorMsg = i18nc("@info:status", 
            "Commit to  <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
            "Commit to <application>Hg</application> repository successfull.");
    emit infoMessage(i18nc("@info:status",
            "Commit <application>Hg</application> repository."));

    HgCommitDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::tag()
{
    HgTagDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::update()
{
    HgUpdateDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::branch()
{
    HgBranchDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::clone()
{
    HgCloneDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::create()
{
    HgCreateDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::config()
{
    HgConfigDialog diag;
    diag.exec();
}

void FileViewHgPlugin::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
        emit errorMessage(m_errorMsg);
    }
    else {
        m_contextItems.clear();
        emit operationCompletedMessage(m_operationCompletedMsg);
        emit versionStatesChanged();
    }
}

void FileViewHgPlugin::slotOperationError()
{
    m_contextItems.clear();
    emit errorMessage(m_errorMsg);
}

#include "fileviewhgplugin.moc"
