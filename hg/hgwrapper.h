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
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtCore/QHash>
#include <kfileitem.h>


enum HgVersionState {
    HgModifiedVersion, HgAddedVersion, HgRemovedVersion, HgCleanVersion,
    HgMissingVersion, HgIgnoredVersion, HgUntrackedVersion
};

class HgWrapper : public QProcess
{
    Q_OBJECT
public:
    HgWrapper(QObject *parent = 0);

    static HgWrapper *instance();
    static void freeInstance();

    void executeCommand(const QString &hgCommand,
                        const QStringList &arguments = QStringList());
    bool executeCommand(const QString &hgCommand,
                        const QStringList &arguments,
                        QString &output);

    QString getBaseDir() const;
    void setBaseAsWorkingDir();
    void setCurrentDir(const QString &directory);
    QHash<QString, HgVersionState>& getVersionStates(bool ignoreParents=false);

    void addFiles(const KFileItemList &fileList);
    void removeFiles(const KFileItemList &fileList);
    bool renameFile(const QString &source, const QString &destination);
    bool commit(const QString &message, 
                const QStringList &files = QStringList(), 
                bool closeCurrentBranch = false);

    QString getParentsOfHead();
    
    QStringList getBranches();
    QStringList getTags();
    bool createBranch(const QString &branchName);

    inline bool isBusy() {
        return (state() == QProcess::Running);
    }

private:
    void updateBaseDir();

private slots:
    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();

private:
    static HgWrapper *m_instance;

    QStringList m_arguments;
    QTextCodec *m_localCodec;

    QString m_hgBaseDir;
    QString m_currentDir;
    QHash<QString, HgVersionState> m_versionStateResult;
};

#endif // HGWRAPPER_H

