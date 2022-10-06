/*
    SPDX-FileCopyrightText: 2020 Kwon-Young Choi <kwon-young.choi@hotmail.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
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
#include <QWidget>

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
        Solid::Device::listFromQuery("[ IS StorageVolume AND IS GenericInterface ]");

    for (const Solid::Device &device : blockDevices) {
        auto genericDevice = device.as<Solid::GenericInterface>();
        if (backingFile == genericDevice->property(QStringLiteral("BackingFile")).toString()) {
            return device;
        }
    }
    return Solid::Device();
}

const QList<Solid::Device> getStorageAccessFromDevice(const Solid::Device &device)
{
    auto genericInterface = device.as<Solid::GenericInterface>();
    // Solid always returns UUID lower-case
    const QString uuid = genericInterface->property(QLatin1String("IdUUID")).value<QString>().toLower();
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

    const QList<Solid::Device> devices = Solid::Device::listFromQuery(
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
    if (fileItemInfos.urlList().size() != 1 || !fileItemInfos.isLocal()) {
        return {};
    };

    const QString mimeType = fileItemInfos.mimeType();

    if (mimeType != QLatin1String("application/x-cd-image")
            && mimeType != QLatin1String("application/x-raw-disk-image")) {
        return {};
    }

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
        const QString title = i18nc("@action:inmenu Action to mount a disk image", "Mount");

        QAction *action = new QAction(icon, title, parentWidget);

        connect(action, &QAction::triggered, this, [file]() { mount(file); });
        return { action };
    } else {
        // fileItem is mounted on device
        const QIcon icon = QIcon::fromTheme(QStringLiteral("media-eject"));
        const QString title = i18nc("@action:inmenu Action to unmount a disk image", "Unmount");

        QAction *action = new QAction(icon, title, parentWidget);

        connect(action, &QAction::triggered, this, [device]() { unmount(device); });
        return { action };
    }

    return {};
}

#include "mountisoaction.moc"
