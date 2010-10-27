/******************************************************************************
 *   Copyright (C) 2010 by Johannes Steffen <johannes.steffen@st.ovgu.de>     *
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

#ifndef TAGDIALOG_H
#define TAGDIALOG_H

#include <kdialog.h>
#include <QSet>

class KTextEdit;
class KLineEdit;
class KComboBox;
class QTextCodec;
class QRadioButton;

class TagDialog : public KDialog
{
    Q_OBJECT

public:
    TagDialog(QWidget* parent = 0);
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
private slots:
    void setOkButtonState();
private:
    inline void setLineEditErrorModeActive(bool active);
private:
    QSet<QString> m_tagNames;
    KTextEdit* m_tagMessageTextEdit;
    KLineEdit* m_tagNameTextEdit;
    KComboBox* m_branchComboBox;
    QRadioButton* branchRadioButton;
    QTextCodec* m_localCodec;
    QPalette m_errorColors;
};

#endif // TAGDIALOG_H
