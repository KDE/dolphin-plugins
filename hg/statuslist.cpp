/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>
    SPDX-FileCopyrightText: 2015 Tomasz Bojczuk <seelook@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "statuslist.h"
#include "hgwrapper.h"

#include <QHash>
#include <QTextCodec>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <KLocalizedString>

HgStatusList::HgStatusList(QWidget *parent):
    QGroupBox(parent),
    m_allWhereChecked(true),
    m_sortIndex(false)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_statusTable = new QTableWidget(this);

    m_statusTable->setColumnCount(3);
    const QStringList headers{
        QStringLiteral("*"),
        QStringLiteral("S"),
        i18n("Filename"),
    };
    m_statusTable->setHorizontalHeaderLabels(headers);
    m_statusTable->verticalHeader()->hide();
    m_statusTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statusTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_statusTable->setSelectionMode(QAbstractItemView::SingleSelection);

    mainLayout->addWidget(m_statusTable);

    setTitle(i18nc("@title:group", "File Status"));
    setLayout(mainLayout);

    reloadStatusTable();

    connect(m_statusTable, &QTableWidget::currentItemChanged,
            this, &HgStatusList::currentItemChangedSlot);
    connect(m_statusTable->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &HgStatusList::headerClickedSlot);
}

void HgStatusList::currentItemChangedSlot()
{
    Q_EMIT itemSelectionChanged(
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
    QHash<QString, KVersionControlPlugin::ItemVersion> hgVsState;
    hgWrapper->getItemVersions(hgVsState);
    QMutableHashIterator<QString, KVersionControlPlugin::ItemVersion> it(hgVsState);
    int rowCount = 0;
    while (it.hasNext()) {
        it.next();
        KVersionControlPlugin::ItemVersion currentStatus = it.value();
        // Get path relative to root directory of repository
        // FIXME: preferred method, but not working :| bad hack below
        // QString currentFile 
        //    = KUrl::relativeUrl(hgWrapper->getBaseDir(), it.key()); 
        QString currentFile = it.key().mid(hgWrapper->getBaseDir().length()+1);
        QString currentStatusString; //one character status indicator

        // Temporarily ignoring
        // TODO: Ask to add file if this is checked by user
        if (currentStatus == KVersionControlPlugin::UnversionedVersion ||
                currentStatus == KVersionControlPlugin::IgnoredVersion) {
            continue;
        }

        QTableWidgetItem *check = new QTableWidgetItem;
        QTableWidgetItem *status = new QTableWidgetItem;
        QTableWidgetItem *fileName = new QTableWidgetItem;

        switch (currentStatus) {
            case KVersionControlPlugin::AddedVersion:
                status->setForeground(Qt::darkCyan);
                fileName->setForeground(Qt::darkCyan);
                check->setCheckState(Qt::Checked);
                currentStatusString = QLatin1String("A");
                break;
            case KVersionControlPlugin::LocallyModifiedVersion:
                status->setForeground(Qt::blue);
                fileName->setForeground(Qt::blue);
                check->setCheckState(Qt::Checked);
                currentStatusString = QLatin1String("M");
                break;
            case KVersionControlPlugin::RemovedVersion:
                status->setForeground(Qt::red);
                fileName->setForeground(Qt::red);
                check->setCheckState(Qt::Checked);
                currentStatusString = QLatin1String("R");
                break;
            case KVersionControlPlugin::UnversionedVersion:
                status->setForeground(Qt::darkMagenta);
                fileName->setForeground(Qt::darkMagenta);
                currentStatusString = QLatin1String("?");
                break;
            case KVersionControlPlugin::IgnoredVersion:
                status->setForeground(Qt::black);
                fileName->setForeground(Qt::black);
                currentStatusString = QLatin1String("I");
                break;
            case KVersionControlPlugin::MissingVersion:
                status->setForeground(Qt::black);
                fileName->setForeground(Qt::black);
                currentStatusString = QLatin1String("!");
                break;
            default:
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
    // if all files are selected, clear the list
    if (nChecked == nRowCount) {
        files.clear();
    }
    // at least one file is checked
    if (nChecked > 0) {
        return true;
    }
    //nothing is selected
    return false;
}

void HgStatusList::headerClickedSlot(int index)
{
    if (index == 0) { // first column with check boxes
        m_allWhereChecked = !m_allWhereChecked;
        Qt::CheckState state = m_allWhereChecked ? Qt::Checked : Qt::Unchecked;
        for (int row = 0; row < m_statusTable->rowCount(); row++) {
            m_statusTable->item(row, 0)->setCheckState(state);
        }
        m_statusTable->horizontalHeader()->setSortIndicatorShown(false); // it might be set by 2-nd column
    } else if (index == 2) { // column with file names
        m_sortIndex = !m_sortIndex;;
        if (m_sortIndex) {
            m_statusTable->horizontalHeader()->setSortIndicator(index, Qt::AscendingOrder);
        }
        else {
            m_statusTable->horizontalHeader()->setSortIndicator(index, Qt::DescendingOrder);
        }
        m_statusTable->horizontalHeader()->setSortIndicatorShown(true);
        m_statusTable->sortByColumn(index, Qt::AscendingOrder);
    }
}




#include "moc_statuslist.cpp"
