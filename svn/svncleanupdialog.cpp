/*
    SPDX-FileCopyrightText: 2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "svncleanupdialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

#include "svncommands.h"

SvnCleanupDialog::SvnCleanupDialog(const QString &workingDir, QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    /*
     * Add actions, establish connections.
     */
    connect(m_ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
    QAction *pickDirectory = m_ui.lineEditDirectory->addAction(QIcon::fromTheme(QStringLiteral("folder")), QLineEdit::TrailingPosition);
    connect(pickDirectory, &QAction::triggered, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(this,
                                                              i18nc("@title:window", "Choose a directory to clean up"),
                                                              m_ui.lineEditDirectory->text(),
                                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!dir.isEmpty()) {
            m_ui.lineEditDirectory->setText(dir);
        }
    });

    /*
     * Additional setup.
     */
    m_ui.lineEditDirectory->setText(workingDir);

    setAttribute(Qt::WA_DeleteOnClose);
    show();
    activateWindow();
}

SvnCleanupDialog::~SvnCleanupDialog() = default;

void SvnCleanupDialog::on_lineEditDirectory_textChanged(const QString &text)
{
    m_ui.buttonOk->setEnabled(QFileInfo(text).isDir());
}

void SvnCleanupDialog::on_buttonOk_clicked()
{
    const QString workDir = m_ui.lineEditDirectory->text();
    const bool removeUnversioned = m_ui.checkBoxUnversioned->isChecked();
    const bool removeIgnored = m_ui.checkBoxIgnored->isChecked();
    const bool includeExternals = m_ui.checkBoxExternals->isChecked();

    const CommandResult result = SvnCommands::cleanup(workDir, removeUnversioned, removeIgnored, includeExternals);
    if (result.success) {
        Q_EMIT operationCompletedMessage(i18nc("@info:status", "SVN clean up completed successfully."));
    } else {
        Q_EMIT errorMessage(i18nc("@info:status", "SVN clean up failed for %1", workDir));
        qDebug() << result.stdErr;
    }

    QDialog::accept();
}

#include "moc_svncleanupdialog.cpp"
