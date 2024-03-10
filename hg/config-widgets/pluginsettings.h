/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HG_PLUGIN_SETTINGS_WIDGET_H
#define HG_PLUGIN_SETTINGS_WIDGET_H

#include "hgconfig.h"
#include <QWidget>

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
