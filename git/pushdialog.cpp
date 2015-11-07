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

#include "pushdialog.h"
#include "gitwrapper.h"

#include <kcombobox.h>
#include <klocale.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

PushDialog::PushDialog (QWidget* parent ):
    KDialog (parent, Qt::Dialog)
{
    this->setCaption(xi18nc("@title:window", "<application>Git</application> Push"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Push"));

    QWidget * boxWidget = new QWidget(this);
    QVBoxLayout * boxLayout = new QVBoxLayout(boxWidget);
    this->setMainWidget(boxWidget);

    //Destination
    QGroupBox * destinationGroupBox = new QGroupBox(boxWidget);
    boxLayout->addWidget(destinationGroupBox);
    destinationGroupBox->setTitle(i18nc("@title:group The remote host", "Destination"));
    QHBoxLayout * destinationHBox = new QHBoxLayout(destinationGroupBox);
    destinationGroupBox->setLayout(destinationHBox);

    QLabel * remoteLabel = new QLabel(i18nc("@label:listbox a git remote", "Remote:"), destinationGroupBox);
    destinationHBox->addWidget(remoteLabel);
    m_remoteComboBox = new KComboBox(false, destinationGroupBox);
    destinationHBox->addWidget(m_remoteComboBox);
    destinationHBox->addStretch();

    //Branches
    QGroupBox* branchesGroupBox = new QGroupBox(boxWidget);
    boxLayout->addWidget(branchesGroupBox);
    branchesGroupBox->setTitle(i18nc("@title:group", "Branches"));
    QHBoxLayout * branchesHBox = new QHBoxLayout(branchesGroupBox);
    branchesGroupBox->setLayout(branchesHBox);

    QLabel * localBranchLabel = new QLabel(i18nc("@label:listbox", "Local Branch:"), branchesGroupBox);
    branchesHBox->addWidget(localBranchLabel);
    m_localBranchComboBox = new KComboBox(false, branchesGroupBox);
    branchesHBox->addWidget(m_localBranchComboBox);

    branchesHBox->addStretch();

    QLabel * remoteBranchLabel = new QLabel(i18nc("@label:listbox", "Remote Branch:"), branchesGroupBox);
    branchesHBox->addWidget(remoteBranchLabel);
    m_remoteBranchComboBox = new KComboBox(false, branchesGroupBox);
    branchesHBox->addWidget(m_remoteBranchComboBox);

    QGroupBox* optionsGroupBox = new QGroupBox(boxWidget);
    boxLayout->addWidget(optionsGroupBox);
    optionsGroupBox->setTitle(i18nc("@title:group", "Options"));
    QHBoxLayout * optionsHBox = new QHBoxLayout(optionsGroupBox);
    optionsGroupBox->setLayout(optionsHBox);
    m_forceCheckBox = new QCheckBox(i18nc("@option:check", "Force"), optionsGroupBox);
    m_forceCheckBox->setToolTip(i18nc("@info:tooltip", "Proceed even if the remote branch is not an ancestor of the local branch."));
    optionsHBox->addWidget(m_forceCheckBox);

    //populate UI
    GitWrapper * gitWrapper = GitWrapper::instance();

    //get destinations
    QStringList remotes = gitWrapper->pushRemotes();
    m_remoteComboBox->addItems(remotes);

    //get branch names
    int currentBranchIndex;
    QStringList branches = gitWrapper->branches(&currentBranchIndex);

    foreach (const QString& branch, branches) {
        if (branch.startsWith(QLatin1String("remotes/"))) {
            const QString remote = branch.section('/', 1, 1);
            const QString name = branch.section('/', 2);
            m_remoteBranches[remote] << name;
        } else {
            m_localBranchComboBox->addItem(branch);
        }
    }
    m_localBranchComboBox->setCurrentText(branches.at(currentBranchIndex));
    remoteSelectionChanged(m_remoteComboBox->currentText());

    //Signals
    connect(m_remoteComboBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(remoteSelectionChanged(QString)));
    connect(m_localBranchComboBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(localBranchSelectionChanged(QString)));
}

QString PushDialog::destination() const
{
    return m_remoteComboBox->currentText();
}

QString PushDialog::localBranch() const
{
    return m_localBranchComboBox->currentText();
}

QString PushDialog::remoteBranch() const
{
    return m_remoteBranchComboBox->currentText();
}

bool PushDialog::force() const
{
    return m_forceCheckBox->isChecked();
}

void PushDialog::remoteSelectionChanged(const QString& newRemote)
{
    m_remoteBranchComboBox->clear();
    m_remoteBranchComboBox->addItems(m_remoteBranches.value(newRemote));
    localBranchSelectionChanged(m_localBranchComboBox->currentText());
}

void PushDialog::localBranchSelectionChanged(const QString& newLocalBranch)
{
    //select matching remote branch if possible
    const int index = m_remoteBranchComboBox->findText(newLocalBranch);
    if (index != -1) {
        m_remoteBranchComboBox->setCurrentIndex(index);
    }
    this->enableButtonOk(m_remoteBranchComboBox->count() > 0);
}

#include "pushdialog.moc"
