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

#include "tagdialog.h"
#include "hgwrapper.h"

#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kmessagebox.h>

HgTagDialog::HgTagDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Tag"));
    this->setButtons(KDialog::None);

    // UI 
    QFrame *frame = new QFrame;
    QVBoxLayout *vbox = new QVBoxLayout;

    m_tagComboBox = new KComboBox;
    m_tagComboBox->setEditable(true);
    vbox->addWidget(m_tagComboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_createTag = new KPushButton(i18n("Create New Tag"));
    m_removeTag = new KPushButton(i18n("Remove Tag"));
    m_updateTag = new KPushButton(i18n("Switch Tag"));
    buttonLayout->addWidget(m_createTag);
    buttonLayout->addWidget(m_removeTag);
    buttonLayout->addWidget(m_updateTag);
    vbox->addLayout(buttonLayout);

    m_createTag->setEnabled(false);
    m_updateTag->setEnabled(false);
    m_removeTag->setEnabled(false);

    frame->setLayout(vbox);
    updateInitialDialog();
    slotUpdateDialog(QString());
    setMainWidget(frame);

    slotUpdateDialog(m_tagComboBox->currentText());

    QLineEdit *m_lineEdit = m_tagComboBox->lineEdit();
    
    // connections
    connect(m_createTag, SIGNAL(clicked()), 
            this, SLOT(slotCreateTag()));
    connect(m_removeTag, SIGNAL(clicked()), 
            this, SLOT(slotRemoveTag()));
    connect(m_updateTag, SIGNAL(clicked()),
            this, SLOT(slotSwitch()));
    connect(m_tagComboBox, SIGNAL(editTextChanged(const QString&)),
            this, SLOT(slotUpdateDialog(const QString&)));
    connect(m_lineEdit, SIGNAL(textChanged(const QString&)), 
                this, SLOT(slotUpdateDialog(const QString&)));
}

void HgTagDialog::updateInitialDialog()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    // update combo box
    m_tagList = hgWrapper->getTags();
    m_tagComboBox->addItems(m_tagList);

}

void HgTagDialog::slotUpdateDialog(const QString &text)
{
    // update pushbuttons
    if (text.length() == 0) {
        m_createTag->setEnabled(false);
        m_updateTag->setEnabled(false);
        m_removeTag->setEnabled(false);
    }
    else if (m_tagList.contains(text)) {
        m_createTag->setEnabled(false);
        m_updateTag->setEnabled(true);
        m_removeTag->setEnabled(true);
    }
    else {
        m_createTag->setEnabled(true);
        m_updateTag->setEnabled(false);
        m_removeTag->setEnabled(false);
    }
}

void HgTagDialog::slotSwitch()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << QLatin1String("-c");
    args << m_tagComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("update"), args, out)) {
        //KMessageBox::information(this, i18n("Updated working directory!"));
        done(KDialog::Ok);
    }
    else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}

void HgTagDialog::slotRemoveTag()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << QLatin1String("--remove");
    args << m_tagComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("tag"), args, out)) {
        //KMessageBox::information(this, i18n("Removed tag successfully!"));
        done(KDialog::Ok);
    }
    else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}

void HgTagDialog::slotCreateTag()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << m_tagComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("tag"), args, out)) {
        KMessageBox::information(this, i18n("Created tag successfully!"));
        done(KDialog::Ok);
    }
    else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}

#include "tagdialog.moc"

