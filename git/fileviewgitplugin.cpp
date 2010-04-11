/***************************************************************************
 *   Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                  *
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

#include "fileviewgitplugin.h"

#include <kdemacros.h>
#include <kfileitem.h>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <KPluginFactory>
#include <KPluginLoader>
K_PLUGIN_FACTORY(FileViewGitPluginFactory, registerPlugin<FileViewGitPlugin>();)
K_EXPORT_PLUGIN(FileViewGitPluginFactory("fileviewgitplugin"))

FileViewGitPlugin::FileViewGitPlugin(QObject* parent, const QList<QVariant>& args) :
    KVersionControlPlugin(parent),
    m_versionInfoHash()
{
    Q_UNUSED(args);
}

FileViewGitPlugin::~FileViewGitPlugin()
{
}

QString FileViewGitPlugin::fileName() const
{
    return QLatin1String(".git");
}

bool FileViewGitPlugin::beginRetrieval(const QString& directory)
{
    Q_ASSERT(directory.endsWith('/'));

    // Clear all entries for this directory including the entries
    // for sub directories
    QMutableHashIterator<QString, VersionState> it(m_versionInfoHash);
    while (it.hasNext()) {
        it.next();
        if (it.key().startsWith(directory)) {
            it.remove();
        }
    }

    QStringList arguments;
    arguments << QLatin1String("status");

    QProcess process;
    process.setWorkingDirectory(directory);
    process.start(QLatin1String("git"), arguments);
    while (process.waitForReadyRead()) {
        char buffer[1024];
        while (process.readLine(buffer, sizeof(buffer)) > 0)  {
            VersionState state = NormalVersion;
            const QString filePath(buffer);
            if (filePath.contains(QLatin1String("modified:"))) {
                const QString file = filePath.simplified().section(QChar(' '), 2);
                state = LocallyModifiedVersion;
                const QString filePath = directory + file;
                m_versionInfoHash.insert(filePath, state);
            }
        }
    }

    return true;
}

void FileViewGitPlugin::endRetrieval()
{
}

KVersionControlPlugin::VersionState FileViewGitPlugin::versionState(const KFileItem& item)
{
    const QString itemUrl = item.localPath();
    if (m_versionInfoHash.contains(itemUrl)) {
        return m_versionInfoHash.value(itemUrl);
    }

    if (!item.isDir()) {
        // files that have not been listed by 'git status' (= m_versionInfoHash)
        // are under version control per definition
        return NormalVersion;
    }

    // The item is a directory. Check whether an item listed by 'git status' (= m_versionInfoHash)
    // is part of this directory. In this case a local modification should be indicated in the
    // directory already.
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

QList<QAction*> FileViewGitPlugin::contextMenuActions(const KFileItemList& items)
{
    Q_UNUSED(items);
    return QList<QAction*>();
}

QList<QAction*> FileViewGitPlugin::contextMenuActions(const QString& directory)
{
    Q_UNUSED(directory);
    return QList<QAction*>();
}
