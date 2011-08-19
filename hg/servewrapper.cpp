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

#include "servewrapper.h"
#include "hgwrapper.h"

#include <QtCore/QTextCodec>
#include <klocale.h>
#include <kdebug.h>

HgServeWrapper *HgServeWrapper::m_instance = 0;

HgServeWrapper::HgServeWrapper(QObject *parent) :
    QObject(parent)
{
}

HgServeWrapper::~HgServeWrapper()
{
    QMutableHashIterator<QString, ServerProcessType*> it(m_serverList);
    while (it.hasNext()) {
        it.next();
        ///terminate server if not terminated already
        if (it.value()->process.state() != QProcess::NotRunning) {
            it.value()->process.terminate();
        }
        it.value()->deleteLater();
        it.remove();
    }
}

HgServeWrapper *HgServeWrapper::instance()
{
    if (m_instance == 0) {
        m_instance = new HgServeWrapper;
    }
    return m_instance;
}

void HgServeWrapper::startServer(const QString &repoLocation, int portNumber)
{
    ServerProcessType *server = m_serverList.value(repoLocation, 0);
    if (server != 0) {
        m_serverList.remove(repoLocation);
        server->deleteLater();
    }
    server = new ServerProcessType;
    m_serverList.insert(repoLocation, server);
    server->port = portNumber;
    server->process.setWorkingDirectory(HgWrapper::instance()->getBaseDir());

    connect(&server->process, SIGNAL(started()), 
            this, SIGNAL(started()));
    connect(&server->process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotFinished(int, QProcess::ExitStatus)));
    connect(server, SIGNAL(readyReadLine(const QString&, const QString&)),
            this, SIGNAL(readyReadLine(const QString&, const QString&)));

    QStringList args;
    args << QLatin1String("-oL");
    args << QLatin1String("hg");
    args << QLatin1String("serve");
    args << QLatin1String("--port");
    args << QString::number(portNumber);
    server->process.start(QLatin1String("stdbuf"), args);
    emit readyReadLine(repoLocation,
            i18n("## Starting Server ##"));
    emit readyReadLine(repoLocation,
            QString("% hg serve --port %1").arg(portNumber));
}

void HgServeWrapper::stopServer(const QString &repoLocation)
{
    ServerProcessType *server = m_serverList.value(repoLocation, 0);
    if (server == 0) {
        return;
    }
    server->process.terminate();
}

bool HgServeWrapper::running(const QString &repoLocation)
{
    ServerProcessType *server = m_serverList.value(repoLocation, 0);
    if (server == 0) {
        return false;
    }
    return ( server->process.state() == QProcess::Running || 
             server->process.state() == QProcess::Starting);
}

void HgServeWrapper::slotFinished(int exitCode, QProcess::ExitStatus status)
{
    if (exitCode == 0 && status == QProcess::NormalExit) {
        emit finished();
    }
    else {
        emit error();
    }
}

QString HgServeWrapper::errorMessage(const QString &repoLocation)
{
    ServerProcessType *server = m_serverList.value(repoLocation, 0);
    if (server == 0) {
        return QString();
    }
    return QTextCodec::codecForLocale()->toUnicode(server->process.readAllStandardError());
}

bool HgServeWrapper::normalExit(const QString &repoLocation)
{
    ServerProcessType *server = m_serverList.value(repoLocation, 0);
    if (server == 0) {
        return true;
    }

    return (server->process.exitStatus() == QProcess::NormalExit &&
            server->process.exitCode() == 0);
}

void HgServeWrapper::cleanUnused()
{
    QMutableHashIterator<QString, ServerProcessType*> it(m_serverList);
    while (it.hasNext()) {
        it.next();
        if (it.value()->process.state() == QProcess::NotRunning) {
            it.value()->deleteLater();
            it.remove();
        }
    }
}

#include "servewrapper.moc"

