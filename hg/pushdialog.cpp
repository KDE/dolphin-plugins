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

#include "pushdialog.h"
#include "hgconfig.h"
#include "fileviewhgpluginsettings.h"

#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtGui/QHeaderView>
#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>

HgPushDialog::HgPushDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog),
    m_haveOutgoingChanges(false)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Push Repository"));
    this->setButtons(KDialog::Ok | KDialog::Details | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Push"));
    this->setButtonText(KDialog::Details, i18nc("@action:button", "Options"));
    m_hgw = HgWrapper::instance();
    
    setupUI();
    slotChangeEditUrl(0);
    
    connect(m_selectPathAlias, SIGNAL(currentIndexChanged(int)), 
            this, SLOT(slotChangeEditUrl(int)));
    connect(m_selectPathAlias, SIGNAL(highlighted(int)), 
            this, SLOT(slotChangeEditUrl(int)));
    connect(m_outgoingChangesButton, SIGNAL(clicked()),
            this, SLOT(slotGetOutgoingChanges()));
}

void HgPushDialog::createOptionGroup()
{
    m_optAllowNewBranch = new QCheckBox(i18nc("@label:checkbox",
                "Allow pushing a new branch"));
    m_optInsecure = new QCheckBox(i18nc("@label:checkbox",
                "Do not verify server certificate"));
    m_optForce = new QCheckBox(i18nc("@label:checkbox",
                "Force Push"));
    m_optionGroup = new QGroupBox(i18nc("@label:group",
                "Options"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_optForce);
    layout->addWidget(m_optAllowNewBranch);
    layout->addWidget(m_optInsecure);
    m_optionGroup->setLayout(layout);

    setDetailsWidget(m_optionGroup);
}

void HgPushDialog::setupUI()
{
    HgConfig hgc(HgConfig::RepoConfig);
    m_pathList = hgc.repoRemotePathList();

    // top url bar
    QHBoxLayout *urlLayout = new QHBoxLayout;
    m_selectPathAlias = new KComboBox;
    m_pushUrlEdit = new KLineEdit;
    m_pushUrlEdit->setReadOnly(true);
    QMutableMapIterator<QString, QString> it(m_pathList);
    while (it.hasNext()) {
        it.next();
        m_selectPathAlias->addItem(it.key());
    }
    m_selectPathAlias->addItem(i18nc("@label:combobox", 
                "<edit>"));
    urlLayout->addWidget(m_selectPathAlias);
    urlLayout->addWidget(m_pushUrlEdit);

    // outgoing
    m_outgoingChangesButton = new KPushButton(i18nc("@label:button", 
            "Show Outgoing Changes"));
    m_outgoingChangesButton->setSizePolicy(QSizePolicy::Fixed,
            QSizePolicy::Fixed);

    // dialog's main widget
    QWidget *widget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(urlLayout);

    // outgoing changes
    createOutgoingChangesGroup();
    mainLayout->addWidget(m_outChangesGroup);

    // bottom bar
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    m_statusProg = new QProgressBar;
    m_statusProg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    bottomLayout->addWidget(m_outgoingChangesButton, Qt::AlignLeft);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_statusProg);
    
    //
    mainLayout->addLayout(bottomLayout);
    mainLayout->addStretch();
    widget->setLayout(mainLayout);

    createOptionGroup();
    setMainWidget(widget);
}

void HgPushDialog::createOutgoingChangesGroup()
{
    m_outChangesGroup = new QGroupBox(i18nc("@label:group", 
                "Outgoing Changes"));    
    QHBoxLayout *hbox = new QHBoxLayout;
    m_outChangesList = new QTableWidget;
    m_changesetInfo = new KTextEdit;

    m_outChangesList->setColumnCount(3);
    m_outChangesList->verticalHeader()->hide();
    m_outChangesList->horizontalHeader()->hide();
    m_outChangesList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_outChangesList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_changesetInfo->setFontFamily(QLatin1String("Monospace"));

    hbox->addWidget(m_outChangesList);
    hbox->addWidget(m_changesetInfo);

    m_outChangesGroup->setLayout(hbox);
    m_outChangesGroup->setVisible(false);

    connect(m_outChangesList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotOutSelChanged()));
}

