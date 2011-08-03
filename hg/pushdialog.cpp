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
#include "pathselector.h"
#include "fileviewhgpluginsettings.h"

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <ktextedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>

HgPushDialog::HgPushDialog(QWidget *parent):
    HgSyncBaseDialog(HgSyncBaseDialog::PushDialog, parent)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Push Repository"));
    this->setButtons(KDialog::Ok | KDialog::Details | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Push"));
    this->setButtonText(KDialog::Details, i18nc("@action:button", "Options"));

    setup();
}

void HgPushDialog::setOptions()
{
    m_optAllowNewBranch = new QCheckBox(i18nc("@label:checkbox",
                "Allow pushing a new branch"));
    m_optInsecure = new QCheckBox(i18nc("@label:checkbox",
                "Do not verify server certificate"));
    m_optForce = new QCheckBox(i18nc("@label:checkbox",
                "Force Push"));
    m_optionGroup = new QGroupBox(i18nc("@label:group",
                "Options"));

    m_options << m_optForce;
    m_options << m_optAllowNewBranch;
    m_options << m_optInsecure;
}

void HgPushDialog::createChangesGroup()
{
    m_changesGroup = new QGroupBox(i18nc("@label:group", 
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

    m_changesGroup->setLayout(hbox);
    m_changesGroup->setVisible(false);

    connect(m_outChangesList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotOutSelChanged()));
    connect(this, SIGNAL(changeListAvailable()), 
            this, SLOT(slotUpdateChangesGeometry()));
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

void HgPushDialog::getHgChangesArguments(QStringList &args)
{
    args << QLatin1String("outgoing");
    args << m_pathSelector->remote();
    args << QLatin1String("--config");
    args << QLatin1String("ui.verbose=False");
    args << QLatin1String("--template");
    args << QLatin1String("Commit: {rev}:{node|short}   "
                    "{date|isodate}   {desc|firstline}\n");
}

void HgPushDialog::parseUpdateChanges(const QString &input)
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

void HgPushDialog::appendOptionArguments(QStringList &args)
{
    if (m_optForce->isChecked()) {
        args << QLatin1String("--force");
    }
    if (m_optAllowNewBranch->isChecked()) {
        args << QLatin1String("--new-branch");
    }
    if (m_optInsecure->isChecked()) {
        args << QLatin1String("--insecure");
    }
}

void HgPushDialog::slotUpdateChangesGeometry()
{
    m_outChangesList->resizeColumnsToContents();
    m_outChangesList->resizeRowsToContents();
    m_outChangesList->horizontalHeader()->setStretchLastSection(true);
}

void HgPushDialog::readBigSize()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    m_bigSize = QSize(settings->pushDialogBigWidth(), settings->pushDialogBigHeight());
}

void HgPushDialog::writeBigSize()
{
    kDebug() << "Saving geometry";
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setPushDialogBigWidth(m_bigSize.width());
    settings->setPushDialogBigHeight(m_bigSize.height());
    settings->writeConfig();
}

void HgPushDialog::noChangesMessage()
{
    KMessageBox::information(this, i18nc("@message:info",
                "No outgoing changes!"));
}

#include "pushdialog.moc"

