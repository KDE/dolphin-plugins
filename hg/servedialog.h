/***************************************************************************
 *   Copyright (C) 2011 by Vishesh Yadav <vishesh3y@gmail.com>             *
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

#ifndef HG_SERVE_DIALOG
#define HG_SERVE_DIALOG

#include <kdialog.h>

class QSpinBox;
class KPushButton;
class QTextEdit;
class QLabel;
class HgServeWrapper;

/**
 * Implements dialog to Start and Stop Mercurial web server.
 * Several server instances can be handled.
 */
class HgServeDialog : public KDialog
{
    Q_OBJECT

public:
    HgServeDialog(QWidget *parent = 0);
    void setupUI();
    void loadConfig();

public slots:
    void slotStart();
    void slotStop();

private slots:
    void slotUpdateButtons();
    void slotServerError();
    void saveGeometry();

    /**
     * Append stdout and stderr to m_logEdit
     */
    void appendServerOutput(const QString &repoLocation, const QString &line);

private:
    QSpinBox *m_portNumber; 
    KPushButton *m_startButton;
    KPushButton *m_stopButton;
    QTextEdit *m_logEdit;
    QLabel *m_repoPathLabel;

    HgServeWrapper *m_serverWrapper;
};

#endif /* HG_SERVE_DIALOG */

