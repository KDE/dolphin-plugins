/***************************************************************************
 *   Copyright (C) 2011 by Vishesh Yadav <vishesh3y@gmail.com>             *
 *   Copyright (C) 2015 by Tomasz Bojczuk <seelook@gmail.com>              *
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


#include <QTextCodec>
#include <QDir>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QDebug>
#include <KMessageBox>
#include <QFileDialog>
#include <KConfig>
#include <KConfigGroup>
#include <KService>
#include <KMimeTypeTrader>

#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY(FileViewHgPluginFactory, registerPlugin<FileViewHgPlugin>();)


//TODO: Build a proper status signal system to sync HgWrapper/Dialogs with this
//TODO: Show error messages and set their message appropriately(hg output)
//TODO: Use xi18nc rather than i18c throughout plugin

FileViewHgPlugin::FileViewHgPlugin(QObject *parent, const QList<QVariant> &args):
    KVersionControlPlugin(parent),
    m_mainContextMenu(0),
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
    m_globalConfigAction(0),
    m_repoConfigAction(0),
    m_pushAction(0),
    m_pullAction(0),
    m_revertAction(0),
    m_revertAllAction(0),
    m_rollbackAction(0),
    m_mergeAction(0),
    m_bundleAction(0),
    m_exportAction(0),
    m_unbundleAction(0),
    m_importAction(0),
    m_diffAction(0),
    m_serveAction(0),
    m_backoutAction(0),
    m_isCommitable(false),
    m_hgWrapper(0),
    m_retrievalHgw(0)
{
    Q_UNUSED(args);

    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");

    m_addAction = new QAction(this);
    m_addAction->setIcon(QIcon::fromTheme("list-add"));
    m_addAction->setText(xi18nc("@action:inmenu",
                               "<application>Hg</application> Add"));
    connect(m_addAction, &QAction::triggered,
            this, &FileViewHgPlugin::addFiles);

    m_removeAction = new QAction(this);
    m_removeAction->setIcon(QIcon::fromTheme("list-remove"));
    m_removeAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Remove"));
    connect(m_removeAction, &QAction::triggered,
            this, &FileViewHgPlugin::removeFiles);

    m_renameAction = new QAction(this);
    m_renameAction->setIcon(QIcon::fromTheme("list-rename"));
    m_renameAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Rename"));
    connect(m_renameAction, &QAction::triggered,
            this, &FileViewHgPlugin::renameFile);

    m_commitAction = new QAction(this);
    m_commitAction->setIcon(QIcon::fromTheme("svn-commit"));
    m_commitAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Commit"));
    connect(m_commitAction, &QAction::triggered,
            this, &FileViewHgPlugin::commit);

    m_tagAction = new QAction(this);
    m_tagAction->setIcon(QIcon::fromTheme("svn-tag"));
    m_tagAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Tag"));
    connect(m_tagAction, &QAction::triggered,
            this, &FileViewHgPlugin::tag);

    m_branchAction = new QAction(this);
    m_branchAction->setIcon(QIcon::fromTheme("svn-branch"));
    m_branchAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Branch"));
    connect(m_branchAction, &QAction::triggered,
            this, &FileViewHgPlugin::branch);

    m_cloneAction = new QAction(this);
    m_cloneAction->setIcon(QIcon::fromTheme("hg-clone"));
    m_cloneAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Clone"));
    connect(m_cloneAction, &QAction::triggered,
            this, &FileViewHgPlugin::clone);

    m_createAction = new QAction(this);
    m_createAction->setIcon(QIcon::fromTheme("hg-create"));
    m_createAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Init"));
    connect(m_createAction, &QAction::triggered,
            this, &FileViewHgPlugin::create);

    m_updateAction = new QAction(this);
    m_updateAction->setIcon(QIcon::fromTheme("svn-update"));
    m_updateAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Update"));
    connect(m_updateAction, &QAction::triggered,
            this, &FileViewHgPlugin::update);

    m_globalConfigAction = new QAction(this);
    m_globalConfigAction->setIcon(QIcon::fromTheme("hg-config"));
    m_globalConfigAction->setText(xi18nc("@action:inmenu",
                          "<application>Hg</application> Global Config"));
    connect(m_globalConfigAction, &QAction::triggered,
            this, &FileViewHgPlugin::global_config);

    m_repoConfigAction = new QAction(this);
    m_repoConfigAction->setIcon(QIcon::fromTheme("hg-config"));
    m_repoConfigAction->setText(xi18nc("@action:inmenu",
                      "<application>Hg</application> Repository Config"));
    connect(m_repoConfigAction, &QAction::triggered,
            this, &FileViewHgPlugin::repo_config);

    m_pushAction = new QAction(this);
    m_pushAction->setIcon(QIcon::fromTheme("git-push"));
    m_pushAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Push"));
    connect(m_pushAction, &QAction::triggered,
            this, &FileViewHgPlugin::push);

    m_pullAction = new QAction(this);
    m_pullAction->setIcon(QIcon::fromTheme("git-pull"));
    m_pullAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Pull"));
    connect(m_pullAction, &QAction::triggered,
            this, &FileViewHgPlugin::pull);

    m_revertAction = new QAction(this);
    m_revertAction->setIcon(QIcon::fromTheme("hg-revert"));
    m_revertAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Revert"));
    connect(m_revertAction, &QAction::triggered,
            this, &FileViewHgPlugin::revert);

    m_revertAllAction = new QAction(this);
    m_revertAllAction->setIcon(QIcon::fromTheme("hg-revert"));
    m_revertAllAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Revert All"));
    connect(m_revertAllAction, &QAction::triggered,
            this, &FileViewHgPlugin::revertAll);

    m_rollbackAction = new QAction(this);
    m_rollbackAction->setIcon(QIcon::fromTheme("hg-rollback"));
    m_rollbackAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Rollback"));
    connect(m_rollbackAction, &QAction::triggered,
            this, &FileViewHgPlugin::rollback);

    m_mergeAction = new QAction(this);
    m_mergeAction->setIcon(QIcon::fromTheme("hg-merge"));
    m_mergeAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Merge"));
    connect(m_mergeAction, &QAction::triggered,
            this, &FileViewHgPlugin::merge);

    m_bundleAction = new QAction(this);
    m_bundleAction->setIcon(QIcon::fromTheme("hg-bundle"));
    m_bundleAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Bundle"));
    connect(m_bundleAction, &QAction::triggered,
            this, &FileViewHgPlugin::bundle);

    m_exportAction = new QAction(this);
    m_exportAction->setIcon(QIcon::fromTheme("hg-export"));
    m_exportAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Export"));
    connect(m_exportAction, &QAction::triggered,
            this, &FileViewHgPlugin::exportChangesets);

    m_importAction = new QAction(this);
    m_importAction->setIcon(QIcon::fromTheme("hg-import"));
    m_importAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Import"));
    connect(m_importAction, &QAction::triggered,
            this, &FileViewHgPlugin::importChangesets);

    m_unbundleAction = new QAction(this);
    m_unbundleAction->setIcon(QIcon::fromTheme("hg-unbundle"));
    m_unbundleAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Unbundle"));
    connect(m_unbundleAction, &QAction::triggered,
            this, &FileViewHgPlugin::unbundle);

    m_serveAction = new QAction(this);
    m_serveAction->setIcon(QIcon::fromTheme("hg-serve"));
    m_serveAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Serve"));
    connect(m_serveAction, &QAction::triggered,
            this, &FileViewHgPlugin::serve);

    m_backoutAction = new QAction(this);
    m_backoutAction->setIcon(QIcon::fromTheme("hg-backout"));
    m_backoutAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Backout"));
    connect(m_backoutAction, &QAction::triggered,
            this, &FileViewHgPlugin::backout);

    m_diffAction = new QAction(this);
    m_diffAction->setIcon(QIcon::fromTheme("hg-diff"));
    m_diffAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Diff"));
    connect(m_diffAction, &QAction::triggered,
            this, &FileViewHgPlugin::diff);

    /* Submenu to make the main menu less cluttered */
    m_mainContextMenu = new QMenu;
    m_mainContextMenu->addAction(m_updateAction);
    m_mainContextMenu->addAction(m_branchAction);
    m_mainContextMenu->addAction(m_tagAction);
    m_mainContextMenu->addAction(m_mergeAction);
    m_mainContextMenu->addAction(m_revertAllAction);
    m_mainContextMenu->addAction(m_rollbackAction);
    m_mainContextMenu->addAction(m_backoutAction);
    m_mainContextMenu->addAction(m_bundleAction);
    m_mainContextMenu->addAction(m_unbundleAction);
    m_mainContextMenu->addAction(m_exportAction);
    m_mainContextMenu->addAction(m_importAction);
    m_mainContextMenu->addAction(m_serveAction);
    m_mainContextMenu->addAction(m_globalConfigAction);
    m_mainContextMenu->addAction(m_repoConfigAction);

    m_menuAction = new QAction(this);
    m_menuAction->setIcon(QIcon::fromTheme("hg-main"));
    m_menuAction->setText(xi18nc("@action:inmenu",
                                  "<application>Mercurial</application>"));
    m_menuAction->setMenu(m_mainContextMenu);
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

    connect(m_hgWrapper, &HgWrapper::primaryOperationFinished,
            this, &FileViewHgPlugin::slotOperationCompleted);
    connect(m_hgWrapper, &HgWrapper::primaryOperationError,
            this, &FileViewHgPlugin::slotOperationError);
}

