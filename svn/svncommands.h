/*
    SPDX-FileCopyrightText: 2019-2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SVNCOMMANDS_H
#define SVNCOMMANDS_H

#include <QString>
#include <QDateTime>
#include <QtGlobal>

#include <Dolphin/KVersionControlPlugin>

class QTemporaryFile;
class QFileDevice;

/**
 * SVN commands execution results.
 */
struct CommandResult {
    bool success;       ///< True if return code is '0' (normal execution).
    QString stdOut;     ///< Process stdout.
    QString stdErr;     ///< Process stderr.
};

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
class SvnCommands {
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
     * From file \p filePath returns full working copy root path this file contains.
     *
     * \return Full local working copy root path, empty QString in case of error.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     */
    static QString localRoot(const QString& filePath);

    /**
     * Updates selected \p filePath to revision \p revision. \p filePath could be a single file or a
     * directory. It also could be an absolute or relative.
     *
     * \return True on success, false either.
     *
     * \note This function can be really time consuming.
     */
    static bool updateToRevision(const QString& filePath, ulong revision);

    /**
     * Discards all local changes in a \p filePath. \p filePath could be a single file or a directory.
     * It also could be an absolute or relative.
     *
     * \return True on success, false either.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     */
    static bool revertLocalChanges(const QString& filePath);

    /**
     * Reverts selected \p filePath to revision \p revision. \p filePath could be a single file or a
     * directory. It also could be an absolute or relative.
     *
     * \return True on success, false either.
     */
    static bool revertToRevision(const QString& filePath, ulong revision);

    /**
     * Runs 'svn cleanup' on a \p dir to remove write locks, resume unfinished operations, etc. Its
     * restores directory state if Subversion client has crushed.
     * Also this command could be used to remove unversioned or ignored files.
     *
     * \return Filled up \p commandResult structure.
     */
    static CommandResult cleanup(const QString& dir, bool removeUnversioned = false, bool removeIgnored = false, bool includeExternals = false);

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
     * \return Full log, nullptr in case of error.
     *
     * \note This function is really time consuming.
     */
    static QSharedPointer< QVector<logEntry> > getLog(const QString& filePath, uint maxEntries = 255, ulong fromRevision = 0);

    /**
     * Check out a working copy of repository \p URL (local URL starts with a 'file://') to a local
     * path \p whereto (could be relative ot absolute).
     *
     * \return True if check out success, false either.
     *
     * \note This function can be really time consuming.
     */
    static bool checkoutRepository(const QString& url, bool ignoreExternals, const QString& whereto);
};

#endif  // SVNCOMMANDS_H
