/***************************************************************************
 *   Copyright (C) 2009-2011 by Peter Penz <peter.penz19@gmail.com>        *
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

#ifndef FILEVIEWSVNPLUGIN_H
#define FILEVIEWSVNPLUGIN_H

#include <kfileitem.h>
#include <kversioncontrolplugin2.h>
#include <QHash>
#include <QProcess>
#include <QTemporaryFile>

/**
 * @brief Subversion implementation for the KVersionControlPlugin interface.
 */
class FileViewSvnPlugin : public KVersionControlPlugin2
{
    Q_OBJECT

public:
    FileViewSvnPlugin(QObject* parent, const QList<QVariant>& args);
    virtual ~FileViewSvnPlugin();
    virtual QString fileName() const;
    virtual bool beginRetrieval(const QString& directory);
    virtual void endRetrieval();
    virtual ItemVersion itemVersion(const KFileItem& item) const;
    virtual QList<QAction*> actions(const KFileItemList& items) const;

private slots:
    void updateFiles();
    void showLocalChanges();
    void commitFiles();
    void addFiles();
    void removeFiles();

    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();

    void slotShowUpdatesToggled(bool checked);

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
    void execSvnCommand(const QString& svnCommand,
                        const QStringList& arguments,
                        const QString& infoMsg,
                        const QString& errorMsg,
                        const QString& operationCompletedMsg);

    void startSvnCommandProcess();

    QList<QAction*> directoryActions(const QString& directory) const;

private:
    bool m_pendingOperation;
    QHash<QString, ItemVersion> m_versionInfoHash;

    QAction* m_updateAction;
    QAction* m_showLocalChangesAction;
    QAction* m_commitAction;
    QAction* m_addAction;
    QAction* m_removeAction;
    QAction* m_showUpdatesAction;

    QString m_command;
    QStringList m_arguments;
    QString m_errorMsg;
    QString m_operationCompletedMsg;

    mutable QString m_contextDir;
    mutable KFileItemList m_contextItems;

    QProcess m_process;
    QTemporaryFile m_tempFile;
};
#endif // FILEVIEWSVNPLUGIN_H

