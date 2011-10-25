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

#ifndef HGPATH_CONFIG_H
#define HGPATH_CONFIG_H

#include <QtGui/QWidget>
#include <QtCore/QMap>
#include <QtCore/QString>

class QTableWidget;
class QTableWidgetItem;
class KPushButton;
class KAction;
class KMenu;

/**
 * UI to add, remove and modify paths in repository's hgrc file. Can be used with
 * repository hgrc file only.
 */
class HgPathConfigWidget : public QWidget
{
    Q_OBJECT

public:
    HgPathConfigWidget(QWidget *parent = 0);

public slots:
    void saveConfig();
    void loadConfig();

private:
    void setupUI();
    
    /**
     * Prepare context menu and its actions for table widget showing path.
     */
    void setupContextMenu();

private slots:
    /**
     * Show context menu and changed enabled status of actions according 
     * to the position where menu is requested.
     */
    void slotContextMenuRequested(const QPoint &pos); 
    void slotCellChanged(int row, int col);
    void slotSelectionChanged();

    void slotAddPath();
    void slotModifyPath();
    void slotDeletePath();

private:
    QTableWidget *m_pathsListWidget;
    bool m_loadingCell;
    bool m_allValidData;
    bool m_newAdd;
    QString m_oldSelValue;

    KPushButton *m_addPathButton;
    KPushButton *m_deletePathButton;
    KPushButton *m_modifyPathButton;

    KAction *m_addAction;
    KAction *m_modifyAction;
    KAction *m_deleteAction;
    KMenu *m_contextMenu;

    QMap<QString, QString> m_remotePathMap;
    QStringList m_removeList;
};

#endif // HGPATH_CONFIG_H

