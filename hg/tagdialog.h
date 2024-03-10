/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGTAGDIALOG_H
#define HGTAGDIALOG_H

#include "dialogbase.h"
#include <QStringList>

class KComboBox;

/**
 * Dialog to create/delete/list tags and update working directory to revision
 * represented by a specific tag.
 */
class HgTagDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgTagDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void slotUpdateDialog(const QString &text);
    void slotCreateTag();
    void slotSwitch();
    void slotRemoveTag();

private:
    void updateInitialDialog();

private:
    KComboBox *m_tagComboBox;
    QPushButton *m_createTag;
    QPushButton *m_updateTag;
    QPushButton *m_removeTag;
    QStringList m_tagList;
};

#endif // HGTAGDIALOG_H
