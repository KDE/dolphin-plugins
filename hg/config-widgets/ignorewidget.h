/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGCONFIG_IGNOREWIDGET_H
#define HGCONFIG_IGNOREWIDGET_H

#include <QWidget>

class QListWidget;
class QPushButton;
class QInputDialog;

/**
 * Widget to manage ignored files. Used .hgignore file in repository.
 * Repository only configuration
 */
class HgIgnoreWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HgIgnoreWidget(QWidget *parent = nullptr);

    void loadConfig();
    void saveConfig();

private Q_SLOTS:
    void slotAddFiles();
    void slotAddPattern();
    void slotRemoveEntries();
    void slotEditEntry();

private:
    void setupUI();
    void setupUntrackedList();

private:
    QListWidget *m_ignoreTable;
    QListWidget *m_untrackedList;
    QPushButton *m_addFiles;
    QPushButton *m_addPattern;
    QPushButton *m_removeEntries;
    QPushButton *m_editEntry;
};

#endif /* HGCONFIG_IGNOREWIDGET_H */
