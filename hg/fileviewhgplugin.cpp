/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>
    SPDX-FileCopyrightText: 2015 Tomasz Bojczuk <seelook@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

#include <KApplicationTrader>
#include <KMessageBox>
#include <QFileDialog>
#include <KConfig>
#include <KConfigGroup>
#include <KService>

#include <KLocalizedString>
#include <KPluginFactory>
#include <kwidgetsaddons_version.h>
#include <QRegularExpression>

K_PLUGIN_CLASS_WITH_JSON(FileViewHgPlugin, "fileviewhgplugin.json")

//TODO: Build a proper status signal system to sync HgWrapper/Dialogs with this
//TODO: Show error messages and set their message appropriately(hg output)
//TODO: Use xi18nc rather than i18c throughout plugin

FileViewHgPlugin::FileViewHgPlugin(QObject *parent, const QList<QVariant> &args):
    KVersionControlPlugin(parent),
    m_mainContextMenu(nullptr),
    m_addAction(nullptr),
    m_removeAction(nullptr),
    m_renameAction(nullptr),
    m_commitAction(nullptr),
    m_branchAction(nullptr),
    m_tagAction(nullptr),
    m_updateAction(nullptr),
    m_cloneAction(nullptr),
    m_createAction(nullptr),
    m_configAction(nullptr),
    m_globalConfigAction(nullptr),
    m_repoConfigAction(nullptr),
    m_pushAction(nullptr),
    m_pullAction(nullptr),
    m_revertAction(nullptr),
    m_revertAllAction(nullptr),
    m_rollbackAction(nullptr),
    m_mergeAction(nullptr),
    m_bundleAction(nullptr),
    m_exportAction(nullptr),
    m_unbundleAction(nullptr),
    m_importAction(nullptr),
    m_diffAction(nullptr),
    m_serveAction(nullptr),
    m_backoutAction(nullptr),
    m_isCommitable(false),
    m_hgWrapper(nullptr),
    m_retrievalHgw(nullptr)
{
    Q_UNUSED(args);

    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");

    m_parentWidget = qobject_cast<QWidget*>(parent);

    m_addAction = new QAction(this);
    m_addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    m_addAction->setText(xi18nc("@action:inmenu",
                               "<application>Hg</application> Add"));
    connect(m_addAction, &QAction::triggered,
            this, &FileViewHgPlugin::addFiles);

    m_removeAction = new QAction(this);
    m_removeAction->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    m_removeAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Remove"));
    connect(m_removeAction, &QAction::triggered,
            this, &FileViewHgPlugin::removeFiles);

    m_renameAction = new QAction(this);
    m_renameAction->setIcon(QIcon::fromTheme(QStringLiteral("list-rename")));
    m_renameAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Rename"));
    connect(m_renameAction, &QAction::triggered,
            this, &FileViewHgPlugin::renameFile);

    m_commitAction = new QAction(this);
    m_commitAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-commit")));
    m_commitAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Commit"));
    connect(m_commitAction, &QAction::triggered,
            this, &FileViewHgPlugin::commit);

    m_tagAction = new QAction(this);
    m_tagAction->setIcon(QIcon::fromTheme(QStringLiteral("svn-tag")));
    m_tagAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Tag"));
    connect(m_tagAction, &QAction::triggered,
            this, &FileViewHgPlugin::tag);

    m_branchAction = new QAction(this);
    m_branchAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-branch")));
    m_branchAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Branch"));
    connect(m_branchAction, &QAction::triggered,
            this, &FileViewHgPlugin::branch);

    m_cloneAction = new QAction(this);
    m_cloneAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-clone")));
    m_cloneAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Clone"));
    connect(m_cloneAction, &QAction::triggered,
            this, &FileViewHgPlugin::clone);

    m_createAction = new QAction(this);
    m_createAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-create")));
    m_createAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Init"));
    connect(m_createAction, &QAction::triggered,
            this, &FileViewHgPlugin::create);

    m_updateAction = new QAction(this);
    m_updateAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-pull")));
    m_updateAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Update"));
    connect(m_updateAction, &QAction::triggered,
            this, &FileViewHgPlugin::update);

    m_globalConfigAction = new QAction(this);
    m_globalConfigAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-config")));
    m_globalConfigAction->setText(xi18nc("@action:inmenu",
                          "<application>Hg</application> Global Config"));
    connect(m_globalConfigAction, &QAction::triggered,
            this, &FileViewHgPlugin::global_config);

    m_repoConfigAction = new QAction(this);
    m_repoConfigAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-config")));
    m_repoConfigAction->setText(xi18nc("@action:inmenu",
                      "<application>Hg</application> Repository Config"));
    connect(m_repoConfigAction, &QAction::triggered,
            this, &FileViewHgPlugin::repo_config);

    m_pushAction = new QAction(this);
    m_pushAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-push")));
    m_pushAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Push"));
    connect(m_pushAction, &QAction::triggered,
            this, &FileViewHgPlugin::push);

    m_pullAction = new QAction(this);
    m_pullAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-pull")));
    m_pullAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Pull"));
    connect(m_pullAction, &QAction::triggered,
            this, &FileViewHgPlugin::pull);

    m_revertAction = new QAction(this);
    m_revertAction->setIcon(QIcon::fromTheme(QStringLiteral("document-revert")));
    m_revertAction->setText(xi18nc("@action:inmenu",
                                  "<application>Hg</application> Revert"));
    connect(m_revertAction, &QAction::triggered,
            this, &FileViewHgPlugin::revert);

    m_revertAllAction = new QAction(this);
    m_revertAllAction->setIcon(QIcon::fromTheme(QStringLiteral("document-revert")));
    m_revertAllAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Revert All"));
    connect(m_revertAllAction, &QAction::triggered,
            this, &FileViewHgPlugin::revertAll);

    m_rollbackAction = new QAction(this);
    m_rollbackAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-rollback")));
    m_rollbackAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Rollback"));
    connect(m_rollbackAction, &QAction::triggered,
            this, &FileViewHgPlugin::rollback);

    m_mergeAction = new QAction(this);
    m_mergeAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-merge")));
    m_mergeAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Merge"));
    connect(m_mergeAction, &QAction::triggered,
            this, &FileViewHgPlugin::merge);

    m_bundleAction = new QAction(this);
    m_bundleAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-bundle")));
    m_bundleAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Bundle"));
    connect(m_bundleAction, &QAction::triggered,
            this, &FileViewHgPlugin::bundle);

    m_exportAction = new QAction(this);
    m_exportAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-export")));
    m_exportAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Export"));
    connect(m_exportAction, &QAction::triggered,
            this, &FileViewHgPlugin::exportChangesets);

    m_importAction = new QAction(this);
    m_importAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-import")));
    m_importAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Import"));
    connect(m_importAction, &QAction::triggered,
            this, &FileViewHgPlugin::importChangesets);

    m_unbundleAction = new QAction(this);
    m_unbundleAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-unbundle")));
    m_unbundleAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Unbundle"));
    connect(m_unbundleAction, &QAction::triggered,
            this, &FileViewHgPlugin::unbundle);

    m_serveAction = new QAction(this);
    m_serveAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-serve")));
    m_serveAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Serve"));
    connect(m_serveAction, &QAction::triggered,
            this, &FileViewHgPlugin::serve);

    m_backoutAction = new QAction(this);
    m_backoutAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-backout")));
    m_backoutAction->setText(xi18nc("@action:inmenu",
                                 "<application>Hg</application> Backout"));
    connect(m_backoutAction, &QAction::triggered,
            this, &FileViewHgPlugin::backout);

    m_diffAction = new QAction(this);
    m_diffAction->setIcon(QIcon::fromTheme(QStringLiteral("vcs-diff")));
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
    m_menuAction->setIcon(QIcon::fromTheme(QStringLiteral("hg-main")));
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

    if (created && m_hgWrapper != nullptr) {
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
    return QStringLiteral(".hg");
}

QString FileViewHgPlugin::localRepositoryRoot(const QString& directory) const
{
    QProcess process;
    process.setWorkingDirectory(directory);
    process.start(QStringLiteral("hg"), {QStringLiteral("root")});
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
    if (m_retrievalHgw == nullptr) {
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

#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    int answer = KMessageBox::questionTwoActions(nullptr,
#else
    int answer = KMessageBox::questionYesNo(nullptr,
#endif
                                            xi18nc("@message:yesorno",
                    "Would you like to remove selected files "
                    "from the repository?"), i18n("Remove Files"), KStandardGuiItem::remove(), KStandardGuiItem::cancel());
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    if (answer == KMessageBox::ButtonCode::SecondaryAction) {
#else
    if (answer == KMessageBox::No) {
#endif
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

    HgRenameDialog dialog(m_contextItems.first(), m_parentWidget);
    dialog.exec();
    m_contextItems.clear();
}


void FileViewHgPlugin::commit()
{
    if (m_hgWrapper->isWorkingDirectoryClean()) {
        KMessageBox::information(nullptr, xi18nc("@message", "No changes for commit!"));
        return;
    }
    //FIXME: Disable emitting of status messages when executing sub tasks.
    m_errorMsg = xi18nc("@info:status",
            "Commit to <application>Hg</application> repository failed.");
    m_operationCompletedMsg = xi18nc("@info:status",
            "Committed to <application>Hg</application> repository.");
    Q_EMIT infoMessage(xi18nc("@info:status",
            "Commit <application>Hg</application> repository."));

    HgCommitDialog dialog(m_parentWidget);
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

    HgTagDialog dialog(m_parentWidget);
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

    HgUpdateDialog dialog(m_parentWidget);
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

    HgBranchDialog dialog(m_parentWidget);
    dialog.exec();
}

void FileViewHgPlugin::clone()
{
    clearMessages();
    HgCloneDialog dialog(m_universalCurrentDirectory, m_parentWidget);
    dialog.exec();
}

void FileViewHgPlugin::create()
{
    clearMessages();
    HgCreateDialog dialog(m_universalCurrentDirectory, m_parentWidget);
    dialog.exec();
}

void FileViewHgPlugin::global_config()
{
    clearMessages();
    HgConfigDialog diag(HgConfig::GlobalConfig, m_parentWidget);
    diag.exec();
}

void FileViewHgPlugin::repo_config()
{
    clearMessages();
    HgConfigDialog diag(HgConfig::RepoConfig, m_parentWidget);
    diag.exec();
}

void FileViewHgPlugin::push()
{
    clearMessages();
    HgPushDialog diag(m_parentWidget);
    diag.exec();
}

void FileViewHgPlugin::pull()
{
    clearMessages();
    HgPullDialog diag(m_parentWidget);
    diag.exec();
}

void FileViewHgPlugin::merge()
{
    clearMessages();
    HgMergeDialog diag(m_parentWidget);
    diag.exec();
}

void FileViewHgPlugin::bundle()
{
    clearMessages();
    HgBundleDialog diag(m_parentWidget);
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
        KMessageBox::error(nullptr, m_hgWrapper->readAllStandardError());
    }
}

void FileViewHgPlugin::importChangesets()
{
    clearMessages();
    HgImportDialog diag(m_parentWidget);
    diag.exec();
}

void FileViewHgPlugin::exportChangesets()
{
    clearMessages();
    HgExportDialog diag(m_parentWidget);
    diag.exec();
}

void FileViewHgPlugin::revert()
{
    clearMessages();
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    int answer = KMessageBox::questionTwoActions(nullptr,
#else
    int answer = KMessageBox::questionYesNo(nullptr,
#endif
                                            xi18nc("@message:yesorno",
                    "Would you like to revert changes "
                    "made to selected files?"), i18n("Revert"), KGuiItem(i18n("Revert")), KStandardGuiItem::cancel());
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    if (answer == KMessageBox::ButtonCode::SecondaryAction) {
#else
    if (answer == KMessageBox::No) {
#endif
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
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    int answer = KMessageBox::questionTwoActions(nullptr,
#else
    int answer = KMessageBox::questionYesNo(nullptr,
#endif
                                            xi18nc("@message:yesorno",
                    "Would you like to revert all changes "
                    "made to current working directory?"), i18n("Revert All"), KGuiItem(i18n("Revert")), KStandardGuiItem::cancel());
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    if (answer == KMessageBox::ButtonCode::SecondaryAction) {
#else
    if (answer == KMessageBox::No) {
#endif
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
    HgServeDialog diag(m_parentWidget);
    diag.exec();
}

void FileViewHgPlugin::backout()
{
    clearMessages();
    m_hgWrapper = HgWrapper::instance();
    if (!m_hgWrapper->isWorkingDirectoryClean()) {
        KMessageBox::error(nullptr, xi18nc("@message:error",
                      "abort: Uncommitted changes in working directory!"));
        return;
    }

    HgBackoutDialog diag(m_parentWidget);
    diag.exec();
}

void FileViewHgPlugin::rollback()
{
    // execute a dry run rollback first to see if there is anything to
    // be rolled back, or check what will be rolled back
    if (!m_hgWrapper->rollback(true)) {
        KMessageBox::error(nullptr, xi18nc("@info:message", "No rollback "
                                        "information available!"));
        return;
    }
    // get what will be rolled back
    QString lastTransaction = m_hgWrapper->readAllStandardOutput();
    int cutOfFrom = lastTransaction.indexOf(QRegularExpression(QStringLiteral("\\d")));
    lastTransaction = lastTransaction.mid(cutOfFrom);

    // ask
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    int answer = KMessageBox::questionTwoActions(nullptr,
#else
    int answer = KMessageBox::questionYesNo(nullptr,
#endif
                                            xi18nc("@message:yesorno",
                    "Would you like to rollback last transaction?")
                        + QLatin1String("\nrevision: ") + lastTransaction, i18n("Rollback"), KGuiItem(i18n("Rollback")), KStandardGuiItem::cancel());
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    if (answer == KMessageBox::ButtonCode::SecondaryAction) {
#else
    if (answer == KMessageBox::No) {
#endif
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
    KMessageBox::information(nullptr, m_hgWrapper->readAllStandardOutput());
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
    KConfig config(QStringLiteral("dolphin-hg"), KConfig::SimpleConfig,
                           QStandardPaths::GenericConfigLocation);

    KConfigGroup group(&config, QStringLiteral("diff"));
    QString result = group.readEntry(QLatin1String("exec"), QString()).trimmed();

    if (result.length() > 0) {
        return result;
    }

    KService::Ptr service = KApplicationTrader::preferredService(QStringLiteral("text/x-diff"));
    return service ? service->exec().split(QLatin1Char(' ')).takeFirst() : QString();
}

#include "fileviewhgplugin.moc"

#include "moc_fileviewhgplugin.cpp"
