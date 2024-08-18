/*
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHECKOUTDIALOG_H
#define CHECKOUTDIALOG_H

#include <QDialog>
#include <QSet>
#include <QString>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QGroupBox;
class QLineEdit;
class QRadioButton;

/**
 * @brief The dialog for checking out Branches or Tags in Git.
 */
class CheckoutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckoutDialog(QWidget *parent = nullptr);
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

private Q_SLOTS:
    void radioButtonToggled(QWidget *acompanyWidget, const QString &baseBranchName, bool checked);
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
    void setDefaultNewBranchName(const QString &baseBranchName);

private:
    inline void setLineEditErrorModeActive(bool active);

private:
    ///@brief true if the user has manually edited the branchName, false otherwise
    bool m_userEditedNewBranchName;
    int m_shortIdLength;
    QSet<QString> m_branchNames;
    QPalette m_errorColors;
    QDialogButtonBox *m_buttonBox;
    QGroupBox *m_branchSelectGroupBox;
    QRadioButton *m_branchRadioButton;
    QComboBox *m_branchComboBox;
    QRadioButton *m_tagRadioButton;
    QComboBox *m_tagComboBox;
    QRadioButton *m_commitRadioButton;
    QLineEdit *m_commitLineEdit;
    QCheckBox *m_newBranchCheckBox;
    QLineEdit *m_newBranchName;
    QCheckBox *m_forceCheckBox;
};

#endif // CHECKOUTDIALOG_H
