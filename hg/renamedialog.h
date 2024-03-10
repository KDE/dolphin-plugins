/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGRENAMEDIALOG_H
#define HGRENAMEDIALOG_H

#include "dialogbase.h"
#include <QString>

class QLineEdit;
class KFileItem;
class QPushButton;

/**
 * Dialog to rename files Mercurial way
 */
class HgRenameDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgRenameDialog(const KFileItem &source, QWidget *parent = nullptr);
    QString source() const;
    QString destination() const;
    void done(int r) override;

private Q_SLOTS:
    void slotTextChanged(const QString &text);

private:
    QString m_source;
    QString m_source_dir;
    QLineEdit *m_destinationFile;
};

#endif // HGRENAMEDIALOG_H
