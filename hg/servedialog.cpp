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

#include <QSpinBox>
#include <QTextEdit>
#include <QLabel>
#include <QDesktopServices>
#include <KLocalizedString>
#include <KMessageBox>

HgServeDialog::HgServeDialog(QWidget *parent) :
    DialogBase(QDialogButtonBox::NoButton)
{
    Q_UNUSED(parent)
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Serve"));

    //
    m_serverWrapper = HgServeWrapper::instance();

    //
    setupUI();
    loadConfig();

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->resize(QSize(settings->serveDialogWidth(),
                               settings->serveDialogHeight()));

    // connections
    connect(this, SIGNAL(finished(int)), this, SLOT(saveGeometry()));
    connect(m_startButton, SIGNAL(clicked()), this, SLOT(slotStart()));
    connect(m_stopButton, SIGNAL(clicked()), this, SLOT(slotStop()));
    connect(m_browseButton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
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

    m_startButton = new QPushButton(xi18nc("@label:button", "Start Server"));
    m_stopButton = new QPushButton(xi18nc("@label:button", "Stop Server"));
    m_browseButton = new QPushButton(xi18nc("@label:button", "Open in browser"));
    m_browseButton->setDisabled(true);

    m_logEdit = new QTextEdit;
    m_repoPathLabel = new QLabel;
    m_logEdit->setReadOnly(true);
    m_logEdit->setFontFamily(QLatin1String("Monospace"));

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(m_browseButton);
    buttonLayout->addStretch(2);

    QHBoxLayout *portLayout = new QHBoxLayout;
    portLayout->addWidget(new QLabel(xi18nc("@label", "Port")));
    portLayout->addWidget(m_portNumber);
    portLayout->addStretch();

    QHBoxLayout *midLayout = new QHBoxLayout;
    midLayout->addWidget(m_logEdit);
    midLayout->addLayout(buttonLayout);

    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(m_repoPathLabel);
    lay->addLayout(portLayout);
    lay->addLayout(midLayout);

    layout()->insertLayout(0, lay);
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
    m_browseButton->setDisabled(false);
}

void HgServeDialog::slotStop()
{
    m_serverWrapper->stopServer(HgWrapper::instance()->getBaseDir());
    m_browseButton->setDisabled(true);
}

void HgServeDialog::slotBrowse()
{
    QDesktopServices::openUrl(QUrl(QString("http://localhost:%1").
        arg(m_portNumber->value())));
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
    settings->save();
}

#include "servedialog.moc"


