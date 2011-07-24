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

#include "ignorewidget.h"
#include "../hgwrapper.h"

#include <QtGui/QListWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QTreeView>
#include <QtGui/QFileSystemModel>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QTextStream>
#include <kurl.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kdebug.h>

HgIgnoreWidget::HgIgnoreWidget(QWidget *parent) :
    QWidget(parent)
{
    setupUI();
    loadConfig();
}

void HgIgnoreWidget::setupUI()
{
    QVBoxLayout *sideBar = new QVBoxLayout;
    m_addFiles = new KPushButton(i18nc("@label:button", "Add Files"));
    m_addPattern = new KPushButton(i18nc("@label:button", "Add Pattern"));
    m_editEntry = new KPushButton(i18nc("@label:button", "Edit Entry"));
    m_removeEntries = new KPushButton(i18nc("@label:button", "Remove Entries"));
    sideBar->addWidget(m_addFiles);
    sideBar->addWidget(m_addPattern);
    sideBar->addWidget(m_editEntry);
    sideBar->addWidget(m_removeEntries);
    sideBar->addStretch();

    m_ignoreTable = new QListWidget;
    m_fsTable = new QTreeView;
    m_fsModel = new QFileSystemModel;

    m_fsModel->setRootPath(HgWrapper::instance()->getBaseDir());
    kDebug() << m_fsModel->rootDirectory();
    m_fsTable->setModel(m_fsModel);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_fsTable);
    mainLayout->addWidget(m_ignoreTable);
    mainLayout->addLayout(sideBar);
    setLayout(mainLayout);

    connect(m_addFiles, SIGNAL(clicked()), this, SLOT(slotAddFiles()));
    connect(m_removeEntries, SIGNAL(clicked()), this, SLOT(slotRemoveEntries()));
    connect(m_addPattern, SIGNAL(clicked()), this, SLOT(slotAddPattern()));
    connect(m_editEntry, SIGNAL(clicked()), this, SLOT(slotEditEntry()));
}

void HgIgnoreWidget::loadConfig()
{
    KUrl url(HgWrapper::instance()->getBaseDir());
    url.addPath(QLatin1String(".hgignore"));

    QFile file(url.path());
    if (!file.open(QFile::ReadOnly)) {
        return;
    }
    
    QTextStream fileStream(&file);

    do {
        QString buffer;
        buffer = fileStream.readLine();
        if (!buffer.isEmpty()) {
            m_ignoreTable->addItem(buffer);
        }
    } while (!fileStream.atEnd());
}

void HgIgnoreWidget::saveConfig()
{
    KUrl url(HgWrapper::instance()->getBaseDir());
    url.addPath(QLatin1String(".hgignore"));

    QFile file(url.path());
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return;
    }
    
    QTextStream fileStream(&file);
    int count = m_ignoreTable->count();
    for (int i=0; i<count; i++) {
        QListWidgetItem *item = m_ignoreTable->item(i);
        fileStream << item->text() << QLatin1String("\n");
        kDebug() << item->text();
    }
}

void HgIgnoreWidget::slotAddFiles()
{
}

void HgIgnoreWidget::slotAddPattern()
{
}

void HgIgnoreWidget::slotRemoveEntries()
{
    QList<QListWidgetItem*> selectedItems = m_ignoreTable->selectedItems();
    foreach (QListWidgetItem *item, selectedItems) {
        m_ignoreTable->takeItem(m_ignoreTable->row(item));
    }
}
void HgIgnoreWidget::slotEditEntry()
{
}


#include "ignorewidget.moc"
