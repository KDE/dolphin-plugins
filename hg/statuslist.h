/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>
    SPDX-FileCopyrightText: 2015 Tomasz Bojczuk <seelook@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATUSLIST_H
#define STATUSLIST_H

#include <QGroupBox>
#include <QString>

class QTableWidget;

/**
 * Shows a list of files and their corresponding version states in a table.
 * Used in commit dialog.
 */
class HgStatusList : public QGroupBox
{
    Q_OBJECT

public:
    explicit HgStatusList(QWidget *parent = nullptr);

    /**
     * Appends the list of selected files whose changes should be 
     * committed. If all files are selected, nothing is appended and true 
     * is returned. If no files are selected, false is returned.
     *
     * @param files Append all the selected files to this. If all files are
     *              selected, nothing is appended
     * @return If at least one file is selected, true is returned; otherwise
     *          false.
     */
    bool getSelectionForCommit(QStringList &files);

private Q_SLOTS:
    void reloadStatusTable();
    void currentItemChangedSlot();
    void headerClickedSlot(int index);

Q_SIGNALS:
    void itemSelectionChanged(const char status, const QString &fileName);

private:
    QString         m_hgBaseDir;
    QTableWidget   *m_statusTable;
    bool            m_allWhereChecked; // state of all check boxes
    bool            m_sortIndex; // true - ascending, false - descending
};

#endif // STATUSLIST_H

