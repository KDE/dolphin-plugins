/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HG_SERVE_DIALOG
#define HG_SERVE_DIALOG

#include "dialogbase.h"

class QSpinBox;
class QTextEdit;
class QLabel;
class HgServeWrapper;

/**
 * Implements dialog to Start and Stop Mercurial web server.
 * Several server instances can be handled.
 */
class HgServeDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgServeDialog(QWidget *parent = nullptr);
    void setupUI();
    void loadConfig();

public Q_SLOTS:
    void slotStart();
    void slotStop();
    void slotBrowse(); // opens system defined browser with localhost:m_portNumber->value()

private Q_SLOTS:
    void slotUpdateButtons();
    void slotServerError();
    void saveGeometry();

    /**
     * Append stdout and stderr to m_logEdit
     */
    void appendServerOutput(const QString &repoLocation, const QString &line);

private:
    QSpinBox          *m_portNumber;
    QPushButton       *m_startButton;
    QPushButton       *m_stopButton;
    QPushButton       *m_browseButton;
    QTextEdit         *m_logEdit;
    QLabel            *m_repoPathLabel;

    HgServeWrapper    *m_serverWrapper;
};

#endif /* HG_SERVE_DIALOG */

