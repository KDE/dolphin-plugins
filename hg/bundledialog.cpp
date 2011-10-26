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

#include "bundledialog.h"
#include "commitinfowidget.h"
#include "pathselector.h"
#include "fileviewhgpluginsettings.h"
#include "hgwrapper.h"

#include <QtGui/QWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QListWidgetItem>
#include <QtGui/QLabel>
#include <QtCore/QTextCodec>
#include <QtCore/QProcess>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kdebug.h>

HgBundleDialog::HgBundleDialog(QWidget *parent) :
    KDialog(parent, Qt::Dialog)
{
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Bundle"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Bundle"));

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->bundleDialogWidth(),
                               settings->bundleDialogHeight()));
    //
    setupUI();

    // connections
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
    connect(m_selectCommitButton, SIGNAL(clicked()), 
            this, SLOT(slotSelectChangeset()));
    connect(m_allChangesets, SIGNAL(stateChanged(int)), 
            this, SLOT(slotAllChangesCheckToggled(int)));
}

void HgBundleDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout;

    // main 
    m_pathSelect = new HgPathSelector;
    m_baseRevision = new KLineEdit;
    m_selectCommitButton = new KPushButton(i18nc("@label:button",
                                "Select Changeset"));
    QLabel *baseRevisionLabel = new QLabel(i18nc("@label", 
                "Base Revision (optional): "));
    m_allChangesets = new QCheckBox(i18nc("@label",
                            "Bundle all changesets in repository."));

    QGridLayout *bodyLayout = new QGridLayout;
    bodyLayout->addWidget(m_pathSelect, 0, 0, 2, 0);
    bodyLayout->addWidget(baseRevisionLabel, 2, 0);
    bodyLayout->addWidget(m_baseRevision, 2, 1);
    bodyLayout->addWidget(m_selectCommitButton, 2, 2);
    bodyLayout->addWidget(m_allChangesets, 3, 0, 2, 0);

    m_mainGroup = new QGroupBox;
    m_mainGroup->setLayout(bodyLayout);

    layout->addWidget(m_mainGroup);

    // options
    m_optionGroup = new QGroupBox(i18nc("@label:group", "Options"));
    m_optForce = new QCheckBox(i18nc("@label:checkbox", 
                                     "Run even when the destination is "
                                     "unrelated (force)"));
    m_optInsecure = new QCheckBox(i18nc("@label:checkbox", 
                             "Do not verify server certificate"));
    
    QVBoxLayout *optionLayout = new QVBoxLayout;
    optionLayout->addWidget(m_optForce);
    optionLayout->addWidget(m_optInsecure);
    m_optionGroup->setLayout(optionLayout);

    layout->addWidget(m_optionGroup);
    //end options

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setMainWidget(widget);
}

void HgBundleDialog::done(int r)
{
    if (r == KDialog::Accepted) {
        QString result = KFileDialog::getSaveFileName();
        if (result.length() > 0) {
            createBundle(result);
            KDialog::done(r);
        }
    }
    else {
        KDialog::done(r);
    }
}

void HgBundleDialog::createBundle(const QString &fileName)
{
    HgWrapper *hgw = HgWrapper::instance();
    QStringList args;
    
    if (m_allChangesets->checkState() == Qt::Checked) {
        args << QLatin1String("--all");
    }
    else {
        if (m_baseRevision->text().trimmed().length() > 0) {
            args << QLatin1String("--base");
            args << m_baseRevision->text().trimmed();
        }
    }

    if (m_optForce->checkState() == Qt::Checked) {
        args << QLatin1String("--force");
    }
    if (m_optInsecure->checkState() == Qt::Checked) {
        args << QLatin1String("--insecure");
    }
    
    args << fileName;
    args << m_pathSelect->remote();

    hgw->executeCommand(QLatin1String("bundle"), args);
}

void HgBundleDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setBundleDialogHeight(this->height());
    settings->setBundleDialogWidth(this->width());
    settings->writeConfig();
}

void HgBundleDialog::loadCommits()
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
    m_commitInfo->clear();

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
            m_commitInfo->addItem(item);
        }
        count = (count + 1)%FINAL;
    }
}

void HgBundleDialog::slotSelectChangeset()
{
    KDialog diag;
    diag.setCaption(i18nc("@title:window", 
                "Select Changeset"));
    diag.setButtons(KDialog::Ok | KDialog::Cancel);
    diag.setDefaultButton(KDialog::Ok);
    diag.setButtonText(KDialog::Ok, i18nc("@action:button", "Select"));

    diag.setMinimumWidth(700);
    
    m_commitInfo = new HgCommitInfoWidget;
    loadCommits();
    diag.setMainWidget(m_commitInfo);
    
    if (diag.exec() == KDialog::Accepted) {
        m_baseRevision->setText(m_commitInfo->selectedChangeset());
    }
}

void HgBundleDialog::slotAllChangesCheckToggled(int state)
{
    if (state == Qt::Checked) {
        m_selectCommitButton->setEnabled(false);
        m_baseRevision->setEnabled(false);
    }
    else {
        m_selectCommitButton->setEnabled(true);
        m_baseRevision->setEnabled(true);
    }
}

#include "bundledialog.moc"

