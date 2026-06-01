/*
 * SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mountedisooverlay.h"

#include "mountedisooverlay_debug.h"

#include <KPluginFactory>

#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/GenericInterface>
#include <Solid/StorageAccess>

using namespace Qt::Literals::StringLiterals;

static constexpr QLatin1StringView g_mountedEmblemIconName{"emblem-mounted"};
static constexpr QLatin1StringView g_unmountedEmblemIconName{"emblem-unmounted"};

static constexpr QLatin1StringView g_backingFileProperty{"BackingFile"};

MountedIsoOverlay::MountedIsoOverlay(QObject *parent)
    : KOverlayIconPlugin(parent)
{
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &MountedIsoOverlay::onDeviceAdded);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &MountedIsoOverlay::onDeviceRemoved);

    const QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume);
    for (const Solid::Device &device : devices) {
        onDeviceAdded(device.udi());
    }
}

QStringList MountedIsoOverlay::getOverlays(const QUrl &url)
{
    if (m_backingFiles.contains(url)) {
        if (m_unmountedFiles.contains(url)) {
            return {g_unmountedEmblemIconName};
        } else {
            return {g_mountedEmblemIconName};
        }
    } else {
        return {};
    }
}

void MountedIsoOverlay::onDeviceAdded(const QString &udi)
{
    Solid::Device device(udi);

    auto genericDevice = device.as<Solid::GenericInterface>();
    if (!genericDevice) {
        return;
    }

    connect(genericDevice, &Solid::GenericInterface::propertyChanged, this, [this, udi](const QMap<QString, int> &changes) {
        onDevicePropertyChanged(udi, changes);
    });

    const QUrl backingFile = QUrl::fromLocalFile(genericDevice->property(g_backingFileProperty).toString());
    if (backingFile.isEmpty()) {
        return;
    }

    // We might not have a storage access for this, so we default to assuming it is mounted.
    if (auto storageAccess = device.as<Solid::StorageAccess>()) {
        connect(storageAccess, &Solid::StorageAccess::accessibilityChanged, this, [this, udi](bool accessible) {
            onDeviceAccessibilityChanged(udi, accessible);
        });
        if (!storageAccess->isAccessible()) {
            m_unmountedFiles.insert(backingFile);
        }
    }

    qCDebug(lcMountedIsoOverlay) << "device" << device.udi() << "is backed by" << backingFile;
    m_backingFiles.insert(backingFile, device.udi());
    Q_EMIT overlaysChanged(backingFile, getOverlays(backingFile));
}

void MountedIsoOverlay::onDeviceRemoved(const QString &udi)
{
    const QUrl url = m_backingFiles.key(udi);
    if (url.isEmpty()) {
        return;
    }

    qCDebug(lcMountedIsoOverlay) << "device" << udi << "backed by" << url << "disappeared";
    m_backingFiles.remove(url);
    m_unmountedFiles.remove(url);
    Q_EMIT overlaysChanged(url, {});
}

void MountedIsoOverlay::onDevicePropertyChanged(const QString &udi, const QMap<QString, int> &changes)
{
    if (!changes.contains(g_backingFileProperty)) {
        return;
    }

    Solid::Device device(udi);
    auto genericDevice = device.as<Solid::GenericInterface>();
    if (!genericDevice) {
        return;
    }

    const QUrl oldBackingFile = m_backingFiles.key(udi);
    m_backingFiles.remove(oldBackingFile); // no "takeKey" on QMap.
    m_unmountedFiles.remove(oldBackingFile);
    Q_EMIT overlaysChanged(oldBackingFile, {});

    const QUrl newBackingFile = QUrl::fromLocalFile(genericDevice->property(g_backingFileProperty).toString());
    qCDebug(lcMountedIsoOverlay) << "device" << udi << "backing file changed from" << oldBackingFile << "to" << newBackingFile;
    if (!newBackingFile.isEmpty()) {
        m_backingFiles.insert(newBackingFile, device.udi());

        // Just to be sure check whether it's accessible and don't just move the URL.
        if (auto storageAccess = device.as<Solid::StorageAccess>(); storageAccess && !storageAccess->isAccessible()) {
            m_unmountedFiles.insert(newBackingFile);
        }

        Q_EMIT overlaysChanged(newBackingFile, getOverlays(newBackingFile));
    }
}

void MountedIsoOverlay::onDeviceAccessibilityChanged(const QString &udi, bool accessible)
{
    const QUrl backingFile = m_backingFiles.key(udi);
    if (backingFile.isEmpty()) {
        return;
    }

    qCDebug(lcMountedIsoOverlay) << "device" << udi << "backed by" << backingFile << "got" << (accessible ? "mounted" : "unmounted");
    if (accessible) {
        m_unmountedFiles.remove(backingFile);
    } else {
        m_unmountedFiles.insert(backingFile);
    }

    Q_EMIT overlaysChanged(backingFile, getOverlays(backingFile));
}

#include "moc_mountedisooverlay.cpp"
