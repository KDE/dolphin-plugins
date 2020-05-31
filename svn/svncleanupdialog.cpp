/***************************************************************************
 *   Copyright (C) 2020                                                    *
 *                  by Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>  *
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

#include "svncleanupdialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>

#include "svncommands.h"

SvnCleanupDialog::SvnCleanupDialog(const QString& workingDir, QWidget *parent) :
    QDialog(parent)
{
    m_ui.setupUi(this);

    /*
     * Add actions, establish connections.
     */
    connect(m_ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
    QAction *pickDirectory = m_ui.lineEditDirectory->addAction(QIcon::fromTheme("folder"), QLineEdit::TrailingPosition);
    connect(pickDirectory, &QAction::triggered, this, [this] () {
        const QString dir = QFileDialog::getExistingDirectory(this, i18nc("@title:window", "Choose a directory to clean up"),
                                                              m_ui.lineEditDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!dir.isEmpty()) {
            m_ui.lineEditDirectory->setText(dir);
        }
    } );

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
        emit operationCompletedMessage(i18nc("@info:status", "SVN clean up completed successfully."));
    } else {
        emit errorMessage(i18nc("@info:status", "SVN clean up failed for %1", workDir));
        qDebug() << result.stderr;
    }

    QDialog::accept();
}
