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

namespace KTextEditor {
    class View;
    class Document;
};

class QListWidget;
class QListWidgetItem;

class HgCommitInfoWidget : public QWidget
{
    Q_OBJECT
public:
    HgCommitInfoWidget(QWidget *parent=0);
    void addItem(const QString &revision, 
                 const QString &changeset,
                 const QString &branch,
                 const QString &author,
                 const QString &log);

    void addItem(QListWidgetItem *item);

    const QString selectedChangeset() const;

    QListWidgetItem* currentItem() const;

    void clear() const;

private slots:
    void slotUpdateInfo();

private:
    void setupUI();

private:
    KTextEditor::View *m_editorView;
    KTextEditor::Document *m_editorDoc;
    QListWidget *m_commitListWidget;
};

#endif /* HGCOMMITINFOWIDGET_H */

