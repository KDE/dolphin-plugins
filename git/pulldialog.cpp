/*
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pulldialog.h"
#include "gitwrapper.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

PullDialog::PullDialog(QWidget* parent):
    QDialog(parent, Qt::Dialog)
{
    this->setWindowTitle(xi18nc("@title:window", "<application>Git</application> Pull"));
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    this->connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    this->connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    okButton->setText(i18nc("@action:button", "Pull"));

    QWidget * boxWidget = new QWidget(this);
    QVBoxLayout * boxLayout = new QVBoxLayout(boxWidget);
    mainLayout->addWidget(boxWidget);

    QGroupBox * sourceGroupBox = new QGroupBox(boxWidget);
    mainLayout->addWidget(sourceGroupBox);
    boxLayout->addWidget(sourceGroupBox);
    sourceGroupBox->setTitle(i18nc("@title:group The source to pull from", "Source"));
    QHBoxLayout * sourceHBox = new QHBoxLayout(sourceGroupBox);
    sourceGroupBox->setLayout(sourceHBox);

    mainLayout->addWidget(m_buttonBox);

    QLabel * remoteLabel = new QLabel(i18nc("@label:listbox a git remote", "Remote:"), sourceGroupBox);
    sourceHBox->addWidget(remoteLabel);
    m_remoteComboBox = new QComboBox(sourceGroupBox);
    sourceHBox->addWidget(m_remoteComboBox);

    QLabel * remoteBranchLabel = new QLabel(i18nc("@label:listbox", "Remote branch:"), sourceGroupBox);
    sourceHBox->addWidget(remoteBranchLabel);
    m_remoteBranchComboBox = new QComboBox(sourceGroupBox);
    sourceHBox->addWidget(m_remoteBranchComboBox);

    //populate UI
    GitWrapper * gitWrapper = GitWrapper::instance();

    //get sources
    m_remoteComboBox->addItems(gitWrapper->pullRemotes());

    //get branch names
    int currentBranchIndex;
    const QStringList branches = gitWrapper->branches(&currentBranchIndex);

    for (const QString& branch : branches) {
        if (branch.startsWith(QLatin1String("remotes/"))) {
            const QString remote = branch.section('/', 1, 1);
            const QString name = branch.section('/', 2);
            m_remoteBranches[remote] << name;
        }
    }
    remoteSelectionChanged(m_remoteComboBox->currentText());
    if (currentBranchIndex >= 0) {
        const int index = m_remoteBranchComboBox->findText(branches.at(currentBranchIndex));
        if (index != -1) {
            m_remoteBranchComboBox->setCurrentIndex(index);
        }
    }

    //Signals
    connect(m_remoteComboBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(remoteSelectionChanged(QString)));
}

QString PullDialog::source() const
{
    return m_remoteComboBox->currentText();
}

QString PullDialog::remoteBranch() const
{
    return m_remoteBranchComboBox->currentText();
}

void PullDialog::remoteSelectionChanged(const QString& newRemote)
{
    m_remoteBranchComboBox->clear();
    m_remoteBranchComboBox->addItems(m_remoteBranches.value(newRemote));
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(m_remoteBranchComboBox->count() > 0);
}

