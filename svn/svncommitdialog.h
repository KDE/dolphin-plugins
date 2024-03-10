/*
    SPDX-FileCopyrightText: 2019-2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SVNCOMMITDIALOG_H
#define SVNCOMMITDIALOG_H

#include <QDialog>
#include <QHash>

#include <Dolphin/KVersionControlPlugin>

class QPlainTextEdit;
class QTableWidget;

/**
 * \brief SVN Commit dialog class.
 */
class SvnCommitDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * \param versionInfo Pointer to a current full list of SVN plugin changed files. This pointer
     *        is saved internaly and used for changes list updates.
     * \param context List of dirs and files for which this dialog is shown. Every directory entry
     *        means "every file in this directory", file stands for a file. This context is used
     *        like a filter for \p versionInfo to make changes list.
     * \param parent Parent widget.
     */
    SvnCommitDialog(const QHash<QString, KVersionControlPlugin::ItemVersion> *versionInfo, const QStringList &context, QWidget *parent = nullptr);

    virtual ~SvnCommitDialog() override;

Q_SIGNALS:
    /**
     * Is emitted for SVN commit.
     *
     * \param context List of files and dirs for which this commit is applied. This not necessarily
     *        the same list which passed to the class contructor because some files might be, for
     *        example, reverted.
     * \param msg Commit message.
     */
    void commit(const QStringList &context, const QString &msg);

    void revertFiles(const QStringList &filesPath);
    void diffFile(const QString &filePath);
    void addFiles(const QStringList &filesPath);

public Q_SLOTS:
    void refreshChangesList();
    void show();

private Q_SLOTS:
    void contextMenu(const QPoint &pos);

private:
    const QHash<QString, KVersionControlPlugin::ItemVersion> *m_versionInfoHash;
    const QStringList m_context;
    QPlainTextEdit *m_editor;
    QTableWidget *m_changes;
    QAction *m_actRevertFile;
    QAction *m_actDiffFile;
    QAction *m_actAddFile;
};

#endif // SVNCOMMITDIALOG_H
