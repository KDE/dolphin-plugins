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

#include "mergedialog.h"
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

HgMergeDialog::HgMergeDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Merge"));
    this->setButtons(KDialog::None);

    // UI 
    QFrame *frame = new QFrame;
    QVBoxLayout *vbox = new QVBoxLayout;

    m_currentChangeset = new QLabel;
    vbox->addWidget(m_currentChangeset);

    m_comboBox = new KComboBox;
    m_comboBox->setEditable(true);
    vbox->addWidget(m_comboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_mergeButton = new KPushButton(i18n("Merge"));
    buttonLayout->addWidget(m_mergeButton);
    vbox->addLayout(buttonLayout);

    frame->setLayout(vbox);
    updateInitialDialog();
    setMainWidget(frame);

    // connections
    connect(m_mergeButton, SIGNAL(clicked()),
            this, SLOT(slotMerge()));
}

void HgMergeDialog::updateInitialDialog()
{
    HgWrapper *hgWrapper = HgWrapper::instance();

    // update label - current branch
    QString out;
    hgWrapper->executeCommand(QLatin1String("branch"), QStringList(), out);
    out = i18n("<b>Current Branch: </b>") + out;
    m_currentChangeset->setText(out);

    // update combo box
}

void HgMergeDialog::slotMerge()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QStringList args;
    QString out;

    if (!m_comboBox->currentText().isEmpty()) {
        args << m_comboBox->currentText();
    }

    if (hgWrapper->executeCommand(QLatin1String("merge"), args, out)) {
        done(KDialog::Ok);
    }
    else {
        KMessageBox::error(this, i18n("Some error occcurred"));
    }
}

#include "mergedialog.moc"

