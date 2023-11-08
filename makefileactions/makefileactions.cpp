/*
 * SPDX-FileCopyrightText: 2022 Pablo Rauzy <r .at. uzy.me>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <unistd.h>
#include <csignal>

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QString>

#include <KConfigGroup>
#include <KDialogJobUiDelegate>
#include <KFileItemListProperties>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KTerminalLauncherJob>
#include <KStringHandler>

#include "makefileactions.h"

#define MAKE_CMD "make" // on Debian install the bmake package and use bmake here to test this plugin using BSD make rather than GNU make

K_PLUGIN_CLASS_WITH_JSON(MakefileActions, "makefileactions.json")

MakefileActions::MakefileActions(QObject *parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent)
{
    KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("dolphinrc")), QStringLiteral("MakefileActionsPlugin"));
    m_openTerminal = config.readEntry("open_terminal", false);
    m_isMaking = false;
    m_trustedFiles = config.readEntry("trusted_files", QStringList());
}

bool MakefileActions::isGNUMake()
{
    QProcess proc;
    proc.start(QStringLiteral(MAKE_CMD), { QStringLiteral("--version") }, QIODevice::ReadOnly);
    while (proc.waitForReadyRead()) {
        char buffer[4096];
        while (proc.readLine(buffer, sizeof(buffer)) > 0) {
            if (QString::fromLocal8Bit(buffer).contains(QLatin1String("GNU"))) {
                proc.kill();
                proc.waitForFinished(500);
                return true; // GNU Make
            }
        }
    }
    proc.kill();
    proc.waitForFinished(500);
    return false; // assume BSD Make
}

QStringList MakefileActions::listTargets_GNU(QProcess &proc, const QString &file) const
{
    /* make -pRr : | sed '/Not a target/,+1 d' | grep -v '^\(#\|\s\)\|^$\| :\?= \|%' | cut -d':' -f1 | uniq | sort */
    // make -pRr :
    proc.start(QStringLiteral(MAKE_CMD), { QStringLiteral("-f"), file, QStringLiteral("-pRr"), QStringLiteral(":") }, QIODevice::ReadOnly);
    // sed '/Not a target/,+1 d' | grep -v '^\(#\|\s\)\|^$\| :\?= \|%' | cut -d':' -f1 | uniq
    QSet<QString> targetSet;
    bool nonTarget = false;
    while (proc.waitForReadyRead()) {
        char buffer[4096];
        while (proc.readLine(buffer, sizeof(buffer)) > 0) {
            // sed '/Not a target/,+1 d'
            if (nonTarget) {
                nonTarget = false;
                continue;
            }
            const QString line = QString::fromLocal8Bit(buffer);
            if (line.contains(QLatin1String("Not a target"))) {
                nonTarget = true;
                continue;
            }
            // | grep -v '^\(#\|\s\)\|^$\| :\?= \|%'
            if (line.size() == 0 || line[0] == QLatin1Char('#') || line[0] == QLatin1Char('\n') || line[0] == QLatin1Char('\t') || line.contains(QLatin1String(" = ")) || line.contains(QLatin1String(" := ")) || line.contains(QLatin1Char('%'))) {
                continue;
            }
            // | cut -d':' -f1
            const QString target = line.section(QLatin1Char(':'), 0, 0);
            // heuristics to remove most special targets like .PHONY, .SILENT, etc.
            if (target[0] == QLatin1Char('.') && target.isUpper()) {
                continue;
            }
            // | uniq
            targetSet << target;
        }
    }
    // | sort
    QStringList targetSortedList(targetSet.constBegin(), targetSet.constEnd());
    targetSortedList.sort();

    return targetSortedList;
}

QStringList MakefileActions::listTargets_BSD(QProcess &proc, const QString &file) const
{
    /* make -r -d g3 : 2>&1 | grep ', flags 0, type \(8\|4,\|1\)' | grep -v '%' | cut -d',' -f1 | sed 's/^# //' | sort */
    // 2>&1
    proc.setProcessChannelMode(QProcess::MergedChannels);
    // make -r -d g3 :
    proc.start(QStringLiteral(MAKE_CMD), { QStringLiteral("-f"), file, QStringLiteral("-r"), QStringLiteral("-d"), QStringLiteral("g3"), QStringLiteral(":") }, QIODevice::ReadOnly);
    // grep ', flags 0, type \(8\|4,\|1\)' | grep -v '%' | cut -d',' -f1 | sed 's/^# //'
    QStringList targetList;
    while (proc.waitForReadyRead()) {
        char buffer[4096];
        while (proc.readLine(buffer, sizeof(buffer)) > 0) {
            const QString line = QString::fromLocal8Bit(buffer).chopped(1);
            // grep ', flags 0, type \(8\|4,\|1\)' | grep -v '%'
            if ((!line.contains(QLatin1String(", flags 0, type 8")) && !line.contains(QLatin1String(", flags 0, type 4,")) && !line.contains(QLatin1String(", flags 0, type 1"))) || line.contains(QLatin1Char('%'))) {
                continue;
            }
            // | cut -d',' -f1 | sed 's/^# //'
            const QString target = line.mid(2).section(QLatin1Char(','), 0, 0);
            // heuristics to remove most special targets like .PHONY, .SILENT, etc
            if (target[0] == QLatin1Char('.') && target.isUpper()) {
                continue;
            }
            targetList << target;
        }
    }
    // | sort
    targetList.sort();

    return targetList;
}

