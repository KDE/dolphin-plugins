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

#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtCore/QStringList>
#include <QtGui/QActionGroup>
#include <kurl.h>
#include <kpushbutton.h>
#include <klocale.h>

HgCommitDialog::HgCommitDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog)
{
    // dialog properties
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Commit"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Commit"));
    this->enableButtonOk(false); // since commit message is empty when loaded

    // To show diff between commit
    KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
    if (!editor) {
        KMessageBox::error(this, 
                i18n("A KDE text-editor component could not be found;"
                     "\nplease check your KDE installation."));
        return;
    }
    m_fileDiffDoc = editor->createDocument(0);
    m_fileDiffView = qobject_cast<KTextEditor::View *>(m_fileDiffDoc->createView(this));
    m_fileDiffDoc->setReadWrite(false);

    // Setup actions
    m_useCurrentBranch= new KAction(this);
    m_useCurrentBranch->setCheckable(true);
    m_useCurrentBranch->setText(i18nc("@action:inmenu",
                               "Commit to current branch"));

    m_newBranch = new KAction(this);
    m_newBranch->setCheckable(true);
    m_newBranch->setText(i18nc("@action:inmenu",
                               "Create new branch"));

    m_closeBranch = new KAction(this);
    m_closeBranch->setCheckable(true);
    m_closeBranch->setText(i18nc("@action:inmenu",
                                 "Close current branch"));

    m_branchMenu = new KMenu(this);
    m_branchMenu->addAction(m_useCurrentBranch);
    m_branchMenu->addAction(m_newBranch);
    m_branchMenu->addAction(m_closeBranch);

    QActionGroup *branchActionGroup = new QActionGroup(this);
    branchActionGroup->addAction(m_useCurrentBranch);
    branchActionGroup->addAction(m_newBranch);
    branchActionGroup->addAction(m_closeBranch);
    m_useCurrentBranch->setChecked(true);
    connect(branchActionGroup, SIGNAL(triggered(QAction *)),
            this, SLOT(slotBranchActions(QAction *)));


    //////////////
    // Setup UI //
    //////////////

    // Top bar of buttons
    QHBoxLayout *topBarLayout = new QHBoxLayout;
    KPushButton *copyMessageButton = new KPushButton(i18n("Copy Message"));
    KPushButton *optionsButton = new KPushButton(i18n("Options"));
    m_branchButton = new KPushButton(i18n("Branch"));

    topBarLayout->addWidget(new QLabel(getParentForLabel()));
    topBarLayout->addStretch();
    topBarLayout->addWidget(copyMessageButton);
    topBarLayout->addWidget(m_branchButton);
    topBarLayout->addWidget(optionsButton);
    m_branchButton->setMenu(m_branchMenu);

    // the commit box itself
    QGroupBox *messageGroupBox = new QGroupBox;
    QVBoxLayout *commitLayout = new QVBoxLayout;
    m_commitMessage = new QPlainTextEdit;
    commitLayout->addWidget(m_commitMessage);
    messageGroupBox->setTitle(i18nc("@title:group", "Commit Message"));
    messageGroupBox->setLayout(commitLayout);

    // Show diff here
    QGroupBox *diffGroupBox = new QGroupBox;
    QVBoxLayout *diffLayout = new QVBoxLayout(diffGroupBox);
    diffLayout->addWidget(m_fileDiffView);
    diffGroupBox->setTitle(i18nc("@title:group", "Diff/Content"));
    diffGroupBox->setLayout(diffLayout);

    // Set up layout for Status, Commit and Diff boxes
    QGridLayout *bodyLayout = new QGridLayout;
    m_statusList = new HgStatusList;
    bodyLayout->addWidget(m_statusList, 0, 0, 0, 1);
    bodyLayout->addWidget(messageGroupBox, 0, 1);
    bodyLayout->addWidget(diffGroupBox, 1, 1);
    bodyLayout->setColumnStretch(0, 1);
    bodyLayout->setColumnStretch(1, 2);
    bodyLayout->setRowStretch(0, 1);
    bodyLayout->setRowStretch(1, 1);

    // Set up layout and container for main dialog
    QFrame *frame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topBarLayout);
    mainLayout->addLayout(bodyLayout);
    frame->setLayout(mainLayout);
    setMainWidget(frame);

    slotInitDiffOutput(); // initialise with whole repo diff

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->commitDialogWidth(),
                               settings->commitDialogHeight()));
    //
    connect(m_statusList, 
        SIGNAL(itemSelectionChanged(const char, const QString &)),
        this, SLOT(slotItemSelectionChanged(const char, const QString &)));
    connect(m_commitMessage, SIGNAL(textChanged()),
         this, SLOT(slotMessageChanged()));
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
}

QString HgCommitDialog::getParentForLabel()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString line("<b>parents:</b> ");
    line += hgWrapper->getParentsOfHead();
    return line;
}

void HgCommitDialog::slotInitDiffOutput()
{
    m_fileDiffDoc->setReadWrite(true);
    m_fileDiffDoc->setModified(false);
    m_fileDiffDoc->closeUrl(false);

    QString diffOut;
    HgWrapper *hgWrapper = HgWrapper::instance();
    hgWrapper->executeCommand(QLatin1String("diff"), QStringList(), diffOut);
    m_fileDiffDoc->setHighlightingMode("diff");
    m_fileDiffDoc->setText(diffOut);
}

void HgCommitDialog::slotItemSelectionChanged(const char status, 
        const QString &fileName)
{
    m_fileDiffDoc->setReadWrite(true);
    m_fileDiffDoc->setModified(false);
    m_fileDiffDoc->closeUrl(false);

    if (status != '?') {
        QStringList arguments;
        QString diffOut;
        HgWrapper *hgWrapper = HgWrapper::instance();

        arguments << fileName;
        hgWrapper->executeCommand(QLatin1String("diff"), arguments, diffOut);
        m_fileDiffDoc->setHighlightingMode("diff");
        m_fileDiffDoc->setText(diffOut);
    }
    else {
        KUrl url(HgWrapper::instance()->getBaseDir());
        url.addPath(fileName);
        m_fileDiffDoc->openUrl(url);
    }
}

void HgCommitDialog::slotMessageChanged()
{
    enableButtonOk(!m_commitMessage->toPlainText().isEmpty());
}

void HgCommitDialog::done(int r)
{
    if (r == KDialog::Accepted) {
        QStringList files;
        if (m_statusList->getSelectionForCommit(files)) {
            HgWrapper *hgWrapper = HgWrapper::instance();
            bool success = hgWrapper->commit(m_commitMessage->toPlainText(),
                    files, m_branchAction==CloseBranch);
            if (success) {
                KDialog::done(r);
            }
            else {
                KMessageBox::error(this, i18n("Commit unsuccessful!"));
            }
        }
        else {
            KMessageBox::error(this, i18n("No files for commit!"));
        }
    }
    else {
        KDialog::done(r);
    }
}

void HgCommitDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setCommitDialogHeight(this->height());
    settings->setCommitDialogWidth(this->width());
    settings->writeConfig();
}

void HgCommitDialog::slotBranchActions(QAction *action)
{
    if (action == m_useCurrentBranch) {
        m_branchAction = NoChanges;
    }
    else if (action == m_newBranch) {
        m_branchAction = NewBranch;
    }
    else if (action == m_closeBranch) {
        m_branchAction = CloseBranch;
    }
}


/* Branch Dialog */

HgCommitDialog::NewBranchDialog::NewBranchDialog(QWidget *parent):
    KDialog(parent, Qt::Dialog)
{
}

#include "commitdialog.moc"

