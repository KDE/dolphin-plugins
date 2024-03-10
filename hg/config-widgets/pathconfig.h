/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGPATH_CONFIG_H
#define HGPATH_CONFIG_H

#include <QMap>
#include <QString>
#include <QWidget>

class QTableWidget;
class QTableWidgetItem;
class QPushButton;
class QAction;
class QMenu;

/**
 * UI to add, remove and modify paths in repository's hgrc file. Can be used with
 * repository hgrc file only.
 */
class HgPathConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HgPathConfigWidget(QWidget *parent = nullptr);

public Q_SLOTS:
    void saveConfig();
    void loadConfig();

private:
    void setupUI();

    /**
     * Prepare context menu and its actions for table widget showing path.
     */
    void setupContextMenu();

private Q_SLOTS:
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

    QPushButton *m_addPathButton;
    QPushButton *m_deletePathButton;
    QPushButton *m_modifyPathButton;

    QAction *m_addAction;
    QAction *m_modifyAction;
    QAction *m_deleteAction;
    QMenu *m_contextMenu;

    QMap<QString, QString> m_remotePathMap;
    QStringList m_removeList;
};

#endif // HGPATH_CONFIG_H
