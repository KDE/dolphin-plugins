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
    bool recursive() const;

private Q_SLOTS:
    void urlChanged();
    void destinationDirChanged();
    void loadFromClipboard();

private:
    QString extractRepositoryName(const QString &input);

    QLineEdit *m_branch;
    QLineEdit *m_url;
    QLineEdit *m_dir;
    QDialogButtonBox *m_buttonBox;
    QCheckBox *m_recursive;
    QPushButton *m_okButton;

    QString m_contextDir;
    QString m_repositoryName;
};

#endif // CLONEDIALOG_H
