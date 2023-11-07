/*
    SPDX-FileCopyrightText: 2019-2020 Nikolai Krasheninnikov <nkrasheninnikov@yandex.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "svncommitdialog.h"

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QTemporaryFile>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QShortcut>
#include <QMenu>
#include <QDebug>

#include <KLocalizedString>
#include <KWindowConfig>
#include <KSharedConfig>

namespace {

// Helper function: returns true if str starts with any string in a list.
bool startsWith(const QStringList &list, const QString &str)
{
    for (const auto &i : qAsConst(list)) {
        if (str.startsWith(i)) {
            return true;
        }
    }

    return false;
}

// Helper function: makes a new list from an existing one and a hashTable. It combines all the list
// records which exists in hashTable. Existence means hashTable entry starts with a list entry.
QStringList makeContext(const QStringList &list, const QHash<QString, KVersionControlPlugin::ItemVersion> *hashTable)
{
    QStringList ret;

    for (const auto &i : qAsConst(list)) {
        for ( auto it = hashTable->cbegin(); it != hashTable->cend(); ++it ) {
            if (it.key().startsWith(i)) {
                ret.append(i);
                break;
            }
        }
    }

    return ret;
}

}

struct svnCommitEntryInfo_t {
    svnCommitEntryInfo_t() :
        localPath(QString()),
        fileVersion( KVersionControlPlugin::NormalVersion )
    {}

    QString localPath;                              ///< Affected local path.
    KVersionControlPlugin::ItemVersion fileVersion; ///< File status in terms of KVersionControlPlugin
};
Q_DECLARE_METATYPE(svnCommitEntryInfo_t);

enum columns_t {
    columnPath,
    columnStatus
};

SvnCommitDialog::SvnCommitDialog(const QHash<QString, KVersionControlPlugin::ItemVersion> *versionInfo, const QStringList& context, QWidget *parent) :
    QDialog(parent),
    m_versionInfoHash(versionInfo),
    m_context(context)
{
    Q_ASSERT(versionInfo);
    Q_ASSERT(!context.empty());

    /*
     * Setup UI.
     */
    QVBoxLayout* boxLayout = new QVBoxLayout(this);

    boxLayout->addWidget(new QLabel(i18nc("@label", "Description:"), this));
    m_editor = new QPlainTextEdit(this);
    boxLayout->addWidget(m_editor, 1);

    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    boxLayout->addWidget(line);

    m_changes = new QTableWidget(this);
    boxLayout->addWidget(m_changes, 3);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    // The more appropriate role is QDialogButtonBox::ActionRole but we use QDialogButtonBox::ResetRole
    // because of QDialogButtonBox automatic button layout (button is separated from others).
    auto refreshButton = buttonBox->addButton(i18nc("@action:button", "Refresh"), QDialogButtonBox::ResetRole);
    refreshButton->setIcon(QIcon::fromTheme("view-refresh"));

    auto okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setText(i18nc("@action:button", "Commit"));
    boxLayout->addWidget(buttonBox);

    /*
     * Add actions, establish connections.
     */
    m_actRevertFile = new QAction(i18nc("@item:inmenu", "Revert"), this);
    m_actRevertFile->setIcon(QIcon::fromTheme("document-revert"));
    connect(m_actRevertFile, &QAction::triggered, this, [this] () {
        const QString filePath = m_actRevertFile->data().value<svnCommitEntryInfo_t>().localPath;
        Q_EMIT revertFiles(QStringList() << filePath);
    } );

    m_actDiffFile = new QAction(i18nc("@item:inmenu", "Show changes"), this);
    m_actDiffFile->setIcon(QIcon::fromTheme("view-split-left-right"));
    connect(m_actDiffFile, &QAction::triggered, this, [this] () {
        const QString filePath = m_actDiffFile->data().value<svnCommitEntryInfo_t>().localPath;
        Q_EMIT diffFile(filePath);
    } );

    m_actAddFile = new QAction(i18nc("@item:inmenu", "Add file"), this);
    m_actAddFile->setIcon(QIcon::fromTheme("list-add"));
    connect(m_actAddFile, &QAction::triggered, this, [this] () {
        const QString filePath = m_actAddFile->data().value<svnCommitEntryInfo_t>().localPath;
        Q_EMIT addFiles(QStringList() << filePath);
    } );

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this] () {
        // Form a new context list from an existing one and a possibly modified m_versionInfoHash (some
        // files from original context might no longer be in m_versionInfoHash).
        QStringList context = makeContext(m_context, m_versionInfoHash);
        Q_EMIT commit(context, m_editor->toPlainText());
        QDialog::accept();
    } );
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(refreshButton, &QPushButton::clicked, this, &SvnCommitDialog::refreshChangesList);

    connect(m_changes, &QWidget::customContextMenuRequested, this, &SvnCommitDialog::contextMenu);

    QShortcut *refreshShortcut = new QShortcut(QKeySequence::Refresh, this);
    connect(refreshShortcut, &QShortcut::activated, this, &SvnCommitDialog::refreshChangesList);
    refreshShortcut->setAutoRepeat(false);

    /*
     * Additional setup.
     */
    setWindowTitle(i18nc("@title:window", "SVN Commit"));

    const QStringList tableHeader = { i18nc("@title:column", "Path"),
                                      i18nc("@title:column", "Status") };
    m_changes->setColumnCount(tableHeader.size());
    m_changes->setHorizontalHeaderLabels(tableHeader);
    m_changes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_changes->horizontalHeader()->setSectionResizeMode(columnStatus, QHeaderView::ResizeToContents);
    m_changes->verticalHeader()->setVisible(false);
    m_changes->setSortingEnabled(false);
    m_changes->setSelectionMode(QAbstractItemView::SingleSelection);
    m_changes->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_changes->setContextMenuPolicy(Qt::CustomContextMenu);

    refreshChangesList();
}

