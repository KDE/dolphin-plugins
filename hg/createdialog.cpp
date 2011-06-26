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

#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QtGui/QApplication>
#include <kurl.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kmessagebox.h>

HgCreateDialog::HgCreateDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Create"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Create"));
    this->enableButtonOk(false);


    //////////////
    // Setup UI //
    //////////////

}

void HgCreateDialog::done(int r)
{
    HgWrapper *hgw = HgWrapper::instance();
    if (r == KDialog::Accepted) {
    }
    else {
        KDialog::done(r);
    }
}

void HgCreateDialog::appendOptionArguments(QStringList &args)
{
}

#include "createdialog.moc"

