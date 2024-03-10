/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ignorewidget.h"
#include "../hgwrapper.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QFile>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QList>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>

HgIgnoreWidget::HgIgnoreWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadConfig();
}

void HgIgnoreWidget::setupUI()
{
    QVBoxLayout *sideBar = new QVBoxLayout;
    m_addFiles = new QPushButton(xi18nc("@label:button", "Add Files"));
    m_addPattern = new QPushButton(xi18nc("@label:button", "Add Pattern"));
    m_editEntry = new QPushButton(xi18nc("@label:button", "Edit Entry"));
    m_removeEntries = new QPushButton(xi18nc("@label:button", "Remove Entries"));
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
    args << QStringLiteral("--unknown");
    QString output;
    hgw->executeCommand(QStringLiteral("status"), args, output);

    const QStringList result = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &file : result) {
        m_untrackedList->addItem(file.mid(2));
    }
}

void HgIgnoreWidget::loadConfig()
{
    QFile file(HgWrapper::instance()->getBaseDir() + QLatin1String("/.hgignore"));
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
    QFile file(HgWrapper::instance()->getBaseDir() + QLatin1String("/.hgignore"));
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return;
    }

    QTextStream fileStream(&file);
    int count = m_ignoreTable->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = m_ignoreTable->item(i);
        fileStream << item->text() << QLatin1String("\n");
    }

    file.close();
}

void HgIgnoreWidget::slotAddFiles()
{
    const QList<QListWidgetItem *> selectedItems = m_untrackedList->selectedItems();
    for (QListWidgetItem *item : selectedItems) {
        m_ignoreTable->addItem(item->text());
        m_untrackedList->takeItem(m_untrackedList->row(item));
    }
}

void HgIgnoreWidget::slotAddPattern()
{
    bool ok;
    QString input = QInputDialog::getText(this, xi18nc("@title:dialog", "Add Pattern"), QString(), QLineEdit::Normal, QString(), &ok);
    if (ok && !input.isEmpty()) {
        m_ignoreTable->addItem(input);
    }
}

void HgIgnoreWidget::slotRemoveEntries()
{
    const QList<QListWidgetItem *> selectedItems = m_ignoreTable->selectedItems();
    for (QListWidgetItem *item : selectedItems) {
        m_ignoreTable->takeItem(m_ignoreTable->row(item));
    }
}
void HgIgnoreWidget::slotEditEntry()
{
    if (m_ignoreTable->currentItem() == nullptr) {
        KMessageBox::error(this, xi18nc("@message:error", "No entry selected for edit!"));
        return;
    }

    bool ok;
    QString input =
        QInputDialog::getText(this, xi18nc("@title:dialog", "Edit Pattern"), QString(), QLineEdit::Normal, m_ignoreTable->currentItem()->text(), &ok);
    if (ok && !input.isEmpty()) {
        m_ignoreTable->currentItem()->setText(input);
    }
}

#include "moc_ignorewidget.cpp"
