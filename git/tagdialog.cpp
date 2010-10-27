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

#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>
#include <ktextedit.h>
#include <kvbox.h>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QTextCodec>
#include <QVBoxLayout>
#include <QLabel>

TagDialog::TagDialog (QWidget* parent ):
    KDialog (parent, Qt::Dialog),
    m_localCodec(QTextCodec::codecForLocale())
{
    this->setCaption(i18nc("@title:window", "<application>Git</application> Create Tag"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Create Tag"));

    KVBox* vbox = new KVBox(this);
    this->setMainWidget(vbox);

    QGroupBox* tagInformationGroupBox = new QGroupBox(vbox);
    tagInformationGroupBox->setTitle(i18nc("@title:group", "Tag Information"));
    QVBoxLayout * tagInformationLayout = new QVBoxLayout(tagInformationGroupBox);
    tagInformationGroupBox->setLayout(tagInformationLayout);

    QLabel* nameLabel = new QLabel(i18nc("@label:textbox", "Tag Name:"), tagInformationGroupBox);
    tagInformationLayout->addWidget(nameLabel);

    m_tagNameTextEdit = new KLineEdit(tagInformationGroupBox);
    tagInformationLayout->addWidget(m_tagNameTextEdit);
    setOkButtonState();
    connect(m_tagNameTextEdit, SIGNAL(textChanged(QString)), this, SLOT(setOkButtonState()));

    QLabel* messageLabel = new QLabel(i18nc("@label:textbox", "Tag Message:"), tagInformationGroupBox);
    tagInformationLayout->addWidget(messageLabel);

    m_tagMessageTextEdit = new KTextEdit(tagInformationGroupBox);
    m_tagMessageTextEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
    m_tagMessageTextEdit->setLineWrapColumnOrWidth(72);
    tagInformationLayout->addWidget(m_tagMessageTextEdit);

    QGroupBox* attachToGroupBox = new QGroupBox(vbox);
    attachToGroupBox->setTitle(i18nc("@title:group", "Attach to"));

    QHBoxLayout* attachToLayout = new QHBoxLayout();
    attachToGroupBox->setLayout(attachToLayout);

    QLabel* branchLabel = new QLabel(i18nc("@label:listbox", "Branch:"), attachToGroupBox);
    attachToLayout->addWidget(branchLabel);

    m_branchComboBox = new KComboBox(false, attachToGroupBox);
    attachToLayout->addWidget(m_branchComboBox);
    attachToLayout->addStretch();

    this->setInitialSize(QSize(300,200));

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
      this->enableButtonOk(toolTip.isEmpty());
      setLineEditErrorModeActive(!toolTip.isEmpty());
      m_tagNameTextEdit->setToolTip(toolTip);
      this->setButtonToolTip(KDialog::Ok, toolTip);

}

void TagDialog::setLineEditErrorModeActive(bool active)
{
    m_tagNameTextEdit->setPalette(active ? m_errorColors : QPalette());
}

#include "tagdialog.moc"
