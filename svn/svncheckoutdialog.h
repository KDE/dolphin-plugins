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

    QString url() const;
    QString directory() const;
    bool omitExternals() const;

public Q_SLOTS:
    void on_leRepository_textChanged(const QString &text);

private:
    Ui::SvnCheckoutDialog m_ui;
    QString m_dir;
};

#endif // SVNCHECKOUTDIALOG_H
