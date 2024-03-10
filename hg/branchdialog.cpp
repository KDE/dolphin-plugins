/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "branchdialog.h"
#include "hgwrapper.h"

#include <KComboBox>
#include <KLocalizedString>
#include <KMessageBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

HgBranchDialog::HgBranchDialog(QWidget *parent)
    : DialogBase(QDialogButtonBox::NoButton, parent)
{
    // dialog properties
    this->setWindowTitle(i18nc("@title:window", "<application>Hg</application> Branch"));

    // UI
    QVBoxLayout *vbox = new QVBoxLayout;

    m_currentBranchLabel = new QLabel;
    vbox->addWidget(m_currentBranchLabel);

    m_branchComboBox = new KComboBox;
    m_branchComboBox->setEditable(true);
    vbox->addWidget(m_branchComboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_createBranch = new QPushButton(i18n("Create New Branch"));
    m_updateBranch = new QPushButton(i18n("Switch Branch"));
    buttonLayout->addWidget(m_createBranch);
    buttonLayout->addWidget(m_updateBranch);
    vbox->addLayout(buttonLayout);

    m_createBranch->setEnabled(false);
    m_updateBranch->setEnabled(false);

    updateInitialDialog();
    slotUpdateDialog(QString());
    layout()->insertLayout(0, vbox);

    slotUpdateDialog(m_branchComboBox->currentText());

    // connections
    connect(m_createBranch, &QAbstractButton::clicked, this, &HgBranchDialog::slotCreateBranch);
    connect(m_updateBranch, &QAbstractButton::clicked, this, &HgBranchDialog::slotSwitch);
    connect(m_branchComboBox, &QComboBox::editTextChanged, this, &HgBranchDialog::slotUpdateDialog);
    connect(m_branchComboBox->lineEdit(), &QLineEdit::textChanged, this, &HgBranchDialog::slotUpdateDialog);
}

void HgBranchDialog::updateInitialDialog()
{
    HgWrapper *hgWrapper = HgWrapper::instance();

    // update label - current branch
    QString out;
    hgWrapper->executeCommand(QLatin1String("branch"), QStringList(), out);
    out = i18n("<b>Current Branch: </b>") + out;
    m_currentBranchLabel->setText(out);

    // update combo box
    m_branchList = hgWrapper->getBranches();
    m_branchComboBox->addItems(m_branchList);
}

void HgBranchDialog::slotUpdateDialog(const QString &text)
{
    // update pushbuttons
    if (text.length() == 0) {
        m_createBranch->setEnabled(false);
        m_updateBranch->setEnabled(false);
    } else if (m_branchList.contains(text)) {
        m_createBranch->setEnabled(false);
        m_updateBranch->setEnabled(true);
    } else {
        m_createBranch->setEnabled(true);
        m_updateBranch->setEnabled(false);
    }
}

void HgBranchDialog::slotSwitch()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << QLatin1String("-c");
    args << m_branchComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("update"), args, out)) {
        // KMessageBox::information(this, i18n("Updated working directory!"));
        done(QDialog::Accepted);
    } else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}

void HgBranchDialog::slotCreateBranch()
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    QString out;
    QStringList args;
    args << m_branchComboBox->currentText();
    if (hgWrapper->executeCommand(QLatin1String("branch"), args, out)) {
        // KMessageBox::information(this, i18n("Created branch successfully!"));
        done(QDialog::Accepted);
    } else {
        KMessageBox::error(this, i18n("Some error occurred"));
    }
}

#include "moc_branchdialog.cpp"
