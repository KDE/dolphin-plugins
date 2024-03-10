/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGGENERAL_CONFIG_WIDGET_H
#define HGGENERAL_CONFIG_WIDGET_H

#include "hgconfig.h"
#include <QWidget>

class QLineEdit;
class QCheckBox;

/**
 * General configuration options, usually found [ui] group of hgrc file.
 * Can be used with both, repository hgrc as well as global hgrc
 */
class HgGeneralConfigWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param type Which configuration file to use, Repo or Global
     */
    explicit HgGeneralConfigWidget(HgConfig::ConfigType type, QWidget *parent = nullptr);

public Q_SLOTS:
    void saveConfig();
    void loadConfig();

private:
    void setupUI();

private:
    QLineEdit *m_userEdit;
    QLineEdit *m_editorEdit;
    QLineEdit *m_mergeEdit;
    QCheckBox *m_verboseCheck;

    HgConfig::ConfigType m_configType;
};

#endif // HGGENERAL_CONFIG_WIDGET_H