QString FileViewHgPlugin::fileName() const
{
    return QLatin1String(".hg");
}

QString FileViewHgPlugin::localRepositoryRoot(const QString& directory) const
{
    QProcess process;
    process.setWorkingDirectory(directory);
    process.start("hg", {"root"});
    if (process.waitForReadyRead(100) && process.exitCode() == 0) {
        return QString::fromUtf8(process.readAll().chopped(1));
    }
    return QString();
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

KVersionControlPlugin::ItemVersion FileViewHgPlugin::itemVersion(const KFileItem &item) const
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
        const QStringList filesInside = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        for (const QString &fileName : filesInside) {
            const QUrl tempUrl(dir.absoluteFilePath(fileName));
            KFileItem tempFileItem(tempUrl);
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

QList<QAction*> FileViewHgPlugin::versionControlActions(const KFileItemList &items) const
{
    //TODO: Make it work with universal context menu when implemented
    //      in dolphin
    qDebug() << items.count();
    if (items.count() == 1 && items.first().isDir()) {
        return directoryContextMenu(m_currentDir);
    }
    else {
        return itemContextMenu(items);
    }
    return QList<QAction*>();
}

QList<QAction*> FileViewHgPlugin::outOfVersionControlActions(const KFileItemList &items) const
{
    Q_UNUSED(items)

    return {};
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
        for (const KFileItem &item : items) {
            m_contextItems.append(item);
        }

        //see which actions should be enabled
        int versionedCount = 0;
        int addableCount = 0;
        int revertableCount = 0;
        for (const KFileItem &item : items) {
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
    actions.append(m_menuAction);
    return actions;
}

void FileViewHgPlugin::addFiles()
{
    Q_ASSERT(!m_contextItems.isEmpty());
    QString infoMsg = xi18nc("@info:status",
         "Adding files to <application>Hg</application> repository...");
    m_errorMsg = xi18nc("@info:status",
         "Adding files to <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
         "Added files to <application>Hg</application> repository.");

    Q_EMIT infoMessage(infoMsg);
    m_hgWrapper->addFiles(m_contextItems);
    Q_EMIT itemVersionsChanged();
}

void FileViewHgPlugin::removeFiles()
{
    Q_ASSERT(!m_contextItems.isEmpty());

    int answer = KMessageBox::questionYesNo(0, xi18nc("@message:yesorno",
                    "Would you like to remove selected files "
                    "from the repository?"));
    if (answer == KMessageBox::No) {
        return;
    }

    QString infoMsg = xi18nc("@info:status",
         "Removing files from <application>Hg</application> repository...");
    m_errorMsg = xi18nc("@info:status",
         "Removing files from <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
         "Removed files from <application>Hg</application> repository.");

    Q_EMIT infoMessage(infoMsg);
    m_hgWrapper->removeFiles(m_contextItems);
}


void FileViewHgPlugin::renameFile()
{
    Q_ASSERT(m_contextItems.size() == 1);

    m_errorMsg = xi18nc("@info:status",
        "Renaming of file in <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
        "Renamed file in <application>Hg</application> repository successfully.");
    Q_EMIT infoMessage(xi18nc("@info:status",
        "Renaming file in <application>Hg</application> repository."));

    HgRenameDialog dialog(m_contextItems.first());
    dialog.exec();
    m_contextItems.clear();
}


void FileViewHgPlugin::commit()
{
    if (m_hgWrapper->isWorkingDirectoryClean()) {
        KMessageBox::information(0, xi18nc("@message", "No changes for commit!"));
        return;
    }
    //FIXME: Disable emitting of status messages when executing sub tasks.
    m_errorMsg = xi18nc("@info:status",
            "Commit to <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
            "Committed to <application>Hg</application> repository.");
    Q_EMIT infoMessage(xi18nc("@info:status",
            "Commit <application>Hg</application> repository."));

    HgCommitDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        Q_EMIT itemVersionsChanged();
    };
}

void FileViewHgPlugin::tag()
{
    m_errorMsg = xi18nc("@info:status",
           "Tag operation in <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
           "Tagging operation in <application>Hg</application> repository is successful.");
    Q_EMIT infoMessage(xi18nc("@info:status",
           "Tagging operation in <application>Hg</application> repository."));

    HgTagDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::update()
{
    m_errorMsg = xi18nc("@info:status",
           "Update of <application>Hg</application> working directory failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
           "Update of <application>Hg</application> working directory is successful.");
    Q_EMIT infoMessage(xi18nc("@info:status",
           "Updating <application>Hg</application> working directory."));

    HgUpdateDialog dialog;
    dialog.exec();
}

void FileViewHgPlugin::branch()
{
    m_errorMsg = xi18nc("@info:status",
           "Branch operation on <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
           "Branch operation on <application>Hg</application> repository completed successfully.");
    Q_EMIT infoMessage(xi18nc("@info:status",
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
    QString bundle = QFileDialog::getOpenFileName();
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
    int answer = KMessageBox::questionYesNo(0, xi18nc("@message:yesorno",
                    "Would you like to revert changes "
                    "made to selected files?"));
    if (answer == KMessageBox::No) {
        return;
    }

    QString infoMsg = xi18nc("@info:status",
         "Reverting files in <application>Hg</application> repository...");
    m_errorMsg = xi18nc("@info:status",
         "Reverting files in <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
         "Reverting files in <application>Hg</application> repository completed successfully.");

    Q_EMIT infoMessage(infoMsg);
    m_hgWrapper->revert(m_contextItems);
}

void FileViewHgPlugin::revertAll()
{
    int answer = KMessageBox::questionYesNo(0, xi18nc("@message:yesorno",
                    "Would you like to revert all changes "
                    "made to current working directory?"));
    if (answer == KMessageBox::No) {
        return;
    }

    QString infoMsg = xi18nc("@info:status",
         "Reverting files in <application>Hg</application> repository...");
    m_errorMsg = xi18nc("@info:status",
         "Reverting files in <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
         "Reverting files in <application>Hg</application> repository completed successfully.");

    Q_EMIT infoMessage(infoMsg);
    m_hgWrapper->revertAll();
}

void FileViewHgPlugin::diff()
{
    QString infoMsg = xi18nc("@info:status",
         "Generating diff for <application>Hg</application> repository...");
    m_errorMsg = xi18nc("@info:status",
         "Could not get <application>Hg</application> repository diff.");
    m_operationCompletedMsg = xi18nc("@info:status",
         "Generated <application>Hg</application> diff successfully.");

    Q_EMIT infoMessage(infoMsg);

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
        KMessageBox::error(0, xi18nc("@message:error",
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
        KMessageBox::error(0, xi18nc("@info:message", "No rollback "
                                        "information available!"));
        return;
    }
    // get what will be rolled back
    QString lastTransaction = m_hgWrapper->readAllStandardOutput();
    int cutOfFrom = lastTransaction.indexOf(QRegExp("\\d"));
    lastTransaction = lastTransaction.mid(cutOfFrom);

    // ask
    int answer = KMessageBox::questionYesNo(0, xi18nc("@message:yesorno",
                    "Would you like to rollback last transaction?")
                        + "\nrevision: " + lastTransaction);
    if (answer == KMessageBox::No) {
        return;
    }

    QString infoMsg = xi18nc("@info:status",
         "Executing Rollback <application>Hg</application> repository...");
    m_errorMsg = xi18nc("@info:status",
         "Rollback of <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
         "Rollback of <application>Hg</application> repository completed successfully.");

    Q_EMIT infoMessage(infoMsg);
    m_hgWrapper->rollback();
    KMessageBox::information(0, m_hgWrapper->readAllStandardOutput());
    Q_EMIT itemVersionsChanged();
}

void FileViewHgPlugin::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
        Q_EMIT errorMessage(m_errorMsg);
    }
    else {
        m_contextItems.clear();
        Q_EMIT operationCompletedMessage(m_operationCompletedMsg);
        Q_EMIT itemVersionsChanged();
    }
}

void FileViewHgPlugin::slotOperationError()
{
    m_contextItems.clear();
    Q_EMIT errorMessage(m_errorMsg);
}

void FileViewHgPlugin::clearMessages() const
{
    m_operationCompletedMsg.clear();
    m_errorMsg.clear();
}

QString FileViewHgPlugin::visualDiffExecPath()
{
    KConfig config("dolphin-hg", KConfig::SimpleConfig,
                           QStandardPaths::GenericConfigLocation);

    KConfigGroup group(&config, QLatin1String("diff"));
    QString result = group.readEntry(QLatin1String("exec"), QString()).trimmed();

    if (result.length() > 0) {
        return result;
    }

    KService::List services = KMimeTypeTrader::self()->query("text/x-diff");
    return services.first()->exec().split(' ').takeFirst();
}

#include "fileviewhgplugin.moc"

