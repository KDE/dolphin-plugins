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

#include "commitdialog.h"
#include "hgwrapper.h"
#include "fileviewhgpluginsettings.h"

#include <klocale.h>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
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

    QGroupBox *messageGroupBox = new QGroupBox;
    QVBoxLayout *commitLayout = new QVBoxLayout;
    m_commitMessage = new QPlainTextEdit;
    commitLayout->addWidget(new QLabel(getParentBranchForLabel()));
    commitLayout->addWidget(m_commitMessage);
    messageGroupBox->setTitle(i18nc("@title:group", "Commit Message"));
    messageGroupBox->setLayout(commitLayout);

    QGroupBox *diffGroupBox = new QGroupBox;
    QVBoxLayout *diffLayout = new QVBoxLayout;
    diffLayout->addWidget(m_fileDiffView);
    diffGroupBox->setTitle(i18nc("@title:group", "Diff/Content"));
    diffGroupBox->setLayout(diffLayout);

    QGridLayout *bodyLayout = new QGridLayout;
    m_statusList = new HgStatusList;
    bodyLayout->addWidget(m_statusList, 0, 0, 0, 1);
    bodyLayout->addWidget(messageGroupBox, 0, 1);
    bodyLayout->addWidget(diffGroupBox, 1, 1);
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

    //this->setMinimumSize(QSize(900, 500)    
    FileViewHgPluginSettings* settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->commitDialogWidth(), 
                settings->commitDialogHeight()));

    this->enableButtonOk(false);

    connect(m_statusList, SIGNAL(itemSelectionChanged(const char, const QString&)),
            this, SLOT(itemSelectionChangedSlot(const char, const QString&)));
    connect(m_commitMessage, SIGNAL(textChanged()), 
            this, SLOT(slotMessageChanged()));
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
}

QString HgCommitDialog::getParentBranchForLabel()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString command =  QLatin1String("hg log -r tip --template {parents}\\n{branches}");
    hgWrapper->start(command);
    QString lines;
    while (hgWrapper->waitForReadyRead()) {
        char buffer[1024];
        hgWrapper->readLine(buffer, sizeof(buffer));
        lines += QString("<b>parent: </b>%1").arg(buffer).trimmed();
        hgWrapper->readLine(buffer, sizeof(buffer));
        lines += " | ";
        lines += QString("<b>branch: </b>%1").arg(buffer).trimmed();
    }
    return lines;
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

void HgCommitDialog::slotMessageChanged()
{
    /*qDebug() << "Hg/Commit: Checking empty message. " 
        << m_commitMessage->toPlainText().isEmpty();*/
    enableButtonOk(!m_commitMessage->toPlainText().isEmpty());
}

void HgCommitDialog::done(int r)
{
    if(r == KDialog::Accepted) {
        QStringList files;
        if (m_statusList->getSelectionForCommit(files)) {
            HgWrapper *hgWrapper = HgWrapper::instance();
            hgWrapper->commit(m_commitMessage->toPlainText(), files);
            hgWrapper->waitForFinished();

            if (hgWrapper->exitCode() == 0
                    && hgWrapper->exitStatus() == QProcess::NormalExit) {
                KDialog::done(r);
            }
            else {
                KMessageBox::error(this, "Commit unsuccessful!");
            }
        }
        else {
            KMessageBox::error(this, "No files for commit!");
        }
    }
    else {
        KDialog::done(r);
    }
}

void HgCommitDialog::saveGeometry()
{
    FileViewHgPluginSettings* settings = FileViewHgPluginSettings::self();
    settings->setCommitDialogHeight(this->height());
    settings->setCommitDialogWidth(this->width());
    settings->writeConfig();
}

#include "commitdialog.moc"

