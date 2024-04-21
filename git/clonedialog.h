/*
    SPDX-FileCopyrightText: 2024 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLONEDIALOG_H
#define CLONEDIALOG_H

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QDialogButtonBox;

/**
 * @brief The dialog for clone repository in Git.
 */
class CloneDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CloneDialog(const QString &contextDir, QWidget *parent = nullptr);

    QString url() const;
    QString directory() const;
    QString branch() const;
    uint depth() const;
    bool recursive() const;
    bool noCheckout() const;
    bool bare() const;

private:
    QLineEdit *m_branch;
    QCheckBox *m_branchCheck;
    QLineEdit *m_depth;
    QCheckBox *m_depthCheck;
    QLineEdit *m_url;
    QLineEdit *m_dir;
    QDialogButtonBox *m_buttonBox;
    QCheckBox *m_recursive;
    QCheckBox *m_noCheckout;
    QCheckBox *m_bare;
};

#endif // CLONEDIALOG_H
