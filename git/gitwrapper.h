/******************************************************************************
 *   Copyright (C) 2010 by Sebastian Doerner <sebastian@sebastian-doerner.de> *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program; if not, write to the                            *
 *   Free Software Foundation, Inc.,                                          *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA               *
 ******************************************************************************/


#ifndef GITWRAPPER_H
#define GITWRAPPER_H

#include <QProcess>
#include <QStringList>
#include <QSet>

class QTextCodec;

/**
 * @brief A git wrapper class to access git functions conveniently using a Singleton interface.
 */
class GitWrapper
{
    private:
        GitWrapper();
        static GitWrapper* m_instance;
        ///size of the line buffers when parsing git output
        static const int BUFFER_SIZE;
        ///size of the line buffers when parsing shorter git output
        static const int SMALL_BUFFER_SIZE;
        QProcess m_process;
        QTextCodec * m_localCodec;
        /**
         * Gets a list of all remote hosts, whose entry ends with \a lineEnd
         * @param lineEnd The end of the lines, which should be returned as entries.
         *                Should be "(pull)" or "(push)".
         */
        QStringList remotes(QLatin1String lineEnd);
    public:
        static GitWrapper* instance();
        static void freeInstance();
        void setWorkingDirectory(const QString& workingDirectory) {
            m_process.setWorkingDirectory(workingDirectory);
        }

        /**
         * @returns The configured user.name of the repository,
                    the empty String (which is NOT null) if not configured.
         */
        QString userName();
        /**
         * @returns The configured user.email of the repository,
                    the empty String (which is NOT null) if not configured.
         */
        QString userEmail();
        /**
         * Gets a list of all branches in the repository.
         * @param currentBranchIndex
         *          Pointer to the location in which to store the index of the current branch
         *          within the returned QStringList. Set to -1 if currently not on any branch.
         * @returns A StringList containing the names of all local and remote branches.
         */
        QStringList branches(int * currentBranchIndex = 0);

        /**
         * Gets a list of all tags in the repository.
         * @returns A list containing the tag names
         */
        QStringList tags();
        /**
         * Gets a set of all tags in the repository.
         * @param result Set to store the results in.
         */
        void tagSet(QSet<QString>& result);

        /**
         * Gets a list of all remote hosts we can pull from.
         * @returns The list containing the remote hosts.
         */
        QStringList pullRemotes();

        /**
         * Gets a list of all remote hosts we can push to.
         * @returns The list containing the remote hosts.
         */
        QStringList pushRemotes();

        /**
         * @returns The last commit message. Null-String if there is no last commit.
         */
        QString lastCommitMessage();
};

#endif // GITWRAPPER_H