SvnCommitDialog::~SvnCommitDialog()
{
    KConfigGroup dialogConfig(KSharedConfig::openConfig("dolphinrc"), QStringLiteral("SvnCommitDialog"));
    KWindowConfig::saveWindowSize(windowHandle(), dialogConfig, KConfigBase::Persistent);
}

void SvnCommitDialog::refreshChangesList()
{
    // Remove all the contents.
    m_changes->clearContents();
    m_changes->setRowCount(0);

    auto it = m_versionInfoHash->cbegin();
    for ( int row = 0 ; it != m_versionInfoHash->cend(); ++it ) {
        // If current item is not in a context list we skip it. Each file must be in a context dir
        // or be in a context itself.
        if (!startsWith(m_context, it.key())) {
            continue;
        }

        QTableWidgetItem *path = new QTableWidgetItem( it.key() );
        QTableWidgetItem *status = new QTableWidgetItem;

        path->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        status->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        m_changes->insertRow(row);
        m_changes->setItem(row, columnPath, path);
        m_changes->setItem(row, columnStatus, status);
        row++;

        svnCommitEntryInfo_t info;
        info.localPath = it.key();
        info.fileVersion = it.value();
        path->setData(Qt::UserRole, QVariant::fromValue(info));
        status->setData(Qt::UserRole, QVariant::fromValue(info));

        switch(it.value()) {
        case KVersionControlPlugin::UnversionedVersion:
            status->setText( i18nc("@item:intable", "Unversioned") );
            break;
        case KVersionControlPlugin::LocallyModifiedVersion:
            status->setText( i18nc("@item:intable", "Modified") );
            break;
        case KVersionControlPlugin::AddedVersion:
            status->setText( i18nc("@item:intable", "Added") );
            break;
        case KVersionControlPlugin::RemovedVersion:
            status->setText( i18nc("@item:intable", "Deleted") );
            break;
        case KVersionControlPlugin::ConflictingVersion:
            status->setText( i18nc("@item:intable", "Conflict") );
            break;
        case KVersionControlPlugin::MissingVersion:
            status->setText( i18nc("@item:intable", "Missing") );
            break;
        case KVersionControlPlugin::UpdateRequiredVersion:
            status->setText( i18nc("@item:intable", "Update required") );
            break;
        default:
            // For SVN normaly we shouldn't be here with:
            // NormalVersion, LocallyModifiedUnstagedVersion, IgnoredVersion.
            // 'default' is for any future changes in ItemVersion enum.
            qWarning() << QString("Unknown SVN status for item %1, ItemVersion = %2").arg(it.key()).arg(it.value());
            status->setText("");
        }
    }

    // Sort by status: unversioned is at the bottom.
    m_changes->sortByColumn(columnStatus, Qt::AscendingOrder);
}

void SvnCommitDialog::show()
{
    QWidget::show();

    // Restore window size after show() for workaround for QTBUG-40584. See KWindowConfig::restoreWindowSize() docs.
    KConfigGroup dialogConfig(KSharedConfig::openConfig("dolphinrc"), QStringLiteral("SvnCommitDialog"));
    KWindowConfig::restoreWindowSize(windowHandle(), dialogConfig);
}

void SvnCommitDialog::contextMenu(const QPoint& pos)
{
    QTableWidgetItem *item = m_changes->item( m_changes->currentRow(), 0 );
    if (item == nullptr) {
        return;
    }

    const QVariant data = item->data(Qt::UserRole);
    m_actRevertFile->setData( data );
    m_actDiffFile->setData( data );
    m_actAddFile->setData( data );

    m_actRevertFile->setEnabled(false);
    m_actDiffFile->setEnabled(false);
    m_actAddFile->setEnabled(false);

    const svnCommitEntryInfo_t info = data.value<svnCommitEntryInfo_t>();
    switch(info.fileVersion) {
    case KVersionControlPlugin::UnversionedVersion:
        m_actAddFile->setEnabled(true);
        break;
    case KVersionControlPlugin::LocallyModifiedVersion:
        m_actRevertFile->setEnabled(true);
        m_actDiffFile->setEnabled(true);
        break;
    case KVersionControlPlugin::AddedVersion:
    case KVersionControlPlugin::RemovedVersion:
        m_actRevertFile->setEnabled(true);
        break;
    case KVersionControlPlugin::MissingVersion:
        m_actRevertFile->setEnabled(true);
        break;
    default:
        // For this items we don't show any menu: return now.
        return;
    }

    QMenu *menu = new QMenu(this);
    menu->addAction(m_actAddFile);
    menu->addAction(m_actRevertFile);
    menu->addAction(m_actDiffFile);

    // Adjust popup menu position for QTableWidget header height.
    const QPoint popupPoint = QPoint(pos.x(), pos.y() + m_changes->horizontalHeader()->height());
    menu->exec( m_changes->mapToGlobal(popupPoint) );
}