TargetTree MakefileActions::targetTree() const
{
    QProcess proc;
    QFileInfo fileInfo(m_file);
    proc.setWorkingDirectory(fileInfo.absoluteDir().absolutePath());
    // LANG=C
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
    proc.setProcessEnvironment(env);

    QStringList targetSortedList(isGNUMake() ? listTargets_GNU(proc, fileInfo.fileName()) : listTargets_BSD(proc, fileInfo.fileName()));

    proc.kill();
    proc.waitForFinished(500);

    // if there is no targets, stop right there
    if (targetSortedList.isEmpty()) {
        return TargetTree();
    }

    /* compute prefixes */
    QSet<QString> prefixSet;
    QString prev = targetSortedList.first();
    for (auto const &target : std::as_const(targetSortedList)) {
        int min = std::min(prev.size(), target.size());
        int i = 0;
        for (i = 0; i < min; ++i) {
            if (prev[i] == target[i]) continue;
            else break;
        }
        prefixSet.insert(prev.left(prev.lastIndexOf(QDir::separator(), i)));
        prev = target;
    }
    QStringList prefixSortedList(prefixSet.constBegin(), prefixSet.constEnd());
    prefixSortedList.sort();

    /* produce the submenu tree structure */
    TargetTree targets;
    for (auto const &prefix : std::as_const(prefixSortedList)) {
        targets.insert(prefix, false);
    }
    for (auto const &target : std::as_const(targetSortedList)) {
        targets.insert(target, true);
    }

    return targets;
}

void MakefileActions::buildMenu(QMenu *menu, const TargetTree &targets, QWidget *mainWindow)
{
    QList<TargetTree> targetSortedList(targets.children());
    std::sort(targetSortedList.begin(), targetSortedList.end(), TargetTree::cmp);
    for (const TargetTree &tree : std::as_const(targetSortedList)) {
        QString title = tree.prefix().mid(targets.prefix().size());
        if (!targets.prefix().isEmpty() && title[0] == QDir::separator()) {
            title = title.mid(1);
        }
        title = KStringHandler::rsqueeze(title);
        if (tree.children().size() > 0) {
            QMenu *submenu = new QMenu(title + QDir::separator(), menu);
            submenu->setIcon(QIcon::fromTheme(QStringLiteral("folder-symbolic")));
            if (tree.isTarget()) {
                addTarget(submenu, tree, title, mainWindow);
                submenu->addSeparator();
            }
            buildMenu(submenu, tree, mainWindow);
            menu->addMenu(submenu);
        } else if (tree.isTarget()) {
            addTarget(menu, tree, title, mainWindow);
        }
    }
}

void MakefileActions::addTarget(QMenu *menu, const TargetTree &target, const QString &title, QWidget *mainWindow)
{
    QAction *action = new QAction(QIcon::fromTheme(QStringLiteral("run-build")), title, menu);
    action->setEnabled(!m_isMaking);
    action->setToolTip(i18n("Make '%1'%2.", target.prefix(), (m_openTerminal ? QStringLiteral(" (in a terminal)") : QStringLiteral(""))));
    connect(action, &QAction::triggered, this, [this, target, mainWindow]() {
        makeTarget(target.prefix(), mainWindow);
    });
    menu->addAction(action);
}

