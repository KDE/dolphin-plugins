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
#include "pushdialog.h"
#include "pulldialog.h"
#include "mergedialog.h"
#include "bundledialog.h"
#include "exportdialog.h"
#include "importdialog.h"
#include "servedialog.h"
#include "backoutdialog.h"


#include <QtCore/QTextCodec>
#include <QtCore/QDir>
#include <kaction.h>
#include <kicon.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kservice.h>
#include <kmimetypetrader.h>

#include <KPluginFactory>
#include <KPluginLoader>
K_PLUGIN_FACTORY(FileViewHgPluginFactory, registerPlugin<FileViewHgPlugin>();)
K_EXPORT_PLUGIN(FileViewHgPluginFactory("fileviewhgplugin"))


//TODO: Build a proper status signal system to sync HgWrapper/Dialgs with this
//TODO: Check if working directory is commitable
//TODO: Organise Context Menu
//TODO: Show error messages and set their message approproately(hg output)
//TODO: Use i18nc rather thn i18c throughout plugin

FileViewHgPlugin::FileViewHgPlugin(QObject *parent, const QList<QVariant> &args):
    KVersionControlPlugin2(parent),
    m_addAction(0),
    m_removeAction(0),
    m_renameAction(0),
    m_commitAction(0),
    m_branchAction(0),
    m_tagAction(0),
    m_updateAction(0),
    m_cloneAction(0),
    m_createAction(0),
    m_configAction(0),
    m_pushAction(0),
    m_pullAction(0),
    m_revertAction(0),
    m_revertAllAction(0),
    m_isCommitable(false),
    m_hgWrapper(0),
    m_retrievalHgw(0)
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
    m_updateAction->setIcon(KIcon("svn-update"));
    m_updateAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Update"));
    connect(m_updateAction, SIGNAL(triggered()),
            this, SLOT(update()));

    m_globalConfigAction = new KAction(this);
    m_globalConfigAction->setIcon(KIcon("hg-config"));
    m_globalConfigAction->setText(i18nc("@action:inmenu",
                          "<application>Hg</application> Global Config"));
    connect(m_globalConfigAction, SIGNAL(triggered()),
            this, SLOT(global_config()));

    m_repoConfigAction = new KAction(this);
    m_repoConfigAction->setIcon(KIcon("hg-config"));
    m_repoConfigAction->setText(i18nc("@action:inmenu",
                      "<application>Hg</application> Repository Config"));
    connect(m_repoConfigAction, SIGNAL(triggered()),
            this, SLOT(repo_config()));

    m_pushAction = new KAction(this);
    m_pushAction->setIcon(KIcon("git-push"));
    m_pushAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Push"));
    connect(m_pushAction, SIGNAL(triggered()),
            this, SLOT(push()));

    m_pullAction = new KAction(this);
    m_pullAction->setIcon(KIcon("git-pull"));
    m_pullAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Pull"));
    connect(m_pullAction, SIGNAL(triggered()),
            this, SLOT(pull()));

    m_revertAction = new KAction(this);
    m_revertAction->setIcon(KIcon("hg-revert"));
    m_revertAction->setText(i18nc("@action:inmenu",
                                  "<application>Hg</application> Revert"));
    connect(m_revertAction, SIGNAL(triggered()),
            this, SLOT(revert()));

    m_revertAllAction = new KAction(this);
    m_revertAllAction->setIcon(KIcon("hg-revert"));
    m_revertAllAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Revert All"));
    connect(m_revertAllAction, SIGNAL(triggered()),
            this, SLOT(revertAll()));

    m_rollbackAction = new KAction(this);
    m_rollbackAction->setIcon(KIcon("hg-rollback"));
    m_rollbackAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Rollback"));
    connect(m_rollbackAction, SIGNAL(triggered()),
            this, SLOT(rollback()));

    m_mergeAction = new KAction(this);
    m_mergeAction->setIcon(KIcon("hg-merge"));
    m_mergeAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Merge"));
    connect(m_mergeAction, SIGNAL(triggered()),
            this, SLOT(merge()));

    m_bundleAction = new KAction(this);
    m_bundleAction->setIcon(KIcon("hg-bundle"));
    m_bundleAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Bundle"));
    connect(m_bundleAction, SIGNAL(triggered()),
            this, SLOT(bundle()));

    m_exportAction = new KAction(this);
    m_exportAction->setIcon(KIcon("hg-export"));
    m_exportAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Export"));
    connect(m_exportAction, SIGNAL(triggered()),
            this, SLOT(exportChangesets()));

    m_importAction = new KAction(this);
    m_importAction->setIcon(KIcon("hg-import"));
    m_importAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Import"));
    connect(m_importAction, SIGNAL(triggered()),
            this, SLOT(importChangesets()));

    m_unbundleAction = new KAction(this);
    m_unbundleAction->setIcon(KIcon("hg-unbundle"));
    m_unbundleAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Unbundle"));
    connect(m_unbundleAction, SIGNAL(triggered()),
            this, SLOT(unbundle()));

    m_serveAction = new KAction(this);
    m_serveAction->setIcon(KIcon("hg-serve"));
    m_serveAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Serve"));
    connect(m_serveAction, SIGNAL(triggered()),
            this, SLOT(serve()));

    m_backoutAction = new KAction(this);
    m_backoutAction->setIcon(KIcon("hg-backout"));
    m_backoutAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Backout"));
    connect(m_backoutAction, SIGNAL(triggered()),
            this, SLOT(backout()));

    m_diffAction = new KAction(this);
    m_diffAction->setIcon(KIcon("hg-diff"));
    m_diffAction->setText(i18nc("@action:inmenu",
                                 "<application>Hg</application> Diff"));
    connect(m_diffAction, SIGNAL(triggered()),
            this, SLOT(diff()));
}

