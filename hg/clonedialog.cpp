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

#include "clonedialog.h"
#include "hgwrapper.h"
#include "fileviewhgpluginsettings.h"

#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QtCore/QStringList>
#include <QtGui/QApplication>
#include <kurl.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

HgCloneDialog::HgCloneDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Clone"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Clone"));
    this->enableButtonOk(false);


    //////////////
    // Setup UI //
    //////////////

    QGroupBox *urlGroup = new QGroupBox(i18n("URL's"));
    QGridLayout *urlLayout = new QGridLayout;
    QLabel *sourceLabel = new QLabel(i18nc("@label", "Source"));
    QLabel *destLabel = new QLabel(i18nc("@lobel", "Destination"));
    KPushButton *m_browse_dest = new KPushButton(i18nc("@button", "Browse"));
    KPushButton *m_browse_source = new KPushButton(i18nc("@button", "Browse"));
    m_source = new KLineEdit;
    m_destination = new KLineEdit;
    urlLayout->addWidget(sourceLabel, 0, 0);
    urlLayout->addWidget(m_source, 0, 1);
    urlLayout->addWidget(m_browse_source, 0, 2);
    urlLayout->addWidget(destLabel, 1, 0);
    urlLayout->addWidget(m_destination, 1, 1);
    urlLayout->addWidget(m_browse_dest, 1, 2);
    urlGroup->setLayout(urlLayout);

    // Options Group
    QGroupBox *optionGroup = new QGroupBox(i18nc("@label", "Options"));
    QVBoxLayout *optionLayout = new QVBoxLayout;

    m_optNoUpdate = new QCheckBox(i18n("Do not update the new working directory"));
    m_optUsePull = new QCheckBox(i18n("Use pull protocol to copy metadata."));
    m_optUseUncmprdTrans = new QCheckBox(i18n("Use uncompressed transfer"));
    m_optNoVerifyServCert = new QCheckBox(i18n("Do not verify server certificate (ignoring web.cacerts config)"));

    optionLayout->addWidget(m_optNoUpdate);
    optionLayout->addWidget(m_optUsePull);
    optionLayout->addWidget(m_optUseUncmprdTrans);
    optionLayout->addWidget(m_optNoVerifyServCert);
    optionGroup->setLayout(optionLayout);
    // end options

    QFrame *frame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(urlGroup);
    mainLayout->addWidget(optionGroup);
    mainLayout->addStretch();
    frame->setLayout(mainLayout);
    
    m_stackLayout = new  QStackedLayout;
    m_outputEdit = new KTextEdit;
    m_outputEdit->setReadOnly(true);
    m_stackLayout->addWidget(frame);
    m_stackLayout->addWidget(m_outputEdit);

    QFrame *mainFrame = new QFrame;
    mainFrame->setLayout(m_stackLayout);
    m_stackLayout->setCurrentIndex(0);

    setMainWidget(mainFrame);
    // Load saved settings
    /*FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->cloneDialogWidth(),
                               settings->cloneDialogHeight()));
    */
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
    connect(m_source, SIGNAL(textChanged(const QString&)), 
            this, SLOT(slotUpdateOkButton(const QString&)));
    connect(m_browse_dest, SIGNAL(clicked()),
            this, SLOT(slotBrowseDestClicked()));
    connect(m_browse_source, SIGNAL(clicked()),
            this, SLOT(slotBrowseSourceClicked()));
}

void HgCloneDialog::browseDirectory(KLineEdit *dest)
{
    QString result = KFileDialog::getExistingDirectory();
    if (result.length() > 0) {
        dest->setText(result);
    }
}

void HgCloneDialog::slotBrowseDestClicked()
{
    browseDirectory(m_destination);
}

void HgCloneDialog::slotBrowseSourceClicked()
{
    browseDirectory(m_source);
}

void HgCloneDialog::done(int r)
{
    HgWrapper *hgw = HgWrapper::instance();
    if (r == KDialog::Accepted) {
        QStringList args;
        args << QLatin1String("--verbose");
        appendOptionArguments(args);
        args << m_source->text() << m_destination->text();

        m_outputEdit->clear();
        m_stackLayout->setCurrentIndex(1);
        QApplication::processEvents();
        enableButtonOk(false);
        if (hgw->executeCommand(QLatin1String("clone"), args, 
                    m_outputEdit)) {
        }
        else {
            KMessageBox::error(this, i18n("Error cloning repository!"));
        }
    }
    else {
        if (hgw->isBusy()) {
            KMessageBox::error(this, i18n("Terminating cloning!"));
            enableButtonOk(true);
            hgw->terminateCurrentProcess();
            m_stackLayout->setCurrentIndex(0);
        }
        else {
            KDialog::done(r);
        }
    }
}

void HgCloneDialog::appendOptionArguments(QStringList &args)
{
}

void HgCloneDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setCloneDialogHeight(this->height());
    settings->setCloneDialogWidth(this->width());
    settings->writeConfig();
}

void HgCloneDialog::slotUpdateOkButton(const QString &text)
{
    if (m_source->text().length() > 0) {
        enableButtonOk(true);
    }
    else {
        enableButtonOk(false);
    }
}

#include "clonedialog.moc"

