/*
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pushdialog.h"
#include "gitwrapper.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <KMessageWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

PushDialog::PushDialog(QWidget *parent)
    : QDialog(parent, Qt::Dialog)
{
    this->setWindowTitle(xi18nc("@title:window", "<application>Git</application> Push"));
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
    okButton->setText(i18nc("@action:button", "Push"));

    QWidget *boxWidget = new QWidget(this);
    QVBoxLayout *boxLayout = new QVBoxLayout(boxWidget);
    mainLayout->addWidget(boxWidget);

    // Destination
    QGroupBox *destinationGroupBox = new QGroupBox(boxWidget);
    mainLayout->addWidget(destinationGroupBox);
    boxLayout->addWidget(destinationGroupBox);
    destinationGroupBox->setTitle(i18nc("@title:group The remote host", "Destination"));
    QHBoxLayout *destinationHBox = new QHBoxLayout(destinationGroupBox);
    destinationGroupBox->setLayout(destinationHBox);

    QLabel *remoteLabel = new QLabel(i18nc("@label:listbox a git remote", "Remote:"), destinationGroupBox);
    destinationHBox->addWidget(remoteLabel);
    m_remoteComboBox = new QComboBox(destinationGroupBox);
    destinationHBox->addWidget(m_remoteComboBox);
    destinationHBox->addStretch();

    // Branches
    QGroupBox *branchesGroupBox = new QGroupBox(boxWidget);
    mainLayout->addWidget(branchesGroupBox);
    boxLayout->addWidget(branchesGroupBox);
    branchesGroupBox->setTitle(i18nc("@title:group", "Branches"));
    QHBoxLayout *branchesHBox = new QHBoxLayout(branchesGroupBox);
    branchesGroupBox->setLayout(branchesHBox);

    QLabel *localBranchLabel = new QLabel(i18nc("@label:listbox", "Local Branch:"), branchesGroupBox);
    branchesHBox->addWidget(localBranchLabel);
    m_localBranchComboBox = new QComboBox(branchesGroupBox);
    branchesHBox->addWidget(m_localBranchComboBox);

    branchesHBox->addStretch();

    QLabel *remoteBranchLabel = new QLabel(i18nc("@label:listbox", "Remote Branch:"), branchesGroupBox);
    branchesHBox->addWidget(remoteBranchLabel);
    m_remoteBranchComboBox = new QComboBox(branchesGroupBox);
    branchesHBox->addWidget(m_remoteBranchComboBox);

    QGroupBox *optionsGroupBox = new QGroupBox(boxWidget);
    mainLayout->addWidget(optionsGroupBox);
    boxLayout->addWidget(optionsGroupBox);
    optionsGroupBox->setTitle(i18nc("@title:group", "Options"));
    QHBoxLayout *optionsHBox = new QHBoxLayout(optionsGroupBox);
    optionsGroupBox->setLayout(optionsHBox);
    m_forceCheckBox = new QCheckBox(i18nc("@option:check", "Force with lease"), optionsGroupBox);
    m_forceCheckBox->setToolTip(
        i18nc("@info:tooltip",
              "Proceed even if the remote branch is not an ancestor of the local branch provided there were no changes in the remote branch since the last fetch."));
    optionsHBox->addWidget(m_forceCheckBox);

    m_noRemoteMessage = new KMessageWidget(i18nc("@info no remote code repositories", "No remotes defined."), boxWidget);
    m_noRemoteMessage->setMessageType(KMessageWidget::Warning);
    m_noRemoteMessage->setCloseButtonVisible(false);
    mainLayout->addWidget(m_noRemoteMessage);
    boxLayout->addWidget(m_noRemoteMessage);

    mainLayout->addWidget(m_buttonBox);

    // populate UI
    GitWrapper *gitWrapper = GitWrapper::instance();

    // get destinations
    QStringList remotes = gitWrapper->pushRemotes();
    m_remoteComboBox->addItems(remotes);
    m_noRemoteMessage->setVisible(remotes.isEmpty());

    // get branch names
    int currentBranchIndex;
    const QStringList branches = gitWrapper->branches(&currentBranchIndex);

    for (const QString &branch : branches) {
        if (branch.startsWith(QLatin1String("remotes/"))) {
            const QString remote = branch.section(QLatin1Char('/'), 1, 1);
            const QString name = branch.section(QLatin1Char('/'), 2);
            m_remoteBranches[remote] << name;
        } else {
            m_localBranchComboBox->addItem(branch);
            const QString remote = m_remoteComboBox->currentText();
            if (!branches.contains(QString::fromUtf8(QLatin1String("remotes/")) + remote + QLatin1String("/") + branch)) {
                m_remoteBranches[remote] << branch;
            }
        }
    }
    if (currentBranchIndex >= 0) {
        m_localBranchComboBox->setCurrentText(branches.at(currentBranchIndex));
    }
    remoteSelectionChanged(m_remoteComboBox->currentText());

    // Signals
    connect(m_remoteComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(remoteSelectionChanged(QString)));
    connect(m_localBranchComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(localBranchSelectionChanged(QString)));
    connect(m_remoteBranchComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(remoteBranchSelectionChanged(QString)));
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

bool PushDialog::forceWithLease() const
{
    return m_forceCheckBox->isChecked();
}

void PushDialog::remoteSelectionChanged(const QString &newRemote)
{
    m_remoteBranchComboBox->clear();
    m_remoteBranchComboBox->addItems(m_remoteBranches.value(newRemote));
    localBranchSelectionChanged(m_localBranchComboBox->currentText());
}

void PushDialog::remoteBranchSelectionChanged(const QString &newRemoteBranch) {
    const bool remoteBranchExists = m_remoteBranches[destination()].contains(newRemoteBranch);
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    if (remoteBranchExists) {
        okButton->setText(i18nc("@action:button", "Push"));
    } else {
        okButton->setText(i18nc("@action:button", "Push New Branch"));
    }
}

void PushDialog::localBranchSelectionChanged(const QString &newLocalBranch)
{
    // select matching remote branch if possible
    const int index = m_remoteBranchComboBox->findText(newLocalBranch);
    if (index != -1) {
        m_remoteBranchComboBox->setCurrentIndex(index);
        remoteBranchSelectionChanged(remoteBranch());
    }
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(m_remoteComboBox->count() > 0 && m_remoteBranchComboBox->count() > 0);
}

#include "moc_pushdialog.cpp"
