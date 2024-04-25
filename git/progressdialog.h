/*
    SPDX-FileCopyrightText: 2024 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

class QProcess;
class QPlainTextEdit;

/**
 * @brief Light-weight dialog for showing progress of git operations.
 *
 * Dialog connects to a process (in a constructor \p ProgressDialog()),
 * shows its stderr and stdout. If the process completes successfully,
 * the dialog auto closes.
 * If the dialog is rejected it terminates the process (for example, when
 * "Cancel" is clicked).
 *
 * @note git produce only stderr, dialog connects stdout anyway.
 */
class ProgressDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProgressDialog(QProcess *process, QWidget *parent = nullptr);

private:
    /**
     * Append a text with respect of carriage-return ('\r') symbol.
     */
    void appendText(const QString &text);

    QPlainTextEdit *m_text;
};

#endif // PROGRESSDIALOG_H
