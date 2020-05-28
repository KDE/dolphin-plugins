/*
 * Copyright (C) 2020 Kwon-Young Choi <kwon-young.choi@hotmail.fr>
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

#include <QAction>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QIcon>
#include <QMap>
#include <QProcess>
#include <QString>
#include <QVariant>

#include <KLocalizedString>
#include <KPluginFactory>
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/GenericInterface>

K_PLUGIN_CLASS_WITH_JSON(MountIsoAction, "mountisoaction.json")

MountIsoAction::MountIsoAction(QObject *parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent)
{
}

/**
 * Get block device udi ("/org/freedesktop/UDisks2/block_devices/loop0") using
 * its backing file name.
 *
 * Use the Solid framework to iterate through all block devices to check if the
 * backing file correspond to the given backingFile.
 *
 * Warning: The use of GenericInterface makes this function non portable,
 * especially to non Unix-like OS.
 *
 * @backingFile: backing file of the device we want.
 *
 * @return: device udi of the found block device. If no corresponding device
 * was found, return a null QString.
 */
const QString getDeviceFromBackingFile(const QString &backingFile)
{
    const QList<Solid::Device> blockDevices =
        Solid::Device::listFromQuery("[ IS Block AND IS GenericInterface ]");

    for (const Solid::Device &device : blockDevices) {
        QMap<QString, QVariant> properties = device.as<Solid::GenericInterface>()->allProperties();
        if (properties.contains("BackingFile")
            && backingFile == properties["BackingFile"].value<QString>()) {
            return device.udi();
        }
    }
    return QString();
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
    QMap<QString, QVariant> options;
    options["read-only"] = QVariant::fromValue(true);

    QDBusInterface manager(
            "org.freedesktop.UDisks2",
            "/org/freedesktop/UDisks2/Manager",
            "org.freedesktop.UDisks2.Manager",
            QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> reply =
        manager.call("LoopSetup", QVariant::fromValue(qtFd), options);

    if (!reply.isValid()) {
        qWarning() << "Error mounting " << file << ":" << reply.error().name()
                   << reply.error().message();
    }
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
    // Empty argument required for Loop Delete method to work
    QMap<QString, QVariant> options;

    QDBusInterface manager(
            "org.freedesktop.UDisks2",
            device,
            "org.freedesktop.UDisks2.Loop",
            QDBusConnection::systemBus());
    manager.call("Delete", options);
}

QList<QAction *> MountIsoAction::actions(const KFileItemListProperties &fileItemInfos,
                                         QWidget *parentWidget)
{
    if (fileItemInfos.urlList().size() != 1
        || fileItemInfos.mimeType() != QLatin1String("application/x-cd-image")
        || !fileItemInfos.isLocal()) {
        return {};
    };

    auto file = fileItemInfos.urlList().at(0).toLocalFile();

    // Check if dbus can handle file descriptor
    auto connection = QDBusConnection::sessionBus();
    QDBusConnection::ConnectionCapabilities capabilities = connection.connectionCapabilities();
    if (!(capabilities & QDBusConnection::UnixFileDescriptorPassing)) {
        return {};
    }

    const QString device = getDeviceFromBackingFile(file);

    if (device.isEmpty()) {
        const QIcon icon = QIcon::fromTheme(QStringLiteral("media-mount"));
        const QString title = i18nc("@action Action to mount an iso image", "Mount this iso image");

        QAction *action = new QAction(icon, title, parentWidget);

        connect(action, &QAction::triggered, this, [file]() { mount(file); });
        return { action };
    } else {
        // fileItem is mounted on device
        const QIcon icon = QIcon::fromTheme(QStringLiteral("media-eject"));
        const QString title =
            i18nc("@action Action to unmount an iso image", "Unmount this iso image");
        QAction *action = new QAction(icon, title, parentWidget);

        connect(action, &QAction::triggered, this, [device]() { unmount(device); });
        return { action };
    }

    return {};
}

#include "mountisoaction.moc"
