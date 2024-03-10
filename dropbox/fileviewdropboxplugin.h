/*
    SPDX-FileCopyrightText: 2014 Emmanuel Pescosta <emmanuelpescosta099@gmail.com>
    SPDX-FileCopyrightText: 2012 Sergei Stolyarov <sergei@regolit.com>
    SPDX-FileCopyrightText: 2010 Thomas Richard <thomas.richard@proan.be>
    SPDX-FileCopyrightText: 2009-2010 Peter Penz <peter.penz19@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILEVIEWDROPBOXPLUGIN_H
#define FILEVIEWDROPBOXPLUGIN_H

#include <Dolphin/KVersionControlPlugin>

#include <QMap>

class QLocalSocket;

// The dropbox protocol info can be found in the dropboxd-protocol file
// it can be found here https://build.opensuse.org/package/view_file/home:faco84/dolphin-box-plugin/dropboxd-protocol
// Thanks Steffen Schuldenzucker!

/**
 * @brief Dropbox implementation for the KVersionControlPlugin interface.
 */
class FileViewDropboxPlugin : public KVersionControlPlugin
{
    Q_OBJECT

private:
    enum SendCommandMode {
        WaitForReply,
        SendCommandOnly,
    };

    enum SendCommandTimeout {
        ShortTimeout,
        LongTimeout,
    };

public:
    FileViewDropboxPlugin(QObject *parent, const QVariantList &args);
    ~FileViewDropboxPlugin() override;

    QString fileName() const override;

    bool beginRetrieval(const QString &directory) override;
    KVersionControlPlugin::ItemVersion itemVersion(const KFileItem &item) const override;
    void endRetrieval() override;

    QList<QAction *> versionControlActions(const KFileItemList &items) const override;
    QList<QAction *> outOfVersionControlActions(const KFileItemList &items) const override;

private Q_SLOTS:
    void handleContextAction(QAction *action);

private:
    QStringList sendCommand(const QString &command,
                            const QStringList &paths,
                            const QPointer<QLocalSocket> &socket,
                            SendCommandMode mode = SendCommandOnly,
                            SendCommandTimeout timeout = ShortTimeout) const;

    bool connectWithDropbox(const QPointer<QLocalSocket> &socket, SendCommandTimeout timeout) const;

private:
    class Private;
    Private *const d;

    static QMap<QString, KVersionControlPlugin::ItemVersion> m_itemVersions;
};

#endif // FILEVIEWDROPBOXPLUGIN_H
