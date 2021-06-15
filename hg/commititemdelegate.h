/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMMITITEMDELEGATE_H
#define COMMITITEMDELEGATE_H

#include <QItemDelegate>

/**
 * Custom Delegate to show Commit info in three lines
 *  - Revision:Changeset (branch)
 *  - Author
 *  - Commit Log | First Line
 */
class CommitItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit CommitItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                  const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

};

#endif // COMMITITEMDELEGATE_H
