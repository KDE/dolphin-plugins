/*
    SPDX-FileCopyrightText: 2009-2011 Peter Penz <peter.penz19@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILEVIEWSVNPLUGIN_H
#define FILEVIEWSVNPLUGIN_H

#include <Dolphin/KVersionControlPlugin>

#include <KFileItem>

#include <QHash>
#include <QProcess>
#include <QTemporaryFile>

/**
 * @brief Subversion implementation for the KVersionControlPlugin interface.
 */
class FileViewSvnPlugin : public KVersionControlPlugin
{
    Q_OBJECT

public:
    FileViewSvnPlugin(QObject *parent, const QList<QVariant> &args);
    ~FileViewSvnPlugin() override;
    QString fileName() const override;
    QString localRepositoryRoot(const QString &directory) const override;
    bool beginRetrieval(const QString &directory) override;
    void endRetrieval() override;
    ItemVersion itemVersion(const KFileItem &item) const override;
    QList<QAction *> versionControlActions(const KFileItemList &items) const override;
    QList<QAction *> outOfVersionControlActions(const KFileItemList &items) const override;

Q_SIGNALS:
    /// Invokes m_showUpdatesAction->setChecked(checked) on the UI thread.
    void setShowUpdatesChecked(bool checked);

    /**
     * Is emitted if current SVN directory status got updated. Not necessarily means
     * it's changed. Emitted right after #endRetrieval().
     */
    void versionInfoUpdated();

private Q_SLOTS:
    void updateFiles();
    void showLocalChanges();
    void commitDialog();
    void addFiles();
    void removeFiles();
    void revertFiles();
    void logDialog();
    void checkoutDialog();
    void cleanupDialog();

    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();

    void slotShowUpdatesToggled(bool checked);

    void revertFiles(const QStringList &filesPath);
    void diffFile(const QString &filePath);
    void diffAgainstWorkingCopy(const QString &localFilePath, ulong rev);
    void diffBetweenRevs(const QString &remoteFilePath, ulong rev1, ulong rev2);
    void addFiles(const QStringList &filesPath);
    void commitFiles(const QStringList &context, const QString &msg);

private:
    /**
     * Executes the command "svn {svnCommand}" for the files that have been
     * set by getting the context menu actions (see contextMenuActions()).
     * @param infoMsg     Message that should be shown before the command is executed.
     * @param errorMsg    Message that should be shown if the execution of the command
     *                    has been failed.
     * @param operationCompletedMsg
     *                    Message that should be shown if the execution of the command
     *                    has been completed successfully.
     */
    void execSvnCommand(const QString &svnCommand,
                        const QStringList &arguments,
                        const QString &infoMsg,
                        const QString &errorMsg,
                        const QString &operationCompletedMsg);

    void startSvnCommandProcess();

    QList<QAction *> directoryActions(const KFileItem &directory) const;

    /**
     * Checks #item parent directory (or its parent directory and so on) is unversioned.
     * @param item Item to check.
     * @return True item is in unversioned directory, false otherwise.
     */
    bool isInUnversionedDir(const KFileItem &item) const;

private:
    bool m_pendingOperation;
    QHash<QString, ItemVersion> m_versionInfoHash;

    QAction *m_updateAction;
    QAction *m_showLocalChangesAction;
    QAction *m_commitAction;
    QAction *m_addAction;
    QAction *m_removeAction;
    QAction *m_revertAction;
    QAction *m_showUpdatesAction;
    QAction *m_logAction;
    QAction *m_checkoutAction;
    QAction *m_cleanupAction;

    QString m_command;
    QStringList m_arguments;
    QString m_errorMsg;
    QString m_operationCompletedMsg;

    QWidget *m_parentWidget;

    mutable QString m_contextDir;
    mutable KFileItemList m_contextItems;

    QProcess m_process;
    QTemporaryFile m_tempFile;
};
#endif // FILEVIEWSVNPLUGIN_H
