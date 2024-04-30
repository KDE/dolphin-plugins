/*
    SPDX-FileCopyrightText: 2019-2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "svncheckoutdialog.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileDialog>
#include <QUrl>

namespace
{

// Helper function: removes trailing slashes.
QString rstrip(const QString &str)
{
    for (int i = str.size() - 1; i >= 0; --i) {
        if (str.at(i) != QLatin1Char('/')) {
            return str.left(i + 1);
        }
    }

    return {};
}

// Helper function: check if path is a valid svn repository URL.
// Information about URL prefix at http://svnbook.red-bean.com/en/1.2/svn-book.html#svn.basic.in-action.wc.tbl-1.
bool isValidSvnRepoUrl(const QString &path)
{
    static const QStringList schemes = {
        QStringLiteral("file"),
        QStringLiteral("http"),
        QStringLiteral("https"),
        QStringLiteral("svn"),
        QStringLiteral("svn+ssh"),
    };

    const QUrl url = QUrl::fromUserInput(path);

    return url.isValid() && schemes.contains(url.scheme());
}

}

SvnCheckoutDialog::SvnCheckoutDialog(const QString &contextDir, QWidget *parent)
    : QDialog(parent)
    , m_dir(contextDir)
{
    m_ui.setupUi(this);

    /*
     * Add actions, establish connections.
     */
    connect(m_ui.pbOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_ui.pbCancel, &QPushButton::clicked, this, &QDialog::reject);
    QAction *pickDirectory = m_ui.leCheckoutDir->addAction(QIcon::fromTheme(QStringLiteral("folder")), QLineEdit::TrailingPosition);
    connect(pickDirectory, &QAction::triggered, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(this,
                                                              i18nc("@title:window", "Choose a directory to checkout"),
                                                              QString(),
                                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!dir.isEmpty()) {
            m_ui.leCheckoutDir->setText(dir);
        }
    });

    /*
     * Additional setup.
     */
    const QString repoPath = QApplication::clipboard()->text();
    if (isValidSvnRepoUrl(repoPath)) {
        m_ui.leRepository->setText(repoPath);
    } else {
        m_ui.leCheckoutDir->setText(m_dir);
    }
}

SvnCheckoutDialog::~SvnCheckoutDialog() = default;

QString SvnCheckoutDialog::url() const
{
    return m_ui.leRepository->text();
}

QString SvnCheckoutDialog::directory() const
{
    return m_ui.leCheckoutDir->text();
}

bool SvnCheckoutDialog::omitExternals() const
{
    return m_ui.cbOmitExternals->isChecked();
}

void SvnCheckoutDialog::on_leRepository_textChanged(const QString &text)
{
    if (isValidSvnRepoUrl(text)) {
        const QString stripped = rstrip(text);
        // If URL ends with a 'trunk' this is a branch - lets consider upper folder name as an
        // extraction path. So for '.../SomeRepo/trunk/' result would be 'SomeRepo'.
        int astart = -1;
        if (stripped.endsWith(QLatin1String("trunk"))) {
            astart = -2;
        }
        const QString suffix = QDir::separator() + stripped.section(QLatin1Char('/'), astart, astart);

        m_ui.leCheckoutDir->setText(m_dir + suffix);
        m_ui.pbOk->setEnabled(true);
    } else {
        m_ui.pbOk->setEnabled(false);
    }
}

#include "moc_svncheckoutdialog.cpp"
