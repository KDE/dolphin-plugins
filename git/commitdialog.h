/*
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMMITDIALOG_H
#define COMMITDIALOG_H

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class KTextEdit;

class CommitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommitDialog(QWidget* parent = nullptr);
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
private Q_SLOTS:
    void signOffButtonClicked();
    void amendCheckBoxStateChanged();
    void saveDialogSize();
    void setOkButtonState();
private:
    KTextEdit* m_commitMessageTextEdit;
    QCheckBox* m_amendCheckBox;
    QDialogButtonBox *m_buttonBox;
    /**
     * @brief Holds an alternative message, that is not displayed currently.
     * One message is the amend message, the other one for normal commits.
     */
    QString m_alternativeMessage;
    ///Cache for GitWrapper::userName();
    QString m_userName;
    ///Cache for GitWrapper::userEmail();
    QString m_userEmail;
};

#endif // COMMITDIALOG_H
