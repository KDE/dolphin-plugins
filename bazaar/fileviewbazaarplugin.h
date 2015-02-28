/***************************************************************************
 *   Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>             *
 *   Copyright (C) 2011 Canonical Ltd.                                     *
 *        By Jonathan Riddell <jriddell@ubuntu.com>                        *
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
    FileViewBazaarPlugin(QObject* parent, const QList<QVariant>& args);
    ~FileViewBazaarPlugin() Q_DECL_OVERRIDE;
    QString fileName() const  Q_DECL_OVERRIDE;
    bool beginRetrieval(const QString& directory)  Q_DECL_OVERRIDE;
    void endRetrieval() Q_DECL_OVERRIDE;
    KVersionControlPlugin::ItemVersion itemVersion(const KFileItem& item) const Q_DECL_OVERRIDE;
    QList<QAction*> actions(const KFileItemList& items) const Q_DECL_OVERRIDE;



private slots:
    QList<QAction*> contextMenuFilesActions(const KFileItemList& items) const;
    QList<QAction*> contextMenuDirectoryActions(const QString& directory) const;
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
    void execBazaarCommand(const QString& bzrCommand,
                        const QStringList& arguments,
                        const QString& infoMsg,
                        const QString& errorMsg,
                        const QString& operationCompletedMsg);

    void startBazaarCommandProcess();

    bool m_pendingOperation;
    QHash<QString, ItemVersion> m_versionInfoHash;

    QAction* m_updateAction;
    QAction* m_pullAction;
    QAction* m_pushAction;
    QAction* m_showLocalChangesAction;
    QAction* m_commitAction;
    QAction* m_addAction;
    QAction* m_removeAction;
    QAction* m_logAction;

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

