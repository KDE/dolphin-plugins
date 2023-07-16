/*
    SPDX-FileCopyrightText: 2014 Emmanuel Pescosta <emmanuelpescosta099@gmail.com>
    SPDX-FileCopyrightText: 2012 Sergei Stolyarov <sergei@regolit.com>
    SPDX-FileCopyrightText: 2010 Thomas Richard <thomas.richard@proan.be>
    SPDX-FileCopyrightText: 2009-2010 Peter Penz <peter.penz19@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fileviewdropboxplugin.h"

#include <KFileItem>
#include <KFileItemListProperties>
#include <KLocalizedString>
#include <KActionCollection>
#include <KPluginFactory>

#include <QDir>
#include <QPointer>
#include <QLocalSocket>
#include <QFileSystemWatcher>
#include <QStringBuilder>

K_PLUGIN_CLASS_WITH_JSON(FileViewDropboxPlugin, "fileviewdropboxplugin.json")

class FileViewDropboxPlugin::Private
{
public:
    Private(FileViewDropboxPlugin* parent) :
        contextFilePaths(),
        controlSocketPath(),
        controlSocket(new QLocalSocket(parent)),
        databaseFileWatcher(new QFileSystemWatcher(parent)),
        contextActions(new KActionCollection(parent))
    {
    }

    QStringList contextFilePaths;
    QString controlSocketPath;
    QPointer<QLocalSocket> controlSocket;
    QPointer<QLocalSocket> itemStateSocket;
    QPointer<QFileSystemWatcher> databaseFileWatcher;
    QPointer<KActionCollection> contextActions;
};

QMap<QString, KVersionControlPlugin::ItemVersion> FileViewDropboxPlugin::m_itemVersions;

FileViewDropboxPlugin::FileViewDropboxPlugin(QObject* parent, const QVariantList& args):
    KVersionControlPlugin(parent),
    d(new Private(this))
{
    Q_UNUSED(args);

    if (m_itemVersions.isEmpty()) {
        m_itemVersions.insert("up to date", KVersionControlPlugin::NormalVersion);
        m_itemVersions.insert("syncing",    KVersionControlPlugin::UpdateRequiredVersion);
        m_itemVersions.insert("unsyncable", KVersionControlPlugin::ConflictingVersion);
        m_itemVersions.insert("unwatched",  KVersionControlPlugin::UnversionedVersion);
    }

    const QString dropboxDir = QDir::home().path() % QDir::separator() % fileName() % QDir::separator();
    d->controlSocketPath = QDir::toNativeSeparators(dropboxDir % QLatin1String("command_socket"));
    d->controlSocket->connectToServer(d->controlSocketPath);

    // Find and watch aggregation.dbx file
    QDir dir(dropboxDir);
    QStringList nameFilter("instance*");
    const QStringList instanceDirs = dir.entryList(nameFilter);
    QString aggregationDB = "";
    for (const QString &instance : instanceDirs) {
        aggregationDB = dropboxDir + '/' + instance + '/' + "aggregation.dbx";
        if (QFile::exists(aggregationDB)) {
            d->databaseFileWatcher->addPath(aggregationDB);
            break;
        }
    }

    connect(d->databaseFileWatcher.data(), &QFileSystemWatcher::fileChanged, this, &KVersionControlPlugin::itemVersionsChanged);
    connect(d->contextActions.data(), &KActionCollection::actionTriggered, this, &FileViewDropboxPlugin::handleContextAction);
}

FileViewDropboxPlugin::~FileViewDropboxPlugin()
{
    delete d;
}

QString FileViewDropboxPlugin::fileName() const
{
    return QLatin1String(".dropbox");
}

bool FileViewDropboxPlugin::beginRetrieval(const QString& directory)
{
    Q_UNUSED(directory);
    Q_ASSERT(directory.endsWith(QLatin1Char('/')));

    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");

    d->itemStateSocket = new QLocalSocket;

    return connectWithDropbox(d->itemStateSocket, LongTimeout);
}

KVersionControlPlugin::ItemVersion FileViewDropboxPlugin::itemVersion(const KFileItem& item) const
{
    const QStringList reply = sendCommand("icon_overlay_file_status\npath\t", QStringList() << QDir(item.localPath()).canonicalPath(),
                                          d->itemStateSocket, WaitForReply, LongTimeout);
    if(reply.count() < 2) {
        // file/dir is not served by dropbox
        return KVersionControlPlugin::UnversionedVersion;
    }

    return m_itemVersions.value(reply.at(1), KVersionControlPlugin::UnversionedVersion);
}

void FileViewDropboxPlugin::endRetrieval()
{
    delete d->itemStateSocket;
}

QList<QAction*> FileViewDropboxPlugin::versionControlActions(const KFileItemList &items) const
{
    Q_ASSERT(!items.isEmpty());

    d->contextActions->clear();
    d->contextFilePaths.clear();

    const KFileItemListProperties properties(items);
    if (!properties.isLocal()) {
        // not all files/dirs are local files/dirs
        return QList<QAction*>();
    }

    for (const KFileItem& item : items) {
        d->contextFilePaths << QDir(item.localPath()).canonicalPath();
    }

    const QStringList reply = sendCommand("icon_overlay_context_options\npaths\t", d->contextFilePaths, d->controlSocket, WaitForReply);
    if (reply.count() < 2) {
        // files/dirs are not served by dropbox
        return QList<QAction*>();
    }

    // analyze item options and dynamically form a menu
    for (const QString& replyLine : reply) {
        const QStringList options = replyLine.split('~');

        if (options.count() > 2) {
            QAction* action = d->contextActions->addAction(options.at(2));
            action->setText(options.at(0));
            action->setToolTip(options.at(1));
            action->setIcon(QIcon::fromTheme("dropbox"));
        }
    }

    return d->contextActions->actions();
}

QList<QAction*> FileViewDropboxPlugin::outOfVersionControlActions(const KFileItemList& items) const
{
    Q_UNUSED(items)

    return {};
}

void FileViewDropboxPlugin::handleContextAction(QAction* action)
{
    sendCommand("icon_overlay_context_action\nverb\t" % action->objectName() % "\npaths\t", d->contextFilePaths, d->controlSocket);
}

QStringList FileViewDropboxPlugin::sendCommand(const QString& command,
                                               const QStringList& paths,
                                               const QPointer<QLocalSocket>& socket,
                                               SendCommandMode mode,
                                               SendCommandTimeout timeout) const
{
    if (!connectWithDropbox(socket, timeout)) {
        return QStringList();
    }

    static const QString parameterSeperator('\t');
    static const QString done("\ndone\n");
    static const QString ok("ok\n");

    const QString request = command % paths.join(parameterSeperator) % done;

    socket->readAll();
    socket->write(request.toUtf8());
    socket->flush();

    if (mode == SendCommandOnly) {
        return QStringList();
    }

    QString reply;
    while (socket->waitForReadyRead(timeout == ShortTimeout ? 100 : 500)) {
        reply.append(QString::fromUtf8(socket->readAll()));

        if (reply.endsWith(done)) {
            break;
        }
    }

    reply.remove(done);
    reply.remove(ok);

    return reply.split(parameterSeperator, Qt::SkipEmptyParts);
}

bool FileViewDropboxPlugin::connectWithDropbox(const QPointer<QLocalSocket>& socket, SendCommandTimeout timeout) const
{
    if (socket->state() != QLocalSocket::ConnectedState) {
        socket->connectToServer(d->controlSocketPath);

        if (!socket->waitForConnected(timeout == ShortTimeout ? 100 : 500)) {
            socket->abort();
            return false;
        }
    }

    return true;
}

#include "fileviewdropboxplugin.moc"

#include "moc_fileviewdropboxplugin.cpp"
