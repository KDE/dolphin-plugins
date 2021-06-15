/*
    SPDX-FileCopyrightText: 2019-2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "svnlogdialog.h"

#include <QProcess>
#include <QMenu>
#include <QTemporaryFile>
#include <QShortcut>

#include "svncommands.h"

namespace {

// Helper function: safe revert to a revision of a possibly modified file. If file could not be
// reverted (for example, repo is inaccessible) it preserves current file.
bool resetAndRevertFileToRevision(const QString &filePath, ulong revision)
{
    QTemporaryFile file;
    if (!file.open()) {
        return false;
    }

    bool preserveFile = true;
    QFile copyFile(filePath);
    if (!copyFile.open(QIODevice::ReadOnly)) {
        preserveFile = false;
    } else {
        const QByteArray data = copyFile.readAll();
        if (file.write(data) != data.size() || !file.flush()) {
            preserveFile = false;
        }
    }

    if (!SvnCommands::revertLocalChanges(filePath)) {
        return false;
    }
    if (!SvnCommands::revertToRevision(filePath, revision)) {
        if (preserveFile) {
            QFile::remove(filePath);
            QFile::copy(file.fileName(), filePath);
        }
        return false;
    }

    return true;
}

}

struct svnLogEntryInfo_t {
    svnLogEntryInfo_t() :
        remotePath(QString()),
        localPath(QString()),
        revision(0)
    {}

    QString remotePath;         ///< Affected remote path.
    QString localPath;          ///< Affected local path.
    ulong revision;             ///< Revision number.
};
Q_DECLARE_METATYPE(svnLogEntryInfo_t);

enum columns_t {
    columnRevision,
    columnAuthor,
    columnDate,
    columnMessage
};

SvnLogDialog::SvnLogDialog(const QString& contextDir, QWidget *parent) :
    QDialog(parent),
    m_contextDir(contextDir),
    m_logLength(100)
{
    m_ui.setupUi(this);

    /*
     * Add actions, establish connections.
     */
    QObject::connect(m_ui.pbOk, &QPushButton::clicked, this, &QWidget::close);
    QObject::connect(m_ui.pbRefresh, &QPushButton::clicked, this, &SvnLogDialog::refreshLog);
    QObject::connect(m_ui.pbNext100, &QPushButton::clicked, this, [this] () {
        m_logLength += 100;
        refreshLog();
    } );

    QObject::connect(m_ui.tLog, &QWidget::customContextMenuRequested, this, &SvnLogDialog::showContextMenuLog);
    QObject::connect(m_ui.lPaths, &QWidget::customContextMenuRequested, this, &SvnLogDialog::showContextMenuChangesList);

    m_updateToRev = new QAction(i18n("Update to revision"), this);
    m_updateToRev->setIcon(QIcon::fromTheme("view-refresh"));
    QObject::connect(m_updateToRev, &QAction::triggered, this, &SvnLogDialog::updateRepoToRevision);

    m_revertToRev = new QAction(i18n("Revert to revision"), this);
    m_revertToRev->setIcon(QIcon::fromTheme("document-revert"));
    QObject::connect(m_revertToRev, &QAction::triggered, this, &SvnLogDialog::revertRepoToRevision);

    m_diffFilePrev = new QAction(i18n("Show changes"), this);
    m_diffFilePrev->setIcon(QIcon::fromTheme("view-split-left-right"));
    QObject::connect(m_diffFilePrev, &QAction::triggered, this, [this] () {
        svnLogEntryInfo_t info = m_diffFilePrev->data().value<svnLogEntryInfo_t>();
        Q_EMIT diffBetweenRevs(info.remotePath, info.revision, info.revision - 1);
    } );

    m_diffFileCurrent = new QAction(i18n("Changes against working copy"), this);
    m_diffFileCurrent->setIcon(QIcon::fromTheme("view-split-left-right"));
    QObject::connect(m_diffFileCurrent, &QAction::triggered, this, [this] () {
        svnLogEntryInfo_t info = m_diffFileCurrent->data().value<svnLogEntryInfo_t>();
        Q_EMIT diffAgainstWorkingCopy(info.localPath, info.revision);
    } );

    m_fileRevertToRev = new QAction(i18n("Revert to revision"), this);
    m_fileRevertToRev->setIcon(QIcon::fromTheme("document-revert"));
    QObject::connect(m_fileRevertToRev, &QAction::triggered, this, &SvnLogDialog::revertFileToRevision);

    QShortcut *refreshShortcut = new QShortcut(QKeySequence::Refresh, this);
    QObject::connect(refreshShortcut, &QShortcut::activated, this, &SvnLogDialog::refreshLog);
    refreshShortcut->setAutoRepeat(false);

    /*
     * Additional setup.
     */
    m_ui.tLog->horizontalHeader()->setSectionResizeMode(columnDate, QHeaderView::ResizeToContents);

    refreshLog();
}

SvnLogDialog::~SvnLogDialog() = default;

void SvnLogDialog::setCurrentRevision(ulong revision)
{
    if (m_log.isNull()) {
        return;
    }

    for (int i = 0; i < m_log->size(); ++i) {
        if (m_log->at(i).revision == revision) {
            QFont font;
            font.setBold(true);

            m_ui.tLog->item(i, columnRevision)->setFont(font);
            m_ui.tLog->item(i, columnAuthor)->setFont(font);
            m_ui.tLog->item(i, columnDate)->setFont(font);
            m_ui.tLog->item(i, columnMessage)->setFont(font);

            m_ui.tLog->selectRow(i);

            break;
        }
    }
}

