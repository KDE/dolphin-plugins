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

#include "exportdialog.h"
#include "fileviewhgpluginsettings.h"
#include "commitinfowidget.h"
#include "hgwrapper.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QListWidget>
#include <QtCore/QProcess>
#include <QtCore/QTextCodec>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

HgExportDialog::HgExportDialog(QWidget *parent) :
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Export"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Export"));

    //
    setupUI();
    loadCommits();

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->exportDialogWidth(),
                               settings->exportDialogHeight()));

    //
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
}

void HgExportDialog::setupUI()
{
    QGroupBox *mainGroup = new QGroupBox;
    QGridLayout *mainLayout = new QGridLayout;
    m_commitInfoWidget = new HgCommitInfoWidget;

    m_commitInfoWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mainLayout->addWidget(m_commitInfoWidget);
    mainGroup->setLayout(mainLayout);

    // options
    m_optionGroup = new QGroupBox(i18nc("@label:group", "Options"));
    m_optText = new QCheckBox(i18nc("@label", "Treat all files as text"));
    m_optGit = new QCheckBox(i18nc("@label", "Use Git extended diff format"));
    m_optNoDates = new QCheckBox(i18nc("@label", "Omit dates from diff headers"));
    QVBoxLayout *optionLayout = new QVBoxLayout;
    optionLayout->addWidget(m_optText);
    optionLayout->addWidget(m_optGit);
    optionLayout->addWidget(m_optNoDates);
    m_optionGroup->setLayout(optionLayout);

    //setup main dialog widget
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mainGroup);
    layout->addWidget(m_optionGroup);
    widget->setLayout(layout);
    setMainWidget(widget);
}

void HgExportDialog::loadCommits()
{
    HgWrapper *hgWrapper = HgWrapper::instance();

    // update heads list
    QProcess process;
    process.setWorkingDirectory(hgWrapper->getBaseDir());

    QStringList args;
    args << QLatin1String("log");
    args << QLatin1String("--template");
    args << QLatin1String("{rev}\n{node|short}\n{branch}\n"
                          "{author}\n{desc|firstline}\n");

    process.start(QLatin1String("hg"), args);
    process.waitForFinished();
    m_commitInfoWidget->clear();

    const int FINAL = 5;
    char buffer[FINAL][1024];
    int count = 0;
    while (process.readLine(buffer[count], sizeof(buffer[count])) > 0) {
        if (count == FINAL - 1) {
            QString rev = QTextCodec::codecForLocale()->toUnicode(buffer[0]).trimmed();
            QString changeset = QTextCodec::codecForLocale()->toUnicode(buffer[1]).trimmed();
            QString branch = QTextCodec::codecForLocale()->toUnicode(buffer[2]).trimmed();
            QString author = QTextCodec::codecForLocale()->toUnicode(buffer[3]).trimmed();
            QString log = QTextCodec::codecForLocale()->toUnicode(buffer[4]).trimmed();

            QListWidgetItem *item = new QListWidgetItem;
            item->setData(Qt::DisplayRole, changeset);
            item->setData(Qt::UserRole + 1, rev);
            item->setData(Qt::UserRole + 2, branch);
            item->setData(Qt::UserRole + 3, author);
            item->setData(Qt::UserRole + 4, log);
            m_commitInfoWidget->addItem(item);
        }
        count = (count + 1)%FINAL;
    }
}

void HgExportDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setExportDialogHeight(this->height());
    settings->setExportDialogWidth(this->width());
    settings->writeConfig();
}

void HgExportDialog::done(int r)
{
    if (r == KDialog::Accepted) {
        QList<QListWidgetItem*> items = m_commitInfoWidget->selectedItems();
        if (items.empty()) {
            KMessageBox::error(this, i18nc("@message:error",
                     "Please select at least one changeset to be exported!"));
            return;
        }

        QStringList args;
        if (m_optText->checkState() == Qt::Checked) {
            args << QLatin1String("--text");
        }
        if (m_optGit->checkState() == Qt::Checked) {
            args << QLatin1String("--git");
        }
        if (m_optNoDates->checkState() == Qt::Checked) {
            args << QLatin1String("--nodates");
        }

        args << QLatin1String("-r");
        foreach (QListWidgetItem *item, items) {
            args << item->data(Qt::DisplayRole).toString();
        }

        QString directory = KFileDialog::getExistingDirectory();
        if (directory.isEmpty()) {
            return;
        }
        if (!directory.endsWith('/')) {
            directory.append('/');
        }

        args << QLatin1String("--output");
        args << directory + QLatin1String("%b_%h.patch");

        HgWrapper *hgw = HgWrapper::instance();
        if (hgw->executeCommandTillFinished(QLatin1String("export"), args)) {
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

#include "exportdialog.moc"

