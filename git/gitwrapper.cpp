/*
    SPDX-FileCopyrightText: 2010 Sebastian Doerner <sebastian@sebastian-doerner.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gitwrapper.h"

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
