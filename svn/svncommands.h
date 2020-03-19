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

#ifndef SVNCOMMANDS_H
#define SVNCOMMANDS_H

#include <QString>
#include <QDateTime>
#include <QtGlobal>

#include <Dolphin/KVersionControlPlugin>

class QTemporaryFile;
class QFileDevice;

/**
 * Path information for log entry.
 */
struct affectedPath {
    QString action;     ///< Action type: "D" for delete, "M" for modified, etc.
    bool propMods;      ///< Property changes by commit.
    bool textMods;      ///< File changes by commit.
    QString kind;       ///< Path type: "file", "dir", etc.

    QString path;       ///< Path itself.
};

/**
 * A single log entry.
 */
struct logEntry {
    ulong revision;                         ///< Revision number.
    QString author;                         ///< Commit author.
    QDateTime date;                         ///< Commit time and date.
    QVector<affectedPath> affectedPaths;    ///< Affected paths (files or dirs).
    QString msg;                            ///< Commit message.
};

/**
 * \brief SVN support functions.
 *
 * \note All functions are synchronous i.e. blocking. Each of them waits for svn process to finish.
 */
class SVNCommands {
public:
    /**
     * Returns file \p filePath local revision. Local revision means last known file revision, not
     * last SVN repository revision.
     *
     * \return Local revision, 0 in case of error.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     * \sa remoteRevision()
     */
    static ulong localRevision(const QString& filePath);

    /**
     * Returns file \p filePath remote revision. Remote revision means last known SVN repository file
     * revision. This function uses only current SVN data and doesn't connect to a remote.
     *
     * \return Local revision, 0 in case of error.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     * \sa localRevision()
     */
    static ulong remoteRevision(const QString& filePath);

    /**
     * For file \p filePath return its full remote repository URL path.
     *
     * \return Remote path, empty QString in case of error.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     */
    static QString remoteItemUrl(const QString& filePath);

    /**
     * From a file \p filePath returns full remote repository URL in which this file located. For
     * every file in the repository URL is the same, i.e. returns path used for initial 'svn co'.
     *
     * \return Remote path, empty QString in case of error.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     */
    static QString remoteRootUrl(const QString& filePath);

    /**
     * From a file \p filePath returns relative repository URL in which this file located. So,
     * for example, a root repository file "file.txt" will have relative URL "^/file.txt".
     *
     * \return Relative repository URL, empty QString in case of error.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     */
    static QString remoteRelativeUrl(const QString& filePath);

    /**
     * Updates selected \p filePath to revision \p revision. \p filePath could be a sigle file or a
     * directory. It also could be an absolute or relative.
     *
     * \return True on success, false either.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     */
    static bool updateToRevision(const QString& filePath, ulong revision);

    /**
     * Discards all local changes in a \p filePath. \p filePath could be a sigle file or a directory.
     * It also could be an absolute or relative.
     *
     * \return True on success, false either.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     */
    static bool revertLocalChanges(const QString& filePath);

    /**
     * Reverts selected \p filePath to revision \p revision. \p filePath could be a sigle file or a
     * directory. It also could be an absolute or relative.
     *
     * \return True on success, false either.
     */
    static bool revertToRevision(const QString& filePath, ulong revision);

    /**
     * Export URL \p path at revision \p rev to a file \p file. URL could be a remote URL to a file
     * or directory or path to a local file or directory (both relative or absolute). File should
     * already be opened or ready to be opened. Freeing resources is up to the caller.
     *
     * \return True if export success, false either.
     *
     * \note \p file should already be created with \p new.
     */
    static bool exportFile(const QUrl& path, ulong rev, QFileDevice *file);
    static bool exportFile(const QUrl& path, ulong rev, QTemporaryFile *file);

    /**
     * Get SVN log for a following \p filePath (could be a directory or separate file, relative or
     * absolute paths accepted). Log starts from revision \p fromRevision and goes for \p maxEntries
     * previous revisions. The default revision (0) means current revision.
     *
     * \return Full log, empty QVector in case of error.
     *
     * \note This function is really time consuming.
     */
    static QSharedPointer< QVector<logEntry> > getLog(const QString& filePath, uint maxEntries = 255, ulong fromRevision = 0);
};

#endif  // SVNCOMMANDS_H
