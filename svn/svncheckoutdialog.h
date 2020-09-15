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

#ifndef SVNCHECKOUTDIALOG_H
#define SVNCHECKOUTDIALOG_H

#include <QDialog>

#include "ui_svncheckoutdialog.h"

class SvnCheckoutDialog : public QDialog {
    Q_OBJECT
public:
    SvnCheckoutDialog(const QString& contextDir, QWidget *parent = nullptr);
    virtual ~SvnCheckoutDialog() override;

public Q_SLOTS:
    void on_leRepository_textChanged(const QString &text);
    void on_pbOk_clicked();

Q_SIGNALS:
    void infoMessage(const QString& msg);
    void errorMessage(const QString& msg);
    void operationCompletedMessage(const QString& msg);

private:
    Ui::SvnCheckoutDialog m_ui;
    QString m_dir;
};

#endif  // SVNCHECKOUTDIALOG_H
