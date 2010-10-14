/******************************************************************************
 *   Copyright (C) 2010 by Sebastian Doerner <sebastian@sebastian-doerner.de> *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program; if not, write to the                            *
 *   Free Software Foundation, Inc.,                                          *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA               *
 ******************************************************************************/

#ifndef COMMITDIALOG_H
#define COMMITDIALOG_H

#include <kdialog.h>

class QCheckBox;
class QTextCodec;
class KTextEdit;

class CommitDialog : public KDialog
{
    Q_OBJECT

public:
    CommitDialog(QWidget* parent = 0);
    /**
     * Returns the commit message given by the user.
     * @returns The commit message.
     */
    QByteArray commitMessage() const;
    /**
     * Indicated whether the user wants to amend the last commit.
     * @returns True if the last commit is to be amended, false otherwise
     */
    bool amend() const;
private slots:
    void signOffButtonClicked();
    void amendCheckBoxStateChanged();
    void saveDialogSize();
    void setOkButtonState();
private:
    KTextEdit* m_commitMessageTextEdit;
    QCheckBox* m_amendCheckBox;
    /**
     * @brief Holds an alternative message, that is not displayed currently.
     * One message is the amend message, the other one for normal commits.
     */
    QString m_alternativeMessage;
    QTextCodec* m_localCodec;
    ///Cache for GitWrapper::userName();
    QString m_userName;
    ///Cache for GitWrapper::userEmail();
    QString m_userEmail;
};

#endif // COMMITDIALOG_H
