/***************************************************************************
 *   Copyright (C) 2011 by Vishesh Yadav <vishesh3y@gmail.com>             *
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

#ifndef HGWRAPPER_H
#define HGWRAPPER_H


#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <kfileitem.h>
#include <kversioncontrolplugin.h>

class QTextCodec;


//TODO: Make HgWrapper contain QProcess, rather than inherit
//TODO: Create signals for infoMessage and errorMessage which will be 
//      caught by main plugin interface.

/**
 * A singleton class providing implementation of many Mercurial commands
 */
class HgWrapper : public QObject
{
    Q_OBJECT
public:
    HgWrapper(QObject *parent = 0);

    static HgWrapper *instance();
    static void freeInstance();

    /**
     * Start a mercurial command with given arguments.
     *
     * @param hgCommand Command to be executed. eg. diff, status
     * @param arguments Arguments for the given command
     */
    void executeCommand(const QString &hgCommand,
                        const QStringList &arguments = QStringList());
    /**
     * Start a mercurial command with given arguments and return until
     * process completes.
     *
     * @param hgCommand Command to be executed. eg. diff, status
     * @param arguments Arguments for the given command
     * @return true if operations completed successfully, otherwise false
     */
    bool executeCommandTillFinished(const QString &hgCommand,
                        const QStringList &arguments = QStringList());
    /**
     * Start a mercurial command with given arguments, write standard output
     * to output parameter and return till finished.
     *
     * @param hgCommand Command to be executed. eg. diff, status
     * @param arguments Arguments for the given command
     * @param output Append standard output of process to this string
     * @return true if operations completed successfully, otherwise false
     */
    bool executeCommand(const QString &hgCommand,
                        const QStringList &arguments,
                        QString &output);

    /**
     * Get the root directory of Mercurial repository. Using 'hg root'
     *
     * @return String containing path of the root directory.
     */
    QString getBaseDir() const;

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
    void getVersionStates(QHash<QString, KVersionControlPlugin::VersionState> &result);

    void addFiles(const KFileItemList &fileList);
    void removeFiles(const KFileItemList &fileList);
    bool renameFile(const QString &source, const QString &destination);
    bool commit(const QString &message, 
                const QStringList &files = QStringList(), 
                bool closeCurrentBranch = false);
    bool createBranch(const QString &name);
    bool switchBranch(const QString &name);
    bool createTag(const QString &name);
    bool switchTag(const QString &name);
    bool revertAll();
    bool revert(const KFileItemList &fileList);
    //bool update(const QString &name, bool discardChanges=false);

    QString getParentsOfHead();
    
    QStringList getBranches();
    QStringList getTags();

    inline QString readAllStandardOutput() {
        return m_process.readAllStandardOutput();
    }
    
    /**
     * Check if some Mercurial operation is currently being executed or
     * about to be started.
     */
    inline bool isBusy() {
        return (m_process.state() == QProcess::Running ||
                m_process.state() == QProcess::Starting);
    }

public slots:
    /**
     * Try to terminate the currently running operation.
     */
    void terminateCurrentProcess();

signals:
    ///equivalent to the signals of QProcess
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void error(QProcess::ProcessError error);
    void started();
    void stateChanged(QProcess::ProcessState state);

private:
    ///Get and update m_hgBaseDir
    void updateBaseDir();

private slots:
    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();

private:
    static HgWrapper *m_instance;

    QProcess m_process;
    QTextCodec *m_localCodec;

    QString m_hgBaseDir;
    QString m_currentDir;
};

#endif // HGWRAPPER_H

