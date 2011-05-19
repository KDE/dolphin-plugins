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

#include "fileviewhgplugin.h"

#include "fileviewhgpluginsettings.h"

#include <KPluginFactory>
#include <KPluginLoader>
K_PLUGIN_FACTORY(FileViewHgPluginFactory, registerPlugin<FileViewHgPlugin>();)
K_EXPORT_PLUGIN(FileViewHgPluginFactory("fileviewhgplugin"))

FileViewHgPlugin::FileViewHgPlugin(QObject* parent, const QList<QVariant>& args) :
    KVersionControlPlugin(parent)
{
    Q_UNUSED(args);
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
    return true;
}

void FileViewHgPlugin::endRetrieval()
{
}

KVersionControlPlugin::VersionState FileViewHgPlugin::versionState(const KFileItem& item)
{
    return NormalVersion;
}

QList<QAction*> FileViewHgPlugin::contextMenuActions(const KFileItemList& items)
{
    return QList<QAction*>();
}

QList<QAction*> FileViewHgPlugin::contextMenuActions(const QString& directory)
{
    return QList<QAction*>();
}

