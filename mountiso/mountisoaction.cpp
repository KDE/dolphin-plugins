/*
 * Copyright (C) 2020 Kwon-Young Choi <kwon-young.choi@hotmail.fr>
 * Copyright (C) 2021 Kai Uwe Broulik <kde@broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 */

#include "mountisoaction.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <QAction>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QIcon>
#include <QMap>
#include <QString>
#include <QVariant>

#include <KLocalizedString>
#include <KPluginFactory>

using VariantMapMap = QMap<QString, QVariantMap>;
using DBusObjectMap = QMap<QDBusObjectPath, VariantMapMap>;

static const QString s_udisks2Service = QStringLiteral("org.freedesktop.UDisks2");
static const QString s_udisks2FilesystemInterface = QStringLiteral("org.freedesktop.UDisks2.Filesystem");
static const QString s_udisks2LoopInterface = QStringLiteral("org.freedesktop.UDisks2.Loop");

K_PLUGIN_CLASS_WITH_JSON(MountIsoAction, "mountisoaction.json")

MountIsoAction::MountIsoAction(QObject *parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent)
{
}

/**
 * Callback function for mounting an iso file as a loop device
 *
 * Uses UDisks2 Manager DBus api to mount the iso file
 *
 * @file: iso file path to mount
 */
void mount(const QString &file)
{
    const int fd = open(file.toLocal8Bit().data(), O_RDONLY);
    if (fd == -1) {
        qWarning() << "Error opening " << file << ": " << strerror(errno);
        return;
    }
    auto qtFd = QDBusUnixFileDescriptor(fd);
    int res = close(fd);
    if (res == -1) {
        qWarning() << "Error closing " << file << ": " << strerror(errno);
        return;
    }

    auto msg = QDBusMessage::createMethodCall(s_udisks2Service,
                                              QStringLiteral("/org/freedesktop/UDisks2/Manager"),
                                              QStringLiteral("org.freedesktop.UDisks2.Manager"),
                                              QStringLiteral("LoopSetup"));
    msg.setArguments({
        QVariant::fromValue(qtFd),
        QVariantMap{
            {QStringLiteral("read-only"), true}
        }
    });

    auto reply = QDBusConnection::systemBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(reply);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, [file](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            qWarning() << "Failed to do a LoopSetup for" << file << reply.error().message();
            return;
        }

        const auto devicePath = reply.value();

        auto msg = QDBusMessage::createMethodCall(s_udisks2Service,
                                                  devicePath.path(),
                                                  s_udisks2FilesystemInterface,
                                                  QStringLiteral("Mount"));
        msg.setArguments({
            QVariantMap() /*options*/
        });

        QDBusConnection::systemBus().call(msg, QDBus::NoBlock);
    });
}

/**
 * Callback function for deleting a loop device
 *
 * Uses UDisks2 DBus api to delete a loop device
 *
 * @file: iso file to mount
 */
void unmount(const QString &device)
{
    // First delete the loop device, it will still remain in Places panel if mounted
    auto msg = QDBusMessage::createMethodCall(s_udisks2Service,
                                              device,
                                              s_udisks2LoopInterface,
                                              QStringLiteral("Delete"));
    msg.setArguments({
        QVariantMap() // options
    });

    auto reply = QDBusConnection::systemBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(reply);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, [device](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            qWarning() << "Failed to delete loop device" << device << reply.error().message();
            return;
        }

        // Now unmount it
        auto msg = QDBusMessage::createMethodCall(s_udisks2Service,
                                                  device,
                                                  s_udisks2FilesystemInterface,
                                                  QStringLiteral("Unmount"));
        msg.setArguments({
            QVariantMap() /*options*/
        });

        QDBusConnection::systemBus().call(msg, QDBus::NoBlock);
    });
}

QList<QAction *> MountIsoAction::actions(const KFileItemListProperties &fileItemInfos,
                                         QWidget *parentWidget)
{
    Q_UNUSED(parentWidget);

    if (fileItemInfos.urlList().size() != 1
            || !fileItemInfos.isLocal()) {
        return {};
    }

    if (fileItemInfos.mimeType() != QLatin1String("application/x-cd-image")
            && fileItemInfos.mimeType() != QLatin1String("application/x-raw-disk-image")) {
        return {};
    }

    // Check if dbus can handle file descriptor
    const auto capabilities = QDBusConnection::systemBus().connectionCapabilities();
    if (!capabilities.testFlag(QDBusConnection::UnixFileDescriptorPassing)) {
        return {};
    }

    const QString file = fileItemInfos.urlList().at(0).toLocalFile();

    QAction *action = new QAction(QIcon::fromTheme(QStringLiteral("media-mount")),
                                  i18nc("@action:inmenu Waiting for determining whether disk image is mounted", "Checking Image..."));
    action->setEnabled(false);

    qDBusRegisterMetaType<DBusObjectMap>();
    qDBusRegisterMetaType<VariantMapMap>();
    auto msg = QDBusMessage::createMethodCall(s_udisks2Service,
                                              QStringLiteral("/org/freedesktop/UDisks2"),
                                              QStringLiteral("org.freedesktop.DBus.ObjectManager"),
                                              QStringLiteral("GetManagedObjects"));
    auto reply = QDBusConnection::systemBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(reply, action);
    connect(watcher, &QDBusPendingCallWatcher::finished, action, [action, file](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<DBusObjectMap> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            qWarning() << "Failed to query UDisks objects for determining whether image at" << file << "is mounted:" << reply.error().message();
            action->setVisible(false);
            return;
        }

        const auto objects = reply.value();

        QString devicePath;

        for (auto it = objects.begin(), end = objects.end(); it != end; ++it) {
            const auto deviceObjectPath = it.key();
            const auto deviceProperties = it.value();

            const QVariantMap loopProperties = deviceProperties.value(s_udisks2LoopInterface);
            if (loopProperties.isEmpty()) {
                continue;
            }

            const QString backingFile = QString::fromUtf8(loopProperties.value(QStringLiteral("BackingFile")).toByteArray());
            if (backingFile != file) {
                continue;
            }

            devicePath = deviceObjectPath.path();
            break;
        }

        if (devicePath.isEmpty()) { // not mounted
            action->setIcon(QIcon::fromTheme(QStringLiteral("media-mount")));
            action->setText(i18nc("@action:inmenu Action to mount a disk image", "Mount Image"));
            connect(action, &QAction::triggered, [file] {
                mount(file);
            });
        } else { // mounted
            action->setIcon(QIcon::fromTheme(QStringLiteral("media-eject")));
            action->setText(i18nc("@action:inmenu Action to unmount a disk image", "Unmount Image"));
            connect(action, &QAction::triggered, [devicePath] {
                unmount(devicePath);
            });
        }

        action->setEnabled(true);
    });

    return {action};
}

#include "mountisoaction.moc"
