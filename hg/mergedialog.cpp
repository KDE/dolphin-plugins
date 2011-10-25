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

#include "mergedialog.h"
#include "hgwrapper.h"
#include "commititemdelegate.h"
#include "commitinfowidget.h"
#include "fileviewhgpluginsettings.h"

#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTextEdit>
#include <QtGui/QListWidgetItem>
#include <QtCore/QTextCodec>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kmessagebox.h>

HgMergeDialog::HgMergeDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Merge"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setButtonText(KDialog::Ok, i18nc("@label:button", "Merge"));

    // UI 

    m_currentChangeset = new QLabel;
    m_commitInfoWidget = new HgCommitInfoWidget;

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(m_currentChangeset);
    vbox->addWidget(m_commitInfoWidget);

    QWidget *widget = new QWidget;
    widget->setLayout(vbox);
    setMainWidget(widget);

    updateInitialDialog();

    // load saved geometry
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->mergeDialogWidth(),
                               settings->mergeDialogHeight()));

    // connections
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
}

void HgMergeDialog::updateInitialDialog()
{
    HgWrapper *hgWrapper = HgWrapper::instance();

    // update label - current branch
    QString line("<b>parents:</b> ");
    line += hgWrapper->getParentsOfHead();
    m_currentChangeset->setText(line);

    // update heads list
    QProcess process;
    process.setWorkingDirectory(hgWrapper->getBaseDir());

    QStringList args;
    args << QLatin1String("heads");
    args << QLatin1String("--template");
    args << QLatin1String("{rev}\n{node|short}\n{branch}\n"
                          "{author}\n{desc|firstline}\n");

    process.start(QLatin1String("hg"), args);
    m_commitInfoWidget->clear();

    const int FINAL = 5;
    char buffer[FINAL][1024];
    int count = 0;
    while (process.waitForReadyRead()) {
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
}

void HgMergeDialog::done(int r)
{
    if (r == KDialog::Accepted) {
        HgWrapper *hgw = HgWrapper::instance();

        QListWidgetItem *currentItem = m_commitInfoWidget->currentItem();
        if (currentItem == 0) {
            KMessageBox::error(this,
                    i18nc("@message", "No head selected for merge!"));
            return;
        }

        QString changeset = m_commitInfoWidget->selectedChangeset();
        QStringList args;

        args << QLatin1String("-r");
        args << changeset;

        if (hgw->executeCommandTillFinished(QLatin1String("merge"), args)) {
            KMessageBox::information(this, hgw->readAllStandardOutput());
            KDialog::done(r);
        }
        else {
            KMessageBox::error(this, hgw->readAllStandardError());
            return;
        }
    }
    else {
        KDialog::done(r);
    }
}

void HgMergeDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setMergeDialogHeight(this->height());
    settings->setMergeDialogWidth(this->width());
    settings->writeConfig();
}

#include "mergedialog.moc"

