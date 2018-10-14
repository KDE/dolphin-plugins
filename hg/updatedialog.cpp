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

#include "updatedialog.h"
#include "hgwrapper.h"

#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <KComboBox>
#include <KLocalizedString>
#include <KMessageBox>


HgUpdateDialog::HgUpdateDialog(QWidget *parent):
    DialogBase(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, parent)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Update"));

    this->okButton()->setText(xi18nc("@action:button", "Update"));

    // UI 
    QGroupBox *selectGroup = new QGroupBox(i18n("New working directory"));
    QVBoxLayout *selectLayout = new QVBoxLayout;
    m_selectType = new KComboBox;
    m_selectFinal = new KComboBox;
    m_selectType->addItem(i18n("Branch"));
    m_selectType->addItem(i18n("Tag"));
    m_selectType->addItem(i18n("Changeset/Revision"));
    selectLayout->addWidget(m_selectType);
    selectLayout->addWidget(m_selectFinal);
    selectGroup->setLayout(selectLayout);

    QGroupBox *infoGroup = new QGroupBox(i18n("Current Parent"));
    QVBoxLayout *infoLayout = new QVBoxLayout;
    m_currentInfo = new QLabel;
    infoLayout->addWidget(m_currentInfo);
    infoGroup->setLayout(infoLayout);

    QGroupBox *optionGroup = new QGroupBox(i18n("Options"));
    QVBoxLayout *optionLayout = new QVBoxLayout;
    m_discardChanges = new QCheckBox(i18n("Discard uncommitted changes"));
    m_discardChanges->setCheckState(Qt::Unchecked);
    optionLayout->addWidget(m_discardChanges);
    optionGroup->setLayout(optionLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(infoGroup);
    mainLayout->addWidget(selectGroup);
    mainLayout->addWidget(optionGroup);

    slotUpdateDialog(0);
    layout()->insertLayout(0, mainLayout);

    // connections
    connect(m_selectType, SIGNAL(currentIndexChanged(int)), this,
            SLOT(slotUpdateDialog(int)));
}

void HgUpdateDialog::slotUpdateDialog(int index)
{
    HgWrapper *hgWrapper = HgWrapper::instance();
    m_selectFinal->clear();
    if (index == 0) {
        m_updateTo = ToBranch;
        m_selectFinal->setEditable(false);
        m_selectFinal->addItems(hgWrapper->getBranches());
    }
    else if (index == 1) {
        m_updateTo = ToTag;
        m_selectFinal->setEditable(false);
        m_selectFinal->addItems(hgWrapper->getTags());
    }
    else if (index == 2) {
        m_updateTo = ToRevision;
        m_selectFinal->setEditable(true);
    }
    m_selectFinal->setFocus();

    /// get parents of current working directory
    /// more precise information using 'hg summary'
    /// but no proper way to retrieve needed data
    QString output;
    QStringList args;
    args << QLatin1String("--template");
    args << QLatin1String("{rev}:{node|short} ({branch})\n");
    hgWrapper->executeCommand(QLatin1String("parents"), args, output);
    output.replace(QLatin1String("\n"), QLatin1String("<br/>"));
    if (output.contains(QLatin1String("()"))) {
        output.replace(QLatin1String("()"), QLatin1String("(default)"));
    }
    m_currentInfo->setText(output);
}

void HgUpdateDialog::done(int r)
{
    if (r == QDialog::Accepted) {
        QStringList args;
        // Should we discard uncommitted changes
        if (m_discardChanges->checkState() == Qt::Checked) {
            args << "-C";
        }
        else {
            args << "-c";
        }
        if (m_updateTo == ToRevision) {
            args << "-r";
        }

        // update to
        args << m_selectFinal->currentText();

        // execute mercurial command
        HgWrapper *hgw = HgWrapper::instance();
        if (hgw->executeCommandTillFinished(QLatin1String("update"), args)) {
            QDialog::done(r);
        }
        else {
            KMessageBox::error(this, i18n("Some error occurred! "
                        "\nMaybe there are uncommitted changes."));
        }
    }
    else {
        QDialog::done(r);
    }
}

