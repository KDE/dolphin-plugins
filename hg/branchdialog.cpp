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

#include "branchdialog.h"
#include "hgwrapper.h"

#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kmessagebox.h>

HgBranchDialog::HgBranchDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Branch"));
    this->setButtons(KDialog::None);

    // UI 
    QFrame *frame = new QFrame;
    QVBoxLayout *vbox = new QVBoxLayout;

    m_currentBranchLabel = new QLabel;
    vbox->addWidget(m_currentBranchLabel);

    m_branchComboBox = new KComboBox;
    m_branchComboBox->setEditable(true);
    vbox->addWidget(m_branchComboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_createBranch = new KPushButton(i18n("Create New Branch"));
    m_updateBranch = new KPushButton(i18n("Switch Branch"));
    buttonLayout->addWidget(m_createBranch);
    buttonLayout->addWidget(m_updateBranch);
    vbox->addLayout(buttonLayout);

    m_createBranch->setEnabled(false);
    m_updateBranch->setEnabled(false);

    frame->setLayout(vbox);
    updateInitialDialog();
    slotUpdateDialog(QString());
    setMainWidget(frame);

    slotUpdateDialog(m_branchComboBox->currentText());
    QLineEdit *m_lineEdit = m_branchComboBox->lineEdit();
    
    // connections
    connect(m_createBranch, SIGNAL(clicked()), 
            this, SLOT(slotCreateBranch()));
    connect(m_updateBranch, SIGNAL(clicked()),
            this, SLOT(slotSwitch()));
    connect(m_branchComboBox, SIGNAL(editTextChanged(const QString&)),
            this, SLOT(slotUpdateDialog(const QString &)));
    connect(m_lineEdit, SIGNAL(textChanged(const QString&)), 
                this, SLOT(slotUpdateDialog(const QString&)));
}

void HgBranchDialog::updateInitialDialog()
{
    HgWrapper *hgWrapper = HgWrapper::instance();

    // update label - current branch
    QString out;
    hgWrapper->executeCommand(QLatin1String("branch"), QStringList(), out);
    out = i18n("<b>Current Branch: </b>") + out;
    m_currentBranchLabel->setText(out);

    // update combo box
    m_branchList = hgWrapper->getBranches();
    m_branchComboBox->addItems(m_branchList);

}

void HgBranchDialog::slotUpdateDialog(const QString &text)
{
    // update pushbuttons
    if (text.length() == 0) {
        m_createBranch->setEnabled(false);
        m_updateBranch->setEnabled(false);
    }
    else if (m_branchList.contains(text)) {
        m_createBranch->setEnabled(false);
        m_updateBranch->setEnabled(true);
    }
    else {
        m_createBranch->setEnabled(true);
        m_updateBranch->setEnabled(false);
    }
}

void HgBranchDialog::slotSwitch()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << QLatin1String("-c");
    args << m_branchComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("update"), args, out)) {
        //KMessageBox::information(this, i18n("Updated working directory!"));
        done(KDialog::Ok);
    }
    else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}

void HgBranchDialog::slotCreateBranch()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << m_branchComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("branch"), args, out)) {
        //KMessageBox::information(this, i18n("Created branch successfully!"));
        done(KDialog::Ok);
    }
    else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}

#include "branchdialog.moc"

