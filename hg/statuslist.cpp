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

#include "statuslist.h"
#include "hgwrapper.h"

#include <QtGui/QVBoxLayout>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtGui/QHeaderView>
#include <QtCore/QTextCodec>
#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>

HgStatusList::HgStatusList(QWidget *parent):
    QGroupBox(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_filter = new KLineEdit(this);
    m_statusTable = new QTableWidget(this);

    m_statusTable->setColumnCount(3);
    QStringList headers;
    headers << "*" << "S" << i18n("Filename");
    m_statusTable->setHorizontalHeaderLabels(headers);
    m_statusTable->verticalHeader()->hide();
    m_statusTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statusTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    mainLayout->addWidget(m_filter);
    mainLayout->addWidget(m_statusTable);

    setTitle(i18nc("@title:group", "File Status"));
    setLayout(mainLayout);

    reloadStatusTable();

    connect(m_statusTable, SIGNAL(itemSelectionChanged()),
            this, SLOT(itemSelectionChangedSlot()));
}

void HgStatusList::itemSelectionChangedSlot()
{
    emit itemSelectionChanged(
        m_statusTable->item(m_statusTable->currentRow(), 1)->text()[0].toLatin1(),
        m_statusTable->item(m_statusTable->currentRow(), 2)->text());
}

void HgStatusList::reloadStatusTable()
{
    m_statusTable->clearContents();
    m_statusTable->resizeRowsToContents();
    m_statusTable->resizeColumnsToContents();
    m_statusTable->horizontalHeader()->setStretchLastSection(true);

    HgWrapper *hgWrapper = HgWrapper::instance();
    QHash<QString, HgVersionState> &hgVsState = hgWrapper->getVersionStates(false);
    QMutableHashIterator<QString, HgVersionState> it(hgVsState);
    int rowCount = 0;
    while (it.hasNext()) {
        it.next();
        HgVersionState currentStatus = it.value();
        // FIXME: preferred method, but not working :| bad hack below
        // QString currentFile 
        //    = KUrl::relativeUrl(hgWrapper->getBaseDir(), it.key()); 
        QString currentFile = it.key().mid(hgWrapper->getBaseDir().length()+1);
        QString currentStatusString;

        // Temporarily ignoring
        // TODO: Ask to add file if this is checked by user
        if (currentStatus == HgUntrackedVersion ||
                currentStatus == HgIgnoredVersion) {
            continue;
        }

        QTableWidgetItem *check = new QTableWidgetItem;
        QTableWidgetItem *status = new QTableWidgetItem;
        QTableWidgetItem *fileName = new QTableWidgetItem;

        switch (currentStatus) {
            case HgAddedVersion:
                status->setForeground(Qt::darkCyan);
                fileName->setForeground(Qt::darkCyan);
                check->setCheckState(Qt::Checked);
                currentStatusString = QLatin1String("A");
                break;
            case HgModifiedVersion:
                status->setForeground(Qt::blue);
                fileName->setForeground(Qt::blue);
                check->setCheckState(Qt::Checked);
                currentStatusString = QLatin1String("M");
                break;
            case HgRemovedVersion:
                status->setForeground(Qt::red);
                fileName->setForeground(Qt::red);
                check->setCheckState(Qt::Checked);
                currentStatusString = QLatin1String("R");
                break;
            case HgUntrackedVersion:
                status->setForeground(Qt::darkMagenta);
                fileName->setForeground(Qt::darkMagenta);
                currentStatusString = QLatin1String("?");
                break;
            case HgIgnoredVersion:
                status->setForeground(Qt::black);
                fileName->setForeground(Qt::black);
                currentStatusString = QLatin1String("I");
                break;
        }

        status->setText(QString(currentStatusString));
        fileName->setText(currentFile);

        m_statusTable->insertRow(rowCount);
        check->setCheckState(Qt::Checked); //Change. except untracked, ignored
        m_statusTable->setItem(rowCount, 0, check);
        m_statusTable->setItem(rowCount, 1, status);
        m_statusTable->setItem(rowCount, 2, fileName);

        ++rowCount;
    }
}

bool HgStatusList::getSelectionForCommit(QStringList &files)
{
    int nChecked = 0;
    int nRowCount = m_statusTable->rowCount();
    for (int row = 0; row < nRowCount; row++) {
        QTableWidgetItem *item = m_statusTable->item(row, 0);
        if (item->checkState() == Qt::Checked) {
            nChecked++;
            files << m_statusTable->item(row, 2)->text();
        }
    }
    if (nChecked == nRowCount) {
        files.clear();
    }
    if (nChecked > 0) {
        return true;
    }
    return false;
}

#include "statuslist.moc"

