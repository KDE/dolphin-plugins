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
#include <QHeaderView>
#include <QTextCodec>
#include <klocale.h>

HgStatusList::HgStatusList(QWidget *parent):
    QGroupBox(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_filter = new KLineEdit(this);
    m_statusTable = new QTableWidget(this);

    m_statusTable->setColumnCount(3);
    QStringList headers;
    headers << "*" << "S" << "Filename";
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
    hgWrapper->executeCommand(QLatin1String("status"), QStringList());
    int rowCount = 0;
    while (hgWrapper->waitForReadyRead()) {
        char buffer[1024];
        while (hgWrapper->readLine(buffer, sizeof(buffer)) > 0)  {
            const QString currentLine(QTextCodec::codecForLocale()->toUnicode(buffer).trimmed());
            char currentStatus = buffer[0];
            QString currentFile = currentLine.mid(2);

            // Temporarily ignoring
            // TODO: Ask to add file if this is checked by user
            if (currentStatus == '?') {
                continue;
            }

            QTableWidgetItem *check = new QTableWidgetItem();
            QTableWidgetItem *status = new QTableWidgetItem();
            QTableWidgetItem *fileName = new QTableWidgetItem();

            status->setText(QString(currentStatus));
            fileName->setText(currentFile);

            m_statusTable->insertRow(rowCount);
            check->setCheckState(Qt::Unchecked);
            m_statusTable->setItem(rowCount, 0, check);
            m_statusTable->setItem(rowCount, 1, status);
            m_statusTable->setItem(rowCount, 2, fileName);

            switch (currentStatus) {
            case 'A':
                status->setForeground(Qt::darkCyan);
                fileName->setForeground(Qt::darkCyan);
                check->setCheckState(Qt::Checked);
                break;
            case 'M':
                status->setForeground(Qt::blue);
                fileName->setForeground(Qt::blue);
                check->setCheckState(Qt::Checked);
                break;
            case 'R':
                status->setForeground(Qt::red);
                fileName->setForeground(Qt::red);
                check->setCheckState(Qt::Checked);
                break;
            case '?':
                status->setForeground(Qt::darkMagenta);
                fileName->setForeground(Qt::darkMagenta);
                break;
            }

            ++rowCount;
        }
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
    if (nChecked) {
        return true;
    }
    return false;
}

#include "statuslist.moc"
