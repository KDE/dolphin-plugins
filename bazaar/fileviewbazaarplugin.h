/*
    SPDX-FileCopyrightText: 2009-2010 Peter Penz <peter.penz@gmx.at>
    SPDX-FileCopyrightText: 2011 Canonical Ltd.
    SPDX-FileContributor: Jonathan Riddell <jriddell@ubuntu.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILEVIEWBAZAARPLUGIN_H
#define FILEVIEWBAZAARPLUGIN_H

#include <Dolphin/KVersionControlPlugin>

#include <KFileItem>

#include <QHash>
#include <QProcess>
#include <QTemporaryFile>

/**
 * @brief Bazaar (bzr) implementation for the KVersionControlPlugin interface.
 */
class FileViewBazaarPlugin : public KVersionControlPlugin
{
    Q_OBJECT

public:
    FileViewBazaarPlugin(QObject *parent, const QList<QVariant> &args);
    ~FileViewBazaarPlugin() override;
    QString fileName() const override;
    bool beginRetrieval(const QString &directory) override;
    void endRetrieval() override;
    KVersionControlPlugin::ItemVersion itemVersion(const KFileItem &item) const override;
    QList<QAction *> versionControlActions(const KFileItemList &items) const override;
    QList<QAction *> outOfVersionControlActions(const KFileItemList &items) const override;

private Q_SLOTS:
    void updateFiles();
    void pullFiles();
    void pushFiles();
    void showLocalChanges();
    void commitFiles();
    void addFiles();
    void removeFiles();
    void log();

    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();

private:
    /**
     * Executes the command "bzr {bzrCommand}" for the files that have been
     * set by getting the context menu actions (see contextMenuActions()).
     * @param infoMsg     Message that should be shown before the command is executed.
     * @param errorMsg    Message that should be shown if the execution of the command
     *                    has been failed.
     * @param operationCompletedMsg
     *                    Message that should be shown if the execution of the command
     *                    has been completed successfully.
     */
    void execBazaarCommand(const QString &bzrCommand,
                           const QStringList &arguments,
                           const QString &infoMsg,
                           const QString &errorMsg,
                           const QString &operationCompletedMsg);

    void startBazaarCommandProcess();

    QList<QAction *> contextMenuFilesActions(const KFileItemList &items) const;
    QList<QAction *> contextMenuDirectoryActions(const QString &directory) const;

    bool m_pendingOperation;
    QHash<QString, ItemVersion> m_versionInfoHash;

    QAction *m_updateAction;
    QAction *m_pullAction;
    QAction *m_pushAction;
    QAction *m_showLocalChangesAction;
    QAction *m_commitAction;
    QAction *m_addAction;
    QAction *m_removeAction;
    QAction *m_logAction;

    QString m_command;
    QStringList m_arguments;
    QString m_errorMsg;
    QString m_operationCompletedMsg;

    mutable QString m_contextDir;
    mutable KFileItemList m_contextItems;

    QProcess m_process;
    QTemporaryFile m_tempFile;
};
#endif // FILEVIEWBAZAARPLUGIN_H
