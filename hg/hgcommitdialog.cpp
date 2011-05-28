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

#include "hgcommitdialog.h"
#include "hgwrapper.h"

#include <klocale.h>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QDebug>
#include <kservice.h>

HgCommitDialog::HgCommitDialog(QWidget* parent):
    KDialog(parent, Qt::Dialog) 
{
    this->setCaption(i18nc("@title:window", "<application>Hg</application> Commit"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);

    // To show diff between commit 
    KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
    if (!editor) {
        KMessageBox::error(this, i18n("A KDE text-editor component could not be found;\n"
                    "please check your KDE installation."));
        return;
    }
    m_fileDiffDoc = editor->createDocument(0);
    m_fileDiffView = qobject_cast<KTextEditor::View*>(m_fileDiffDoc->createView(this));
    m_fileDiffDoc->setHighlightingMode("diff");
    m_fileDiffDoc->setReadWrite(false);


    QHBoxLayout *topBarLayout = new QHBoxLayout;
    m_fileFilter = new KLineEdit;
    m_optionsButton = new KPushButton;
    topBarLayout->addWidget(m_fileFilter);
    topBarLayout->addWidget(m_optionsButton);

    QGridLayout *bodyLayout = new QGridLayout;
    m_fileList = new KListWidget;
    m_commitMessage = new QTextEdit;
    bodyLayout->addWidget(m_fileList, 0, 0, 0, 1);
    bodyLayout->addWidget(m_commitMessage, 0, 1);
    bodyLayout->addWidget(m_fileDiffView, 1, 1);
    bodyLayout->setColumnStretch(0, 1);
    bodyLayout->setColumnStretch(1, 3);
    bodyLayout->setRowStretch(0, 1);
    bodyLayout->setRowStretch(1, 1);

    QFrame *frame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout; 
    mainLayout->addLayout(topBarLayout);
    mainLayout->addLayout(bodyLayout);
    frame->setLayout(mainLayout);
    setMainWidget(frame);
}


void HgCommitDialog::slotButtonClicked(int button)
{
    if(button == KDialog::Ok) {
        HgWrapper *hgi = HgWrapper::instance();
        hgi->setWorkingDirectory(m_hgBaseDir);
    }   
}

