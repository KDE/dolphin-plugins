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
#include <kaction.h>
#include <kversioncontrolplugin.h>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QProcess>
#include <QtCore/QStringList>

//TODO: Redesign Push/Pull dialog. Make a base class implementing their common features. High Priority!!!!!!!!!

class FileViewHgPlugin : public KVersionControlPlugin
{
    Q_OBJECT

public:
    FileViewHgPlugin(QObject *parent, const QList<QVariant> &args);
    virtual ~FileViewHgPlugin();
    virtual QString fileName() const;
    virtual bool beginRetrieval(const QString &directory);
    virtual void endRetrieval();
    virtual KVersionControlPlugin::VersionState versionState(const KFileItem &item);
    virtual QList<QAction*> contextMenuActions(const KFileItemList &items);
    virtual QList<QAction*> contextMenuActions(const QString &directory);
    virtual QList<QAction*> universalContextMenuActions(const QString &directory);

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
    void config();
    void push();
    void pull();

    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();

private:
    QHash<QString, VersionState> m_versionInfoHash;

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
    KAction *m_pushAction;
    KAction *m_pullAction;

    KFileItemList m_contextItems;
    QString m_operationCompletedMsg;
    QString m_errorMsg;
    HgWrapper *m_hgWrapper;
};

#endif // FILEVIEWHGPLUGIN_H

