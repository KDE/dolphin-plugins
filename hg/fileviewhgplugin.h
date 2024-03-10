/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILEVIEWHGPLUGIN_H
#define FILEVIEWHGPLUGIN_H

#include "hgwrapper.h"

#include <Dolphin/KVersionControlPlugin>
#include <KFileItem>
#include <QHash>
#include <QProcess>
#include <QString>

class QAction;
class QMenu;

class FileViewHgPlugin : public KVersionControlPlugin
{
    Q_OBJECT

public:
    FileViewHgPlugin(QObject *parent, const QList<QVariant> &args);
    ~FileViewHgPlugin() override;
    QString fileName() const override;
    QString localRepositoryRoot(const QString &directory) const override;
    bool beginRetrieval(const QString &directory) override;
    void endRetrieval() override;
    KVersionControlPlugin::ItemVersion itemVersion(const KFileItem &item) const override;
    QList<QAction *> versionControlActions(const KFileItemList &items) const override;
    virtual QList<QAction *> outOfVersionControlActions(const KFileItemList &items) const override;

private:
    /**
     * Check if HgWrapper is created and connect some signals/slots. Created
     * to ensure that HgWrapper singleton is instantiated not during
     * plugin construction hence not in other thread which ends up giving
     * a lot of warnings.
     */
    void createHgWrapper() const;

    /**
     * Simply clear status messages ie m_errorMsg and m_operationCompletedMsg
     */
    void clearMessages() const;

    /**
     * Read executable file path to open diff patches with from
     * $HOME/.dolphin-hg file in INI format
     */
    QString visualDiffExecPath();

    QList<QAction *> itemContextMenu(const KFileItemList &items) const;
    QList<QAction *> directoryContextMenu(const QString &directory) const;
    QList<QAction *> universalContextMenuActions(const QString &directory) const;

private Q_SLOTS:
    void addFiles();
    void removeFiles();
    void renameFile();
    void commit();
    void branch();
    void tag();
    void update();
    void clone();
    void create();
    void global_config();
    void repo_config();
    void push();
    void pull();
    void revert();
    void revertAll();
    void rollback();
    void backout();
    void diff();
    void serve();
    void merge();
    void bundle();
    void unbundle();
    void exportChangesets();
    void importChangesets();

    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();

private:
    QHash<QString, ItemVersion> m_versionInfoHash;

    QMenu *m_mainContextMenu;

    QAction *m_menuAction;
    QAction *m_addAction;
    QAction *m_removeAction;
    QAction *m_renameAction;
    QAction *m_commitAction;
    QAction *m_branchAction;
    QAction *m_tagAction;
    QAction *m_updateAction;
    QAction *m_cloneAction;
    QAction *m_createAction;
    QAction *m_configAction;
    QAction *m_globalConfigAction;
    QAction *m_repoConfigAction;
    QAction *m_pushAction;
    QAction *m_pullAction;
    QAction *m_revertAction;
    QAction *m_revertAllAction;
    QAction *m_rollbackAction;
    QAction *m_mergeAction;
    QAction *m_bundleAction;
    QAction *m_exportAction;
    QAction *m_unbundleAction;
    QAction *m_importAction;
    QAction *m_diffAction;
    QAction *m_serveAction;
    QAction *m_backoutAction;

    QWidget *m_parentWidget;

    mutable KFileItemList m_contextItems;
    mutable QString m_universalCurrentDirectory;
    mutable QString m_currentDir;
    bool m_isCommitable;

    mutable QString m_operationCompletedMsg;
    mutable QString m_errorMsg;
    mutable HgWrapper *m_hgWrapper;
    HgWrapper *m_retrievalHgw;
};

#endif // FILEVIEWHGPLUGIN_H
