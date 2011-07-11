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

#include "createdialog.h"
#include "hgwrapper.h"
#include "fileviewhgpluginsettings.h"

#include <QtGui/QHBoxLayout>
#include <QtCore/QStringList>
#include <QtGui/QFrame>
#include <klocale.h>
#include <kmessagebox.h>

HgCreateDialog::HgCreateDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Create Repository"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Create"));
    //this->enableButtonOk(false);


    //////////////
    // Setup UI //
    //////////////
    
    HgWrapper *hgw = HgWrapper::instance();
    
    m_directory = new QLabel("<b>" + hgw->getCurrentDir() + "</b>");
    m_repoNameEdit = new KLineEdit;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_directory);
    mainLayout->addWidget(m_repoNameEdit);

    QFrame *frame = new QFrame;
    frame->setLayout(mainLayout);
    setMainWidget(frame);
    m_repoNameEdit->setFocus();
}

void HgCreateDialog::done(int r)
{
    HgWrapper *hgw = HgWrapper::instance();
    if (r == KDialog::Accepted) {
        QStringList args;
        if (!m_repoNameEdit->text().isEmpty()) {
            args << m_repoNameEdit->text();
        }
        if (hgw->executeCommandTillFinished(QLatin1String("init"), args)) {
            KDialog::done(r);
        }
        else {
            KMessageBox::error(this, i18nc("error message", "Error creating repository!"));
        }
    }
    else {
        KDialog::done(r);
    }
}

#include "createdialog.moc"

