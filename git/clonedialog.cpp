/*
    SPDX-FileCopyrightText: 2024 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "clonedialog.h"

#include <KLocalizedString>

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontMetrics>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include <limits>

CloneDialog::CloneDialog(const QString &contextDir, QWidget *parent)
    : QDialog(parent, Qt::Dialog)
{
    static const auto minWidth = fontMetrics().horizontalAdvance(QStringLiteral("123456789_text"));

    m_url = new QLineEdit;
    m_dir = new QLineEdit;
    m_url->setMinimumWidth(minWidth * 2);
    m_dir->setMinimumWidth(minWidth * 2);

    m_depthCheck = new QCheckBox(i18nc("@option:check", "Depth:"));
    m_depth = new QLineEdit;
    m_depth->setMinimumWidth(minWidth);
    m_depth->setMaximumWidth(minWidth);
    m_depth->setEnabled(false);
    m_depth->setValidator(new QIntValidator(1, std::numeric_limits<int>::max()));

    m_branchCheck = new QCheckBox(i18nc("@option:check", "Branch:"));
    m_branch = new QLineEdit;
    m_branch->setMinimumWidth(minWidth);
    m_branch->setMaximumWidth(minWidth);
    m_branch->setEnabled(false);

    m_recursive = new QCheckBox(i18nc("@option:check", "Recursive"));
    m_noCheckout = new QCheckBox(i18nc("@option:check", "No Checkout"));
    m_bare = new QCheckBox(i18nc("@option:check", "Clone Into Bare Repo"));

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(new QLabel(i18nc("@label:textbox", "URL:")), 0, 0);
    gridLayout->addWidget(m_url, 0, 1, 1, -1);
    gridLayout->addWidget(new QLabel(i18nc("@label:textbox", "Directory:")), 1, 0);
    gridLayout->addWidget(m_dir, 1, 1, 1, -1);
    gridLayout->addWidget(m_recursive, 2, 0);
    gridLayout->addWidget(m_depthCheck, 3, 0);
    gridLayout->addWidget(m_depth, 3, 1);
    gridLayout->addWidget(m_noCheckout, 3, 2);
    gridLayout->addWidget(m_branchCheck, 4, 0);
    gridLayout->addWidget(m_branch, 4, 1);
    gridLayout->addWidget(m_bare, 4, 2);
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(2, 1);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setText(i18nc("@action:button", "Clone"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);

    /*
     * Add actions, establish connections.
     */
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_depthCheck, &QCheckBox::clicked, this, [this](bool enabled) {
        m_depth->setEnabled(enabled);
        if (enabled) {
            m_depth->setFocus();
        }
    });
    connect(m_branchCheck, &QCheckBox::clicked, this, [this](bool enabled) {
        m_branch->setEnabled(enabled);
        if (enabled) {
            m_branch->setFocus();
        }
    });
    QAction *pickDirectory = m_dir->addAction(QIcon::fromTheme(QStringLiteral("folder")), QLineEdit::TrailingPosition);
    connect(pickDirectory, &QAction::triggered, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(this,
                                                              i18nc("@title:window", "Choose a clone directory"),
                                                              QString(),
                                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!dir.isEmpty()) {
            m_dir->setText(dir);
        }
    });

    /*
     * Additional setup.
     */
    auto input = QApplication::clipboard()->text();
    // Some websites have a 'git clone ...' for copy to clipboard. Let's remove this to have URL.
    if (input.startsWith(QStringLiteral("git clone "))) {
        input = input.sliced(10);
    }
    const auto repoPath = QUrl::fromUserInput(input, {}, QUrl::AssumeLocalFile);
    if (repoPath.isValid()) {
        m_url->setText(repoPath.url());
        auto name = repoPath.fileName();
        if (name.endsWith(QStringLiteral(".git"))) {
            name.chop(4);
        }
        m_dir->setText(contextDir + QDir::separator() + name);
    } else {
        m_dir->setText(contextDir);
    }

    setWindowTitle(xi18nc("@title:window", "<application>Git</application> Clone"));
}

QString CloneDialog::url() const
{
    return m_url->text();
}

QString CloneDialog::directory() const
{
    return m_dir->text();
}

QString CloneDialog::branch() const
{
    if (m_branchCheck->checkState() == Qt::Checked) {
        return m_branch->text();
    } else {
        return {};
    }
}

uint CloneDialog::depth() const
{
    if (m_depthCheck->checkState() == Qt::Checked) {
        return m_depth->text().toInt();
    } else {
        return 0;
    }
}

bool CloneDialog::recursive() const
{
    return m_recursive->checkState() == Qt::Checked;
}

bool CloneDialog::noCheckout() const
{
    return m_noCheckout->checkState() == Qt::Checked;
}

bool CloneDialog::bare() const
{
    return m_bare->checkState() == Qt::Checked;
}

#include "moc_clonedialog.cpp"
