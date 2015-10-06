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

#include "configdialog.h"
#include "hgwrapper.h"
#include "fileviewhgpluginsettings.h"

#include "config-widgets/generalconfig.h"
#include "config-widgets/pathconfig.h"
#include "config-widgets/ignorewidget.h"
#include "config-widgets/pluginsettings.h"

#include <QWidget>
#include <QDialogButtonBox>
#include <KLocalizedString>
#include <QDebug>

HgConfigDialog::HgConfigDialog(HgConfig::ConfigType type, QWidget *parent):
    KPageDialog(parent),
    m_configType(type)
{
    // dialog properties
    if (m_configType == HgConfig::RepoConfig) {
        this->setWindowTitle(xi18nc("@title:window",
                    "<application>Hg</application> Repository Configuration"));
    } else  {
        this->setWindowTitle(xi18nc("@title:window",
                    "<application>Hg</application> Global Configuration"));
    }
    this->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);

    setupUI();
    loadGeometry();

    connect(this, SIGNAL(finished(int)), this, SLOT(saveGeometry()));
}

void HgConfigDialog::setupUI()
{
    m_generalConfig = new HgGeneralConfigWidget(m_configType);
    addPage(m_generalConfig, xi18nc("@label:group", "General Settings"));

    if (m_configType == HgConfig::RepoConfig) {
        m_pathConfig = new HgPathConfigWidget;
        addPage(m_pathConfig, xi18nc("@label:group", "Repository Paths"));

        m_ignoreWidget = new HgIgnoreWidget;
        addPage(m_ignoreWidget, xi18nc("@label:group", "Ignored Files"));
    }
    else if (m_configType == HgConfig::GlobalConfig) {
        m_pluginSetting = new HgPluginSettingsWidget;
        addPage(m_pluginSetting, xi18nc("@label:group", "Plugin Settings"));
    }
}

void HgConfigDialog::saveSettings()
{
    qDebug() << "Saving Mercurial configuration";
    m_generalConfig->saveConfig();
    if (m_configType == HgConfig::RepoConfig) {
        m_pathConfig->saveConfig();
        m_ignoreWidget->saveConfig();
    }
    else if (m_configType == HgConfig::GlobalConfig) {
        m_pluginSetting->saveConfig();
    }
}

void HgConfigDialog::done(int r)
{
    if (r == QDialog::Accepted) {
        saveSettings();
        QDialog::done(r);
    }
    else {
        QDialog::done(r);
    }
}

void HgConfigDialog::loadGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->resize(QSize(settings->configDialogWidth(),
                               settings->configDialogHeight()));
}

void HgConfigDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setConfigDialogHeight(this->height());
    settings->setConfigDialogWidth(this->width());
    settings->save();
}

#include "configdialog.moc"

