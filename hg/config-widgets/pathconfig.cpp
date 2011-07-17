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

#include "pathconfig.h"
#include "hgconfig.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTableWidget>
#include <QtGui/QHeaderView>
#include <QtCore/QEvent>
#include <kpushbutton.h>
#include <kaction.h>
#include <klocale.h>
#include <kmenu.h>
#include <kdebug.h>

HgPathConfigWidget::HgPathConfigWidget(QWidget *parent):
    QWidget(parent)
{
    setupUI();
    loadConfig();
}

/**
 * Context menu shown by right clicking on the path table
 */
void HgPathConfigWidget::setupContextMenu()
{
    m_addAction = new KAction(this);
    m_addAction->setIcon(KIcon("add"));
    m_addAction->setText(i18nc("@action:inmenu",
                                  "Add"));
    connect(m_addAction, SIGNAL(triggered()), this, SLOT(slotAddPath()));

    m_modifyAction = new KAction(this);
    m_modifyAction->setIcon(KIcon("edit"));
    m_modifyAction->setText(i18nc("@action:inmenu",
                                  "Edit"));
    connect(m_modifyAction, SIGNAL(triggered()), this, SLOT(slotModifyPath()));

    m_deleteAction = new KAction(this);
    m_deleteAction->setIcon(KIcon("remove"));
    m_deleteAction->setText(i18nc("@action:inmenu",
                                  "Remove"));
    connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(slotDeletePath()));
    
    m_contextMenu = new KMenu(this);
    m_contextMenu->addAction(m_addAction);
    m_contextMenu->addAction(m_modifyAction);
    m_contextMenu->addAction(m_deleteAction);

    connect(m_pathsListWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotContextMenuRequested(const QPoint&)));
}

void HgPathConfigWidget::setupUI()
{
    // add, remove, modify buttons goes here
    QHBoxLayout *actionsLayout = new QHBoxLayout; 
    m_addPathButton = new KPushButton(i18nc("@label:button", "Add"));
    m_modifyPathButton = new KPushButton(i18nc("@label:button", "Edit"));
    m_deletePathButton = new KPushButton(i18nc("@label:button", "Remove"));
    actionsLayout->addWidget(m_addPathButton);
    actionsLayout->addWidget(m_modifyPathButton);
    actionsLayout->addWidget(m_deletePathButton);

    /// create and setup the Table Widget
    m_pathsListWidget = new QTableWidget;
    setupContextMenu(); // make context menu

    m_pathsListWidget->setColumnCount(2);
    m_pathsListWidget->verticalHeader()->hide();
    m_pathsListWidget->horizontalHeader()->hide();
    m_pathsListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pathsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pathsListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pathsListWidget->horizontalHeader()->setStretchLastSection(true);
    m_pathsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    // setup main layout 
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_pathsListWidget);
    mainLayout->addLayout(actionsLayout);
    setLayout(mainLayout);
}

void HgPathConfigWidget::loadConfig()
{
    HgConfig hgc(HgConfig::RepoConfig);
    m_remotePathMap = hgc.repoRemotePathList();

    m_pathsListWidget->clearContents();

    QMutableMapIterator<QString, QString> it(m_remotePathMap);
    int count = 0;
    while (it.hasNext()) {
        it.next();
        
        QTableWidgetItem *alias = new QTableWidgetItem;
        QTableWidgetItem *path = new QTableWidgetItem;

        alias->setText(it.key());
        path->setText(it.value());

        m_pathsListWidget->insertRow(count);
        m_pathsListWidget->setItem(count, 0, alias);
        m_pathsListWidget->setItem(count, 1, path);
    }

    m_pathsListWidget->resizeRowsToContents();
}

void HgPathConfigWidget::saveConfig()
{
    HgConfig hgc(HgConfig::GlobalConfig);
}

void HgPathConfigWidget::slotContextMenuRequested(const QPoint &pos)
{
    if (m_pathsListWidget->indexAt(pos).isValid()) {
        m_contextMenu->exec(m_pathsListWidget->mapToGlobal(pos));
    }
}

void HgPathConfigWidget::slotAddPath()
{
}

void HgPathConfigWidget::slotDeletePath()
{
}

void HgPathConfigWidget::slotModifyPath()
{
}

#include "pathconfig.moc"

