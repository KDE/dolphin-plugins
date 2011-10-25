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

#include "pulldialog.h"
#include "hgwrapper.h"
#include "hgconfig.h"
#include "pathselector.h"
#include "fileviewhgpluginsettings.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtGui/QHeaderView>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QTableWidget>
#include <QtCore/QString>
#include <ktextedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>

HgPullDialog::HgPullDialog(QWidget *parent):
    HgSyncBaseDialog(HgSyncBaseDialog::PullDialog, parent)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Pull Repository"));
    this->setButtons(KDialog::Ok | KDialog::Details | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Pull"));
    this->setButtonText(KDialog::Details, i18nc("@action:button", "Options"));

    setup();
}

void HgPullDialog::setOptions()
{
    m_optUpdate = new QCheckBox(i18nc("@label:checkbox",
                "Update to new branch head if changesets were pulled"));
    m_optInsecure = new QCheckBox(i18nc("@label:checkbox",
                "Do not verify server certificate"));
    m_optForce = new QCheckBox(i18nc("@label:checkbox",
                "Force Pull"));
    m_optionGroup = new QGroupBox(i18nc("@label:group",
                "Options"));

    m_options << m_optForce;
    m_options << m_optUpdate;
    m_options << m_optInsecure;
}

void HgPullDialog::createChangesGroup()
{
    m_changesGroup = new QGroupBox(i18nc("@label:group", 
                "Incoming Changes"));    
    QHBoxLayout *hbox = new QHBoxLayout;
    m_changesList = new QTableWidget;

    m_changesList->setColumnCount(4);
    m_changesList->verticalHeader()->hide();
    m_changesList->horizontalHeader()->hide();
    m_changesList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_changesList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    hbox->addWidget(m_changesList);

    m_changesGroup->setLayout(hbox);
    m_changesGroup->setVisible(false);

    connect(this, SIGNAL(changeListAvailable()), 
            this, SLOT(slotUpdateChangesGeometry()));
}

void HgPullDialog::getHgChangesArguments(QStringList &args)
{
    args << QLatin1String("incoming");
    args << m_pathSelector->remote();
    args << QLatin1String("--config");
    args << QLatin1String("ui.verbose=False");
    args << QLatin1String("--template");
    args << QLatin1String("Commit: {rev}:{node|short}   "
                    "{author}  "
                    "{date|isodate}   {desc|firstline}\n");
}

void HgPullDialog::parseUpdateChanges(const QString &input)
{
    QStringList list = input.split("  ", QString::SkipEmptyParts);
    QTableWidgetItem *author = new QTableWidgetItem;
    QTableWidgetItem *changeset = new QTableWidgetItem;
    QTableWidgetItem *date = new QTableWidgetItem;
    QTableWidgetItem *summary = new QTableWidgetItem;

    author->setForeground(Qt::darkRed);
    changeset->setForeground(Qt::red);
    date->setForeground(Qt::blue);

    author->setText(list.takeFirst());
    changeset->setText(list.takeFirst());
    date->setText(list.takeFirst());
    summary->setText(list.takeFirst());

    int rowCount = m_changesList->rowCount();
    m_changesList->insertRow(rowCount);
    m_changesList->setItem(rowCount, 0, author);
    m_changesList->setItem(rowCount, 1, changeset);
    m_changesList->setItem(rowCount, 2, date);
    m_changesList->setItem(rowCount, 3, summary);
}

void HgPullDialog::appendOptionArguments(QStringList &args)
{
    if (m_optForce->isChecked()) {
        args << QLatin1String("--force");
    }
    if (m_optUpdate->isChecked()) {
        args << QLatin1String("--update");
    }
    if (m_optInsecure->isChecked()) {
        args << QLatin1String("--insecure");
    }
}

void HgPullDialog::slotUpdateChangesGeometry()
{
    m_changesList->resizeColumnsToContents();
    m_changesList->resizeRowsToContents();
    m_changesList->horizontalHeader()->setStretchLastSection(true);
}

void HgPullDialog::readBigSize()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    m_bigSize = QSize(settings->pullDialogBigWidth(), settings->pushDialogBigHeight());
}

void HgPullDialog::writeBigSize()
{
    kDebug() << "Saving geometry";
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setPullDialogBigWidth(m_bigSize.width());
    settings->setPullDialogBigHeight(m_bigSize.height());
    settings->writeConfig();
}

void HgPullDialog::noChangesMessage()
{
    KMessageBox::information(this, i18nc("@message:info",
                "No incoming changes!"));
}

#include "pulldialog.moc"

