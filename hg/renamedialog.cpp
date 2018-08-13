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

    connect(m_destinationFile, SIGNAL(textChanged(const QString &)),
            this, SLOT(slotTextChanged(const QString &)));
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


