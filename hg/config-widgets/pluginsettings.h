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

#ifndef HG_PLUGIN_SETTINGS_WIDGET_H
#define HG_PLUGIN_SETTINGS_WIDGET_H

#include <QWidget>
#include "hgconfig.h"

class QLineEdit;
class KConfig;
class QPushButton;

/**
 * Plugin Specific settings. Not those supposed to be saved in 
 * .hgrc file, but in $HOME/.dolphin-hg 
 */
class HgPluginSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HgPluginSettingsWidget(QWidget *parent = nullptr);
    ~HgPluginSettingsWidget() override;

public Q_SLOTS:
    void saveConfig();
    void loadConfig();

private Q_SLOTS:
    void browse_diff();

private:
    void setupUI();

private:
    QLineEdit *m_diffProg;
    KConfig *m_config;
    QPushButton *m_diffBrowseButton;
};

#endif // HG_PLUGIN_SETTINGS_WIDGET_H

