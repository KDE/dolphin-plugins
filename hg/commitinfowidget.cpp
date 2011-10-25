/***************************************************************************
 *   Copyright (C) 2011 by Vishesh Yadav <vishesh3y@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "commitinfowidget.h"
#include "commititemdelegate.h"
#include "hgwrapper.h"

#include <QtCore/QString>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListWidget>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/EditorChooser>
#include <KTextEditor/Cursor>
#include <kmessagebox.h>
#include <klocale.h>

HgCommitInfoWidget::HgCommitInfoWidget(QWidget *parent) :
    QWidget(parent)
{
    setupUI();

    connect(m_commitListWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotUpdateInfo()));
}

void HgCommitInfoWidget::setupUI()
{
    m_commitListWidget = new QListWidget;
    m_commitListWidget->setItemDelegate(new CommitItemDelegate);

    KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
    if (!editor) {
        KMessageBox::error(this, 
                i18n("A KDE text-editor component could not be found;"
                     "\nplease check your KDE installation."));
        return;
    }
    m_editorDoc = editor->createDocument(0);
    m_editorView = qobject_cast<KTextEditor::View*>(m_editorDoc->createView(this));
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

#include "commitinfowidget.h"

