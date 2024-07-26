/*
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gitwrapper.h"

#include <dolphingit_log.h>

#include <QDir>

GitWrapper *GitWrapper::m_instance = nullptr;
const int GitWrapper::BUFFER_SIZE = 256;
const int GitWrapper::SMALL_BUFFER_SIZE = 128;

GitWrapper::GitWrapper()
{
}

GitWrapper *GitWrapper::instance()
{
    if (m_instance == nullptr) {
        m_instance = new GitWrapper();
    }
    return m_instance;
}

void GitWrapper::freeInstance()
{
    delete m_instance;
    m_instance = nullptr;
}

int GitWrapper::shortIdLength()
{
    m_process.start(QStringLiteral("git"), {QStringLiteral("rev-parse"), QStringLiteral("--short"), QStringLiteral("HEAD")});
    while (!m_process.waitForFinished())
        ;
    const auto line = m_process.readLine().trimmed();

    return line.size();
}

bool GitWrapper::isCommitIdValid(const QString &commitSha)
{
    m_process.start(QStringLiteral("git"), {QStringLiteral("cat-file"), QStringLiteral("commit"), commitSha});
    while (!m_process.waitForFinished())
        ;

    return m_process.exitStatus() == QProcess::NormalExit && m_process.exitCode() == EXIT_SUCCESS;
}

QStringList GitWrapper::remoteBranches(const QString &remote)
{
    static QString heads(QStringLiteral("refs/heads/"));
    static QString tags(QStringLiteral("refs/tags/"));
    static QString ref(QStringLiteral("ref: refs/heads/"));

    const QStringList arguments = {QStringLiteral("ls-remote"),
                                   QStringLiteral("--quiet"), // do not print remote url to strerr
                                   QStringLiteral("--symref"), // show default branch at first line
                                   remote};

    QProcess process;
    process.start(QStringLiteral("git"), arguments);

    // Output is in the form, first line:
    // ref: refs/heads/<branch> HEAD
    // others:
    // [ref]\t[refs/heads/<branch> | refs/tags/<tag>]\n
    // Separate by lines and then extract name.
    QStringList remotes;
    // Infinite wait for data: operation depends on connection and might take unknown time.
    while (process.waitForReadyRead(-1)) {
        while (process.canReadLine()) {
            const auto line = QString::fromLocal8Bit(process.readLine()).trimmed();

            if (line.endsWith(QStringLiteral("^{}"))) {
                // No need of preudo-refs.
                continue;
            }

            if (line.startsWith(ref)) {
                remotes << line.split(QLatin1Char('\t'))[0].sliced(ref.size());
                continue;
            }

            const auto idx = line.lastIndexOf(QLatin1Char('\t'));
            if (idx > 0) {
                const auto name = line.last(line.length() - idx - 1);

                // Name starts from 'refs/heads/' or 'refs/tags/', which we remove.
                if (name.startsWith(heads)) {
                    remotes << name.sliced(heads.size());
                } else if (name.startsWith(tags)) {
                    remotes << name.sliced(tags.size());
                } else {
                    // 'HEAD', 'refs/merge-requests', 'refs/backups', etc: just ignore it.
                }
            } else {
                qCWarning(DolphinGitLog) << "Error proccessing `git ls-remote` output: can't find `\\t` in:" << line;
            }
        }
    }

    return remotes;
}

QStringList GitWrapper::listUntracked()
{
    m_process.start(QStringLiteral("git"),
                    {QStringLiteral("ls-files"), QStringLiteral("--others"), QStringLiteral("--directory"), QStringLiteral("--exclude-standard")});

    QStringList untracked;
    while (m_process.waitForReadyRead()) {
        while (m_process.canReadLine()) {
            // One line is one entry.
            const auto line = QString::fromLocal8Bit(m_process.readLine()).trimmed();
            if (line.endsWith(QDir::separator())) {
                untracked << line.chopped(1);
            } else {
                untracked << line;
            }
        }
    }

    return untracked;
}

QString GitWrapper::userName()
{
    QString result;
    char buffer[SMALL_BUFFER_SIZE];
    m_process.start(QStringLiteral("git"), {QStringLiteral("config"), QStringLiteral("--get"), QStringLiteral("user.name")});
    while (m_process.waitForReadyRead()) {
        if (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            result = QString::fromLocal8Bit(buffer).trimmed();
        }
    }
    return result;
}

QString GitWrapper::userEmail()
{
    QString result;
    char buffer[SMALL_BUFFER_SIZE];
    m_process.start(QStringLiteral("git"), {QStringLiteral("config"), QStringLiteral("--get"), QStringLiteral("user.email")});
    while (m_process.waitForReadyRead()) {
        if (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            result = QString::fromLocal8Bit(buffer).trimmed();
        }
    }
    return result;
}

QStringList GitWrapper::branches(int *currentBranchIndex)
{
    QStringList result;
    if (currentBranchIndex != nullptr) {
        *currentBranchIndex = -1;
    }
    m_process.start(QStringLiteral("git"), {QStringLiteral("branch"), QStringLiteral("-a")});
    while (m_process.waitForReadyRead()) {
        char buffer[BUFFER_SIZE];
        while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            const QString branchName = QString::fromLocal8Bit(buffer).mid(2).trimmed();
            // don't list non-branches and HEAD-branches directly pointing to other branches
            if (!branchName.contains(QLatin1String("->")) && !branchName.startsWith(QLatin1Char('('))) {
                result.append(branchName);
                if (currentBranchIndex != nullptr && buffer[0] == '*') {
                    *currentBranchIndex = result.size() - 1;
                }
            }
        }
    }
    return result;
}

void GitWrapper::tagSet(QSet<QString> &result)
{
    m_process.start(QStringLiteral("git"), {QStringLiteral("tag")});
    while (m_process.waitForReadyRead()) {
        char buffer[BUFFER_SIZE];
        while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            const QString tagName = QString::fromLocal8Bit(buffer).trimmed();
            result.insert(tagName);
        }
    }
}

QStringList GitWrapper::tags()
{
    QStringList result;
    m_process.start(QStringLiteral("git"), {QStringLiteral("tag")});
    while (m_process.waitForReadyRead()) {
        char buffer[BUFFER_SIZE];
        while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            const QString tagName = QString::fromLocal8Bit(buffer).trimmed();
            result.append(tagName);
        }
    }
    return result;
}

inline QStringList GitWrapper::remotes(QLatin1String lineEnd)
{
    QStringList result;
    m_process.start(QStringLiteral("git"), {QStringLiteral("remote"), QStringLiteral("-v")});
    while (m_process.waitForReadyRead()) {
        char buffer[BUFFER_SIZE];
        while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            const QString line = QString::fromLocal8Bit(buffer).simplified();
            if (line.endsWith(lineEnd)) {
                result.append(line.section(QLatin1Char(' '), 0, 0));
            }
        }
    }
    return result;
}

QStringList GitWrapper::pullRemotes()
{
    return remotes(QLatin1String("(fetch)"));
}

QStringList GitWrapper::pushRemotes()
{
    return remotes(QLatin1String("(push)"));
}

QString GitWrapper::lastCommitMessage()
{
    QString result;
    char buffer[BUFFER_SIZE];
    m_process.start(QStringLiteral("git"), {QStringLiteral("log"), QStringLiteral("-1")});
    while (m_process.waitForReadyRead()) {
        bool inMessage = false;
        QStringList message;
        while (m_process.readLine(buffer, sizeof(buffer)) > 0) {
            const QString currentLine = QString::fromLocal8Bit(buffer);
            if (inMessage) {
                message << QString::fromLocal8Bit(buffer).trimmed();
            } else if (currentLine.startsWith(QLatin1String("Date:"))) {
                m_process.readLine();
                inMessage = true;
            }
        }
        result = message.join(QLatin1Char('\n'));
    }
    return result;
}