FileViewHgPlugin::~FileViewHgPlugin()
{
}

void FileViewHgPlugin::createHgWrapper() const
{
    static bool created = false;

    if (created && m_hgWrapper != 0) {
        return;
    }

    created = true;

    m_hgWrapper = HgWrapper::instance();

    connect(m_hgWrapper,
            SIGNAL(primaryOperationFinished(int, QProcess::ExitStatus)),
            this, SLOT(slotOperationCompleted(int, QProcess::ExitStatus)));
    connect(m_hgWrapper, SIGNAL(primaryOperationError(QProcess::ProcessError)),
            this, SLOT(slotOperationError()));
}

QString FileViewHgPlugin::fileName() const
{
    return QLatin1String(".hg");
}

bool FileViewHgPlugin::beginRetrieval(const QString &directory)
{
    clearMessages();
    m_currentDir = directory;
    m_versionInfoHash.clear();
    //createHgWrapper();
    //m_hgWrapper->setCurrentDir(directory);
    //m_hgWrapper->getItemVersions(m_versionInfoHash);
    if (m_retrievalHgw == 0) {
        m_retrievalHgw = new HgWrapper;
    }
    m_retrievalHgw->setCurrentDir(directory);
    m_retrievalHgw->getItemVersions(m_versionInfoHash);
    return true;
}

void FileViewHgPlugin::endRetrieval()
{
}

KVersionControlPlugin2::ItemVersion FileViewHgPlugin::itemVersion(const KFileItem &item) const
{
    //FIXME: When folder is empty or all files within untracked.
    const QString itemUrl = item.localPath();
    if (item.isDir()) {
        QHash<QString, ItemVersion>::const_iterator it
                                    = m_versionInfoHash.constBegin();
        while (it != m_versionInfoHash.constEnd()) {
            if (it.key().startsWith(itemUrl)) {
                const ItemVersion state = m_versionInfoHash.value(it.key());
                if (state == LocallyModifiedVersion ||
                        state == AddedVersion ||
                        state == RemovedVersion) {
                    return LocallyModifiedVersion;
                }
            }
            ++it;
        }

        // Making folders with all files within untracked 'Unversioned'
        // will disable the context menu there. Will enable recursive
        // add however.
        QDir dir(item.localPath());
        QStringList filesInside = dir.entryList();
        foreach (const QString &fileName, filesInside) {
            if (fileName == "." || fileName == ".." ) {
                continue;
            }
            KUrl tempUrl(dir.absoluteFilePath(fileName));
            KFileItem tempFileItem(KFileItem::Unknown,
                    KFileItem::Unknown, tempUrl);
            if (itemVersion(tempFileItem) == NormalVersion) {
               return NormalVersion;
          }
        }
        return UnversionedVersion;
    }
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
    }
    return NormalVersion;
}

