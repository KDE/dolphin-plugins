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
#include <QDir>
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

    m_contextDir = contextDir;

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
    gridLayout->addWidget(new QLabel(i18nc("@label:textbox", "Url:")), 0, 0);
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
    m_okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    m_okButton->setShortcut(Qt::Key_Return);
    m_okButton->setText(i18nc("@action:button", "Clone"));
    m_okButton->setEnabled(false);

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
                                                              m_contextDir,
                                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty()) {
            m_dir->setText(dir);
        }
    });

    m_dir->setText(m_contextDir);
    connect(m_url, &QLineEdit::textChanged, this, &CloneDialog::urlChanged);
    connect(m_dir, &QLineEdit::textChanged, this, &CloneDialog::destinationDirChanged);

    /*
     * Additional setup.
     */
    loadFromClipboard();

    setWindowTitle(xi18nc("@title:window", "<application>Git</application> Clone"));
}

void CloneDialog::loadFromClipboard()
{
    auto input = QApplication::clipboard()->text();
    if (!input.isEmpty()
        && (input.startsWith(QStringLiteral("http")) || input.startsWith(QStringLiteral("git@")) || input.startsWith(QStringLiteral("git:"))
            || input.startsWith(QStringLiteral("git clone ")) || QDir(m_contextDir, input).exists())) {
        // default input from clipboard
        m_url->setText(input);
        if (m_okButton->isEnabled()) {
            m_okButton->setFocus();
        } else {
            // clear url
            m_url->setText(QString{});
            m_url->setFocus();
        }
    } else {
        m_url->setFocus();
    }
}

QString CloneDialog::extractRepositoryName(const QString &_input)
{
    auto input = _input;
    QString repositoryName;
    const auto repoUrl = QUrl::fromUserInput(input, m_contextDir).adjusted(QUrl::StripTrailingSlash);
    if (repoUrl.isValid() && (!repoUrl.scheme().startsWith(QStringLiteral("http")) || repoUrl.host().contains(QLatin1Char('.')))) {
        // https://invent.kde.org/graphics/okular.git
        // ../okular.git
        // kde/src/okular
        repositoryName = repoUrl.fileName();
        if (repoUrl.isLocalFile()) {
            const auto repositoryPath = repoUrl.toLocalFile();
            const auto info = QFileInfo(repositoryPath);
            if (!info.exists() || !info.isDir()) {
                repositoryName.clear();
            } else {
                const auto dirItem = QDir(repositoryPath);
                const auto hasDotGitDir = QDir(dirItem.filePath(QStringLiteral(".git"))).exists();
                // for bare repositories
                const auto hasHeadFile = QFile::exists(dirItem.filePath(QStringLiteral("HEAD")));
                if (!hasDotGitDir && !hasHeadFile) {
                    // in case local repo is not a valid dir
                    repositoryName.clear();
                }
            }
        }
    } else if (input.startsWith(QStringLiteral("git@")) || input.startsWith(QStringLiteral("git://"))) {
        // git@invent.kde.org:graphics/okular.git
        // git://example.com/git.git/
        if (input.endsWith(QLatin1Char('/'))) {
            input.chop(1);
        }
        int from = input.lastIndexOf(QLatin1Char('/'));
        repositoryName = input.mid(from == -1 ? 0 : (from + 1));
    }

    if (!repositoryName.isEmpty() && repositoryName.endsWith(QStringLiteral(".git"))) {
        repositoryName.chop(4);
    }

    return repositoryName;
}

void CloneDialog::urlChanged()
{
    auto input = m_url->text();
    // allow to work with "git clone ..." command
    if (input.startsWith(QStringLiteral("git clone "))) {
        input = input.mid(10);
        m_url->setText(input);
        return;
    }
    QString name = extractRepositoryName(input);

    m_okButton->setEnabled(!name.isEmpty());

    if (!name.isEmpty()) {
        QString destPath = QDir(m_contextDir).filePath(name);

        if (m_dir->text().isEmpty() || m_dir->text() == m_contextDir || m_dir->text() == destPath.chopped(1)) {
            m_dir->setText(destPath);
            return;
        }

        m_okButton->setEnabled(!QFile::exists(m_dir->text()));
    }
}

void CloneDialog::destinationDirChanged()
{
    bool destinationValid = false;
    auto destItem = QFileInfo(m_dir->text());
    if (destItem.exists()) {
        if (destItem.isDir()) {
            auto destDir = QDir(m_dir->text());
            // an empty directory
            destinationValid = destDir.isEmpty(QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::Hidden);
        }
    } else {
        destinationValid = true;
    }

    const QString name = extractRepositoryName(m_url->text());
    m_okButton->setEnabled(!name.isEmpty() && destinationValid);
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
