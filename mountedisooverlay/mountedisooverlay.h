/*
 * SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MOUNTEDISOOVERLAY_H
#define MOUNTEDISOOVERLAY_H

#include <KOverlayIconPlugin>

#include <QMap>
#include <QSet>

class MountedIsoOverlay : public KOverlayIconPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.overlayicon.mountediso")

public:
    explicit MountedIsoOverlay(QObject *parent = nullptr);

    QStringList getOverlays(const QUrl &url) override;

private:
    void onDeviceAdded(const QString &udi);
    void onDeviceRemoved(const QString &udi);
    void onDevicePropertyChanged(const QString &udi, const QMap<QString, int> &changes);
    void onDeviceAccessibilityChanged(const QString &udi, bool accessible);

    // Url is key since getOverlays is the hot path.
    QMap<QUrl, QString /*udi*/> m_backingFiles;
    QSet<QUrl> m_unmountedFiles;
};

#endif // MOUNTEDISOOVERLAY_H