QList<QAction*> FileViewHgPlugin::actions(const KFileItemList &items) const
{
    //TODO: Make it work with universal context menu when imlpemented
    //      in dolphin
    kDebug() << items.count();
    if (items.count() == 1 && items.first().isDir()) {
        return directoryContextMenu(m_currentDir);
    }
    else {
        return itemContextMenu(items);
    }
    return QList<QAction*>();
}

QList<QAction*> FileViewHgPlugin::universalContextMenuActions(const QString &directory) const
{
    QList<QAction*> result;
    m_universalCurrentDirectory = directory;
    result.append(m_createAction);
    result.append(m_cloneAction);
    return result;
}

QList<QAction*> FileViewHgPlugin::itemContextMenu(const KFileItemList &items) const
{
    Q_ASSERT(!items.isEmpty());

    clearMessages();
    createHgWrapper();
    m_hgWrapper->setCurrentDir(m_currentDir);
    if (!m_hgWrapper->isBusy()) {
        m_contextItems.clear();
        foreach (const KFileItem &item, items) {
            m_contextItems.append(item);
        }

        //see which actions should be enabled
        int versionedCount = 0;
        int addableCount = 0;
        int revertableCount = 0;
        foreach (const KFileItem &item, items) {
            const ItemVersion state = itemVersion(item);
            if (state != UnversionedVersion && state != RemovedVersion) {
                ++versionedCount;
            }
            if (state == UnversionedVersion ||
                    state == LocallyModifiedUnstagedVersion) {
                ++addableCount;
            }
            if (state == LocallyModifiedVersion ||
                    state == AddedVersion ||
                    state == RemovedVersion) {
                ++revertableCount;
            }
        }

        m_addAction->setEnabled(addableCount == items.count());
        m_removeAction->setEnabled(versionedCount == items.count());
        m_revertAction->setEnabled(revertableCount == items.count());
        m_diffAction->setEnabled(revertableCount == items.count() &&
                items.size() == 1);
        m_renameAction->setEnabled(items.size() == 1 &&
                itemVersion(items.first()) != UnversionedVersion);
    }
    else {
        m_addAction->setEnabled(false);
        m_removeAction->setEnabled(false);
        m_renameAction->setEnabled(false);
        m_revertAction->setEnabled(false);
        m_diffAction->setEnabled(false);
    }

    QList<QAction*> actions;
    actions.append(m_addAction);
    actions.append(m_removeAction);
    actions.append(m_renameAction);
    actions.append(m_revertAction);
    actions.append(m_diffAction);

    return actions;
}

QList<QAction*> FileViewHgPlugin::directoryContextMenu(const QString &directory) const
{
    QList<QAction*> actions;
    clearMessages();
    createHgWrapper();
    m_hgWrapper->setCurrentDir(directory);
    if (!m_hgWrapper->isBusy()) {
        actions.append(m_commitAction);
    }
    actions.append(m_pushAction);
    actions.append(m_pullAction);
    actions.append(m_diffAction);
    actions.append(m_updateAction);
    actions.append(m_branchAction);
    actions.append(m_tagAction);
    actions.append(m_mergeAction);
    actions.append(m_revertAllAction);
    actions.append(m_rollbackAction);
    actions.append(m_backoutAction);
    actions.append(m_bundleAction);
    actions.append(m_unbundleAction);
    actions.append(m_exportAction);
    actions.append(m_importAction);
    actions.append(m_serveAction);
    actions.append(m_globalConfigAction);
    actions.append(m_repoConfigAction);
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

    int answer = KMessageBox::questionYesNo(0, i18nc("@message:yesorno",
                    "Would you like to remove selected files "
                    "from the repository?"));
    if (answer == KMessageBox::No) {
        return;
    }

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
            "Commit to <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
            "Committed to <application>Hg</application> repository.");
    emit infoMessage(i18nc("@info:status",
            "Commit <application>Hg</application> repository."));

    HgCommitDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        emit itemVersionsChanged();
    };
}

