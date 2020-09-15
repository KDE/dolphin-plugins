/***************************************************************************
 *   Copyright (C) 2011 by Vishesh Yadav <vishesh3y@gmail.com>             *
 *   Copyright (C) 2015 by Tomasz Bojczuk <seelook@gmail.com>              *
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

#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QActionGroup>
#include <QPlainTextEdit>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QSplitter>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KMessageBox>
#include <KLocalizedString>

HgCommitDialog::HgCommitDialog(QWidget *parent):
    DialogBase(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, parent)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Commit"));

    okButton()->setText(xi18nc("@action:button", "Commit"));
    okButton()->setDisabled(true);

    // To show diff between commit
    KTextEditor::Editor *editor = KTextEditor::Editor::instance();
    if (!editor) {
        KMessageBox::error(this, 
                i18n("The KTextEditor component could not be found;"
                     "\nplease check your KDE Frameworks installation."));
        return;
    }
    m_fileDiffDoc = editor->createDocument(0);
    m_fileDiffView = qobject_cast<KTextEditor::View*>(m_fileDiffDoc->createView(this));
    m_fileDiffView->setStatusBarEnabled(false);
    m_fileDiffDoc->setReadWrite(false);

    // Setup actions
    m_useCurrentBranch = new QAction(this);
    m_useCurrentBranch->setCheckable(true);
    m_useCurrentBranch->setText(xi18nc("@action:inmenu",
                               "Commit to current branch"));

    m_newBranch = new QAction(this);
    m_newBranch->setCheckable(true);
    m_newBranch->setText(xi18nc("@action:inmenu",
                               "Create new branch"));

    m_closeBranch = new QAction(this);
    m_closeBranch->setCheckable(true);
    m_closeBranch->setText(xi18nc("@action:inmenu",
                                 "Close current branch"));

    m_branchMenu = new QMenu(this);
    m_branchMenu->addAction(m_useCurrentBranch);
    m_branchMenu->addAction(m_newBranch);
    m_branchMenu->addAction(m_closeBranch);

    QActionGroup *branchActionGroup = new QActionGroup(this);
    branchActionGroup->addAction(m_useCurrentBranch);
    branchActionGroup->addAction(m_newBranch);
    branchActionGroup->addAction(m_closeBranch);
    m_useCurrentBranch->setChecked(true);
    connect(branchActionGroup, &QActionGroup::triggered,
            this, &HgCommitDialog::slotBranchActions);


    //////////////
    // Setup UI //
    //////////////

    // Top bar of buttons
    QHBoxLayout *topBarLayout = new QHBoxLayout;
    m_copyMessageButton = new QPushButton(i18n("Copy Message"));
    m_branchButton = new QPushButton(i18n("Branch"));

    m_copyMessageMenu = new QMenu(this);
    createCopyMessageMenu();

    topBarLayout->addWidget(new QLabel(getParentForLabel()));
    topBarLayout->addStretch();
    topBarLayout->addWidget(m_branchButton);
    topBarLayout->addWidget(m_copyMessageButton);
    m_branchButton->setMenu(m_branchMenu);
    m_copyMessageButton->setMenu(m_copyMessageMenu);

    // the commit box itself
    QGroupBox *messageGroupBox = new QGroupBox;
    QVBoxLayout *commitLayout = new QVBoxLayout;
    m_commitMessage = editor->createDocument(0);
    KTextEditor::View *messageView =
              qobject_cast<KTextEditor::View*>(m_commitMessage->createView(this));
    messageView->setStatusBarEnabled(false);
    messageView->setMinimumHeight(fontMetrics().height() * 4);
    commitLayout->addWidget(messageView);
    messageGroupBox->setTitle(xi18nc("@title:group", "Commit Message"));
    messageGroupBox->setLayout(commitLayout);

    // Show diff here
    QGroupBox *diffGroupBox = new QGroupBox;
    QVBoxLayout *diffLayout = new QVBoxLayout(diffGroupBox);
    diffLayout->addWidget(m_fileDiffView);
    diffGroupBox->setTitle(xi18nc("@title:group", "Diff/Content"));
    diffGroupBox->setLayout(diffLayout);

    // Set up layout for Status, Commit and Diff boxes
    m_verticalSplitter = new QSplitter(Qt::Horizontal);
    m_horizontalSplitter = new QSplitter(Qt::Vertical);
    m_horizontalSplitter->addWidget(messageGroupBox);
    m_horizontalSplitter->addWidget(diffGroupBox);
    m_statusList = new HgStatusList;
    m_verticalSplitter->addWidget(m_statusList);
    m_verticalSplitter->addWidget(m_horizontalSplitter);

    // Set up layout and container for main dialog
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topBarLayout);
    mainLayout->addWidget(m_verticalSplitter);
    layout()->insertLayout(0, mainLayout);

    slotBranchActions(m_useCurrentBranch);
    slotInitDiffOutput(); // initialize with whole repo diff

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->resize(QSize(settings->commitDialogWidth(),
                               settings->commitDialogHeight()));

    m_verticalSplitter->setSizes(settings->verticalSplitterSizes());
    m_horizontalSplitter->setSizes(settings->horizontalSplitterSizes());

    messageView->setFocus(); // message editor ready to get a text

    connect(m_statusList, &HgStatusList::itemSelectionChanged,
        this, &HgCommitDialog::slotItemSelectionChanged);
    connect(m_commitMessage, &KTextEditor::Document::textChanged,
         this, &HgCommitDialog::slotMessageChanged);
    connect(this, SIGNAL(finished(int)), this, SLOT(saveGeometry()));
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
    m_fileDiffDoc->closeUrl(true);

    QString diffOut;
    HgWrapper *hgWrapper = HgWrapper::instance();
    hgWrapper->executeCommand(QLatin1String("diff"), QStringList(), diffOut);
    m_fileDiffDoc->setHighlightingMode("diff");
    m_fileDiffDoc->setText(diffOut);
    m_fileDiffView->setCursorPosition( KTextEditor::Cursor(0, 0) );
    m_fileDiffDoc->setReadWrite(false);
}

void HgCommitDialog::slotItemSelectionChanged(const char status, 
        const QString &fileName)
{
    m_fileDiffDoc->setReadWrite(true);
    m_fileDiffDoc->setModified(false);
    m_fileDiffDoc->closeUrl(true);

    if (status != '?') {
        QStringList arguments;
        QString diffOut;
        HgWrapper *hgWrapper = HgWrapper::instance();

        arguments << fileName;
        hgWrapper->executeCommand(QLatin1String("diff"), arguments, diffOut);
        m_fileDiffDoc->setText(diffOut);
        m_fileDiffDoc->setHighlightingMode("diff");
    }
    else {
        QUrl url = QUrl::fromLocalFile(HgWrapper::instance()->getBaseDir());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + '/' + fileName);
        m_fileDiffDoc->openUrl(url);
    }

    m_fileDiffDoc->setReadWrite(false);
    m_fileDiffView->setCursorPosition( KTextEditor::Cursor(0, 0) );
}

void HgCommitDialog::slotMessageChanged()
{
    okButton()->setDisabled(m_commitMessage->isEmpty());
}

void HgCommitDialog::done(int r)
{
    if (r == QDialog::Accepted) {
        QStringList files;
        if (m_statusList->getSelectionForCommit(files)) {
            HgWrapper *hgWrapper = HgWrapper::instance();
            if (m_branchAction == NewBranch) {
                if (!hgWrapper->createBranch(m_newBranchName)) {
                    KMessageBox::error(this,
                            i18n("Could not create branch! Aborting commit!"));
                    return;
                }
            }
            bool success = hgWrapper->commit(m_commitMessage->text(),
                    files, m_branchAction==CloseBranch);
            if (success) {
                QDialog::done(r);
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
        QDialog::done(r);
    }
}

void HgCommitDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setCommitDialogHeight(this->height());
    settings->setCommitDialogWidth(this->width());
    settings->setHorizontalSplitterSizes(m_horizontalSplitter->sizes());
    settings->setVerticalSplitterSizes(m_verticalSplitter->sizes());
    settings->save();
}

void HgCommitDialog::slotBranchActions(QAction *action)
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString currentBranch;
    hgWrapper->executeCommand(QLatin1String("branch"), QStringList(), currentBranch);
    currentBranch.replace('\n', QString());
    currentBranch = " (" + currentBranch + ')';
    if (action == m_useCurrentBranch) {
        m_branchAction = NoChanges;
        m_branchButton->setText(i18n("Branch: Current Branch") + currentBranch);
    }
    else if (action == m_newBranch) {
        NewBranchDialog diag;
        if (diag.exec() == QDialog::Accepted) {
           m_branchAction = NewBranch;
           m_newBranchName = diag.getBranchName(); 
           m_branchButton->setText(i18n("Branch: ") + m_newBranchName);
        }
        else { // restore previous check state
            if (m_branchAction == NoChanges) {
                m_useCurrentBranch->setChecked(true);
            }
            else if (m_branchAction == CloseBranch) {
                m_closeBranch->setChecked(true);
            }
        }
    }
    else if (action == m_closeBranch) {
        m_branchAction = CloseBranch;
        m_branchButton->setText(i18n("Branch: Close Current") + currentBranch);
    }
}

/*****************/
/* Branch Dialog */
/*****************/

