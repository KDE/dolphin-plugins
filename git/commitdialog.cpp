/******************************************************************************
 *   Copyright (C) 2010 by Sebastian Doerner <sebastian@sebastian-doerner.de> *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program; if not, write to the                            *
 *   Free Software Foundation, Inc.,                                          *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA               *
 ******************************************************************************/

#include "commitdialog.h"
#include "fileviewgitpluginsettings.h"
#include "gitwrapper.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <ktextedit.h>

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTextCodec>
#include <QVBoxLayout>

CommitDialog::CommitDialog (QWidget* parent ):
    QDialog (parent, Qt::Dialog),
    m_localCodec(QTextCodec::codecForLocale())
{
    this->setWindowTitle(xi18nc("@title:window", "<application>Git</application> Commit"));
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    this->connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    this->connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    okButton->setText(i18nc("@action:button", "Commit"));

    QWidget* boxWidget = new QWidget(this);
    QVBoxLayout* boxLayout = new QVBoxLayout(boxWidget);
    mainLayout->addWidget(boxWidget);

    QGroupBox* messageGroupBox = new QGroupBox(boxWidget);
    mainLayout->addWidget(messageGroupBox);
    boxLayout->addWidget(messageGroupBox);
    messageGroupBox->setTitle(i18nc("@title:group", "Commit message"));

    mainLayout->addWidget(m_buttonBox);

    QVBoxLayout * messageVBox = new QVBoxLayout(messageGroupBox);
    messageGroupBox->setLayout(messageVBox);

    m_commitMessageTextEdit = new KTextEdit(messageGroupBox);
    m_commitMessageTextEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
    m_commitMessageTextEdit->setLineWrapColumnOrWidth(72);
    messageVBox->addWidget(m_commitMessageTextEdit);
    setOkButtonState();
    connect(m_commitMessageTextEdit, SIGNAL(textChanged()), this, SLOT(setOkButtonState()));

    QHBoxLayout* messageHBox = new QHBoxLayout();
    messageVBox->addLayout(messageHBox);

    m_amendCheckBox = new QCheckBox(i18nc("@option:check", "Amend last commit"), messageGroupBox);
    messageHBox->addWidget(m_amendCheckBox);
    //read last commit message
    m_alternativeMessage = GitWrapper::instance()->lastCommitMessage();
    if (m_alternativeMessage.isNull()) {
        m_amendCheckBox->setEnabled(false);
        m_amendCheckBox->setToolTip(i18nc("@info:tooltip", "There is nothing to amend."));
    } else {
        connect(m_amendCheckBox, SIGNAL(stateChanged(int)),
                this, SLOT(amendCheckBoxStateChanged()));
    }

    QPushButton * signOffButton = new QPushButton(
        i18nc("@action:button Add Signed-Off line to the message widget", "Sign off"),messageGroupBox);
    signOffButton->setToolTip(i18nc("@info:tooltip", "Add Signed-off-by line at the end of the commit message."));
    messageHBox->addStretch();
    messageHBox->addWidget(signOffButton);

    //restore dialog size
    FileViewGitPluginSettings* settings = FileViewGitPluginSettings::self();
    this->resize(QSize(settings->commitDialogWidth(), settings->commitDialogHeight()));

    connect(this, SIGNAL(finished()), this, SLOT(saveDialogSize()));
    connect(signOffButton, SIGNAL(clicked(bool)), this, SLOT(signOffButtonClicked()));
}

void CommitDialog::signOffButtonClicked()
{
    if (m_userName.isNull()) {
        GitWrapper * gitWrapper= GitWrapper::instance();
        m_userName = gitWrapper->userName();
        m_userEmail = gitWrapper->userEmail();
    }
    //append Signed-off line
    QString lastline = m_commitMessageTextEdit->document()->lastBlock().text();
    bool noNewLine = lastline.startsWith(QLatin1String("Signed-off")) || lastline.isEmpty();
    m_commitMessageTextEdit->append(QString(noNewLine ? "" : "\n") + "Signed-off-by: "
            + m_userName + " <" + m_userEmail + '>');
}

QByteArray CommitDialog::commitMessage() const
{
    return m_localCodec->fromUnicode(m_commitMessageTextEdit->toPlainText());
}

bool CommitDialog::amend() const
{
    return m_amendCheckBox->isChecked();
}

void CommitDialog::amendCheckBoxStateChanged()
{
    QString tmp = m_commitMessageTextEdit->toPlainText();
    m_commitMessageTextEdit->setText(m_alternativeMessage);
    m_alternativeMessage = tmp;
}

void CommitDialog::saveDialogSize()
{
    FileViewGitPluginSettings* settings = FileViewGitPluginSettings::self();
    settings->setCommitDialogHeight(this->height());
    settings->setCommitDialogWidth(this->width());
    settings->save();
}

void CommitDialog::setOkButtonState()
{
    bool enable = !m_commitMessageTextEdit->toPlainText().isEmpty();
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(enable);
    okButton->setToolTip(enable ?
            "" : i18nc("@info:tooltip", "You must enter a commit message first."));
}

