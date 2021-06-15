/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "renamedialog.h"
#include "hgwrapper.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QDir>
#include <QLineEdit>
#include <KLocalizedString>
#include <KFileItem>

HgRenameDialog::HgRenameDialog(const KFileItem &source, QWidget *parent):
    DialogBase(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, parent),
    m_source(source.name()),
    m_source_dir(QDir(source.url().fileName()).dirName()) //FIXME seems to be unused at all, anyway be careful porting to pure KF5
//     m_source_dir(source.url().directory())
{
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Rename"));

    okButton()->setText(xi18nc("@action:button", "Rename"));
    okButton()->setIcon(QIcon::fromTheme("list-rename"));

    QGridLayout *mainLayout = new QGridLayout(this);

    QLabel *sourceLabel = new QLabel(xi18nc("@label:label to source file",
                "Source "), this);
    QLabel *sourceFileLabel = new QLabel("<b>" + m_source + "</b>");
    mainLayout->addWidget(sourceLabel, 0, 0);
    mainLayout->addWidget(sourceFileLabel, 0, 1);

    QLabel *destinationLabel 
        = new QLabel(xi18nc("@label:rename", "Rename to "), this);
    m_destinationFile = new QLineEdit(m_source, this);
    mainLayout->addWidget(destinationLabel, 1, 0);
    mainLayout->addWidget(m_destinationFile, 1, 1);

    layout()->insertLayout(0, mainLayout);

    m_destinationFile->setFocus();
    m_destinationFile->selectAll();

    connect(m_destinationFile, &QLineEdit::textChanged,
            this, &HgRenameDialog::slotTextChanged);
}

void HgRenameDialog::slotTextChanged(const QString &text)
{
    okButton()->setEnabled(text.length() != 0);
}

void HgRenameDialog::done(int r)
{
    if (r == QDialog::Accepted) {
        HgWrapper *hgi = HgWrapper::instance();
        hgi->renameFile(source(), destination());
    }
    QDialog::done(r);
}

QString HgRenameDialog::source() const
{
    return m_source;
}

QString HgRenameDialog::destination() const
{
    return m_destinationFile->text();
}


