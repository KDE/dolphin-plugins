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
#include "pathselector.h"
#include "fileviewhgpluginsettings.h"

#include <QtGui/QApplication>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QProgressBar>
#include <klineedit.h>
#include <ktextedit.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>

HgSyncBaseDialog::HgSyncBaseDialog(DialogType dialogType, QWidget *parent):
    KDialog(parent, Qt::Dialog),
    m_haveChanges(false),
    m_terminated(false),
    m_dialogType(dialogType)
{
    m_hgw = HgWrapper::instance();
}

void HgSyncBaseDialog::setup()
{
    createChangesGroup();
    readBigSize();
    setupUI();
    
    connect(m_changesButton, SIGNAL(clicked()),
            this, SLOT(slotGetChanges()));
    connect(&m_process, SIGNAL(stateChanged(QProcess::ProcessState)), 
            this, SLOT(slotUpdateBusy(QProcess::ProcessState)));
    connect(&m_main_process, SIGNAL(stateChanged(QProcess::ProcessState)), 
            this, SLOT(slotUpdateBusy(QProcess::ProcessState)));
    connect(&m_main_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotOperationComplete(int, QProcess::ExitStatus)));
    connect(&m_main_process, SIGNAL(error(QProcess::ProcessError)), 
            this, SLOT(slotOperationError()));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), 
            this, SLOT(slotChangesProcessError()));
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotChangesProcessComplete(int, QProcess::ExitStatus)));
    connect(this, SIGNAL(finished()), this, SLOT(slotWriteBigSize()));
}

void HgSyncBaseDialog::createOptionGroup()
{
    setOptions();
    QVBoxLayout *layout = new QVBoxLayout;

    foreach (QCheckBox *cb, m_options) {
        layout->addWidget(cb);
    }

    m_optionGroup = new QGroupBox;
    m_optionGroup->setLayout(layout);
    setDetailsWidget(m_optionGroup);
}

void HgSyncBaseDialog::setupUI()
{
    // top url bar
    m_pathSelector = new HgPathSelector;

    // changes button
    //FIXME not very good idea. Bad HACK 
    if (m_dialogType == PullDialog) {
        m_changesButton = new KPushButton(i18nc("@label:button", 
                "Show Incoming Changes"));
    }
    else {
        m_changesButton = new KPushButton(i18nc("@label:button", 
                "Show Outgoing Changes"));
    }
    m_changesButton->setSizePolicy(QSizePolicy::Fixed,
            QSizePolicy::Fixed);
    m_changesButton->setCheckable(true);

    // dialog's main widget
    QWidget *widget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_pathSelector);

    // changes
    m_changesGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    widget->setLayout(mainLayout);

    createOptionGroup();
    setMainWidget(widget);
}

void HgSyncBaseDialog::slotGetChanges()
{
    if (m_haveChanges) {
        m_changesGroup->setVisible(!m_changesGroup->isVisible());
        m_changesButton->setChecked(m_changesGroup->isVisible());
        if (m_changesGroup->isVisible()) {
            loadBigSize();
        }
        else {
            loadSmallSize();
        }
        return;
    }
    if (m_process.state() == QProcess::Running) {
        return;
    }

    QStringList args;
    getHgChangesArguments(args);
    m_process.setWorkingDirectory(m_hgw->getBaseDir());
    m_process.start(QLatin1String("hg"), args);
}


