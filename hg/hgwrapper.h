/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGWRAPPER_H
#define HGWRAPPER_H

#include <Dolphin/KVersionControlPlugin>
#include <KFileItem>
#include <QHash>
#include <QProcess>
#include <QString>

// TODO: Create signals for infoMessage and errorMessage which will be
//       caught by main plugin interface.

/**
 * A singleton class providing implementation of many Mercurial commands
 */
class HgWrapper : public QObject
{
    Q_OBJECT
public:
    explicit HgWrapper(QObject *parent = nullptr);

    static HgWrapper *instance();
    static void freeInstance();

    /**
     * Start a mercurial command with given arguments.
     *
     * @param hgCommand Command to be executed. eg. diff, status
     * @param arguments Arguments for the given command
     * @param primaryOperation Will emit primaryOperationFinished,
     *               primaryOperationError signals.
     */
    void executeCommand(const QString &hgCommand, const QStringList &arguments = QStringList(), bool primaryOperation = false);
    /**
     * Start a mercurial command with given arguments and return until
     * process completes.
     *
     * @param hgCommand Command to be executed. eg. diff, status
     * @param arguments Arguments for the given command
     * @param primaryOperation Will emit primaryOperationCompleted,
     *               primaryOperationError signals.
     * @return true if operations completed successfully, otherwise false
     */
    bool executeCommandTillFinished(const QString &hgCommand, const QStringList &arguments = QStringList(), bool primaryOperation = false);
    /**
     * Start a mercurial command with given arguments, write standard output
     * to output parameter and return till finished.
     *
     * @param hgCommand Command to be executed. eg. diff, status
     * @param arguments Arguments for the given command
     * @param output Append standard output of process to this string
     * @param primaryOperation Will emit primaryOperationCompleted,
     *               primaryOperationError signals.
     * @return true if operations completed successfully, otherwise false
     */
    bool executeCommand(const QString &hgCommand, const QStringList &arguments, QString &output, bool primaryOperation = false);

    /**
     * Get the root directory of Mercurial repository. Using 'hg root'
     *
     * @return String containing path of the root directory.
     */
    QString getBaseDir() const;

    /**
     * Sets the current directory being browsed with plugin enabled.
     * Updates base directory of repository accordingly
     */
    void setCurrentDir(const QString &directory);

    /**
     * Get the directory path that is currently used as the working directory
     * to execute the commands by the HgWrapper.
     */
    QString getCurrentDir() const;

    /**
     * Set the root directory of repository as working directory.
     */
    void setBaseAsWorkingDir();

    /**
     * Get FileName-ItemVersion pairs of the repository returned by
     *
     * $hg status --modified --added --removed --deleted --unknown --ignored
     *
     * Hence returns files with ItemVersion
     *      - LocallyModifiedVersion
     *      - AddedVersion
     *      - RemovedVersion
     *      - RemovedVersion
     *      - UnversionedVersion
     *      - IgnoredVersion
     *      - MissingVersion
     *
     * @param result A hashmap containing FileName-ItemVersion pairs
     *
     */
    void getItemVersions(QHash<QString, KVersionControlPlugin::ItemVersion> &result);

    void addFiles(const KFileItemList &fileList);
    void removeFiles(const KFileItemList &fileList);
    bool renameFile(const QString &source, const QString &destination);

    /**
     * Commits changes made to the working directory.
     * @param message Commit message. Should not be empty.
     * @param files List of files to be committed. Files changed but not
     *              listed here will be ignored during commit.
     *              If the list is empty, all modified files will be
     *              committed, the default behavior.
     * @param closeCurrentBranch Closes the current branch after commit.
     * @return true if successful, otherwise false
     */
    bool commit(const QString &message, const QStringList &files = QStringList(), bool closeCurrentBranch = false);

    /**
     * Create a new branch
     * @param name Name of new branch to be createdialog
     * @return true if successfully created, otherwise false
     */
    bool createBranch(const QString &name);

    /**
     * Update current working directory to another branch
     * @param name Name of the branch to which working directory
     *              has to be updated.
     * @return true if successful, otherwise false
     */
    bool switchBranch(const QString &name);

    /**
     * Create tag for current changeset(the changeset of working directory)
     * @param name Name of the new tag to be createdialog
     * @return true if successful, otherwise false
     */
    bool createTag(const QString &name);

    /**
     * Update working directory to a changeset named by given tag
     * @param name Tag of the changeset to which working directory
     *              has to be updated.
     * @return true if successful, otherwise false
     */
    bool switchTag(const QString &name);

    /**
     * Reverts all local changes made to working directory. Will update to
     * last changeset of current branch, ie state just after last commit.
     */
    bool revertAll();

    /**
     * Reverts local changes made to selected files. All changes made to
     * these files after last commit will be lost.
     */
    bool revert(const KFileItemList &fileList);

    /**
     * Undo's last transaction. Like commit, pull, push(to this repo).
     * Does not alter working directory.
     *
     * Use with care. Rollback cant be undone. See Mercurial man page FOR
     * more info.
     *
     * @param dryRun Do not actually perform action, but just print output
     *              Used to check if Rollback can be done, and if yes then
     *              what will be rolled back.
     * @return true if successful, otherwise false
     */
    bool rollback(bool dryRun = false);

    /**
     * Checks if the working directory is clean, ie there are no
     * uncommitted changes present.
     *
     * @return true if clean otherwise false
     */
    bool isWorkingDirectoryClean();

    QString getParentsOfHead();

    /**
     * Returns list of all branch names.
     */
    QStringList getBranches();

    /**
     * Returns list of all tags
     */
    QStringList getTags();

    inline QString readAllStandardOutput()
    {
        return QString::fromLocal8Bit(m_process.readAllStandardOutput());
    }

    inline QString readAllStandardError()
    {
        return QString::fromLocal8Bit(m_process.readAllStandardError());
    }

    /**
     * Check if some Mercurial operation is currently being executed or
     * about to be started.
     */
    inline bool isBusy()
    {
        return (m_process.state() == QProcess::Running || m_process.state() == QProcess::Starting);
    }

public Q_SLOTS:
    /**
     * Try to terminate the currently running operation.
     */
    void terminateCurrentProcess();

Q_SIGNALS:
    /// equivalent to the signals of QProcess
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void errorOccurred(QProcess::ProcessError error);
    void started();
    void stateChanged(QProcess::ProcessState state);
    void primaryOperationFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void primaryOperationError(QProcess::ProcessError error);

private:
    /// Get and update m_hgBaseDir
    void updateBaseDir();

private Q_SLOTS:
    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError(QProcess::ProcessError error);

private:
    static HgWrapper *m_instance;

    QProcess m_process;

    QString m_hgBaseDir;
    QString m_currentDir;

    bool m_primaryOperation; // to differentiate intermediate process
};

#endif // HGWRAPPER_H
