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

#include "backoutdialog.h"
#include "commitinfowidget.h"
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
#include <klocale.h>
#include <kdebug.h>

HgBackoutDialog::HgBackoutDialog(QWidget *parent) :
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Backout"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Backout"));
    this->enableButtonOk(false);

    //
    setupUI();

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->backoutDialogWidth(),
                               settings->backoutDialogHeight()));

    // connections
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
    connect(m_selectBaseCommitButton, SIGNAL(clicked()), 
            this, SLOT(slotSelectBaseChangeset()));
    connect(m_selectParentCommitButton, SIGNAL(clicked()), 
            this, SLOT(slotSelectParentChangeset()));
    connect(m_baseRevision, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotUpdateOkButton(const QString&)));
}

void HgBackoutDialog::setupUI()
{
    m_mainGroup = new QGroupBox;
    m_baseRevision = new KLineEdit;
    m_parentRevision = new KLineEdit;
    m_optMerge = new QCheckBox(i18nc("@label:checkbox", 
                        "Merge with old dirstate parent after backout"));
    m_selectParentCommitButton = new KPushButton(i18nc("@label:button",
                                                "Select Changeset"));
    m_selectBaseCommitButton = new KPushButton(i18nc("@label:button", 
                                            "Select Changeset"));
    QGridLayout *mainGroupLayout = new QGridLayout;

    mainGroupLayout->addWidget(new QLabel(i18nc("@label", 
                                    "Revision to Backout: ")), 0, 0);
    mainGroupLayout->addWidget(m_baseRevision, 0, 1);
    mainGroupLayout->addWidget(m_selectBaseCommitButton, 0, 2);

    mainGroupLayout->addWidget(new QLabel(i18nc("@label", 
                                    "Parent Revision (optional): ")), 1, 0);
    mainGroupLayout->addWidget(m_parentRevision, 1, 1);
    mainGroupLayout->addWidget(m_selectParentCommitButton, 1, 2);

    mainGroupLayout->addWidget(m_optMerge, 2, 0, 1, 0);

    m_mainGroup->setLayout(mainGroupLayout);

    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_mainGroup);
    widget->setLayout(layout);
    setMainWidget(widget);
}

void HgBackoutDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setBackoutDialogHeight(this->height());
    settings->setBackoutDialogWidth(this->width());
    settings->writeConfig();
}

void HgBackoutDialog::loadCommits()
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

QString HgBackoutDialog::selectChangeset()
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
        return m_commitInfo->selectedChangeset();
    }
    return QString();
}

void HgBackoutDialog::slotSelectBaseChangeset()
{
    QString changeset = selectChangeset();
    if (!changeset.isEmpty()) {
        m_baseRevision->setText(changeset);
    }
}

void HgBackoutDialog::slotSelectParentChangeset()
{
    QString changeset = selectChangeset();
    if (!changeset.isEmpty()) {
        m_parentRevision->setText(changeset);
    }
}

void HgBackoutDialog::done(int r)
{
    if (r == KDialog::Accepted) {
        HgWrapper *hgw = HgWrapper::instance();
        QStringList args;
        args << QLatin1String("--rev");
        args << m_baseRevision->text();

        if (!m_parentRevision->text().isEmpty()) {
            args << QLatin1String("--parent");
            args << m_parentRevision->text();
        }

        if (m_optMerge->checkState() == Qt::Checked) {
            args << QLatin1String("--merge");
        }

        if (hgw->executeCommandTillFinished(QLatin1String("backout"), args)) {
            KMessageBox::information(this, hgw->readAllStandardOutput());
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

void HgBackoutDialog::slotUpdateOkButton(const QString &text)
{
    kDebug() << "text";
    enableButtonOk(!text.isEmpty());
}

#include "backoutdialog.moc"

