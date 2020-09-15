/***************************************************************************
 *   Copyright (C) 2019-2020                                               *
 *                  by Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>  *
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

#ifndef SVNLOGDIALOG_H
#define SVNLOGDIALOG_H

#include <QWidget>
#include <QVector>

#include "svncommands.h"

#include "ui_svnlogdialog.h"

class SvnLogDialog : public QWidget {
    Q_OBJECT
public:
    SvnLogDialog(const QString& contextDir, QWidget *parent = nullptr);
    virtual ~SvnLogDialog() override;

public Q_SLOTS:
    void setCurrentRevision(ulong revision);
    void refreshLog();
    void on_tLog_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

Q_SIGNALS:
    void errorMessage(const QString& msg);
    void operationCompletedMessage(const QString& msg);
    void diffAgainstWorkingCopy(const QString& localFilePath, ulong rev);
    void diffBetweenRevs(const QString& remoteFilePath, ulong rev1, ulong rev2);

private Q_SLOTS:
    void showContextMenuLog(const QPoint &pos);
    void showContextMenuChangesList(const QPoint &pos);
    void updateRepoToRevision();
    void revertRepoToRevision();
    void revertFileToRevision();

private:
    Ui::SvnLogDialog m_ui;
    QSharedPointer< QVector<logEntry> > m_log;
    const QString m_contextDir;
    uint m_logLength;
    QAction *m_updateToRev;
    QAction *m_revertToRev;
    QAction *m_diffFilePrev;
    QAction *m_diffFileCurrent;
    QAction *m_fileRevertToRev;
};

#endif  // SVNLOGDIALOG_H
