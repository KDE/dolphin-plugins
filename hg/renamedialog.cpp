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

#include "renamedialog.h"

#include <klocale.h>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>

RenameDialog::RenameDialog(QString source, QWidget* parent):
    KDialog(parent, Qt::Dialog), m_source(source)
{
    this->setCaption(i18nc("@title:window", "<application>Hg</application> Rename"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Rename"));

    QVBoxLayout *vbox = new QVBoxLayout;
    QFrame *frame = new QFrame;

    QHBoxLayout *sourceHBox = new QHBoxLayout;
    QHBoxLayout *destinationHBox = new QHBoxLayout;

    QLabel *sourceLabel = new QLabel(i18nc("@label:label to source file", "Source:"));
    QLabel *sourceFileLabel = new QLabel("<b>" + source + "</b>");
    QLabel *destinationLabel = new QLabel(i18nc("@label:rename", 
                "Rename to:") );
    m_destinationFile = new KLineEdit(source);

    sourceHBox->addWidget(sourceLabel);
    sourceHBox->addWidget(sourceFileLabel);
    destinationHBox->addWidget(destinationLabel);
    destinationHBox->addWidget(m_destinationFile);

    vbox->addLayout(sourceHBox);
    vbox->addLayout(destinationHBox);
    frame->setLayout(vbox);
    setMainWidget(frame);

    m_destinationFile->setFocus();
    m_destinationFile->selectAll();

    connect(m_destinationFile, SIGNAL(textChanged(const QString&)), 
            this, SLOT(enableDisableOkButton(const QString&)));
}

void RenameDialog::enableDisableOkButton(const QString &text)
{
    enableButtonOk(text.length()!=0);
}

QString RenameDialog::source() const 
{
    return m_source;
}

QString RenameDialog::destination() const 
{
    return m_destinationFile->text();
}

#include "renamedialog.moc"

