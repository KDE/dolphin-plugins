/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGPULLDIALOG_H
#define HGPULLDIALOG_H

#include "syncdialogbase.h"

class QCheckBox;
class QTableWidget;
class KTextEdit;
class QString;

/**
 * Dialog to implement pull operation
 */
class HgPullDialog : public HgSyncBaseDialog
{
    Q_OBJECT

public:
    explicit HgPullDialog(QWidget *parent = nullptr);

private:
    void setOptions() override;
    void parseUpdateChanges(const QString &input) override;
    void appendOptionArguments(QStringList &args) override;
    void createChangesGroup() override;
    void getHgChangesArguments(QStringList &args) override;
    void noChangesMessage() override;

private Q_SLOTS:
    void slotUpdateChangesGeometry();
    void readBigSize() override;
    void writeBigSize() override;

private:
    // Options
    QCheckBox *m_optUpdate;
    QCheckBox *m_optInsecure;
    QCheckBox *m_optForce;
    QGroupBox *m_optionGroup;

    // incoming Changes
    QTableWidget *m_changesList;
};

#endif // HGPULLDIALOG_H

