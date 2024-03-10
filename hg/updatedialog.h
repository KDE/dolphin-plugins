/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGUPDATEDIALOG_H
#define HGUPDATEDIALOG_H

#include "dialogbase.h"
#include <QString>

class KComboBox;
class QLabel;
class QCheckBox;

/**
 * Dialog to update working directory to specific revision/changeset/branch/tag.
 * Also shows working directory summary.
 */
class HgUpdateDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgUpdateDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void slotUpdateDialog(int index);

private:
    void done(int r) override;

private:
    enum { ToBranch, ToTag, ToRevision } m_updateTo;
    KComboBox *m_selectType;
    KComboBox *m_selectFinal;
    QLabel *m_currentInfo;
    QStringList m_selectList;
    QCheckBox *m_discardChanges;
};

#endif // HGUPDATEDIALOG_H
