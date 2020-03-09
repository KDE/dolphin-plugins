/***************************************************************************
 *   Copyright (C) 2019-2020                                               *
 *                  by Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>  *
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

#include "svncommands.h"

#include <QProcess>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDir>

namespace {

// Helper function: returns template file name for QTemporaryFile.
QString templateFileName(const QString& url, ulong rev)
{
    const QString tmpFileName = url.section('/', -1);

    return QDir::tempPath() + QString("/%1.r%2.XXXXXX").arg(tmpFileName).arg(rev);
}

}

ulong SVNCommands::localRevision(const QString& filePath)
{
    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("info"),
            QStringLiteral("--show-item"),
            QStringLiteral("last-changed-revision"),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return 0;
    }

    QTextStream stream(&process);
    ulong revision = 0;
    stream >> revision;

    if (stream.status() == QTextStream::Ok) {
        return revision;
    } else {
        return 0;
    }
}

QString SVNCommands::remoteItemUrl(const QString& filePath)
{
    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("info"),
            QStringLiteral("--show-item"),
            QStringLiteral("url"),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return 0;
    }

    QTextStream stream(&process);
    QString url;
    stream >> url;

    if (stream.status() == QTextStream::Ok) {
        return url;
    } else {
        return QString();
    }
}

bool SVNCommands::exportRemoteFile(const QString& remoteUrl, ulong rev, QFileDevice *file)
{
    if (file == nullptr) {
        return false;
    }
    if (!file->isOpen() && !file->open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("export"),
            QStringLiteral("--force"),
            QStringLiteral("-r%1").arg(rev),
            remoteUrl,
            file->fileName()
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return false;
    } else {
        return true;
    }
}

bool SVNCommands::exportRemoteFile(const QString& remoteUrl, ulong rev, QTemporaryFile *file)
{
    if (file == nullptr) {
        return false;
    }

    file->setFileTemplate( templateFileName(remoteUrl, rev) );

    return exportRemoteFile(remoteUrl, rev, dynamic_cast<QFileDevice*>(file));
}

bool SVNCommands::exportLocalFile(const QString& filePath, ulong rev, QFileDevice *file)
{
    if (file == nullptr) {
        return false;
    }

    const QString fileUrl = remoteItemUrl(filePath);
    if (fileUrl.isEmpty()) {
        return false;
    }

    if (!exportRemoteFile(fileUrl, rev, file)) {
        return false;
    } else {
        return true;
    }
}

bool SVNCommands::exportLocalFile(const QString& filePath, ulong rev, QTemporaryFile *file)
{
    if (file == nullptr) {
        return false;
    }

    file->setFileTemplate( templateFileName(filePath, rev) );

    return exportLocalFile(filePath, rev, dynamic_cast<QFileDevice*>(file));
}
