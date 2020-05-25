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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
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
    QList<QAction *> actions(const KFileItemListProperties &fileItemInfos,
                             QWidget *parentWidget) override;
};

#endif
