/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tagdialog.h"
#include "hgwrapper.h"

#include <QLineEdit>
#include <KComboBox>
#include <KLocalizedString>
#include <KMessageBox>

HgTagDialog::HgTagDialog(QWidget *parent):
    DialogBase(QDialogButtonBox::NoButton, parent)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Tag"));

    // UI 
    QVBoxLayout *vbox = new QVBoxLayout;

    m_tagComboBox = new KComboBox;
    m_tagComboBox->setEditable(true);
    vbox->addWidget(m_tagComboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_createTag = new QPushButton(i18n("Create New Tag"));
    m_removeTag = new QPushButton(i18n("Remove Tag"));
    m_updateTag = new QPushButton(i18n("Switch Tag"));
    buttonLayout->addWidget(m_createTag);
    buttonLayout->addWidget(m_removeTag);
    buttonLayout->addWidget(m_updateTag);
    vbox->addLayout(buttonLayout);

    m_createTag->setEnabled(false);
    m_updateTag->setEnabled(false);
    m_removeTag->setEnabled(false);

    updateInitialDialog();
    slotUpdateDialog(QString());
    layout()->insertLayout(0, vbox);

    slotUpdateDialog(m_tagComboBox->currentText());
    
    // connections
    connect(m_createTag, &QAbstractButton::clicked,
            this, &HgTagDialog::slotCreateTag);
    connect(m_removeTag, &QAbstractButton::clicked,
            this, &HgTagDialog::slotRemoveTag);
    connect(m_updateTag, &QAbstractButton::clicked,
            this, &HgTagDialog::slotSwitch);
    connect(m_tagComboBox, &QComboBox::editTextChanged,
            this, &HgTagDialog::slotUpdateDialog);
    connect(m_tagComboBox->lineEdit(), &QLineEdit::textChanged,
            this, &HgTagDialog::slotUpdateDialog);
}

void HgTagDialog::updateInitialDialog()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    // update combo box
    m_tagList = hgWrapper->getTags();
    m_tagComboBox->addItems(m_tagList);

}

void HgTagDialog::slotUpdateDialog(const QString &text)
{
    // update pushbuttons
    if (text.length() == 0) {
        m_createTag->setEnabled(false);
        m_updateTag->setEnabled(false);
        m_removeTag->setEnabled(false);
    }
    else if (m_tagList.contains(text)) {
        m_createTag->setEnabled(false);
        m_updateTag->setEnabled(true);
        m_removeTag->setEnabled(true);
    }
    else {
        m_createTag->setEnabled(true);
        m_updateTag->setEnabled(false);
        m_removeTag->setEnabled(false);
    }
}

void HgTagDialog::slotSwitch()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << QLatin1String("-c");
    args << m_tagComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("update"), args, out)) {
        //KMessageBox::information(this, i18n("Updated working directory!"));
        done(QDialog::Accepted);
    }
    else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}

void HgTagDialog::slotRemoveTag()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << QLatin1String("--remove");
    args << m_tagComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("tag"), args, out)) {
        //KMessageBox::information(this, xi18nc("Removed tag successfully!"));
        done(QDialog::Accepted);
    }
    else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}

void HgTagDialog::slotCreateTag()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << m_tagComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("tag"), args, out)) {
        KMessageBox::information(this, i18n("Created tag successfully!"));
        done(QDialog::Accepted);
    }
    else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}


