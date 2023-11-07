/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "hgconfig.h"
#include "hgwrapper.h"

#include <QDir>
#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

HgConfig::HgConfig(ConfigType configType) :
    m_configType(configType),
    m_config(nullptr)
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
            m_configFilePath = HgWrapper::instance()->getBaseDir() + "/.hg/hgrc";
            break;
        }
    case GlobalConfig:
        {
            m_configFilePath = QDir::homePath() + "/.hgrc";
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

    KConfigGroup group(m_config, QStringLiteral("paths"));
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

    KConfigGroup group(m_config, QStringLiteral("paths"));
    return group.entryMap();
}

//

