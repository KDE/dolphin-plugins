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

#ifndef HGCONFIG_H
#define HGCONFIG_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtCore/QMap>
#include <kconfig.h>

class HgConfig 
{
public:
    enum ConfigType {
        RepoConfig, GlobalConfig, TempConfig
    };

    HgConfig(ConfigType configFile);
    ~HgConfig();
    
    // Related to config file
    QString configFilePath() const;

    // Repo specific
    void setRepoRemotePath(const QString &alias, const QString &url);
    void deleteRepoRemotePath(const QString &alias);
    QString repoRemotePath(const QString &alias) const;
    QMap<QString, QString> repoRemotePathList() const;

    //  
    QString property(const QString &section, const QString &propertyName)const;
    void setProperty(const QString &section, const QString &propertyName,
            const QString &propertyValue);

    // user interface section
    QString username() const;
    void setUsername(const QString &userName);
    QString editor() const;
    void setEditor(const QString &pathToEditor);
    QString merge() const;
    void setMerge(const QString &pathToMergeTool);

private:
    bool getConfigFilePath();
    bool loadConfig();

private:
    ConfigType m_configType;
    QString m_configFilePath;
    KConfig *m_config;

};

#endif //HGCONFIG_H

