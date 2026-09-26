/*
    SPDX-FileCopyrightText: 2010 Peter Penz <peter.penz@gmx.at>
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILEVIEWGITPLUGIN_H
#define FILEVIEWGITPLUGIN_H

#include <Dolphin/KVersionControlPlugin>
#include <KDialogJobUiDelegate>
#include <KIO/CommandLauncherJob>
#include <KJobUiDelegate>

#include <KFileItem>

#include <QHash>
#include <QList>
#include <QProcess>

#include <QString>
#include <memory>

class FileViewGitPlugin;

/**
 * What one reading of a directory found, so that a single plugin instance can answer for every
 * view at once.
 */
class GitVersions : public KVersionControlPluginVersions
{
public:
    GitVersions(FileViewGitPlugin *plugin, const QString &directory, QHash<QString, KVersionControlPlugin::ItemVersion> versions);

    KVersionControlPlugin::ItemVersion itemVersion(const KFileItem &item) const override;
    QList<QAction *> versionControlActions(const KFileItemList &items) const override;
    QList<QAction *> outOfVersionControlActions(const KFileItemList &items) const override;

private:
    FileViewGitPlugin *const m_plugin;
    const QString m_directory;
    const QHash<QString, KVersionControlPlugin::ItemVersion> m_versions;
};

/**
 * @brief Git implementation for the KVersionControlPlugin2 interface.
 */
class FileViewGitPlugin : public KVersionControlPlugin
{
    Q_OBJECT

public:
    FileViewGitPlugin(QObject *parent, const QList<QVariant> &args);
    ~FileViewGitPlugin() override;
    QString fileName() const override;
    QString localRepositoryRoot(const QString &directory) const override;
    std::unique_ptr<KVersionControlPluginVersions> readVersions(const QString &directory) override;
    bool beginRetrieval(const QString &directory) override;
    void endRetrieval() override;
    ItemVersion itemVersion(const KFileItem &item) const override;
    QList<QAction *> versionControlActions(const KFileItemList &items) const override;
    QList<QAction *> outOfVersionControlActions(const KFileItemList &items) const override;

    /// Reads the versions of everything in @p directory without keeping any of it.
    QHash<QString, ItemVersion> readVersionsFor(const QString &directory);
    /// Takes on one reading for as long as it takes to build the actions for its directory.
    void adoptVersions(const QString &directory, const QHash<QString, ItemVersion> &versions);
    static ItemVersion versionOf(const KFileItem &item, const QHash<QString, ItemVersion> &versions, const QString &directory);

private Q_SLOTS:
    void addFiles();
    void revertFiles();
    void showLocalChanges();
    void removeFiles();
    void checkout();
    void commit();
    void createTag();
    void push();
    void pull();
    void log();
    void showDiff(const QUrl &link);
    void merge();
    void restoreStaged();
    void clone();

    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();

private:
    QList<QAction *> contextMenuFilesActions(const KFileItemList &items) const;
    QList<QAction *> contextMenuDirectoryActions(const QString &directory) const;
    /**
     * Reads into buffer from device until we reach the next \0 or maxChars have been read.
     * @returns The number of characters read.
     */
    int readUntilZeroChar(QIODevice *device, char *buffer, const int maxChars);
    /**
     * Parses the output of the git push command and returns an appropriate message,
     * that should be displayed to the user.
     * @returns The error or success message to be printed to the user
     */
    QString parsePushOutput();
    /**
     * Parses the output of the git pull command and returns an appropriate message,
     * that should be displayed to the user.
     * @returns The error or success message to be printed to the user
     */
    QString parsePullOutput();
    /**
     * Executes the command "git {svnCommand}" for the files that have been
     * set by getting the context menu actions (see contextMenuActions()).
     * @param infoMsg     Message that should be shown before the command is executed.
     * @param errorMsg    Message that should be shown if the execution of the command
     *                    has failed.
     * @param operationCompletedMsg
     *                    Message that should be shown if the execution of the command
     *                    has been completed successfully.
     */
    void execGitCommand(const QString &gitCommand,
                        const QStringList &arguments,
                        const QString &infoMsg,
                        const QString &errorMsg,
                        const QString &operationCompletedMsg);
    void startGitCommandProcess();

private:
    bool m_pendingOperation;
    /**
     * Contains all files in the current directory, whose version state is not
     * NormalVersion and directories containing such files (except for directories
     * whose only special contained file type is IgnoredVersion).
     */
    QHash<QString, ItemVersion> m_versionInfoHash;
    QAction *m_addAction;
    QAction *m_revertAction;
    QAction *m_showLocalChangesAction;
    QAction *m_removeAction;
    QAction *m_checkoutAction;
    QAction *m_commitAction;
    QAction *m_tagAction;
    QAction *m_pushAction;
    QAction *m_pullAction;
    QAction *m_logAction;
    QAction *m_mergeAction;
    QAction *m_restoreStagedAction;
    QAction *m_cloneAction;

    QString m_currentDir;
    QProcess m_process;
    QString m_command;
    QStringList m_arguments;
    QString m_operationCompletedMsg;
    QString m_errorMsg;

    QWidget *m_parentWidget;

    // Current targets. m_contextItems is used if and only if m_contextDir is empty.
    mutable QString m_contextDir;
    mutable KFileItemList m_contextItems;

    // Utility method, because the method call is the same except for the command
    void runCommand(const QString &command)
    {
        auto *job = new KIO::CommandLauncherJob(command);
        job->setWorkingDirectory(m_currentDir);
        job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
        job->start();
    }

    void setSSHAskPassEnvironment(QProcess *process);
};
#endif // FILEVIEWGITPLUGIN_H
