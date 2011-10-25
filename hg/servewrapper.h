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

#ifndef HG_SERVE_WRAPPER_H
#define HG_SERVE_WRAPPER_H

#include <QtCore/QHash>
#include <QtCore/QProcess>
#include <QtCore/QTextCodec>
#include <QtCore/QString>
#include <klocale.h>

class ServerProcessType;

/**
 * Wrapper to manage web server instaces of mercurial repository. More than one 
 * server can be handled. 
 *
 * This wrapper should be singleton hence only one instance should be created 
 * and used. Hence, never create an object statically or use 'new' operator.
 * Use instance() method to get(create) an instance.
 */
class HgServeWrapper : public QObject
{
    Q_OBJECT

public: 
    HgServeWrapper(QObject *parent=0);
    ~HgServeWrapper();

    /**
     * Returns pointer to singleton instance of the wrapper. An instance is 
     * created if none exists. 
     *
     * @return Pointer to the instance 
     */
    static HgServeWrapper *instance();

    /**
     * Starts a new instance of server.
     *
     * @param repoLocation Path to repository which is to be hosted on server
     * @param portNumber Port to create server on.
     *
     */
    void startServer(const QString &repoLocation, int portNumber);

    /**
     * Stops a server.
     *
     * @param repoLocation Path to repository whose server instance in this
     *                 has to be stopped.
     */
    void stopServer(const QString &repoLocation);

    /**
     * Checks whether a web server for given repository is running under this
     * wrapper.
     *
     * @param repoLocation Path to repository
     * @return true if runnning/started else false
     */
    bool running(const QString &repoLocation);

    /**
     * Cleans all resources in this wrapper used by non-running servers
     */
    void cleanUnused();

    /**
     * If terminated, get error message of that server instaces
     */
    QString errorMessage(const QString &repoLocation);

    /**
     * Check if the just stopped server for given repository was
     * exited successfully.
     *
     * @param repoLocation Path to repository
     * @return true if exitCode == 0 and exitStatus == Normal, otherwise false
     */
    bool normalExit(const QString &repoLocation);

signals:
    // These signals are emitted for every ServerProcessType in list. Its upto
    // the owner of this wrapper to decide which server was updated. If any of
    // these signals are emitted cleanUnused() is called and every initiater
    // of server should check status and act according.
    void finished();
    void error();
    void started();
    void readyReadLine(const QString &repoLocation, const QString &line);

private:

private slots:
    void slotFinished(int exitCode, QProcess::ExitStatus status);

private:
    QHash<QString, ServerProcessType*> m_serverList;
    static HgServeWrapper *m_instance;
};

//FIXME: Had to change struct to class and make it unnested. 
// Hide member variables.

/**
 * Represents a Mercurial Server instace. 
 */
class ServerProcessType : public QObject {
    Q_OBJECT

public:
    QProcess process;
    int port;

    ServerProcessType() 
    {
        connect(&process, SIGNAL(readyReadStandardOutput()), 
                this, SLOT(slotAppendOutput()));
        connect(&process, SIGNAL(readyReadStandardError()),
                this, SLOT(slotAppendRemainingOutput()));
        connect(&process, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(slotFinished()));
    }

signals:    
    void readyReadLine(const QString &repoLocation, const QString &line);

private slots:
    void slotAppendOutput() 
    {
        if (process.canReadLine()) {
            emit readyReadLine(process.workingDirectory(),
                QTextCodec::codecForLocale()->toUnicode(process.readAllStandardOutput()).trimmed());
        }
    }

    void slotAppendRemainingOutput() 
    {
            emit readyReadLine(process.workingDirectory(),
                QTextCodec::codecForLocale()->toUnicode(process.readAllStandardError()).trimmed());
    }

    void slotFinished() 
    {
        emit readyReadLine(process.workingDirectory(),
                               i18n("## Server Stopped! ##\n"));
    }
};

#endif /* HG_SERVE_WRAPPER_H */


