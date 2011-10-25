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

#include <QtGui/QWidget>
#include "hgconfig.h"

class KLineEdit;
class KConfig;
class KPushButton;

/**
 * Plugin Specific settings. Not those supposed to be saved in 
 * .hgrc file, but in $HOME/.dolphin-hg 
 */
class HgPluginSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    HgPluginSettingsWidget(QWidget *parent = 0);

public slots:
    void saveConfig();
    void loadConfig();

private slots:
    void browse_diff();

private:
    void setupUI();

private:
    KLineEdit *m_diffProg;
    KConfig *m_config;
    KPushButton *m_diffBrowseButton;
};

#endif // HG_PLUGIN_SETTINGS_WIDGET_H

