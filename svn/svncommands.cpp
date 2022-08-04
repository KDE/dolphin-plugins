/*
    SPDX-FileCopyrightText: 2019-2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "svncommands.h"

#include <QProcess>
#include <QTextStream>
#include <QTemporaryFile>
#include <QUrl>
#include <QDir>
#include <QXmlStreamReader>

namespace {

// Helper function: returns template file name for QTemporaryFile.
QString templateFileName(const QString& url, ulong rev)
{
    const QString tmpFileName = url.section('/', -1);

    return QDir::tempPath() + QString("/%1.r%2.XXXXXX").arg(tmpFileName).arg(rev);
}

}

ulong SvnCommands::localRevision(const QString& filePath)
{
    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("info"),
            QStringLiteral("--show-item"),
            QStringLiteral("last-changed-revision"),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return 0;
    }

    QTextStream stream(&process);
    ulong revision = 0;
    stream >> revision;

    if (stream.status() == QTextStream::Ok) {
        return revision;
    } else {
        return 0;
    }
}

ulong SvnCommands::remoteRevision(const QString& filePath)
{
    const QString url = SvnCommands::remoteItemUrl(filePath);

    if (url.isNull()) {
        return 0;
    }

    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("info"),
            QStringLiteral("--show-item"),
            QStringLiteral("last-changed-revision"),
            url
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return 0;
    }

    QTextStream stream(&process);
    ulong revision = 0;
    stream >> revision;

    if (stream.status() == QTextStream::Ok) {
        return revision;
    } else {
        return 0;
    }
}

QString SvnCommands::remoteItemUrl(const QString& filePath)
{
    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("info"),
            QStringLiteral("--show-item"),
            QStringLiteral("url"),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return nullptr;
    }

    QTextStream stream(&process);
    QString url;
    stream >> url;

    if (stream.status() == QTextStream::Ok) {
        return url;
    } else {
        return QString();
    }
}

QString SvnCommands::remoteRootUrl(const QString& filePath)
{
    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("info"),
            QStringLiteral("--show-item"),
            QStringLiteral("repos-root-url"),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return nullptr;
    }

    QTextStream stream(&process);
    QString url;
    stream >> url;

    if (stream.status() == QTextStream::Ok) {
        return url;
    } else {
        return QString();
    }
}

QString SvnCommands::remoteRelativeUrl(const QString& filePath)
{
    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("info"),
            QStringLiteral("--show-item"),
            QStringLiteral("relative-url"),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return nullptr;
    }

    QTextStream stream(&process);
    QString url;
    stream >> url;

    if (stream.status() == QTextStream::Ok) {
        return url;
    } else {
        return QString();
    }
}

QString SvnCommands::localRoot(const QString& filePath)
{
    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("info"),
            QStringLiteral("--show-item"),
            QStringLiteral("wc-root"),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return nullptr;
    }

    QTextStream stream(&process);
    QString wcroot;
    stream >> wcroot;

    if (stream.status() == QTextStream::Ok) {
        return wcroot;
    } else {
        return QString();
    }
}

bool SvnCommands::updateToRevision(const QString& filePath, ulong revision)
{
    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("update"),
            QStringLiteral("-r%1").arg(revision),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return false;
    }

    return true;
}

bool SvnCommands::revertLocalChanges(const QString& filePath)
{
    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("revert"),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return false;
    } else {
        return true;
    }
}

bool SvnCommands::revertToRevision(const QString& filePath, ulong revision)
{
    // TODO: No conflict resolve while merging.

    ulong currentRevision = SvnCommands::localRevision(filePath);
    if (currentRevision == 0) {
        return false;
    }

    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("merge"),
            QStringLiteral("-r%1:%2").arg(currentRevision).arg(revision),
            filePath
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return false;
    }

    return true;
}

CommandResult SvnCommands::cleanup(const QString& dir, bool removeUnversioned, bool removeIgnored, bool includeExternals)
{
    QStringList arguments;
    arguments << QStringLiteral("cleanup") << dir;
    if (removeUnversioned) {
        arguments << QStringLiteral("--remove-unversioned");
    }
    if (removeIgnored) {
        arguments << QStringLiteral("--remove-ignored");
    }
    if (includeExternals) {
        arguments << QStringLiteral("--include-externals");
    }

    QProcess process;
    process.start(
        QLatin1String("svn"),
        arguments
    );

    CommandResult result;
    if (!process.waitForFinished() || process.exitCode() != 0) {
        result.success = false;
    } else {
        result.success = true;
    }
    result.stdOut = process.readAllStandardOutput();
    result.stdErr = process.readAllStandardError();

    return result;
}

bool SvnCommands::exportFile(const QUrl& path, ulong rev, QFileDevice *file)
{
    if (file == nullptr || !path.isValid()) {
        return false;
    }

    QString remoteUrl;
    if (path.isLocalFile()) {
        remoteUrl = remoteItemUrl(path.path());
        if (remoteUrl.isEmpty()) {
            return false;
        }
    } else {
        remoteUrl = path.url();
    }

    if (!file->isOpen() && !file->open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QProcess process;

    process.start(
        QLatin1String("svn"),
        QStringList {
            QStringLiteral("export"),
            QStringLiteral("--force"),
            QStringLiteral("-r%1").arg(rev),
            remoteUrl,
            file->fileName()
        }
    );

    if (!process.waitForFinished() || process.exitCode() != 0) {
        return false;
    } else {
        return true;
    }
}

bool SvnCommands::exportFile(const QUrl& path, ulong rev, QTemporaryFile *file)
{
    if (file == nullptr || !path.isValid()) {
        return false;
    }

    file->setFileTemplate( templateFileName(path.fileName(), rev) );

    return exportFile(path, rev, dynamic_cast<QFileDevice*>(file));
}

QSharedPointer< QVector<logEntry> > SvnCommands::getLog(const QString& filePath, uint maxEntries, ulong fromRevision)
{
    ulong rev = fromRevision;
    if (rev == 0) {
        rev = SvnCommands::remoteRevision(filePath);
        if (rev == 0) {
            return QSharedPointer< QVector<logEntry> >{};
        }
    }

    auto log = QSharedPointer< QVector<logEntry> >::create();
    while (true) {
        // We do 'xml' output as it is the most full output and already in a ready-to-parse format.
        // Max xml svn log is 255 entries. We should do a while here if there is not enough log
        // entries parsed already.
        QProcess process;
        process.start(
            QLatin1String("svn"),
            QStringList {
                QStringLiteral("log"),
                QStringLiteral("-r%1:0").arg(rev),
                QStringLiteral("-l %1").arg(maxEntries),
                QStringLiteral("--verbose"),
                QStringLiteral("--xml"),
                filePath
            }
        );

        if (!process.waitForFinished() || process.exitCode() != 0) {
            process.setReadChannel( QProcess::StandardError );

            // If stderr contains 'E195012' that means repo doesn't exist in the revision range.
            // It's not an error: let's return everything we've got already.
            const QLatin1String errorCode("svn: E195012:"); // Error: 'Unable to find repository location for <path> in revision <revision>'.
            if (QTextStream(&process).readAll().indexOf(errorCode) != -1) {
                return log;
            } else {
                return QSharedPointer< QVector<logEntry> >{};
            }
        }

        QXmlStreamReader xml(&process);
        int itemsAppended = 0;
        if (xml.readNextStartElement() && xml.name() == QLatin1String("log")) {
            while (!xml.atEnd() && xml.readNext() != QXmlStreamReader::EndDocument) {
                if (!xml.isStartElement() || xml.name() != QLatin1String("logentry")) {
                    continue;
                }

                logEntry entry;
                entry.revision = xml.attributes().value("revision").toULong();

                if (xml.readNextStartElement() && xml.name() == QLatin1String("author")) {
                    entry.author = xml.readElementText();
                }
                if (xml.readNextStartElement() && xml.name() == QLatin1String("date")) {
                    entry.date = QDateTime::fromString(xml.readElementText(), Qt::ISODateWithMs);
                }

                if (xml.readNextStartElement() && xml.name() == QLatin1String("paths")) {
                    while (xml.readNextStartElement() && xml.name() == QLatin1String("path")) {
                        affectedPath path;

                        path.action = xml.attributes().value("action").toString();
                        path.propMods = xml.attributes().value("prop-mods").toString() == QLatin1String("true");
                        path.textMods = xml.attributes().value("text-mods").toString() == QLatin1String("true");
                        path.kind = xml.attributes().value("kind").toString();

                        path.path = xml.readElementText();

                        entry.affectedPaths.push_back(path);
                    }
                }

                if (xml.readNextStartElement() && xml.name() == QLatin1String("msg")) {
                    entry.msg = xml.readElementText();
                }

                log->append(entry);
                itemsAppended++;
            }
        }
        if (xml.hasError()) {
            // SVN log output parsing failed.
            return QSharedPointer< QVector<logEntry> >{};
        }

        if (static_cast<uint>(log->size()) >= maxEntries || itemsAppended == 0) {
            break;
        } else {
            rev = log->back().revision - 1;
        }
    }

    return log;
}

bool SvnCommands::checkoutRepository(const QString& url, bool ignoreExternals, const QString& whereto)
{
    QStringList params;
    params.append(QStringLiteral("checkout"));
    params.append(url);
    if (ignoreExternals) {
        params.append(QStringLiteral("--ignore-externals"));
    }
    params.append(whereto);

    QProcess process;
    process.start(QLatin1String("svn"), params);

    // Without timeout because it could be expensive time consuming operation.
    if (!process.waitForFinished(-1) || process.exitCode() != 0) {
        return false;
    } else {
        return true;
    }
}
