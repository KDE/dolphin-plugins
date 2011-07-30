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

#include "commititemdelegate.h"

#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>

CommitItemDelegate::CommitItemDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

void CommitItemDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QString changeset = index.data(Qt::DisplayRole).toString();
    QString revision = index.data(Qt::UserRole + 1).toString();
    QString branch = index.data(Qt::UserRole + 2).toString();
    QString authorName = index.data(Qt::UserRole + 3).toString();
    QString commitLog = index.data(Qt::UserRole + 4).toString();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
    }

    QFont font = option.font;
    QFontMetrics fm(font);
    QRect rect = option.rect.adjusted(4, 4, 4, 4);

    QString top;
    if (!revision.isEmpty()) {
        top = QString("%1:").arg(revision);
    }
    top += changeset;

    if (!branch.isEmpty()) {
        top += QString(" (%1)").arg(branch);
    }
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(rect, Qt::AlignLeft, top);

    font.setPixelSize(0.60f * static_cast<float>(fm.height()));
    font.setBold(false);
    painter->setFont(font);
    rect = rect.adjusted(0, fm.height(), 0, fm.height());
    painter->drawText(rect, Qt::AlignLeft, authorName, &rect);

    int fs = 0.60f * static_cast<float>(fm.height());
    font.setPixelSize(fs);
    font.setBold(false);
    painter->setFont(font);
    rect = rect.adjusted(0, fs+4, 0, fs+4);
    painter->drawText(rect, Qt::AlignLeft, commitLog, &rect);
}


QSize CommitItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QItemDelegate::sizeHint(option, index);
    QFont font = option.font;
    font.setBold(true);
    QFontMetrics fm(font);
    int height = static_cast<float>(option.fontMetrics.height()) * (1.2f) + fm.height() + 15;
    size.setHeight(height);

    return size;
}

#include "commititemdelegate.moc"


