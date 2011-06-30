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

HgPullDialog::HgPullDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog),
    m_haveIncomingChanges(false),
    m_currentTask(NoTask)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Pull Repository"));
    this->setButtons(KDialog::Ok | KDialog::Details | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Pull"));
    this->setButtonText(KDialog::Details, i18nc("@action:button", "Options"));
    m_hgw = HgWrapper::instance();
    
    setupUI();
    slotChangeEditUrl(0);
    
    connect(m_selectPathAlias, SIGNAL(currentIndexChanged(int)), 
            this, SLOT(slotChangeEditUrl(int)));
    connect(m_selectPathAlias, SIGNAL(highlighted(int)), 
            this, SLOT(slotChangeEditUrl(int)));
    connect(m_incomingChangesButton, SIGNAL(clicked()),
            this, SLOT(slotGetIncomingChanges()));
}

void HgPullDialog::createOptionGroup()
{
    m_optUpdate = new QCheckBox(i18nc("@label:checkbox",
                "Update to new branch head if changesets were pulled"));
    m_optInsecure = new QCheckBox(i18nc("@label:checkbox",
                "Do not verify server certificate"));
    m_optForce = new QCheckBox(i18nc("@label:checkbox",
                "Force Pull"));
    m_optionGroup = new QGroupBox(i18nc("@label:group",
                "Options"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_optForce);
    layout->addWidget(m_optUpdate);
    layout->addWidget(m_optInsecure);
    m_optionGroup->setLayout(layout);

    setDetailsWidget(m_optionGroup);
}

void HgPullDialog::setupUI()
{
    HgConfig hgc(HgConfig::RepoConfig);
    m_pathList = hgc.repoRemotePathList();

    // top url bar
    QHBoxLayout *urlLayout = new QHBoxLayout;
    m_selectPathAlias = new KComboBox;
    m_pullUrlEdit = new KLineEdit;
    m_pullUrlEdit->setReadOnly(true);
    QMutableMapIterator<QString, QString> it(m_pathList);
    while (it.hasNext()) {
        it.next();
        m_selectPathAlias->addItem(it.key());
    }
    m_selectPathAlias->addItem(i18nc("@label:combobox", 
                "<edit>"));
    urlLayout->addWidget(m_selectPathAlias);
    urlLayout->addWidget(m_pullUrlEdit);

    // incoming
    m_incomingChangesButton = new KPushButton(i18nc("@label:button", 
            "Show Incoming Changes"));
    m_incomingChangesButton->setSizePolicy(QSizePolicy::Fixed,
            QSizePolicy::Fixed);

    // dialog's main widget
    QWidget *widget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(urlLayout);

    // incoming changes
    createIncomingChangesGroup();
    mainLayout->addWidget(m_incChangesGroup);

    // bottom bar
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    m_statusProg = new QProgressBar;
    m_statusProg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    bottomLayout->addWidget(m_incomingChangesButton, Qt::AlignLeft);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_statusProg);
    
    //
    mainLayout->addLayout(bottomLayout);
    mainLayout->addStretch();
    widget->setLayout(mainLayout);

    createOptionGroup();
    setMainWidget(widget);
}

void HgPullDialog::createIncomingChangesGroup()
{
    m_incChangesGroup = new QGroupBox(i18nc("@label:group", 
                "Incoming Changes"));    
    QHBoxLayout *hbox = new QHBoxLayout;
    m_incChangesList = new QTableWidget;
    //m_changesetInfo = new KTextEdit;

    m_incChangesList->setColumnCount(4);
    m_incChangesList->verticalHeader()->hide();
    m_incChangesList->horizontalHeader()->hide();
    m_incChangesList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_incChangesList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //m_changesetInfo->setFontFamily(QLatin1String("Monospace"));

    hbox->addWidget(m_incChangesList);
    //hbox->addWidget(m_changesetInfo);

    m_incChangesGroup->setLayout(hbox);
    m_incChangesGroup->setVisible(false);

    /*connect(m_incChangesList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotIncSelChanged()));*/
}
/*
void HgPullDialog::slotIncSelChanged()
{
    if (m_hgw->isBusy()) {
        return;
    }

    QString changeset = m_incChangesList->item(m_incChangesList->currentRow(), 0)->text().split(' ', QString::SkipEmptyParts).takeLast();

    QStringList args; 
    args << QLatin1String("-r");
    args << changeset;
    args << QLatin1String("-v");
    args << QLatin1String("-p");

    QString output;
    m_hgw->executeCommand(QLatin1String("log"), args, output);
    m_changesetInfo->clear();
    m_changesetInfo->setText(output);
}*/

void HgPullDialog::slotChangeEditUrl(int index)
{
    if (index == m_selectPathAlias->count() - 1) {
        m_pullUrlEdit->setReadOnly(false);
        m_pullUrlEdit->clear();
        m_pullUrlEdit->setFocus();
    }
    else {
        QString url = m_pathList[m_selectPathAlias->itemText(index)];
        m_pullUrlEdit->setText(url);
        m_pullUrlEdit->setReadOnly(true);
    }
}

