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

#ifndef HGCLONEDIALOG_H
#define HGCLONEDIALOG_H

#include <QString>
#include <QProcess>
#include "dialogbase.h"

class QLineEdit;
class KPushButton;
class KTextEdit;
class QStackedLayout;
class QCheckBox;

//TODO: Enable to enter username/passwords if not found in config as well
//  as override within dialog

/**
 * Implements dialog to clone repository.
 */
class HgCloneDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgCloneDialog(const QString &directory, QWidget *parent = nullptr);
    void setWorkingDirectory(const QString &directory);

private Q_SLOTS:
    void saveGeometry();

    /**
     * Enables dialog's Ok button when user has entered some input in 
     * source LineEdit
     */
    void slotUpdateOkButton();
    void slotBrowseDestClicked();
    void slotBrowseSourceClicked();
    void slotCloningStarted();
    void slotCloningFinished(int exitCode, QProcess::ExitStatus);

    /** 
     * Show output of clone operation in TextEdit component.
     */
    void slotUpdateCloneOutput();

private:
    void done(int r) override;
    void browseDirectory(QLineEdit *dest);
    void appendOptionArguments(QStringList &args);

private:
    QLineEdit *m_source;
    QLineEdit *m_destination;
    KPushButton *m_browse_dest;
    KPushButton *m_browse_source;
    KTextEdit *m_outputEdit;
    QStackedLayout *m_stackLayout;

    bool m_cloned;
    bool m_terminated;
    QString m_workingDirectory;
    QProcess m_process;

    // option checkboxes
    QCheckBox *m_optNoUpdate;
    QCheckBox *m_optUsePull;
    QCheckBox *m_optUseUncmprdTrans;
    QCheckBox *m_optNoVerifyServCert;

};

#endif // HGCLONEDIALOG_H

