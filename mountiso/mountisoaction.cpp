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
#include <errno.h>

#include <QAction>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QEventLoop>
#include <QIcon>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QVariant>

#include <KLocalizedString>
#include <KPluginFactory>
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/GenericInterface>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>

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
const Solid::Device getDeviceFromBackingFile(const QString &backingFile)
{
    const QList<Solid::Device> blockDevices =
        Solid::Device::listFromQuery("[ IS Block AND IS GenericInterface ]");

    for (const Solid::Device &device : blockDevices) {
        QMap<QString, QVariant> properties = device.as<Solid::GenericInterface>()->allProperties();
        if (properties.contains("BackingFile")
            && backingFile == properties["BackingFile"].value<QString>()) {
            return device;
        }
    }
    return Solid::Device();
}

const QList<Solid::Device> getStorageAccessFromDevice(const Solid::Device &device)
{
    auto genericInterface = device.as<Solid::GenericInterface>();
    const QString uuid = genericInterface->property(QLatin1String("IdUUID")).value<QString>();
    auto query = QString("[ StorageVolume.uuid == '%1' AND IS StorageAccess ]").arg(uuid);
    return Solid::Device::listFromQuery(query);
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
        return;
    }

    // Need to wait for UDisks2 to send a signal to Solid to update its database
    auto notifier = Solid::DeviceNotifier::instance();

    // The following code can not be put into a slot because the MountIsoAction object is destroyed
    // as soon as this function ends
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(notifier, &Solid::DeviceNotifier::deviceAdded, &loop, &QEventLoop::quit);

    int i = 0, maxDeviceRace = 4;
    Solid::Device device;
    while (i < maxDeviceRace) {
        timer.start(5000); // 5s timeout
        loop.exec();

        device = Solid::Device(reply.value().path());
        if (!device.is<Solid::StorageVolume>()) {
            i++;
        } else {
            break;
        }
    }

    if (i == maxDeviceRace) {
        // Something really wrong happened
        return;
    }

    auto storageVolume = device.as<Solid::StorageVolume>();
    const QString uuid = storageVolume->uuid();

    QList<Solid::Device> devices = Solid::Device::listFromQuery(
        QStringLiteral("[ StorageVolume.uuid == '%1' AND IS StorageAccess ]").arg(uuid));
    for (auto dev : devices) {
        auto storageAccess = dev.as<Solid::StorageAccess>();
        storageAccess->setup();
    }
}

/**
 * Callback function for deleting a loop device
 *
 * Uses UDisks2 DBus api to delete a loop device
 *
 * @file: iso file to mount
 */
void unmount(const Solid::Device &device)
{
    const QList<Solid::Device> devices = getStorageAccessFromDevice(device);
    for (Solid::Device storageAccessDevice : devices) {
        auto storageAccess = storageAccessDevice.as<Solid::StorageAccess>();
        if (storageAccess->isAccessible()) {
            storageAccess->teardown();
        }
    }

    // Empty argument required for Loop Delete method to work
    QMap<QString, QVariant> options;

    QDBusInterface manager(
            "org.freedesktop.UDisks2",
            device.udi(),
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

    const Solid::Device device = getDeviceFromBackingFile(file);

    if (!device.isValid()) {
        const QIcon icon = QIcon::fromTheme(QStringLiteral("media-mount"));
        const QString title = i18nc("@action:inmenu Action to mount an ISO image", "Mount ISO");

        QAction *action = new QAction(icon, title, parentWidget);

        connect(action, &QAction::triggered, this, [file]() { mount(file); });
        return { action };
    } else {
        // fileItem is mounted on device
        const QIcon icon = QIcon::fromTheme(QStringLiteral("media-eject"));
        const QString title =
            i18nc("@action:inmenu Action to unmount an ISO image", "Unmount ISO");
        QAction *action = new QAction(icon, title, parentWidget);

        connect(action, &QAction::triggered, this, [device]() { unmount(device); });
        return { action };
    }

    return {};
}

#include "mountisoaction.moc"
