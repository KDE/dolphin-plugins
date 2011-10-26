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

#include "importdialog.h"
#include "fileviewhgpluginsettings.h"
#include "commititemdelegate.h"
#include "hgwrapper.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QListWidget>
#include <QtCore/QProcess>
#include <QtCore/QTextCodec>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kfiledialog.h>

HgImportDialog::HgImportDialog(QWidget *parent) :
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Import"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Import"));

    //
    setupUI();

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->importDialogWidth(),
                               settings->importDialogHeight()));

    //
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
    connect(m_addPatches, SIGNAL(clicked()), 
            this, SLOT(slotAddPatches()));
    connect(m_removePatches, SIGNAL(clicked()),
            this, SLOT(slotRemovePatches()));
}

void HgImportDialog::setupUI()
{
    QGroupBox *mainGroup = new QGroupBox;
    QGridLayout *mainLayout = new QGridLayout;
    m_patchList = new QListWidget;

    m_patchList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_patchList->setItemDelegate(new CommitItemDelegate);
    mainLayout->addWidget(m_patchList);
    mainGroup->setLayout(mainLayout);

    // options
    m_optionGroup = new QGroupBox(i18nc("@label:group", "Options"));
    m_optNoCommit = new QCheckBox(i18nc("@label", 
                      "Do not commit, just update the working directory"));
    m_optForce = new QCheckBox(i18nc("@label", 
                      "Skip test for outstanding uncommitted changes"));
    m_optExact = new QCheckBox(i18nc("@label",
                   "Apply patch to the nodes from which it was generated"));
    m_optBypass = new QCheckBox(i18nc("@label", 
                      "Apply patch without touching working directory"));

    QVBoxLayout *optionLayout = new QVBoxLayout;
    optionLayout->addWidget(m_optNoCommit);
    optionLayout->addWidget(m_optForce);
    optionLayout->addWidget(m_optExact);
    optionLayout->addWidget(m_optBypass);
    m_optionGroup->setLayout(optionLayout);

    // top buttons
    QHBoxLayout *topButtons = new QHBoxLayout;
    m_addPatches = new KPushButton(i18nc("@label:button",
                        "Add Patches"));
    m_removePatches = new KPushButton(i18nc("@label:button",
                        "Remove Patches"));
    topButtons->addWidget(m_addPatches);
    topButtons->addWidget(m_removePatches);
    topButtons->addStretch();

    //setup main dialog widget
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(topButtons);
    layout->addWidget(mainGroup);
    layout->addWidget(m_optionGroup);
    widget->setLayout(layout);
    setMainWidget(widget);
}

void HgImportDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setImportDialogHeight(this->height());
    settings->setImportDialogWidth(this->width());
    settings->writeConfig();
}

void HgImportDialog::done(int r)
{
    if (r == KDialog::Accepted) {
        QStringList args;
        if (m_optForce->checkState() == Qt::Checked) {
            args << QLatin1String("--force");
        }

        if (m_optBypass->checkState() == Qt::Checked) {
            args << QLatin1String("--bypass");
        }

        if (m_optNoCommit->checkState() == Qt::Checked) {
            args << QLatin1String("--no-commit");
        }
        if (m_optExact->checkState() == Qt::Checked) {
            args << QLatin1String("--exact");
        }

        int countRows = m_patchList->count();
        for (int i=0; i<countRows; i++) {
            QListWidgetItem *item = m_patchList->item(i);
            args << item->data(Qt::UserRole + 5).toString();
        }

        HgWrapper *hgw = HgWrapper::instance();
        if (hgw->executeCommandTillFinished(QLatin1String("import"), args)) {
            KDialog::done(r);
        }
        else {
            KMessageBox::error(this, hgw->readAllStandardError());
        }
    }
    else {
        KDialog::done(r);
    }
}

void HgImportDialog::getPatchInfo(const QString &fileName)
{
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QTextStream fileStream(&file);

    QListWidgetItem *item = new QListWidgetItem;
    item->setData(Qt::UserRole + 1, QString());
    item->setData(Qt::UserRole + 2, QString());
    item->setData(Qt::UserRole + 5, fileName);

    bool gotInfo = false;

    do {
        QString line = fileStream.readLine();
        if (line.startsWith(QLatin1String("diff"))) {
            break;
        }
        else if (line.startsWith(QLatin1String("# User"))) {
            item->setData(Qt::UserRole + 3, 
                    line.remove(QLatin1String("# User")).trimmed());
        }
        else if (line.startsWith(QLatin1String("# Node ID"))) {
            QString node = line.remove(QLatin1String("# Node ID")).trimmed();
            if (!m_patchList->findItems(node, Qt::MatchExactly).empty()) {
                return;
            }
            item->setData(Qt::DisplayRole, node);
        }
        else if (line.startsWith(QLatin1String("# Parent"))) {
            gotInfo = true;
        }
        else if (gotInfo) {
            item->setData(Qt::UserRole + 4, line.trimmed());
            break;
        }
    } while (!fileStream.atEnd());

    m_patchList->addItem(item);

    file.close();
}

void HgImportDialog::slotAddPatches()
{
    QStringList patches = KFileDialog::getOpenFileNames();
    foreach (QString fileName, patches) {
        getPatchInfo(fileName);
    }
}

void HgImportDialog::slotRemovePatches()
{
    int count = m_patchList->count();
    for (int i=0; i<count; i++) {
        m_patchList->takeItem(i);
    }
}

#include "importdialog.moc"

