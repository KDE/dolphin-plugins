/*
    SPDX-FileCopyrightText: 2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SVNCHECKOUTDIALOG_H
#define SVNCHECKOUTDIALOG_H

#include <QDialog>

#include "ui_svncheckoutdialog.h"

class SvnCheckoutDialog : public QDialog
{
    Q_OBJECT
public:
    SvnCheckoutDialog(const QString &contextDir, QWidget *parent = nullptr);
    virtual ~SvnCheckoutDialog() override;

public Q_SLOTS:
    void on_leRepository_textChanged(const QString &text);
    void on_pbOk_clicked();

Q_SIGNALS:
    void infoMessage(const QString &msg);
    void errorMessage(const QString &msg);
    void operationCompletedMessage(const QString &msg);

private:
    Ui::SvnCheckoutDialog m_ui;
    QString m_dir;
};

#endif // SVNCHECKOUTDIALOG_H