NewBranchDialog::NewBranchDialog(QWidget *parent):
    QDialog(parent)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Commit: New Branch"));
    QDialogButtonBox *buttonBox= new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    m_okButton->setDisabled(true);
    m_okButton->setDefault(true);

    m_branchList = HgWrapper::instance()->getBranches();

    QLabel *message = new QLabel(xi18nc("@label", "Enter new branch name"));
    m_branchNameInput = new QLineEdit;
    m_errorLabel = new QLabel;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(message);
    layout->addWidget(m_branchNameInput);
    layout->addWidget(m_errorLabel);
    layout->addWidget(buttonBox);

    setLayout(layout);

    connect(m_branchNameInput, &QLineEdit::textChanged,
            this, &NewBranchDialog::slotTextChanged);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void NewBranchDialog::slotTextChanged(const QString &text)
{
    if (m_branchList.contains(text)) {
        m_errorLabel->setText(xi18nc("@label", "<b>Branch already exists!</b>"));
        m_okButton->setDisabled(true);
    }
    else if (text.length() > 0) {
        m_errorLabel->clear();
        m_okButton->setDisabled(false);
    }
    else {
        m_errorLabel->setText(xi18nc("@label", "<b>Enter some text!</b>"));
        m_okButton->setDisabled(true);
    }
}

QString NewBranchDialog::getBranchName() const
{
    return m_branchNameInput->text();
}

void HgCommitDialog::createCopyMessageMenu()
{
    QActionGroup *actionGroup = new QActionGroup(this);
    connect(actionGroup, &QActionGroup::triggered,
            this, &HgCommitDialog::slotInsertCopyMessage);

    QStringList args;
    args << QLatin1String("--limit");
    args << QLatin1String("7");
    args << QLatin1String("--template");
    args << QLatin1String("{desc}\n");
//     args << QLatin1String("{desc|short}\n");

    HgWrapper *hgw = HgWrapper::instance();
    QString output;
    hgw->executeCommand(QLatin1String("log"), args, output);

    const QStringList messages = output.split('\n', QString::SkipEmptyParts);
    for (const QString &msg : messages) {
        QAction *action = m_copyMessageMenu->addAction(msg.left(40)); // max 40 characters
        action->setData(msg); // entire description into action data
        actionGroup->addAction(action);
    }
}

void HgCommitDialog::slotInsertCopyMessage(QAction *action)
{
    m_commitMessage->setText(action->data().toString());
}


