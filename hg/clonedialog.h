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

#ifndef HGCLONEDILAOG_H
#define HGCLONEDILAOG_H

#include <QtCore/QString>
#include <QtCore/QProcess>
#include <kdialog.h>

class KLineEdit;
class KPushButton;
class KTextEdit;
class QStackedLayout;
class QCheckBox;

//TODO: Enable to enter username/passwords if not found in config as well
//  as override within dialog

/**
 * Implements dialog to clone repository.
 */
class HgCloneDialog : public KDialog
{
    Q_OBJECT

public:
    HgCloneDialog(const QString &directory, QWidget *parent = 0);
    void setWorkingDirectory(const QString &directory);

private slots:
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
    void done(int r);
    void browseDirectory(KLineEdit *dest);
    void appendOptionArguments(QStringList &args);

private:
    KLineEdit *m_source;
    KLineEdit *m_destination;
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

#endif // HGCLONEDILAOG_H

