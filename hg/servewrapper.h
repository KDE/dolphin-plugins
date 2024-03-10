/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HG_SERVE_WRAPPER_H
#define HG_SERVE_WRAPPER_H

#include <QHash>
#include <QProcess>
#include <QString>
#include <KLocalizedString>

class ServerProcessType;

/**
 * Wrapper to manage web server instances of mercurial repository. More than one 
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
    explicit HgServeWrapper(QObject *parent=nullptr);
    ~HgServeWrapper() override;

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
     * @return true if running/started else false
     */
    bool running(const QString &repoLocation);

    /**
     * Cleans all resources in this wrapper used by non-running servers
     */
    void cleanUnused();

    /**
     * If terminated, get error message of that server instances
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

Q_SIGNALS:
    // These signals are emitted for every ServerProcessType in list. Its upto
    // the owner of this wrapper to decide which server was updated. If any of
    // these signals are emitted cleanUnused() is called and every initiater
    // of server should check status and act according.
    void finished();
    void error();
    void started();
    void readyReadLine(const QString &repoLocation, const QString &line);

private:

private Q_SLOTS:
    void slotFinished(int exitCode, QProcess::ExitStatus status);

private:
    QHash<QString, ServerProcessType*> m_serverList;
    static HgServeWrapper *m_instance;
};

//FIXME: Had to change struct to class and make it unnested. 
// Hide member variables.

/**
 * Represents a Mercurial Server instance. 
 */
class ServerProcessType : public QObject {
    Q_OBJECT

public:
    QProcess process;
    int port;

    ServerProcessType() 
    {
        connect(&process, &QProcess::readyReadStandardOutput,
                this, &ServerProcessType::slotAppendOutput);
        connect(&process, &QProcess::readyReadStandardError,
                this, &ServerProcessType::slotAppendRemainingOutput);
        connect(&process, &QProcess::finished,
                this, &ServerProcessType::slotFinished);
    }

Q_SIGNALS:    
    void readyReadLine(const QString &repoLocation, const QString &line);

private Q_SLOTS:
    void slotAppendOutput() 
    {
        if (process.canReadLine()) {
            Q_EMIT readyReadLine(process.workingDirectory(),
                QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed());
        }
    }

    void slotAppendRemainingOutput() 
    {
            Q_EMIT readyReadLine(process.workingDirectory(),
                QString::fromLocal8Bit(process.readAllStandardError()).trimmed());
    }

    void slotFinished() 
    {
        Q_EMIT readyReadLine(process.workingDirectory(),
                               i18n("## Server Stopped! ##\n"));
    }
};

#endif /* HG_SERVE_WRAPPER_H */


