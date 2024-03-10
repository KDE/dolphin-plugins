/*
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PULLDIALOG_H
#define PULLDIALOG_H

#include <QDialog>
#include <QHash>
class QComboBox;
class QDialogButtonBox;

class PullDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PullDialog(QWidget *parent = nullptr);
    QString source() const;
    QString remoteBranch() const;

private:
    QDialogButtonBox *m_buttonBox;
    QComboBox *m_remoteComboBox;
    QComboBox *m_remoteBranchComboBox;
    QHash<QString, QStringList> m_remoteBranches;
private Q_SLOTS:
    void remoteSelectionChanged(const QString &newRemote);
};

#endif // PULLDIALOG_H
