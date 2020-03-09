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
#include <QtGlobal>

#include <Dolphin/KVersionControlPlugin>

class QTemporaryFile;
class QFileDevice;

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
     */
    static ulong localRevision(const QString& filePath);

    /**
     * For file \p filePath return its full remote repository URL path.
     *
     * \return Remote path, empty QString in case of error.
     *
     * \note This function uses only local SVN data without connection to a remote so it's fast.
     */
    static QString remoteItemUrl(const QString& filePath);

    /**
     * Export remote URL \p remoteUrl at revision \p rev to a file \p file. File should already be
     * opened or ready to be opened. Freeing resources is up to the caller.
     *
     * \return True if export success, false either.
     *
     * \note \p file should already be created with \p new.
     */
    static bool exportRemoteFile(const QString& remoteUrl, ulong rev, QFileDevice *file);
    static bool exportRemoteFile(const QString& remoteUrl, ulong rev, QTemporaryFile *file);

    /**
     * Export local file \p filePath at revision \p rev to a file \p file. File should already be
     * opened or ready to be opened. Freeing resources is up to the caller.
     *
     * \return True if export success, false either.
     *
     * \note \p file should already be created with \p new.
     */
    static bool exportLocalFile(const QString& filePath, ulong rev, QFileDevice *file);
    static bool exportLocalFile(const QString& filePath, ulong rev, QTemporaryFile *file);
};

#endif  // SVNCOMMANDS_H
