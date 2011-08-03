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
#include <QtGui/QInputDialog>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QTextStream>
#include <kurl.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

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
    m_untrackedList = new QListWidget;
    setupUntrackedList();

    m_ignoreTable->setSelectionMode(QListWidget::ExtendedSelection);
    m_untrackedList->setSelectionMode(QListWidget::ExtendedSelection);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_untrackedList);
    mainLayout->addWidget(m_ignoreTable);
    mainLayout->addLayout(sideBar);
    setLayout(mainLayout);

    connect(m_addFiles, SIGNAL(clicked()), this, SLOT(slotAddFiles()));
    connect(m_removeEntries, SIGNAL(clicked()), this, SLOT(slotRemoveEntries()));
    connect(m_addPattern, SIGNAL(clicked()), this, SLOT(slotAddPattern()));
    connect(m_editEntry, SIGNAL(clicked()), this, SLOT(slotEditEntry()));
}

void HgIgnoreWidget::setupUntrackedList()
{
    HgWrapper *hgw = HgWrapper::instance();
    QStringList args;
    args << QLatin1String("--unknown");
    QString output;
    hgw->executeCommand(QLatin1String("status"), args, output);
    
    QStringList result = output.split('\n', QString::SkipEmptyParts);
    foreach (QString file, result) {
        m_untrackedList->addItem(file.mid(2));
    }
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

    file.close();
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
    }

    file.close();
}

void HgIgnoreWidget::slotAddFiles()
{
    QList<QListWidgetItem*> selectedItems = m_untrackedList->selectedItems();
    foreach (QListWidgetItem *item, selectedItems) {
        m_ignoreTable->addItem(item->text());
        m_untrackedList->takeItem(m_untrackedList->row(item));
    }
}

void HgIgnoreWidget::slotAddPattern()
{
    bool ok;
    QString input = QInputDialog::getText(this, 
                        i18nc("@title:dialog", "Add Pattern"),  
                        QString(),
                        QLineEdit::Normal,
                        QString(),
                        &ok);
    if (ok && !input.isEmpty()) {
        m_ignoreTable->addItem(input);
    }
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
    if (m_ignoreTable->currentItem() == 0) {
        KMessageBox::error(this, i18nc("@message:error",
                    "No entry selected for edit!"));
        return;
    }

    bool ok;
    QString input = QInputDialog::getText(this, 
                        i18nc("@title:dialog", "Edit Pattern"),  
                        QString(),
                        QLineEdit::Normal,
                        m_ignoreTable->currentItem()->text(),
                        &ok);
    if (ok && !input.isEmpty()) {
        m_ignoreTable->currentItem()->setText(input);
    }
}


#include "ignorewidget.moc"

