/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGPUSHDIALOG_H
#define HGPUSHDIALOG_H

#include "hgwrapper.h"
#include "syncdialogbase.h"

class QCheckBox;
class QTableWidget;
class KTextEdit;
class QGroupBox;

/**
 * Dialog to implement Push operation
 */
class HgPushDialog : public HgSyncBaseDialog 
{
    Q_OBJECT

public:
    explicit HgPushDialog(QWidget *parent = nullptr);

private:
    void setOptions() override;
    void createChangesGroup() override;
    void parseUpdateChanges(const QString &input) override;
    void appendOptionArguments(QStringList &args) override;
    void getHgChangesArguments(QStringList &args) override;
    void noChangesMessage() override;

private Q_SLOTS:
    void slotOutSelChanged();
    void slotUpdateChangesGeometry();
    void readBigSize() override;
    void writeBigSize() override;

private:
    // Options
    QCheckBox *m_optAllowNewBranch;
    QCheckBox *m_optInsecure;
    QCheckBox *m_optForce;
    QGroupBox *m_optionGroup;

    // outgoing Changes
    QTableWidget *m_outChangesList;
    KTextEdit *m_changesetInfo;
};

#endif // HGPUSHDIALOG_H

