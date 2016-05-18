/******************************************************************************
 *   Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                     *
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

#ifndef FILEVIEWGITPLUGIN_H
#define FILEVIEWGITPLUGIN_H

#include <Dolphin/KVersionControlPlugin>

#include <KFileItemList>

#include <QList>
#include <QHash>
#include <QProcess>
#include <QString>

/**
 * @brief Git implementation for the KVersionControlPlugin2 interface.
 */
class FileViewGitPlugin : public KVersionControlPlugin
{
    Q_OBJECT

public:
    FileViewGitPlugin(QObject* parent, const QList<QVariant>& args);
    ~FileViewGitPlugin() override;
    QString fileName() const override;
    bool beginRetrieval(const QString& directory) override;
    void endRetrieval() override;
    ItemVersion itemVersion(const KFileItem& item) const override;
    QList<QAction*> actions(const KFileItemList &items) const override;

private slots:
    void addFiles();
    void showLocalChanges();
    void removeFiles();
    void checkout();
    void commit();
    void createTag();
    void push();
    void pull();

    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();
private:
    QList<QAction*> contextMenuFilesActions(const KFileItemList& items) const;
    QList<QAction*> contextMenuDirectoryActions(const QString& directory) const;
    /**
     * Reads into buffer from device until we reach the next \0 or maxChars have been read.
     * @returns The number of characters read.
     */
    int readUntilZeroChar(QIODevice* device, char* buffer, const int maxChars);
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
    void execGitCommand(const QString& gitCommand,
                        const QStringList& arguments,
                        const QString& infoMsg,
                        const QString& errorMsg,
                        const QString& operationCompletedMsg);
    void startGitCommandProcess();
private:
    bool m_pendingOperation;
    /**
     * Contains all files in the current directory, whose version state is not
     * NormalVersion and directories containing such files (except for directories
     * whose only special contained file type is IgnoredVersion).
     */
    QHash<QString, ItemVersion> m_versionInfoHash;
    QAction* m_addAction;
    QAction* m_showLocalChangesAction;
    QAction* m_removeAction;
    QAction* m_checkoutAction;
    QAction* m_commitAction;
    QAction* m_tagAction;
    QAction* m_pushAction;
    QAction* m_pullAction;

    QString m_currentDir;
    QProcess m_process;
    QString m_command;
    QStringList m_arguments;
    QString m_operationCompletedMsg;
    QString m_errorMsg;

    //Current targets. m_contextItems is used if and only if m_contextDir is empty.
    mutable QString m_contextDir;
    mutable KFileItemList m_contextItems;
};
#endif // FILEVIEWGITPLUGIN_H

