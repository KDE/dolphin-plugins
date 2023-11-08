/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mergedialog.h"
#include "hgwrapper.h"
#include "commititemdelegate.h"
#include "commitinfowidget.h"
#include "fileviewhgpluginsettings.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QListWidgetItem>
#include <QTextCodec>
#include <KLocalizedString>
#include <KMessageBox>

HgMergeDialog::HgMergeDialog(QWidget *parent):
    DialogBase(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, parent)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Merge"));
    okButton()->setText(xi18nc("@label:button", "Merge"));

    // UI 

    m_currentChangeset = new QLabel;
    m_commitInfoWidget = new HgCommitInfoWidget;

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(m_currentChangeset);
    vbox->addWidget(m_commitInfoWidget);

    layout()->insertLayout(0, vbox);

    updateInitialDialog();

    // load saved geometry
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->resize(QSize(settings->mergeDialogWidth(),
                               settings->mergeDialogHeight()));

    // connections
    connect(this, SIGNAL(finished(int)), this, SLOT(saveGeometry()));
}

void HgMergeDialog::updateInitialDialog()
{
    HgWrapper *hgWrapper = HgWrapper::instance();

    // update label - current branch
    const QString line = QLatin1String("<b>parents:</b> ") + hgWrapper->getParentsOfHead();
    m_currentChangeset->setText(line);

    // update heads list
    QProcess process;
    process.setWorkingDirectory(hgWrapper->getBaseDir());

    const QStringList args{
        QStringLiteral("heads"),
        QStringLiteral("--template"),
        QStringLiteral("{rev}\n{node|short}\n{branch}\n"
                       "{author}\n{desc|firstline}\n"),
    };

    process.start(QStringLiteral("hg"), args);
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
    if (r == QDialog::Accepted) {
        HgWrapper *hgw = HgWrapper::instance();

        QListWidgetItem *currentItem = m_commitInfoWidget->currentItem();
        if (currentItem == nullptr) {
            KMessageBox::error(this,
                    xi18nc("@message", "No head selected for merge!"));
            return;
        }

        QString changeset = m_commitInfoWidget->selectedChangeset();
        const QStringList args{
            QStringLiteral("-r"),
            changeset,
        };

        if (hgw->executeCommandTillFinished(QStringLiteral("merge"), args)) {
            KMessageBox::information(this, hgw->readAllStandardOutput());
            QDialog::done(r);
        }
        else {
            KMessageBox::error(this, hgw->readAllStandardError());
            return;
        }
    }
    else {
        QDialog::done(r);
    }
}

void HgMergeDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setMergeDialogHeight(this->height());
    settings->setMergeDialogWidth(this->width());
    settings->save();
}



#include "moc_mergedialog.cpp"
