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
#include <QtCore/QProcess>
#include <QtCore/QMap>

class KConfig;

/**
 * Provides a wrapper to read and write Mercurial settings in hgrc files.
 * Also provides interface to read/write other settings as well, for which
 * a direct function is not provided.
 */
class HgConfig 
{
public:
    /**
     * Type of hgrc file. 
     *
     * RepoConfig: hgrc of the local/current repository.
     * GlobalConfig: Global hgrc file. Under *nix, ~/.hgrc file
     * TempConfig: A temporary  config file, that can be used by some commands
     *             for just one time. Usually after taking input from user to
     *             override default settings.
     */
    enum ConfigType {
        RepoConfig, GlobalConfig, TempConfig
    };

    HgConfig(ConfigType configFile);
    ~HgConfig();
    
    // Related to config file
    
    /**
     * Return the path of hgrc file currently loaded
     */
    QString configFilePath() const;

    // Repo specific

    /**
     * Set path of repository associated with given given alias
     */
    void setRepoRemotePath(const QString &alias, const QString &url);

    /**
     * Delete alias/path of repository associated with given given alias
     */
    void deleteRepoRemotePath(const QString &alias);

    /**
     * Return path of repository associated with given given alias
     */
    QString repoRemotePath(const QString &alias) const;

    /**
     * Return a list of alias-path pair of remote repository paths
     */
    QMap<QString, QString> repoRemotePathList() const;

    ///////////

    /**
     * Get the value of a property in a given section.
     *
     * @param section The settings group in hgrc file.
     * @param propertyName Name of the property in section whose value is\
     *                      required
     * @return Value of property in the section given.
     */
    QString property(const QString &section, const QString &propertyName)const;

    /**
     * Set the value of a property in a given section.
     *
     * @param section The settings group in hgrc file.
     * @param propertyName Name of the property in section whose value is
     *                     to be modified.
     * @param propertyValue Value to be set of given propery. Deletes the 
     *                      entry if empty.
     */
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
    /**
     * Used during construction to determine the path of config files
     * based on given ConfigType enum
     */
    bool getConfigFilePath();
    bool loadConfig();

private:
    ConfigType m_configType;
    QString m_configFilePath;
    KConfig *m_config;

};

#endif //HGCONFIG_H

