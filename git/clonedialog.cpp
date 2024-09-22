/*
    SPDX-FileCopyrightText: 2024 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "clonedialog.h"

#include "gitwrapper.h"

#include <KLocalizedString>

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFontMetrics>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <QtConcurrent>

CloneDialog::CloneDialog(const QString &contextDir, QWidget *parent)
    : QDialog(parent, Qt::Dialog)
{
    static const auto minWidth = fontMetrics().horizontalAdvance(QStringLiteral("https://invent.kde.org/sdk/dolphin-plugins.git"));

    m_contextDir = contextDir;

    m_url = new QLineEdit;
    m_url->setPlaceholderText(i18nc("a placeholder", "a git repository url or path"));
    m_dir = new QLineEdit;
    m_url->setMinimumWidth(minWidth * 1.3);

    m_branch = new QComboBox;
    m_branch->setEditable(true);
    m_branch->lineEdit()->setPlaceholderText(QStringLiteral("HEAD"));
    m_branch->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
    m_recursive = new QCheckBox();
    m_recursive->setChecked(true);
    m_recursive->setToolTip(i18nc("@info:tooltip submodules as in git submodules", "Recursively clone submodules"));

    auto layout = new QFormLayout();
    layout->addRow(new QLabel(i18nc("@label:textbox", "Url:")), m_url);
    layout->addRow(new QLabel(i18nc("@label:textbox", "Directory:")), m_dir);

    layout->addRow(new QLabel(i18nc("@label:textbox", "Branch:")), m_branch);
    layout->addRow(new QLabel(i18nc("@label:textbox", "Recursive")), m_recursive);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    m_okButton->setShortcut(Qt::Key_Return);
    m_okButton->setText(i18nc("@action:button", "Clone"));
    m_okButton->setEnabled(false);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);

    /*
     * Add actions, establish connections.
     */
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
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
    auto input = QApplication::clipboard()->text().trimmed();
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
    if (input.isEmpty()) {
        m_okButton->setEnabled(false);
        m_branch->clear();
        return;
    }

    // allow to work with "git clone ..." command
    if (input.startsWith(QStringLiteral("git clone "))) {
        input = input.mid(10);
        m_url->setText(input);
        return;
    }

    m_repositoryName = extractRepositoryName(input);
    if (m_repositoryName.isEmpty()) {
        m_okButton->setEnabled(false);
        m_branch->clear();
        return;
    }

    m_branch->clear();
    QtConcurrent::run(&GitWrapper::remoteBranches, GitWrapper::instance(), input).then([this, input](QStringList ret) {
        // A protection against inserting branches from different URL if user change remote.
        if (input == m_url->text()) {
            const auto text = m_branch->currentText();
            m_branch->clearEditText();
            m_branch->addItems(ret);
            if (!text.isEmpty()) {
                // Preserve current input if any.
                m_branch->setCurrentText(text);
                m_branch->setEditText(text);
            }
        }
    });

    QString destPath = QDir(m_contextDir).filePath(m_repositoryName);
    if (m_dir->text().isEmpty() || m_dir->text() == m_contextDir || m_dir->text() == destPath.chopped(1)) {
        m_dir->setText(destPath);
        return;
    }

    m_okButton->setEnabled(!QFile::exists(m_dir->text()));
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

    m_okButton->setEnabled(!m_repositoryName.isEmpty() && destinationValid);
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
    return m_branch->currentText();
}

bool CloneDialog::recursive() const
{
    return m_recursive->checkState() == Qt::Checked;
}

#include "moc_clonedialog.cpp"
