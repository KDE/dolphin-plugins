/*
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GITWRAPPER_H
#define GITWRAPPER_H

#include <QProcess>
#include <QSet>
#include <QStringList>

/**
 * @brief A git wrapper class to access git functions conveniently using a Singleton interface.
 */
class GitWrapper
{
private:
    GitWrapper();
    static GitWrapper *m_instance;
    /// size of the line buffers when parsing git output
    static const int BUFFER_SIZE;
    /// size of the line buffers when parsing shorter git output
    static const int SMALL_BUFFER_SIZE;
    QProcess m_process;
    /**
     * Gets a list of all remote hosts, whose entry ends with \a lineEnd
     * @param lineEnd The end of the lines, which should be returned as entries.
     *                Should be "(pull)" or "(push)".
     */
    QStringList remotes(QLatin1String lineEnd);

public:
    static GitWrapper *instance();
    static void freeInstance();
    void setWorkingDirectory(const QString &workingDirectory)
    {
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
    QStringList branches(int *currentBranchIndex = nullptr);

    /**
     * Gets a list of all tags in the repository.
     * @returns A list containing the tag names
     */
    QStringList tags();
    /**
     * Gets a set of all tags in the repository.
     * @param result Set to store the results in.
     */
    void tagSet(QSet<QString> &result);

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