void HgSyncBaseDialog::slotChangesProcessComplete(int exitCode, QProcess::ExitStatus status)
{

    if (exitCode != 0 || status != QProcess::NormalExit) {
        QString message = QTextCodec::codecForLocale()->toUnicode(m_process.readAllStandardError());
        if (message.isEmpty()) {
            message = i18nc("@message", "No changes found!");
        }
        KMessageBox::error(this, message);
        return;
    }

    char buffer[512];

    /**
     * unwantedRead boolean to ensure that unwanted information messages
     * by mercurial are filtered out.
     *  eg. Comparing with /path/to/repository
     *      Searching for changes
     */
    bool unwantedRead = false;

    /**
     * hasChanges boolean checks whether there are any changes to be sent 
     * or received and invoke noChangesMessage() if its false
     */
    bool hasChanges = false;

    while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
        QString line(QTextCodec::codecForLocale()->toUnicode(buffer));
        if (unwantedRead ) {
            line.remove(QLatin1String("Commit: "));
            parseUpdateChanges(line.trimmed());
            hasChanges = true;;
        }
        else if (line.startsWith(QLatin1String("Commit: "))) {
            unwantedRead = true;
            line.remove(QLatin1String("Commit: "));
            parseUpdateChanges(line.trimmed());
            hasChanges = true;
        }
    }

    if (!hasChanges) {
        noChangesMessage();
    }

    m_changesGroup->setVisible(true);
    m_changesButton->setChecked(true);
    loadBigSize();
    m_haveChanges = true; 
    emit changeListAvailable();
}

void HgSyncBaseDialog::slotChangesProcessError()
{
    kDebug() << "Cant get changes";
    KMessageBox::error(this, i18n("Error!"));
}

void HgSyncBaseDialog::loadSmallSize()
{
    m_bigSize = size();
    resize(m_smallSize);
}

void HgSyncBaseDialog::loadBigSize()
{
    m_smallSize = size();
    resize(m_bigSize);
}

void HgSyncBaseDialog::slotWriteBigSize()
{
    if (m_changesGroup->isVisible()) {
        m_bigSize = size();
    }
    writeBigSize();
}

void HgSyncBaseDialog::done(int r)
{
    if (r == KDialog::Accepted) {
        if (m_main_process.state() == QProcess::Running ||
                m_main_process.state() == QProcess::Starting) {
            kDebug() << "HgWrapper already busy";
            return;
        }

        QStringList args;
        QString command = (m_dialogType==PullDialog)?"pull":"push";
        args << command;
        args << m_pathSelector->remote();
        appendOptionArguments(args);

        m_terminated = false;
        
        m_main_process.setWorkingDirectory(m_hgw->getBaseDir());
        m_main_process.start(QLatin1String("hg"), args);
    }
    else {
        if (m_process.state() == QProcess::Running || 
            m_process.state() == QProcess::Starting ||
            m_main_process.state() == QProcess::Running ||
            m_main_process.state() == QProcess::Starting) 
        {
            if (m_process.state() == QProcess::Running ||
                    m_process.state() == QProcess::Starting) {
                m_process.terminate();
            }
            if (m_main_process.state() == QProcess::Running ||
                     m_main_process.state() == QProcess::Starting) {
                kDebug() << "terminating pull/push process";
                m_terminated = true;
                m_main_process.terminate();
            }
        }
        else {
            KDialog::done(r);
        }
    }
}

void HgSyncBaseDialog::slotOperationComplete(int exitCode, QProcess::ExitStatus status)
{
    if (exitCode == 0 && status == QProcess::NormalExit) {
        KDialog::done(KDialog::Accepted);
    }
    else {
        if (!m_terminated) {
            KMessageBox::error(this, i18n("Error!"));
        }
    }
}

void HgSyncBaseDialog::slotOperationError()
{
    KMessageBox::error(this, i18n("Error!"));
}

void HgSyncBaseDialog::slotUpdateBusy(QProcess::ProcessState state)
{
    if (state == QProcess::Running || state == QProcess::Starting) {
        m_statusProg->setRange(0, 0);
        m_changesButton->setEnabled(false);
        m_changesButton->setChecked(true);
        this->enableButtonOk(false);
    }
    else {
        m_statusProg->setRange(0, 100);
        m_changesButton->setEnabled(true);
        this->enableButtonOk(true);
    }
    m_statusProg->repaint();
    QApplication::processEvents();
}

#include "syncdialogbase.moc"

