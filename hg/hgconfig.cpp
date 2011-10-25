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

#include "hgconfig.h"
#include "hgwrapper.h"

#include <QtCore/QDir>
#include <kdebug.h>
#include <kurl.h>
#include <kconfig.h>
#include <kconfiggroup.h>

HgConfig::HgConfig(ConfigType configType) :
    m_configType(configType),
    m_config(0)
{
    getConfigFilePath();
    loadConfig();
}

HgConfig::~HgConfig()
{
    delete m_config;
}

QString HgConfig::configFilePath() const
{
    return m_configFilePath;
}

bool HgConfig::getConfigFilePath()
{
    switch (m_configType) {
    case RepoConfig:
        {
            KUrl repoBase = KUrl(HgWrapper::instance()->getBaseDir());
            repoBase.addPath(QLatin1String(".hg/hgrc"));
            m_configFilePath = repoBase.path();
            break;
        }
    case GlobalConfig:
        {
            KUrl homeUrl = KUrl(QDir::homePath());
            homeUrl.addPath(QLatin1String(".hgrc"));
            m_configFilePath = homeUrl.path();
            break;
        }
    case TempConfig:
        break;
    }
    return true;
}

bool HgConfig::loadConfig()
{
    m_config = new KConfig(m_configFilePath, KConfig::SimpleConfig);
    return true;
}

QString HgConfig::property(const QString &section,
                           const QString &propertyName) const
{
    KConfigGroup group(m_config, section);
    return group.readEntry(propertyName, QString()).trimmed();
}

void HgConfig::setProperty(const QString &section, 
                              const QString &propertyName,
                              const QString &propertyValue)
{
    KConfigGroup uiGroup(m_config, section);
    if (propertyValue.isEmpty()) {
        uiGroup.deleteEntry(propertyName, KConfigGroup::Normal);
        return;
    }
    uiGroup.writeEntry(propertyName, propertyValue.trimmed());
}

/* User Interface [ui] Section */

QString HgConfig::username() const
{
    return property(QLatin1String("ui"), QLatin1String("username"));
}

void HgConfig::setUsername(const QString &userName)
{
    setProperty(QLatin1String("ui"), QLatin1String("username"), userName);
}

QString HgConfig::editor() const 
{
    return property(QLatin1String("ui"), QLatin1String("editor"));
}

void HgConfig::setEditor(const QString &pathToEditor)
{
    setProperty(QLatin1String("ui"), QLatin1String("editor"), pathToEditor);
}

QString HgConfig::merge() const 
{
    return property(QLatin1String("ui"), QLatin1String("merge"));
}

void HgConfig::setMerge(const QString &pathToMergeTool)
{
    setProperty(QLatin1String("ui"), QLatin1String("merge"), pathToMergeTool);
}

/* Repo specific */

void HgConfig::setRepoRemotePath(const QString &alias, const QString &url)
{
    Q_ASSERT(m_configType == RepoConfig);
    setProperty(QLatin1String("paths"), alias, url);
}

void HgConfig::deleteRepoRemotePath(const QString &alias)
{
    Q_ASSERT(m_configType == RepoConfig);

    KConfigGroup group(m_config, QLatin1String("paths"));
    group.deleteEntry(alias);
}

QString HgConfig::repoRemotePath(const QString &alias) const
{
    Q_ASSERT(m_configType == RepoConfig);
    return property(QLatin1String("paths"), alias);
}

QMap<QString, QString> HgConfig::repoRemotePathList() const
{
    Q_ASSERT(m_configType == RepoConfig);

    KConfigGroup group(m_config, QLatin1String("paths"));
    return group.entryMap();
}

//

