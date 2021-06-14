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
    explicit PullDialog(QWidget* parent = nullptr);
    QString source() const;
    QString remoteBranch() const;
private:
    QDialogButtonBox *m_buttonBox;
    QComboBox * m_remoteComboBox;
    QComboBox * m_remoteBranchComboBox;
    QHash<QString, QStringList> m_remoteBranches;
private Q_SLOTS:
    void remoteSelectionChanged(const QString& newRemote);
};

#endif // PULLDIALOG_H
