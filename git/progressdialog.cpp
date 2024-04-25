/*
    SPDX-FileCopyrightText: 2024 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "progressdialog.h"

#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

ProgressDialog::ProgressDialog(QProcess *process, QWidget *parent)
    : QDialog(parent, Qt::Dialog)
{
    m_text = new QPlainTextEdit;

    QVBoxLayout *layout = new QVBoxLayout;
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(m_text);
    layout->addWidget(buttonBox);
    setLayout(layout);

    /*
     * Add actions, establish connections.
     */
    connect(buttonBox, &QDialogButtonBox::rejected, process, &QProcess::terminate);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(process, &QProcess::finished, process, [this, buttonBox](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == EXIT_SUCCESS && exitStatus == QProcess::ExitStatus::NormalExit) {
            close();
        }
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
    });
    // git commands outputs only to stderr but we connect stdout anyway.
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        const auto input = QString::fromLocal8Bit(process->readAllStandardOutput());
        appendText(input);
    });
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        const auto input = QString::fromLocal8Bit(process->readAllStandardError());
        appendText(input);
    });

    /*
     * Additional setup.
     */
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(sizeHint() + QSize{200, 0});
}

void ProgressDialog::appendText(const QString &text)
{
    const auto list = text.split(QLatin1Char('\r'), Qt::SkipEmptyParts);
    m_text->moveCursor(QTextCursor::End);
    for (auto &i : std::as_const(list)) {
        m_text->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        m_text->textCursor().removeSelectedText();
        m_text->insertPlainText(i);
    }
}

#include "moc_progressdialog.cpp"
