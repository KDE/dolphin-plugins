/***************************************************************************
 *   Copyright (C) 2019-2020                                               *
 *                  by Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

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
class SvnCommitDialog : public QDialog {
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
    SvnCommitDialog(const QHash<QString, KVersionControlPlugin::ItemVersion> *versionInfo, const QStringList& context, QWidget *parent = nullptr);

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
    void commit(const QStringList& context, const QString& msg);

    void revertFiles(const QStringList& filesPath);
    void diffFile(const QString& filePath);
    void addFiles(const QStringList& filesPath);

public Q_SLOTS:
    void refreshChangesList();
    void show();

private Q_SLOTS:
    void contextMenu(const QPoint& pos);

private:
    const QHash<QString, KVersionControlPlugin::ItemVersion> *m_versionInfoHash;
    const QStringList m_context;
    QPlainTextEdit *m_editor;
    QTableWidget *m_changes;
    QAction *m_actRevertFile;
    QAction *m_actDiffFile;
    QAction *m_actAddFile;
};

#endif  // SVNCOMMITDIALOG_H
