/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGCONFIGDIALOG_H
#define HGCONFIGDIALOG_H

#include "hgconfig.h"
#include <KPageDialog>

class HgGeneralConfigWidget;
class HgPathConfigWidget;
class HgIgnoreWidget;
class HgPluginSettingsWidget;

/**
 * Implements a dialog which provides an easy way to edit several
 * configuration options for Mercurial and the plugin.
 */
class HgConfigDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit HgConfigDialog(HgConfig::ConfigType type, QWidget *parent = nullptr);

private:
    void done(int r) override;

    // user interface
    void setupUI();

private Q_SLOTS:
    void saveSettings();
    void saveGeometry();
    void loadGeometry();

private:
    HgGeneralConfigWidget *m_generalConfig;
    HgPathConfigWidget *m_pathConfig;
    HgIgnoreWidget *m_ignoreWidget;
    HgPluginSettingsWidget *m_pluginSetting;

    HgConfig::ConfigType m_configType;
};

#endif // HGCONFIGDIALOG_H

