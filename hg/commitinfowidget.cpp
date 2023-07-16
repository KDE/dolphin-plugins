/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "commitinfowidget.h"
#include "commititemdelegate.h"
#include "hgwrapper.h"

#include <QString>
#include <QHBoxLayout>
#include <QListWidget>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/Cursor>
#include <KLocalizedString>
#include <KMessageBox>

HgCommitInfoWidget::HgCommitInfoWidget(QWidget *parent) :
    QWidget(parent)
{
    setupUI();

    connect(m_commitListWidget, &QListWidget::itemSelectionChanged,
            this, &HgCommitInfoWidget::slotUpdateInfo);
}

void HgCommitInfoWidget::setupUI()
{
    m_commitListWidget = new QListWidget;
    m_commitListWidget->setItemDelegate(new CommitItemDelegate);

    KTextEditor::Editor *editor = KTextEditor::Editor::instance();
    if (!editor) {
        KMessageBox::error(this, 
                i18n("A KDE text-editor component could not be found;"
                     "\nplease check your KDE installation."));
        return;
    }
    m_editorDoc = editor->createDocument(nullptr);
    m_editorView = qobject_cast<KTextEditor::View*>(m_editorDoc->createView(this));
    m_editorView->setStatusBarEnabled(false);
    m_editorDoc->setReadWrite(false);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_commitListWidget, 1);
    layout->addWidget(m_editorView, 2);

    setLayout(layout);
}

void HgCommitInfoWidget::addItem(const QString &revision, 
                                 const QString &changeset,
                                 const QString &branch,
                                 const QString &author,
                                 const QString &log)
{

    QListWidgetItem *item = new QListWidgetItem;
    item->setData(Qt::DisplayRole, changeset);
    item->setData(Qt::UserRole + 1, revision);
    item->setData(Qt::UserRole + 2, branch);
    item->setData(Qt::UserRole + 3, author);
    item->setData(Qt::UserRole + 4, log);
    m_commitListWidget->addItem(item);
}

void HgCommitInfoWidget::addItem(QListWidgetItem *item)
{
    m_commitListWidget->addItem(item);
}

QListWidgetItem* HgCommitInfoWidget::currentItem() const
{
    return m_commitListWidget->currentItem();
}

const QString HgCommitInfoWidget::selectedChangeset() const
{
    return m_commitListWidget->currentItem()->data(Qt::DisplayRole).toString();
}

void HgCommitInfoWidget::clear() const
{
    m_commitListWidget->clear();
}

void HgCommitInfoWidget::slotUpdateInfo()
{
    HgWrapper *hgw = HgWrapper::instance();
    QString changeset = selectedChangeset();
    QString output;
    QStringList args;

    args << QLatin1String("-p");
    args << QLatin1String("-v");
    args << QLatin1String("-r");
    args << changeset;
    hgw->executeCommand(QLatin1String("log"), args, output);

    m_editorDoc->setReadWrite(true);
    m_editorDoc->setModified(false);
    m_editorDoc->closeUrl(true);
    m_editorDoc->setText(output);
    m_editorDoc->setHighlightingMode("diff");
    m_editorView->setCursorPosition( KTextEditor::Cursor(0, 0) );
    m_editorDoc->setReadWrite(false);
}

void HgCommitInfoWidget::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    m_commitListWidget->setSelectionMode(mode);
}

QList<QListWidgetItem*> HgCommitInfoWidget::selectedItems() const
{
    return m_commitListWidget->selectedItems();
}


#include "moc_commitinfowidget.cpp"
