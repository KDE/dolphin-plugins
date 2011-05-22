/***************************************************************************
 *   Copyright (C) 2009-2010 by Vishesh Yadav <vishesh3y@gmail.com>        *
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

#include <kfileitem.h>
#include <kversioncontrolplugin.h>
#include <QHash>
#include <QString>

class FileViewHgPlugin : public KVersionControlPlugin
{
    Q_OBJECT

public:
    FileViewHgPlugin(QObject* parent, const QList<QVariant>& args);
    virtual ~FileViewHgPlugin();
    virtual QString fileName() const;
    virtual bool beginRetrieval(const QString& directory);
    virtual void endRetrieval();
    virtual KVersionControlPlugin::VersionState versionState(const KFileItem& item);
    virtual QList<QAction*> contextMenuActions(const KFileItemList& items);
    virtual QList<QAction*> contextMenuActions(const QString& directory);

private:
    QHash<QString, VersionState> m_versionInfoHash;
    
    QAction* m_addAction;
    QAction* m_removeAction;
    QAction* m_renameAction;

    KFileItemList m_contextItems;
    QString m_contextDir;

    bool m_pendingOperation;
 
};

#endif // FILEVIEWHGPLUGIN_H

