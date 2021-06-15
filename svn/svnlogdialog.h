/*
    SPDX-FileCopyrightText: 2019-2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SVNLOGDIALOG_H
#define SVNLOGDIALOG_H

#include <QDialog>
#include <QVector>

#include "svncommands.h"

#include "ui_svnlogdialog.h"

class SvnLogDialog : public QDialog {
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