void SvnLogDialog::refreshLog()
{
    m_log = SvnCommands::getLog(m_contextDir, m_logLength);

    if (m_log.isNull()) {
        return;
    }

    m_ui.tLog->clearContents();
    m_ui.teMessage->clear();
    m_ui.lPaths->clear();

    m_ui.tLog->setRowCount( m_log->size() );
    for (int i = 0; i < m_log->size(); ++i) {
        QTableWidgetItem *revision = new QTableWidgetItem(QString::number(m_log->at(i).revision));
        QTableWidgetItem *author = new QTableWidgetItem(m_log->at(i).author);
        QTableWidgetItem *date = new QTableWidgetItem(m_log->at(i).date.toString("yyyy.MM.dd hh:mm:ss"));
        QTableWidgetItem *msg = new QTableWidgetItem(m_log->at(i).msg);

        revision->setData(Qt::UserRole, QVariant::fromValue(m_log->at(i).revision));

        m_ui.tLog->setItem(i, columnRevision, revision);
        m_ui.tLog->setItem(i, columnAuthor, author);
        m_ui.tLog->setItem(i, columnDate, date);
        m_ui.tLog->setItem(i, columnMessage, msg);
    }

    SvnLogDialog::setCurrentRevision( SvnCommands::localRevision(m_contextDir) );
}

void SvnLogDialog::on_tLog_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)

    if (currentRow < 0) {
        return;
    }
    if (m_log.isNull()) {
        return;
    }
    if (m_log->size() < currentRow) {
        return;
    }
    if (m_log->empty()) {
        return;
    }

    const QString rootUrl = SvnCommands::remoteRootUrl(m_contextDir);
    if (rootUrl.isEmpty()) {
        return;
    }

    m_ui.teMessage->setPlainText( m_log->at(currentRow).msg );
    m_ui.lPaths->clear();

    for (const auto &i : qAsConst(m_log->at(currentRow).affectedPaths)) {
        svnLogEntryInfo_t info;
        info.remotePath = rootUrl + i.path;
        info.localPath = m_contextDir + i.path;
        info.revision = m_log->at(currentRow).revision;

        // For user: let's show relative path from 'svn log'.
        QListWidgetItem *item = new QListWidgetItem(i.path, m_ui.lPaths);
        item->setData(Qt::UserRole, QVariant::fromValue(info));
        m_ui.lPaths->addItem(item);
    }
}

void SvnLogDialog::showContextMenuLog(const QPoint &pos)
{
    QTableWidgetItem *item = m_ui.tLog->item( m_ui.tLog->currentRow(), columnRevision );
    if (item == nullptr) {
        return;
    }

    m_updateToRev->setData( item->data(Qt::UserRole) );
    m_revertToRev->setData( item->data(Qt::UserRole) );

    QMenu *menu = new QMenu(this);
    menu->addAction(m_updateToRev);
    menu->addAction(m_revertToRev);

    // Adjust popup menu position for QTableWidget header height.
    const QPoint popupPoint = QPoint(pos.x(), pos.y() + m_ui.tLog->horizontalHeader()->height());
    menu->exec( m_ui.tLog->mapToGlobal(popupPoint) );
}

void SvnLogDialog::showContextMenuChangesList(const QPoint &pos)
{
    QListWidgetItem *item = m_ui.lPaths->currentItem();
    if (item == nullptr) {
        return;
    }
    const svnLogEntryInfo_t info = item->data(Qt::UserRole).value<svnLogEntryInfo_t>();

    m_diffFilePrev->setData( QVariant::fromValue(info) );
    m_diffFileCurrent->setData( QVariant::fromValue(info) );
    m_fileRevertToRev->setData( QVariant::fromValue(info) );

    QMenu *menu = new QMenu(this);
    menu->addAction(m_diffFilePrev);
    menu->addAction(m_diffFileCurrent);
    menu->addAction(m_fileRevertToRev);

    menu->exec( m_ui.lPaths->mapToGlobal(pos) );
}

void SvnLogDialog::updateRepoToRevision()
{
    bool convert = false;
    uint revision = m_updateToRev->data().toUInt(&convert);
    if (!convert || !SvnCommands::updateToRevision(m_contextDir, revision)) {
        Q_EMIT errorMessage(i18nc("@info:status", "SVN log: update to revision failed."));
    } else {
        Q_EMIT operationCompletedMessage(i18nc("@info:status", "SVN log: update to revision %1 successful.", revision));
        SvnLogDialog::refreshLog();
    }
}

void SvnLogDialog::revertRepoToRevision()
{
    bool convert = false;
    uint revision = m_revertToRev->data().toUInt(&convert);
    if (!convert || !SvnCommands::revertToRevision(m_contextDir, revision)) {
        Q_EMIT errorMessage(i18nc("@info:status", "SVN log: revert to revision failed."));
    } else {
        Q_EMIT operationCompletedMessage(i18nc("@info:status", "SVN log: revert to revision %1 successful.", revision));
    }
}

void SvnLogDialog::revertFileToRevision()
{
    svnLogEntryInfo_t info = m_fileRevertToRev->data().value<svnLogEntryInfo_t>();

    if (!resetAndRevertFileToRevision(info.localPath, info.revision)) {
        Q_EMIT errorMessage(i18nc("@info:status", "SVN log: revert to revision failed."));
    } else {
        Q_EMIT operationCompletedMessage(i18nc("@info:status", "SVN log: revert to revision %1 successful.", info.revision));
    }
}
