/******************************************************************************
 *   Copyright (C) 2010 by Johannes Steffen <johannes.steffen@st.ovgu.de>     *
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

#include "tagdialog.h"
#include "gitwrapper.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <ktextedit.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QTextCodec>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

TagDialog::TagDialog (QWidget* parent ):
    QDialog (parent, Qt::Dialog),
    m_localCodec(QTextCodec::codecForLocale())
{
    this->setWindowTitle(xi18nc("@title:window", "<application>Git</application> Create Tag"));
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
    m_buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    okButton->setText(i18nc("@action:button", "Create Tag"));

    QWidget* boxWidget = new QWidget(this);
    QVBoxLayout* boxLayout = new QVBoxLayout(boxWidget);
    mainLayout->addWidget(boxWidget);

    QGroupBox* tagInformationGroupBox = new QGroupBox(boxWidget);
    mainLayout->addWidget(tagInformationGroupBox);
    boxLayout->addWidget(tagInformationGroupBox);
    tagInformationGroupBox->setTitle(i18nc("@title:group", "Tag Information"));
    QVBoxLayout * tagInformationLayout = new QVBoxLayout(tagInformationGroupBox);
    tagInformationGroupBox->setLayout(tagInformationLayout);

    QLabel* nameLabel = new QLabel(i18nc("@label:textbox", "Tag Name:"), tagInformationGroupBox);
    tagInformationLayout->addWidget(nameLabel);

    m_tagNameTextEdit = new QLineEdit(tagInformationGroupBox);
    tagInformationLayout->addWidget(m_tagNameTextEdit);
    setOkButtonState();
    connect(m_tagNameTextEdit, SIGNAL(textChanged(QString)), this, SLOT(setOkButtonState()));

    QLabel* messageLabel = new QLabel(i18nc("@label:textbox", "Tag Message:"), tagInformationGroupBox);
    tagInformationLayout->addWidget(messageLabel);

    m_tagMessageTextEdit = new KTextEdit(tagInformationGroupBox);
    m_tagMessageTextEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
    m_tagMessageTextEdit->setLineWrapColumnOrWidth(72);
    tagInformationLayout->addWidget(m_tagMessageTextEdit);

    QGroupBox* attachToGroupBox = new QGroupBox(boxWidget);
    mainLayout->addWidget(attachToGroupBox);
    boxLayout->addWidget(attachToGroupBox);
    attachToGroupBox->setTitle(i18nc("@title:group", "Attach to"));

    mainLayout->addWidget(m_buttonBox);

    QHBoxLayout* attachToLayout = new QHBoxLayout();
    attachToGroupBox->setLayout(attachToLayout);

    QLabel* branchLabel = new QLabel(i18nc("@label:listbox", "Branch:"), attachToGroupBox);
    attachToLayout->addWidget(branchLabel);

    m_branchComboBox = new QComboBox(attachToGroupBox);
    attachToLayout->addWidget(m_branchComboBox);
    attachToLayout->addStretch();

    this->resize(QSize(300,200));

    //initialize alternate color scheme for errors
    m_errorColors = m_tagNameTextEdit->palette();
    m_errorColors.setColor(QPalette::Normal, QPalette::Base, Qt::red);
    m_errorColors.setColor(QPalette::Inactive, QPalette::Base, Qt::red);

    //get branch & tag names
    GitWrapper * gitWrapper = GitWrapper::instance();

    int currentIndex;
    const QStringList branches = gitWrapper->branches(&currentIndex);
    m_branchComboBox->addItems(branches);
    m_branchComboBox->setCurrentIndex(currentIndex);

    gitWrapper->tagSet(m_tagNames);
}


QByteArray TagDialog::tagMessage() const
{
    return m_localCodec->fromUnicode(m_tagMessageTextEdit->toPlainText());
}

QString TagDialog::tagName() const
{
    return m_tagNameTextEdit->text().trimmed();
}

QString TagDialog::baseBranch() const
{
    return  m_branchComboBox->currentText();
}

void TagDialog::setOkButtonState()
{
      const QString tagName = m_tagNameTextEdit->text().trimmed();
      QString toolTip;
      if (tagName.isEmpty()) {
        toolTip = i18nc("@info:tooltip", "You must enter a tag name first.");
      }
      else if (tagName.contains(QRegExp("\\s"))) {
        toolTip = i18nc("@info:tooltip", "Tag names may not contain any whitespace.");
      }
      else if (m_tagNames.contains(tagName)) {
        toolTip = i18nc("@info:tooltip", "A tag named '%1' already exists.", tagName);
      }
      QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
      okButton->setEnabled(toolTip.isEmpty());
      setLineEditErrorModeActive(!toolTip.isEmpty());
      m_tagNameTextEdit->setToolTip(toolTip);
      okButton->setToolTip(toolTip);
}

void TagDialog::setLineEditErrorModeActive(bool active)
{
    m_tagNameTextEdit->setPalette(active ? m_errorColors : QPalette());
}

