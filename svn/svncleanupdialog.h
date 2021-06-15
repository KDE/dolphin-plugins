/*
    SPDX-FileCopyrightText: 2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SVNCLEANUPDIALOG_H
#define SVNCLEANUPDIALOG_H

#include <QDialog>

#include "ui_svncleanupdialog.h"

/**
 * \brief Dialog for SVN cleanup operation.
 *
 * \note Dialog sets up a Qt::WA_DeleteOnClose attribute so it's delete itself on close.
 */
class SvnCleanupDialog : public QDialog {
    Q_OBJECT
public:
    /**
     * \param[in] workingDir Directory to call 'svn cleanup' on.
     * \param[in,out] parent Parent widget.
     */
    SvnCleanupDialog(const QString& workingDir, QWidget *parent = nullptr);
    virtual ~SvnCleanupDialog() override;

public Q_SLOTS:
    void on_lineEditDirectory_textChanged(const QString &text);
    void on_buttonOk_clicked();

Q_SIGNALS:
    /**
     * Is emitted if an error occuers with a message \a msg.
     */
    void errorMessage(const QString& msg);

    /**
     * Is emitted for successful operation with a message \a msg.
     */
    void operationCompletedMessage(const QString& msg);

private:
    Ui::SvnCleanupDialog m_ui;
};

#endif  // SVNCLEANUPDIALOG_H
