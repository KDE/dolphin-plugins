/*****************************************************************************
 *   Copyright (C) 2014 by Emmanuel Pescosta <emmanuelpescosta099@gmail.com> *
 *   Copyright (C) 2012 by Sergei Stolyarov <sergei@regolit.com>             *
 *   Copyright (C) 2010 by Thomas Richard <thomas.richard@proan.be>          *
 *   Copyright (C) 2009-2010 by Peter Penz <peter.penz19@gmail.com>          *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA              *
 *****************************************************************************/

#ifndef FILEVIEWDROPBOXPLUGIN_H
#define FILEVIEWDROPBOXPLUGIN_H

#include <Dolphin/KVersionControlPlugin>

#include <QMap>

class QLocalSocket;

//The dropbox protocol info can be found in the dropboxd-protocol file
//it can be found here https://build.opensuse.org/package/view_file/home:faco84/dolphin-box-plugin/dropboxd-protocol
//Thanks Steffen Schuldenzucker!

/**
 * @brief Dropbox implementation for the KVersionControlPlugin interface.
 */
class FileViewDropboxPlugin : public KVersionControlPlugin
{
    Q_OBJECT

private:
    enum SendCommandMode
    {
        WaitForReply,
        SendCommandOnly
    };

    enum SendCommandTimeout
    {
        ShortTimeout,
        LongTimeout
    };

public:
    FileViewDropboxPlugin(QObject* parent, const QVariantList& args);
    ~FileViewDropboxPlugin() override;

    QString fileName() const override;

    bool beginRetrieval(const QString& directory) override;
    KVersionControlPlugin::ItemVersion itemVersion(const KFileItem& item) const override;
    void endRetrieval() override;

    QList<QAction*> actions(const KFileItemList& items) const override;

private slots:
    void handleContextAction(QAction* action);

private:
    QStringList sendCommand(const QString& command,
                            const QStringList& paths,
                            const QPointer<QLocalSocket>& socket,
                            SendCommandMode mode = SendCommandOnly,
                            SendCommandTimeout timeout = ShortTimeout) const;

    bool connectWithDropbox(const QPointer<QLocalSocket>& socket, SendCommandTimeout timeout) const;

private:
    class Private;
    Private* const d;

    static QMap<QString, KVersionControlPlugin::ItemVersion> m_itemVersions;
};

#endif // FILEVIEWDROPBOXPLUGIN_H

