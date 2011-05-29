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
#include <QtCore/QStringList>
#include <QDebug>
#include <kservice.h>
#include <kurl.h>

HgCommitDialog::HgCommitDialog(QWidget* parent):
    KDialog(parent, Qt::Dialog) 
{
    this->setCaption(i18nc("@title:window", "<application>Hg</application> Commit"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Commit"));

    // To show diff between commit 
    KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
    if (!editor) {
        KMessageBox::error(this, i18n("A KDE text-editor component could not be found;"
                    "\nplease check your KDE installation."));
        return;
    }
    m_fileDiffDoc = editor->createDocument(0);
    m_fileDiffView = qobject_cast<KTextEditor::View*>(m_fileDiffDoc->createView(this));
    m_fileDiffDoc->setReadWrite(false);


    QHBoxLayout *topBarLayout = new QHBoxLayout;
    m_optionsButton = new KPushButton;
    topBarLayout->addWidget(m_optionsButton);

    QGridLayout *bodyLayout = new QGridLayout;
    m_statusList = new HgStatusList;
    m_commitMessage = new QTextEdit;
    bodyLayout->addWidget(m_statusList, 0, 0, 0, 1);
    bodyLayout->addWidget(m_commitMessage, 0, 1);
    bodyLayout->addWidget(m_fileDiffView, 1, 1);
    bodyLayout->setColumnStretch(0, 1);
    bodyLayout->setColumnStretch(1, 2);
    bodyLayout->setRowStretch(0, 1);
    bodyLayout->setRowStretch(1, 1);

    QFrame *frame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout; 
    mainLayout->addLayout(topBarLayout);
    mainLayout->addLayout(bodyLayout);
    frame->setLayout(mainLayout);
    setMainWidget(frame);

    this->setMinimumSize(QSize(900, 500));

    connect(m_statusList, SIGNAL(itemSelectionChanged(const char, const QString&)),
            this, SLOT(itemSelectionChangedSlot(const char, const QString&)));
}


void HgCommitDialog::itemSelectionChangedSlot(const char status, const QString &fileName)
{
    qDebug() << "Caught signal itemSelectionChanged from HgStatusList";
    m_fileDiffDoc->setReadWrite(true);
    m_fileDiffDoc->setModified(false);
    m_fileDiffDoc->closeUrl(false);

    if (status != '?') {
        QStringList arguments;
        QString diffOut;
        HgWrapper *hgWrapper = HgWrapper::instance();

        arguments << fileName;
        hgWrapper->executeCommand(QLatin1String("diff"), arguments);
        while (hgWrapper->waitForReadyRead()) {
            char buffer[2048];
            while (hgWrapper->readLine(buffer, sizeof(buffer)) > 0) {
                diffOut += buffer;
            }
            m_fileDiffDoc->setHighlightingMode("diff");
            m_fileDiffDoc->setText(diffOut);
        }
    }
    else {
        KUrl url(HgWrapper::instance()->getBaseDir());
        url.addPath(fileName);
        m_fileDiffDoc->openUrl(url);
    }

    m_fileDiffDoc->setReadWrite(false);
}

void HgCommitDialog::slotButtonClicked(int button)
{
}

