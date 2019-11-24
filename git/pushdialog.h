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
    explicit PushDialog(QWidget* parent = 0);
    QString destination() const;
    QString localBranch() const;
    QString remoteBranch() const;
    bool force() const;
private slots:
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
