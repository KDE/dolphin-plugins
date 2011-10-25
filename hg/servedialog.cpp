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

#include "servedialog.h"
#include "servewrapper.h"
#include "fileviewhgpluginsettings.h"
#include "hgwrapper.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QSpinBox>
#include <QtGui/QTextEdit>
#include <QtGui/QLabel>
#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

HgServeDialog::HgServeDialog(QWidget *parent) :
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Serve"));
    this->setButtons(KDialog::None);

    //
    m_serverWrapper = HgServeWrapper::instance();

    //
    setupUI();
    loadConfig();

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->serveDialogWidth(),
                               settings->serveDialogHeight()));

    // connections
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
    connect(m_startButton, SIGNAL(clicked()), this, SLOT(slotStart()));
    connect(m_stopButton, SIGNAL(clicked()), this, SLOT(slotStop()));
    connect(m_serverWrapper, SIGNAL(finished()), 
            this, SLOT(slotUpdateButtons()));
    connect(m_serverWrapper, SIGNAL(started()), 
            this, SLOT(slotUpdateButtons()));
    connect(m_serverWrapper, SIGNAL(error()), 
            this, SLOT(slotUpdateButtons()));
    connect(m_serverWrapper, SIGNAL(error()), 
            this, SLOT(slotServerError()));
    connect(m_serverWrapper, 
            SIGNAL(readyReadLine(const QString&, const QString&)),
            this,
            SLOT(appendServerOutput(const QString&, const QString&)));
}

void HgServeDialog::setupUI()
{
    m_portNumber = new QSpinBox;
    m_portNumber->setMinimum(0);
    m_portNumber->setMaximum(65535);
    m_portNumber->setValue(8000);

    m_startButton = new KPushButton(i18nc("@label:button", "Start Server"));
    m_stopButton = new KPushButton(i18nc("@label:button", "Stop Server"));

    m_logEdit = new QTextEdit;
    m_repoPathLabel = new QLabel;
    m_logEdit->setReadOnly(true);
    m_logEdit->setFontFamily(QLatin1String("Monospace"));

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addStretch();

    QHBoxLayout *portLayout = new QHBoxLayout;
    portLayout->addWidget(new QLabel(i18nc("@label", "Port")));
    portLayout->addWidget(m_portNumber);
    portLayout->addStretch();

    QHBoxLayout *midLayout = new QHBoxLayout;
    midLayout->addWidget(m_logEdit);
    midLayout->addLayout(buttonLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_repoPathLabel);
    layout->addLayout(portLayout);
    layout->addLayout(midLayout);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setMainWidget(widget);
}

void HgServeDialog::loadConfig()
{
    HgWrapper *hgw = HgWrapper::instance();
    m_repoPathLabel->setText("<b>" + hgw->getBaseDir() + "</b>");

    slotUpdateButtons();
}

void HgServeDialog::slotUpdateButtons()
{
    if (m_serverWrapper->running(HgWrapper::instance()->getBaseDir())) {
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        m_portNumber->setEnabled(false);
    }
    else {
        m_startButton->setEnabled(true);
        m_stopButton->setEnabled(false);
        m_portNumber->setEnabled(true);
        m_serverWrapper->cleanUnused();
    }
}

void HgServeDialog::slotStart()
{
    m_serverWrapper->startServer(HgWrapper::instance()->getBaseDir(),
                                 m_portNumber->value());
}

void HgServeDialog::slotStop()
{
    m_serverWrapper->stopServer(HgWrapper::instance()->getBaseDir());
}

void HgServeDialog::slotServerError()
{
    m_serverWrapper->cleanUnused();
}

void HgServeDialog::appendServerOutput(const QString &repoLocation, const QString &line) 
{
    if (HgWrapper::instance()->getBaseDir() == repoLocation) {
        m_logEdit->append(line);
    }
}

void HgServeDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setServeDialogHeight(this->height());
    settings->setServeDialogWidth(this->width());
    settings->writeConfig();
}

#include "servedialog.moc"


