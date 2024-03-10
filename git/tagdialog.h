/*
    SPDX-FileCopyrightText: 2010 Johannes Steffen <johannes.steffen@st.ovgu.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TAGDIALOG_H
#define TAGDIALOG_H

#include <QDialog>
#include <QSet>

class KTextEdit;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QRadioButton;

class TagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TagDialog(QWidget *parent = nullptr);
    /**
     * Returns the tag message given by the user.
     * @returns The tag message.
     */
    QByteArray tagMessage() const;
    /**
     * Returns the tag name given by the user.
     * @return The tag name.
     */
    QString tagName() const;
    /**
     * @returns The name of the branch the tag should point to or HEAD if
                the tag should point to the current HEAD.
     */

    QString baseBranch() const;
private Q_SLOTS:
    void setOkButtonState();

private:
    inline void setLineEditErrorModeActive(bool active);

private:
    QSet<QString> m_tagNames;
    KTextEdit *m_tagMessageTextEdit;
    QLineEdit *m_tagNameTextEdit;
    QComboBox *m_branchComboBox;
    QDialogButtonBox *m_buttonBox;
    QRadioButton *branchRadioButton;
    QPalette m_errorColors;
};

#endif // TAGDIALOG_H
