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

#include "syncdialogbase.h"
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

HgSyncBaseDialog::HgSyncBaseDialog(DialogType dialogType, QWidget *parent):
    KDialog(parent, Qt::Dialog),
    m_dialogType(dialogType),
    m_haveChanges(false),
    m_currentTask(NoTask)
{
    m_hgw = HgWrapper::instance();
    
    setupUI();
    slotChangeEditUrl(0);
    
    connect(m_selectPathAlias, SIGNAL(currentIndexChanged(int)), 
            this, SLOT(slotChangeEditUrl(int)));
    connect(m_selectPathAlias, SIGNAL(highlighted(int)), 
            this, SLOT(slotChangeEditUrl(int)));
    connect(m_changesButton, SIGNAL(clicked()),
            this, SLOT(slotGetChanges()));
}

void HgSyncBaseDialog::createOptionGroup()
{
    setOptions();
    QVBoxLayout *layout = new QVBoxLayout;

    foreach (QCheckBox *cb, m_options) {
        layout->addWidget(cb);
    }

    m_optionGroup->setLayout(layout);
    setDetailsWidget(m_optionGroup);
}

void HgSyncBaseDialog::setupUI()
{
    HgConfig hgc(HgConfig::RepoConfig);
    m_pathList = hgc.repoRemotePathList();

    // top url bar
    QHBoxLayout *urlLayout = new QHBoxLayout;
    m_selectPathAlias = new KComboBox;
    m_urlEdit = new KLineEdit;
    m_urlEdit->setReadOnly(true);
    QMutableMapIterator<QString, QString> it(m_pathList);
    while (it.hasNext()) {
        it.next();
        m_selectPathAlias->addItem(it.key());
    }
    m_selectPathAlias->addItem(i18nc("@label:combobox", 
                "<edit>"));
    urlLayout->addWidget(m_selectPathAlias);
    urlLayout->addWidget(m_urlEdit);

    // changes button
    //FIXME not very good idea. Bad HACK 
    if (dialogType == PullDialog) {
        m_changesButton = new KPushButton(i18nc("@label:button", 
                "Show Incoming Changes"));
    }
    else {
        m_changesButton = new KPushButton(i18nc("@label:button", 
                "Show Outgoing Changes"));
    }
    m_changesButton->setSizePolicy(QSizePolicy::Fixed,
            QSizePolicy::Fixed);

    // dialog's main widget
    QWidget *widget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(urlLayout);

    // incoming changes
    createChangesGroup();
    mainLayout->addWidget(m_changesGroup);

    // bottom bar
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    m_statusProg = new QProgressBar;
    m_statusProg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    bottomLayout->addWidget(m_changesButton, Qt::AlignLeft);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_statusProg);
    
    //
    mainLayout->addLayout(bottomLayout);
    mainLayout->addStretch();
    widget->setLayout(mainLayout);

    createOptionGroup();
    setMainWidget(widget);
}

void HgSyncBaseDialog::slotChangeEditUrl(int index)
{
    if (index == m_selectPathAlias->count() - 1) {
        m_urlEdit->setReadOnly(false);
        m_urlEdit->clear();
        m_urlEdit->setFocus();
    }
    else {
        QString url = m_pathList[m_selectPathAlias->itemText(index)];
        m_urlEdit->setText(url);
        m_urlEdit->setReadOnly(true);
    }
}

void HgSyncBaseDialog::slotGetChanges()
{
    if (m_haveChanges) {
        m_changesGroup->setVisible(!m_changesGroup->isVisible());
        return;
    }
    if (m_process.state() == QProcess::Running) {
        return;
    }
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), 
            this, SLOT(slotChangesProcessError(QProcess::ProcessError)));
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotChangesProcessComplete(int, QProcess::ExitStatus)));
    m_statusProg->setRange(0, 0);
    m_changesButton->setEnabled(false);
    
    QStringList args;
    getHgChangesArguments(args);
    m_process.setWorkingDirectory(m_hgw->getBaseDir());
    m_process.start(QLatin1String("hg"), args);
}

QString HgSyncBaseDialog::remoteUrl() const
{
    return (m_selectPathAlias->currentIndex() == m_selectPathAlias->count()-1)?m_urlEdit->text():m_selectPathAlias->currentText();

}

void HgSyncBaseDialog::slotChangesProcessError(QProcess::ProcessError error) {
    kDebug() << "Cant get changes";
    KMessageBox::error(this, i18n("Error!"));
    m_changesButton->setEnabled(true);
}

void HgSyncBaseDialog::slotChangesProcessComplete(int exitCode, QProcess::ExitStatus status)
{
    m_statusProg->setRange(0, 100);
    m_changesButton->setEnabled(true);

    disconnect(&m_process, SIGNAL(error(QProcess::ProcessError)), 
            this, SLOT(slotChangesProcessError(QProcess::ProcessError)));
    disconnect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotChangesProcessComplete(int, QProcess::ExitStatus)));

    char buffer[512];
    bool unwantedRead = false;

    while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
        QString line(QTextCodec::codecForLocale()->toUnicode(buffer));
        if (unwantedRead ) {
            line.remove(QLatin1String("Commit: "));
            parseUpdateChanges(line.trimmed());
        }
        else if (line.startsWith(QLatin1String("Commit: "))) {
            unwantedRead = true;
            line.remove(QLatin1String("Commit: "));
            parseUpdateChanges(line.trimmed());
        }
    }

    m_changesGroup->setVisible(true);
    m_haveChanges = true; 
}

void HgSyncBaseDialog::done(int r)
{
    if (r == KDialog::Accepted) {
        QStringList args;
        args << remoteUrl();
        appendOptionArguments(args);

        enableButtonOk(false);
        m_statusProg->setRange(0, 0);
        connect(m_hgw, SIGNAL(finished(int, QProcess::ExitStatus)), 
                this, SLOT(slotOperationComplete(int, QProcess::ExitStatus)));
        connect(m_hgw, SIGNAL(error(QProcess::ProcessError)), 
                this, SLOT(slotOperationError(QProcess::ProcessError)));
        
        QString command = (dialogType==PullDialog)?"pull":"push";
        m_hgw->executeCommand(command, args);
    }
    else {
        KDialog::done(r);
    }
}

void HgSyncBaseDialog::slotOperationComplete(int exitCode, QProcess::ExitStatus status)
{
    m_statusProg->setRange(0, 100);
    disconnect(m_hgw, SIGNAL(finished()), this, SLOT(slotOperationComplete()));

    if (exitCode == 0) {
        KDialog::done(KDialog::Accepted);
    }
    else {
        enableButtonOk(true);
        KMessageBox::error(this, i18n("Error!"));
    }
}

void HgSyncBaseDialog::slotOperationError(QProcess::ProcessError)
{
    disconnect(m_hgw, SIGNAL(finished()), this, SLOT(slotOperationComplete()));
    enableButtonOk(true);
    KMessageBox::error(this, i18n("Error!"));
}

#include "syncdialogbase.moc"

