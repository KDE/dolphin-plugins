/*
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PUSHDIALOG_H
#define PUSHDIALOG_H

#include <QDialog>
#include <QHash>
class QCheckBox;
class QComboBox;
class QDialogButtonBox;

class PushDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PushDialog(QWidget* parent = nullptr);
    QString destination() const;
    QString localBranch() const;
    QString remoteBranch() const;
    bool force() const;
private Q_SLOTS:
    void remoteSelectionChanged(const QString& newRemote);
    void localBranchSelectionChanged(const QString& newLocalBranch);
private:
    QHash<QString, QStringList> m_remoteBranches;

    QComboBox * m_remoteComboBox;
    QComboBox * m_localBranchComboBox;
    QComboBox * m_remoteBranchComboBox;
    QCheckBox * m_forceCheckBox;
    QDialogButtonBox * m_buttonBox;
};

#endif // PUSHDIALOG_H