void HgPushDialog::slotOutSelChanged()
{
    if (m_hgw->isBusy()) {
        return;
    }

    QString changeset = m_outChangesList->item(m_outChangesList->currentRow(), 0)->text().split(' ', QString::SkipEmptyParts).takeLast();

    QStringList args; 
    args << QLatin1String("-r");
    args << changeset;
    args << QLatin1String("-v");
    args << QLatin1String("-p");

    QString output;
    m_hgw->executeCommand(QLatin1String("log"), args, output);
    m_changesetInfo->clear();
    m_changesetInfo->setText(output);
}

void HgPushDialog::slotChangeEditUrl(int index)
{
    if (index == m_selectPathAlias->count() - 1) {
        m_pushUrlEdit->setReadOnly(false);
        m_pushUrlEdit->clear();
        m_pushUrlEdit->setFocus();
    }
    else {
        QString url = m_pathList[m_selectPathAlias->itemText(index)];
        m_pushUrlEdit->setText(url);
        m_pushUrlEdit->setReadOnly(true);
    }
}

void HgPushDialog::slotGetOutgoingChanges()
{
    if (m_haveOutgoingChanges) {
        m_outChangesGroup->setVisible(!m_outChangesGroup->isVisible());
        return;
    }
    if (m_process.state() == QProcess::Running) {
        return;
    }
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), 
            this, SLOT(slotOutChangesProcessError(QProcess::ProcessError)));
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotOutChangesProcessComplete(int, QProcess::ExitStatus)));
    m_statusProg->setRange(0, 0);

    QStringList args;
    args << QLatin1String("outgoing");
    args << QLatin1String("--config");
    args << QLatin1String("ui.verbose=False");
    args << QLatin1String("--template");
    args << QLatin1String("Commit: {rev}:{node|short}   "
                    "{date|isodate}   {desc|firstline}\n");
    m_process.setWorkingDirectory(m_hgw->getBaseDir());
    m_process.start(QLatin1String("hg"), args);
}

void HgPushDialog::slotOutChangesProcessError(QProcess::ProcessError error) {
    kDebug() << "Cant get outgoing changes";
}

void HgPushDialog::slotOutChangesProcessComplete(int exitCode, QProcess::ExitStatus status)
{
    m_statusProg->setRange(0, 100);
    m_outChangesList->clearContents();
    //m_outChangesList->horizontalHeader()->setStretchLastSection(true);
    disconnect(&m_process, SIGNAL(error(QProcess::ProcessError)), 
            this, SLOT(slotOutChangesProcessError(QProcess::ProcessError)));
    disconnect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotOutChangesProcessComplete(int, QProcess::ExitStatus)));

    char buffer[512];
    bool unwantedRead = false;
    while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
        QString line(QTextCodec::codecForLocale()->toUnicode(buffer));
        if (unwantedRead ) {
            line.remove(QLatin1String("Commit: "));
            parseUpdateOutgoingChanges(line.trimmed());
        }
        else if (line.startsWith(QLatin1String("Commit: "))) {
            unwantedRead = true;
            line.remove(QLatin1String("Commit: "));
            parseUpdateOutgoingChanges(line.trimmed());
        }
    }

    m_outChangesList->resizeColumnsToContents();
    m_outChangesList->resizeRowsToContents();
    m_outChangesGroup->setVisible(true);
    m_haveOutgoingChanges = true; 
}

void HgPushDialog::parseUpdateOutgoingChanges(const QString &input)
{
    QStringList list = input.split("  ", QString::SkipEmptyParts);
    QTableWidgetItem *changeset = new QTableWidgetItem;
    QTableWidgetItem *date = new QTableWidgetItem;
    QTableWidgetItem *summary = new QTableWidgetItem;

    changeset->setForeground(Qt::red);
    date->setForeground(Qt::blue);

    changeset->setText(list.takeFirst());
    date->setText(list.takeFirst());
    summary->setText(list.takeFirst());

    int rowCount = m_outChangesList->rowCount();
    m_outChangesList->insertRow(rowCount);
    m_outChangesList->setItem(rowCount, 0, changeset);
    m_outChangesList->setItem(rowCount, 1, date);
    m_outChangesList->setItem(rowCount, 2, summary);
}

void HgPushDialog::done(int r)
{
    if (r == KDialog::Accepted) {
    }
    else {
        KDialog::done(r);
    }
}

void HgPushDialog::appendOptionArguments(QStringList &args)
{
}

#include "pushdialog.moc"

