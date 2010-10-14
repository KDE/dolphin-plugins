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

#ifndef CHECKOUTDIALOG_H
#define CHECKOUTDIALOG_H

#include <kdialog.h>
#include <QSet>
#include <QString>

class KComboBox;
class KLineEdit;
class QCheckBox;
class QGroupBox;
class QRadioButton;

/**
 * @brief The dialog for checking out Branches or Tags in Git.
 */
class CheckoutDialog : public KDialog
{
    Q_OBJECT

public:
    CheckoutDialog(QWidget* parent = 0);
    /**
     * Returns the name of the selected tag or branch to be checkout out
     * @returns The name of the selected tag or branch
     */
    QString checkoutIdentifier() const;
    /**
     * @returns True if the user selected a forced checkout, false otherwise.
     */
    bool force() const;
    /**
     * @returns The user selected name of the new branch, if a new branch is to be
     *          created, empty String otherwise.
     */
    QString newBranchName() const;

private slots:
    void branchRadioButtonToggled(bool checked);
    void newBranchCheckBoxStateToggled(int state);
   /**
    * Checks whether the values of all relevant widgets are valid.
    * Enables or disables the OK button and sets tooltips accordingly.
    */
    void setOkButtonState();
    void noteUserEditedNewBranchName();
    /**
     * Inserts a default name for the new branch into m_newBranchName unless the user
     * has already edited the content.
     * @param baseBranchName The base name to derive the new name of.
     */
    void setDefaultNewBranchName(const QString & baseBranchName);
private:
    inline void setLineEditErrorModeActive(bool active);
private:
    ///@brief true if the user has manually edited the branchName, false otherwise
    bool m_userEditedNewBranchName;
    QSet<QString> m_branchNames;
    QPalette m_errorColors;
    QGroupBox * m_branchSelectGroupBox;
    QRadioButton * m_branchRadioButton;
    KComboBox * m_branchComboBox;
    KComboBox * m_tagComboBox;
    QCheckBox * m_newBranchCheckBox;
    KLineEdit * m_newBranchName;
    QCheckBox * m_forceCheckBox;
};

#endif // CHECKOUTDIALOG_H