void FileViewHgPlugin::tag()
{
    m_errorMsg = i18nc("@info:status",
           "Tag operation in <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
           "Tagging operation in <application>Hg</application> repository is successful.");
    emit infoMessage(i18nc("@info:status",
           "Tagging operation in <application>Hg</application> repository."));

    HgTagDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::update()
{
    m_errorMsg = i18nc("@info:status",
           "Update of <application>Hg</application> working directory failed.");
    m_operationCompletedMsg = i18nc("@info:status",
           "Update of <application>Hg</application> working directory is successful.");
    emit infoMessage(i18nc("@info:status",
           "Updating <application>Hg</application> working directory."));

    HgUpdateDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::branch()
{
    m_errorMsg = i18nc("@info:status",
           "Branch operation on <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
           "Branch operation on <application>Hg</application> repository completed successfully.");
    emit infoMessage(i18nc("@info:status",
           "Branch operation on <application>Hg</application> repository."));

    HgBranchDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::clone()
{
    clearMessages();
    HgCloneDialog dialog(m_universalCurrentDirectory);
    dialog.exec();
}

void FileViewHgPlugin::create()
{
    clearMessages();
    HgCreateDialog dialog(m_universalCurrentDirectory);
    dialog.exec();
}

void FileViewHgPlugin::global_config()
{
    clearMessages();
    HgConfigDialog diag(HgConfig::GlobalConfig);
    diag.exec();
}

void FileViewHgPlugin::repo_config()
{
    clearMessages();
    HgConfigDialog diag(HgConfig::RepoConfig);
    diag.exec();
}

void FileViewHgPlugin::push()
{
    clearMessages();
    HgPushDialog diag;
    diag.exec();
}

void FileViewHgPlugin::pull()
{
    clearMessages();
    HgPullDialog diag;
    diag.exec();
}

void FileViewHgPlugin::merge()
{
    clearMessages();
    HgMergeDialog diag;
    diag.exec();
}

void FileViewHgPlugin::bundle()
{
    clearMessages();
    HgBundleDialog diag;
    diag.exec();
}

void FileViewHgPlugin::unbundle()
{
    clearMessages();
    QString bundle = KFileDialog::getOpenFileName();
    if (bundle.isEmpty()) {
        return;
    }

    QStringList args;
    args << bundle;
    if (m_hgWrapper->executeCommandTillFinished(QLatin1String("unbundle"), args)) {
    }
    else {
        KMessageBox::error(0, m_hgWrapper->readAllStandardError());
    }
}

void FileViewHgPlugin::importChangesets()
{
    clearMessages();
    HgImportDialog diag;
    diag.exec();
}

void FileViewHgPlugin::exportChangesets()
{
    clearMessages();
    HgExportDialog diag;
    diag.exec();
}

void FileViewHgPlugin::revert()
{
    clearMessages();
    int answer = KMessageBox::questionYesNo(0, i18nc("@message:yesorno",
                    "Would you like to revert changes "
                    "made to selected files?"));
    if (answer == KMessageBox::No) {
        return;
    }

    QString infoMsg = i18nc("@info:status",
         "Reverting files in <application>Hg</application> repository...");
    m_errorMsg = i18nc("@info:status",
         "Reverting files in <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
         "Reverting files in <application>Hg</application> repository completed successfully.");

    emit infoMessage(infoMsg);
    m_hgWrapper->revert(m_contextItems);
}

void FileViewHgPlugin::revertAll()
{
    int answer = KMessageBox::questionYesNo(0, i18nc("@message:yesorno",
                    "Would you like to revert all changes "
                    "made to current working directory?"));
    if (answer == KMessageBox::No) {
        return;
    }

    QString infoMsg = i18nc("@info:status",
         "Reverting files in <application>Hg</application> repository...");
    m_errorMsg = i18nc("@info:status",
         "Reverting files in <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
         "Reverting files in <application>Hg</application> repository completed successfully.");

    emit infoMessage(infoMsg);
    m_hgWrapper->revertAll();
}

void FileViewHgPlugin::diff()
{
    QString infoMsg = i18nc("@info:status",
         "Generating diff for <application>Hg</application> repository...");
    m_errorMsg = i18nc("@info:status",
         "Could not get <application>Hg</application> repository diff.");
    m_operationCompletedMsg = i18nc("@info:status",
         "Generated <application>Hg</application> diff successfully.");

    emit infoMessage(infoMsg);

    QStringList args;
    args << QLatin1String("--config");
    args << QLatin1String("extensions.hgext.extdiff=");
    args << QLatin1String("-p");
    args << this->visualDiffExecPath();

    if (m_contextItems.length() == 1) {
        args << m_contextItems.takeFirst().localPath();
    }

    m_hgWrapper->executeCommand(QLatin1String("extdiff"), args);
}

void FileViewHgPlugin::serve()
{
    clearMessages();
    HgServeDialog diag;
    diag.exec();
}

void FileViewHgPlugin::backout()
{
    clearMessages();
    m_hgWrapper = HgWrapper::instance();
    if (!m_hgWrapper->isWorkingDirectoryClean()) {
        KMessageBox::error(0, i18nc("@message:error",
                      "abort: Uncommitted changes in working directory!"));
        return;
    }

    HgBackoutDialog diag;
    diag.exec();
}

void FileViewHgPlugin::rollback()
{
    // execute a dry run rollback first to see if there is anything to
    // be rolled back, or check what will be rolled back
    if (!m_hgWrapper->rollback(true)) {
        KMessageBox::error(0, i18nc("@info:message", "No rollback "
                                        "information available!"));
        return;
    }
    // get what will be rolled back
    QString lastTransaction = m_hgWrapper->readAllStandardOutput();
    int cutOfFrom = lastTransaction.indexOf(QRegExp("\\d"));
    lastTransaction = lastTransaction.mid(cutOfFrom);

    // ask
    int answer = KMessageBox::questionYesNo(0, i18nc("@message:yesorno",
                    "Would you like to rollback last transaction?")
                        + "\nrevision: " + lastTransaction);
    if (answer == KMessageBox::No) {
        return;
    }

    QString infoMsg = i18nc("@info:status",
         "Executing Rollback <application>Hg</application> repository...");
    m_errorMsg = i18nc("@info:status",
         "Rollback of <application>Hg</application> repository failed.");
    m_operationCompletedMsg = i18nc("@info:status",
         "Rollback of <application>Hg</application> repository completed successfully.");

    emit infoMessage(infoMsg);
    m_hgWrapper->rollback();
    KMessageBox::information(0, m_hgWrapper->readAllStandardOutput());
    emit itemVersionsChanged();
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

void FileViewHgPlugin::clearMessages() const
{
    m_operationCompletedMsg.clear();
    m_errorMsg.clear();
}

QString FileViewHgPlugin::visualDiffExecPath()
{
    KUrl url = KUrl::fromPath(QDir::homePath());
    url.addPath(".dolphin-hg");
    KConfig config(url.path(), KConfig::SimpleConfig);

    KConfigGroup group(&config, QLatin1String("diff"));
    QString result = group.readEntry(QLatin1String("exec"), QString()).trimmed();

    if (result.length() > 0) {
        return result;
    }

    KService::List services = KMimeTypeTrader::self()->query("text/x-diff");
    return services.first()->exec().split(' ').takeFirst();
}

#include "fileviewhgplugin.moc"

