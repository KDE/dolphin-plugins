/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "commititemdelegate.h"

#include <QPainter>
#include <QFontMetrics>

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




#include "moc_commititemdelegate.cpp"