void HgPullDialog::slotGetIncomingChanges()
{
    if (m_haveIncomingChanges) {
        m_incChangesGroup->setVisible(!m_incChangesGroup->isVisible());
        return;
    }
    if (m_process.state() == QProcess::Running) {
        return;
    }
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), 
            this, SLOT(slotIncChangesProcessError(QProcess::ProcessError)));
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotIncChangesProcessComplete(int, QProcess::ExitStatus)));
    m_statusProg->setRange(0, 0);
    m_incomingChangesButton->setEnabled(false);

    QStringList args;
    args << QLatin1String("incoming");
    args << pullPath();
    args << QLatin1String("--config");
    args << QLatin1String("ui.verbose=False");
    args << QLatin1String("--template");
    args << QLatin1String("Commit: {rev}:{node|short}   "
                    "{author}  "
                    "{date|isodate}   {desc|firstline}\n");
    m_process.setWorkingDirectory(m_hgw->getBaseDir());
    m_process.start(QLatin1String("hg"), args);
}

QString HgPullDialog::pullPath() const
{
    return (m_selectPathAlias->currentIndex() == m_selectPathAlias->count()-1)?m_pullUrlEdit->text():m_selectPathAlias->currentText();

}

void HgPullDialog::slotIncChangesProcessError(QProcess::ProcessError error) {
    kDebug() << "Cant get incoming changes";
    KMessageBox::error(this, i18n("Can't get incoming changes!"));
    m_incomingChangesButton->setEnabled(true);
}

void HgPullDialog::slotIncChangesProcessComplete(int exitCode, QProcess::ExitStatus status)
{
    m_statusProg->setRange(0, 100);
    m_incomingChangesButton->setEnabled(true);
    m_incChangesList->clearContents();
    //m_incChangesList->horizontalHeader()->setStretchLastSection(true);
    disconnect(&m_process, SIGNAL(error(QProcess::ProcessError)), 
            this, SLOT(slotIncChangesProcessError(QProcess::ProcessError)));
    disconnect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotIncChangesProcessComplete(int, QProcess::ExitStatus)));

    char buffer[512];
    bool unwantedRead = false;
    while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
        QString line(QTextCodec::codecForLocale()->toUnicode(buffer));
        if (unwantedRead ) {
            line.remove(QLatin1String("Commit: "));
            parseUpdateIncomingChanges(line.trimmed());
        }
        else if (line.startsWith(QLatin1String("Commit: "))) {
            unwantedRead = true;
            line.remove(QLatin1String("Commit: "));
            parseUpdateIncomingChanges(line.trimmed());
        }
    }

    m_incChangesList->resizeColumnsToContents();
    m_incChangesList->resizeRowsToContents();
    m_incChangesGroup->setVisible(true);
    m_haveIncomingChanges = true; 
}

void HgPullDialog::parseUpdateIncomingChanges(const QString &input)
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

    int rowCount = m_incChangesList->rowCount();
    m_incChangesList->insertRow(rowCount);
    m_incChangesList->setItem(rowCount, 0, author);
    m_incChangesList->setItem(rowCount, 1, changeset);
    m_incChangesList->setItem(rowCount, 2, date);
    m_incChangesList->setItem(rowCount, 3, summary);
}

void HgPullDialog::done(int r)
{
    if (r == KDialog::Accepted) {
        QStringList args;
        args << pullPath();
        appendOptionArguments(args);

        enableButtonOk(false);
        m_statusProg->setRange(0, 0);
        connect(m_hgw, SIGNAL(finished(int, QProcess::ExitStatus)), 
                this, SLOT(slotPullComplete(int, QProcess::ExitStatus)));
        connect(m_hgw, SIGNAL(error(QProcess::ProcessError)), 
                this, SLOT(slotPullError(QProcess::ProcessError)));
        
        m_hgw->executeCommand(QLatin1String("pull"), args);
    }
    else {
        KDialog::done(r);
    }
}

void HgPullDialog::slotPullComplete(int exitCode, QProcess::ExitStatus status)
{
    m_statusProg->setRange(0, 100);
    disconnect(m_hgw, SIGNAL(finished()), this, SLOT(slotPullComplete()));

    if (exitCode == 0) {
        KDialog::done(KDialog::Accepted);
    }
    else {
        enableButtonOk(true);
        KMessageBox::error(this, i18n("Error while pulling the changes!"));
    }
}

void HgPullDialog::slotPullError(QProcess::ProcessError)
{
    disconnect(m_hgw, SIGNAL(finished()), this, SLOT(slotPullComplete()));
    enableButtonOk(true);
    KMessageBox::error(this, i18n("Error while pulling the changes!"));
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

#include "pulldialog.moc"

