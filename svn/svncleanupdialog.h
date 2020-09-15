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
