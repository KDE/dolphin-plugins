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
 * Wrapper to manage web server instaces of mercurial repository
 */
class HgServeWrapper : public QObject
{
    Q_OBJECT

public: 
    HgServeWrapper(QObject *parent=0);
    ~HgServeWrapper();

    static HgServeWrapper *instance();

    void startServer(const QString &repoLocation, int portNumber);
    void stopServer(const QString &repoLocation);

    bool running(const QString &repoLocation);

    void cleanUnused();

    QString errorMessage(const QString &repoLocation);
    bool normalExit(const QString &repoLocation);

signals:
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