void MakefileActions::makeTarget(const QString &target, QWidget *mainWindow)
{
    if (m_isMaking) {
        return;
    }
    QFileInfo fileInfo(m_file);
    if (!m_openTerminal) {
        if (!m_proc.isNull()) {
            delete m_proc;
        }
        m_proc = new QProcess(mainWindow);
        m_proc->setWorkingDirectory(fileInfo.absoluteDir().absolutePath());
        m_proc->setProgram(QStringLiteral(MAKE_CMD));
        m_proc->setArguments({QStringLiteral("-f"), fileInfo.fileName(), target});
        connect(m_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, mainWindow, target](int exitCode, QProcess::ExitStatus exitStatus) {
            if (!m_isMaking) {
                return;
            }
            if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                QMessageBox::warning(mainWindow, i18n("Makefile Actions"), i18n("An error occurred while making target '%1'.", target));
            }
            mainWindow->setCursor(Qt::ArrowCursor);
            m_isMaking = false;
            m_runningTarget.clear();
        });
        connect(m_proc, &QProcess::errorOccurred, this, [this, mainWindow, target](QProcess::ProcessError) {
            if (!m_isMaking) { // process has been canceled by the user
                QMessageBox::information(mainWindow, i18n("Makefile Actions"), i18n("Running process for '%1' successfully stopped.", target));
            } else {
                QMessageBox::critical(mainWindow, i18n("Makefile Actions"), i18n("An error occurred trying to make target '%1'.", target));
                m_isMaking = false;
            }
            m_runningTarget.clear();
            mainWindow->setCursor(Qt::ArrowCursor);
        });
        m_isMaking = true;
        m_runningTarget = target;
        m_proc->start();
        mainWindow->setCursor(Qt::BusyCursor);
    } else {
        KTerminalLauncherJob *job = new KTerminalLauncherJob(QLatin1String(MAKE_CMD " -f ") + fileInfo.fileName() + QLatin1Char(' ') + target, mainWindow);
        job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, mainWindow));
        job->setWorkingDirectory(fileInfo.absoluteDir().absolutePath());
        job->start();
    }
}

QList<QAction *> MakefileActions::actions(const KFileItemListProperties &fileItemInfos, QWidget *mainWindow)
{
    // we shall not be root
    if (geteuid() == 0) {
        return {};
    }

    // only a single local file
    if (fileItemInfos.urlList().size() != 1 || !fileItemInfos.isLocal()) {
        return {};
    }

    m_file = fileItemInfos.urlList()[0].toLocalFile();

    QMenu *menu = new QMenu(i18n("&Make…"), mainWindow);
    menu->setIcon(QIcon::fromTheme(QStringLiteral("text-x-makefile")));

    bool trustedFile = m_trustedFiles.contains(m_file);
    QAction *trust = new QAction(menu);
    trust->setToolTip(i18n("Only trusted files can be used by the Makefile Actions plugin."));
    trust->setCheckable(true);
    trust->setChecked(trustedFile);
    trust->setText(trustedFile ? i18n("Trusted file — uncheck to remove trust") : i18n("Untrusted file — check to trust"));
    trust->setIcon(trustedFile ? QIcon::fromTheme(QStringLiteral("checkbox")) : QIcon::fromTheme(QStringLiteral("action-unavailable-symbolic")));
    connect(trust, &QAction::toggled, this, [this, trustedFile, mainWindow]() {
        KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("dolphinrc")), QStringLiteral("MakefileActionsPlugin"));
        if (trustedFile) {
            m_trustedFiles.removeAll(m_file);
        } else if (QMessageBox::question(mainWindow,
                                         i18n("Dolphin Makefile Plugin"),
                                         i18n("<b>Are you sure you can trust this file?</b><br>"
                                              "Trusted files may execute arbitrary code on context-menu invocation."))
                   == QMessageBox::Yes) {
            m_trustedFiles.append(m_file);
        }
        config.writeEntry("trusted_files", m_trustedFiles);
    });
    menu->addAction(trust);

    // if the file is not trusted, we don't go further
    if (!trustedFile) {
        return { menu->menuAction() };
    }

    QAction *openTerminal = new QAction(QIcon::fromTheme(QStringLiteral("utilities-terminal")), i18n("Open a terminal window"), menu);
    openTerminal->setToolTip(i18n("Open a new terminal window to see the output of making the chosen target."));
    openTerminal->setCheckable(true);
    openTerminal->setChecked(m_openTerminal);
    connect(openTerminal, &QAction::toggled, this, [this](bool checked) {
        m_openTerminal = checked;
        KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("dolphinrc")), QStringLiteral("MakefileActionsPlugin"));
        config.writeEntry("open_terminal", checked);
    });
    menu->addAction(openTerminal);

    if (m_isMaking) {
        QAction *cancel = new QAction(QIcon::fromTheme(QStringLiteral("process-stop")), i18n("Cancel running process (%1)", KStringHandler::rsqueeze(m_runningTarget)), menu);
        cancel->setToolTip(i18n("Interrupt the currently running process (%1).", m_runningTarget));
        cancel->setEnabled(true);
        connect(cancel, &QAction::triggered, this, [this](){
            m_isMaking = false;
            m_runningTarget.clear();
            m_proc->kill(); // send ^C to the running process
        });
        menu->addAction(cancel);
    }

    menu->addSeparator();

    buildMenu(menu, targetTree(), mainWindow);

    return { menu->menuAction() };
}

#include "makefileactions.moc"
