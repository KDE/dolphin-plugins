/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGPATHSELECTOR_H
#define HGPATHSELECTOR_H

#include <QMap>
#include <QWidget>

class KComboBox;
class QLineEdit;

/**
 * A simple widget which presents a ComboBox to select list of Path aliases
 * stored in .hgrc file and show their URL. URL's can be entered manually
 * as well.
 */
class HgPathSelector : public QWidget
{
    Q_OBJECT

public:
    explicit HgPathSelector(QWidget *parent = nullptr);

    /**
     * @return Return QString containing the selected/entered alias/URL
     */
    const QString remote() const;

public Q_SLOTS:
    void reload();

private:
    void setupUI();

private Q_SLOTS:
    void slotChangeEditUrl(int index);

private:
    QMap<QString, QString> m_pathList;
    KComboBox *m_selectPathAlias;
    QLineEdit *m_urlEdit;
};

#endif /* HGPATHSELECTOR_H */
