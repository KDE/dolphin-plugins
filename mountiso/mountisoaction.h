/*
    SPDX-FileCopyrightText: 2020 Kwon-Young Choi <kwon-young.choi@hotmail.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MOUNTISOACTION_H
#define MOUNTISOACTION_H

#include <KAbstractFileItemActionPlugin>
#include <KFileItemListProperties>

class QAction;
class QWidget;

class KFileItemListProperties;

/**
 * File item action plugin to mount and unmount an iso file.
 *
 * This class adds a mount or unmount entry to an iso file contextual menu. If
 * the iso file is already mounted, only the unmount entry will be shown while
 * if the iso file is not mounted, only the mount entry will be shown.
 *
 * The current implementation uses a loop device to mount the iso file.
 *
 */
class MountIsoAction : public KAbstractFileItemActionPlugin
{
    Q_OBJECT

public:
    MountIsoAction(QObject *parent, const QVariantList &args);

    /**
     * Adds a mount or unmount entry to the contextual menu of an iso file.
     *
     * For a menu entry to be shown, only one file should be selected, the
     * mimetype of the file should be "application/x-cd-image" and the file
     * should be local. Then, the mount state of the iso file is checked and if
     * the iso file is already mounted, we get the corresponding block device.
     * If the iso file is not already mounted, we show a mount entry and create
     * a callback to mount the iso file using udisksctl.
     * If the iso file is already mounted, we show a unmount entry and create a
     * callback to unmount the iso file using udisksctl.
     */
    QList<QAction *> actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget) override;
};

#endif
