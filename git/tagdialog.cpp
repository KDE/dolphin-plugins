/*
    SPDX-FileCopyrightText: 2010 Johannes Steffen <johannes.steffen@st.ovgu.de>
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tagdialog.h"
#include "gitwrapper.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KTextEdit>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

TagDialog::TagDialog(QWidget *parent)
    : QDialog(parent, Qt::Dialog)
{
    this->setWindowTitle(xi18nc("@title:window", "<application>Git</application> Create Tag"));
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    this->connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    this->connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    okButton->setText(i18nc("@action:button", "Create Tag"));

    QWidget *boxWidget = new QWidget(this);
    QVBoxLayout *boxLayout = new QVBoxLayout(boxWidget);
    mainLayout->addWidget(boxWidget);

    QGroupBox *tagInformationGroupBox = new QGroupBox(boxWidget);
    mainLayout->addWidget(tagInformationGroupBox);
    boxLayout->addWidget(tagInformationGroupBox);
    tagInformationGroupBox->setTitle(i18nc("@title:group", "Tag Information"));
    QVBoxLayout *tagInformationLayout = new QVBoxLayout(tagInformationGroupBox);
    tagInformationGroupBox->setLayout(tagInformationLayout);

    QLabel *nameLabel = new QLabel(i18nc("@label:textbox", "Tag Name:"), tagInformationGroupBox);
    tagInformationLayout->addWidget(nameLabel);

    m_tagNameTextEdit = new QLineEdit(tagInformationGroupBox);
    tagInformationLayout->addWidget(m_tagNameTextEdit);
    setOkButtonState();
    connect(m_tagNameTextEdit, &QLineEdit::textChanged, this, &TagDialog::setOkButtonState);

    QLabel *messageLabel = new QLabel(i18nc("@label:textbox", "Tag Message:"), tagInformationGroupBox);
    tagInformationLayout->addWidget(messageLabel);

    m_tagMessageTextEdit = new KTextEdit(tagInformationGroupBox);
    m_tagMessageTextEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
    m_tagMessageTextEdit->setLineWrapColumnOrWidth(72);
    tagInformationLayout->addWidget(m_tagMessageTextEdit);

    QGroupBox *attachToGroupBox = new QGroupBox(boxWidget);
    mainLayout->addWidget(attachToGroupBox);
    boxLayout->addWidget(attachToGroupBox);
    attachToGroupBox->setTitle(i18nc("@title:group", "Attach to"));

    mainLayout->addWidget(m_buttonBox);

    QHBoxLayout *attachToLayout = new QHBoxLayout();
    attachToGroupBox->setLayout(attachToLayout);

    QLabel *branchLabel = new QLabel(i18nc("@label:listbox", "Branch:"), attachToGroupBox);
    attachToLayout->addWidget(branchLabel);

    m_branchComboBox = new QComboBox(attachToGroupBox);
    attachToLayout->addWidget(m_branchComboBox);
    attachToLayout->addStretch();

    this->resize(QSize(300, 200));

    // initialize alternate color scheme for errors
    m_errorColors = m_tagNameTextEdit->palette();
    m_errorColors.setColor(QPalette::Normal, QPalette::Base, Qt::red);
    m_errorColors.setColor(QPalette::Inactive, QPalette::Base, Qt::red);

    // get branch & tag names
    GitWrapper *gitWrapper = GitWrapper::instance();

    int currentIndex;
    const QStringList branches = gitWrapper->branches(&currentIndex);
    m_branchComboBox->addItems(branches);
    m_branchComboBox->setCurrentIndex(currentIndex);

    gitWrapper->tagSet(m_tagNames);
}

QByteArray TagDialog::tagMessage() const
{
    return m_tagMessageTextEdit->toPlainText().toLocal8Bit();
}

QString TagDialog::tagName() const
{
    return m_tagNameTextEdit->text().trimmed();
}

QString TagDialog::baseBranch() const
{
    return m_branchComboBox->currentText();
}

void TagDialog::setOkButtonState()
{
    const QString tagName = m_tagNameTextEdit->text().trimmed();
    QString toolTip;
    if (tagName.isEmpty()) {
        toolTip = i18nc("@info:tooltip", "You must enter a tag name first.");
    } else if (tagName.contains(QRegularExpression(QStringLiteral("\\s")))) {
        toolTip = i18nc("@info:tooltip", "Tag names may not contain any whitespace.");
    } else if (m_tagNames.contains(tagName)) {
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

#include "moc_tagdialog.cpp"
