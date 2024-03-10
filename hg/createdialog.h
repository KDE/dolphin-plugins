/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGCREATEDIALOG_H
#define HGCREATEDIALOG_H

#include "dialogbase.h"
#include <QString>

class QLineEdit;
class QLabel;

/**
 * Dialog to initialize new mercurial repository
 */
class HgCreateDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgCreateDialog(const QString &directory, QWidget *parent = nullptr);

private:
    void done(int r) override;
    void setWorkingDirectory(const QString &directory);

private:
    QString m_workingDirectory;
    QLineEdit *m_repoNameEdit;
    QLabel *m_directory;
};

#endif // HGCREATEDIALOG_H
