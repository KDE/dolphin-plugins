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
#include "fileviewhgpluginsettings.h"

#include <QtGui/QHBoxLayout>
#include <QtCore/QProcess>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>

HgCreateDialog::HgCreateDialog(const QString &directory, QWidget *parent):
    KDialog(parent, Qt::Dialog),
    m_workingDirectory(directory)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Initialize Repository"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Initialize Repository"));
    //this->enableButtonOk(false);


    //////////////
    // Setup UI //
    //////////////
    
    m_directory = new QLabel("<b>" + m_workingDirectory + "</b>");
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
    if (r == KDialog::Accepted) {
        QProcess process;
        QStringList args;
        args << QLatin1String("init");
        if (!m_repoNameEdit->text().isEmpty()) {
            args << m_repoNameEdit->text();
        }
        process.setWorkingDirectory(m_workingDirectory);
        process.start(QLatin1String("hg"), args);
        process.waitForFinished();
        
        if (process.exitCode() == 0 && process.exitStatus() == QProcess::NormalExit) {
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

