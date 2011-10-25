/***************************************************************************
 *   Copyright (C) 2011 by Vishesh Yadav <vishesh3y@gmail.com>             *
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

#ifndef HGCOMMITINFOWIDGET_H
#define HGCOMMITINFOWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QAbstractItemView>
#include <QtCore/QList>

namespace KTextEditor {
    class View;
    class Document;
};

class QListWidget;
class QListWidgetItem;

/**
 * Shows Changesets using a custom deltegate CommitItemDelegate. Shows
 * changeset information in a KTextEditor widget when a changeset entry is
 * highlighted.
 * */
class HgCommitInfoWidget : public QWidget
{
    Q_OBJECT
public:
    HgCommitInfoWidget(QWidget *parent=0);

    /**
     * Adds a new entry in the ListWidget with the chageset paramters
     * provided in its parameter.
     *
     * @param revision Revision number of changeset
     * @param changeset Changeset identification hash (shortened)
     * @param author Author of the CommitItemDelegate
     * @param log Commit Message of that changeset
     *
     */
    void addItem(const QString &revision, 
                 const QString &changeset,
                 const QString &branch,
                 const QString &author,
                 const QString &log);

    /**
     * Adds a new QListWidgetItem into list. Expects the changeset information
     * in stored in different roles of data. 
     *
     * DisplayRole    => Changeset Identification Hash
     * UserRole + 1   => Revision Number
     * UserRole + 2   => Branch
     * UserRole + 3   => Author
     * UserRole + 4   => Log/Commit Message
     *
     * @param item Pointer to the QListWidgetItem object to be added
     */
    void addItem(QListWidgetItem *item);

    /**
     * Returns changeset identification hash of selected changeset in ListWidget.
     *
     * @return String containing the changeset identification hash of selected 
     *      chageset
     *
     */
    const QString selectedChangeset() const;

    /**
     * @return Returns a list of selected changesets
     */
    QList<QListWidgetItem*> selectedItems() const;

    /**
     * @return Returns pointer to QListWidgetItem of selected changeset.
     */
    QListWidgetItem* currentItem() const;
    
    /**
     * Calls QListWidget::setSelectionMode(QAbstractItemView::SelectionMode)
     * on the m_commitListWidget
     */
    void setSelectionMode(QAbstractItemView::SelectionMode mode);

    /**
     * Clears all entries in m_commitListWidget
     */
    void clear() const;

private slots:
    /**
     * Show selected changeset information when an entry is selected
     */
    void slotUpdateInfo();

private:
    void setupUI();

private:
    KTextEditor::View *m_editorView;
    KTextEditor::Document *m_editorDoc;
    QListWidget *m_commitListWidget;
};

#endif /* HGCOMMITINFOWIDGET_H */

