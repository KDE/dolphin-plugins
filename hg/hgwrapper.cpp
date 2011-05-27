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

#include "hgwrapper.h"

HgWrapper* HgWrapper::m_instance = 0;
bool HgWrapper::m_pendingOperation = 0;

HgWrapper::HgWrapper(QObject *parent) :
    QProcess(parent)
{
    connect(this, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotOperationCompleted(int, QProcess::ExitStatus)));
    connect(this, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(slotOperationError()));
}

HgWrapper*  HgWrapper::instance()
{
    if (!m_instance ) {
        m_instance = new HgWrapper;
    }
    return m_instance;
}


void HgWrapper::freeInstance( )
{
    if ( m_instance ) {
        delete m_instance;
        m_instance = 0;
    }
}

bool HgWrapper::isBusy() const {
    return m_pendingOperation;
}

void HgWrapper::slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_pendingOperation = false;
    m_arguments.clear( );
}

void HgWrapper::slotOperationError()
{
    m_pendingOperation = false;
    m_arguments.clear( );
}

void HgWrapper::executeCommand(const QString& hgCommand,
                               const QStringList& arguments)
{
    Q_ASSERT(this->state() == QProcess::NotRunning);
    m_pendingOperation = true;

    m_arguments << hgCommand;
    m_arguments << arguments;
    start(QLatin1String("hg"), m_arguments);
}

QString HgWrapper::getBaseDir(const QString& directory)
{
    m_pendingOperation = true;
    setWorkingDirectory(directory);
    start(QLatin1String("hg root"));
    QString hgBaseDir;
    while (waitForReadyRead()) {
        char buffer[512];
        hgBaseDir = QString(this->readAll());
    }
    return hgBaseDir;
}

void HgWrapper::addFiles(const KFileItemList& fileList)
{
    Q_ASSERT(this->state() == QProcess::NotRunning);
    if (isBusy()) {
        return;
    }

    m_arguments << "add";
    foreach (const KFileItem& item, fileList) {
        m_arguments << item.localPath();
    }
    m_pendingOperation = true;
    start(QLatin1String("hg"), m_arguments);
}

void HgWrapper::removeFiles(const KFileItemList& fileList)
{
    Q_ASSERT(this->state() == QProcess::NotRunning);
    if (isBusy()) {
        return;
    }

    m_arguments << "remove";
    foreach (const KFileItem& item, fileList) {
        m_arguments << item.localPath();
    }
    m_pendingOperation = true;
    start(QLatin1String("hg"), m_arguments);
}

#include "hgwrapper.moc"

