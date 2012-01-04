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

#ifndef FILEVIEWHGPLUGIN_H
#define FILEVIEWHGPLUGIN_H

#include "hgwrapper.h"

#include <kfileitem.h>
#include <kversioncontrolplugin2.h>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QProcess>

class KAction;

class FileViewHgPlugin : public KVersionControlPlugin2
{
    Q_OBJECT

public:
    FileViewHgPlugin(QObject *parent, const QList<QVariant> &args);
    virtual ~FileViewHgPlugin();
    virtual QString fileName() const;
    virtual bool beginRetrieval(const QString &directory);
    virtual void endRetrieval();
    virtual KVersionControlPlugin2::ItemVersion itemVersion(const KFileItem &item) const;
    virtual QList<QAction*> actions(const KFileItemList &items) const;

private: 
    /**
     * Check if HgWrapper is created and connect some signals/slots. Created
     * to ensure that HgWrapper singleton is instantiated not during
     * plugin contruction hence not in other thread which ends up giving 
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

    QList<QAction*> itemContextMenu(const KFileItemList &items) const;
    QList<QAction*> directoryContextMenu(const QString &directory) const;
    QList<QAction*> universalContextMenuActions(const QString &directory) const;

private slots:
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

    KAction *m_addAction;
    KAction *m_removeAction;
    KAction *m_renameAction;
    KAction *m_commitAction;
    KAction *m_branchAction;
    KAction *m_tagAction;
    KAction *m_updateAction;
    KAction *m_cloneAction;
    KAction *m_createAction;
    KAction *m_configAction;
    KAction *m_globalConfigAction;
    KAction *m_repoConfigAction;
    KAction *m_pushAction;
    KAction *m_pullAction;
    KAction *m_revertAction;
    KAction *m_revertAllAction;
    KAction *m_rollbackAction;
    KAction *m_mergeAction;
    KAction *m_bundleAction;
    KAction *m_exportAction;
    KAction *m_unbundleAction;
    KAction *m_importAction;
    KAction *m_diffAction;
    KAction *m_serveAction;
    KAction *m_backoutAction;

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

