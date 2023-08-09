/*
 * SPDX-FileCopyrightText: 2022 Pablo Rauzy <r .at. uzy.me>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MAKEFILEACTIONS_H
#define MAKEFILEACTIONS_H

#include <QAction>
#include <QFileInfo>
#include <QList>
#include <QMenu>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QString>
#include <QVariantList>
#include <QWidget>

#include <KAbstractFileItemActionPlugin>
#include <KFileItemListProperties>

#include "targettree.h"

/**
 * File item action plugin to execute Makefile targets.
 *
 * This class adds a "Make…" submenu to Makefiles' context menu which contains one action per target to launch it.
 */
class MakefileActions : public KAbstractFileItemActionPlugin
{
    Q_OBJECT

public:
    MakefileActions(QObject *parent, const QVariantList &args);
    QList<QAction *> actions(const KFileItemListProperties &fileItemInfos, QWidget *mainWindow) override;

private:
    QStringList m_trustedFiles;
    QString m_file;
    bool m_openTerminal;
    QPointer<QProcess> m_proc;
    QString m_runningTarget;
    bool m_isMaking;

    static bool isGNUMake();
    QStringList listTargets_GNU(QProcess &proc, const QString &file) const;
    QStringList listTargets_BSD(QProcess &proc, const QString &file) const;

    TargetTree targetTree() const;
    void buildMenu(QMenu *menu, const TargetTree &targets, QWidget *mainWindow);
    void addTarget(QMenu *menu, const TargetTree &target, const QString &title, QWidget *mainWindow);

public Q_SLOTS:
    void makeTarget(const QString &target, QWidget *mainWindow);
};

#endif
