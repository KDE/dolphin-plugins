/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pushdialog.h"
#include "hgconfig.h"
#include "pathselector.h"
#include "fileviewhgpluginsettings.h"

#include <QStringList>
#include <QString>
#include <QGridLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <KTextEdit>
#include <KLocalizedString>
#include <KMessageBox>


HgPushDialog::HgPushDialog(QWidget *parent):
    HgSyncBaseDialog(HgSyncBaseDialog::PushDialog, parent)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Push Repository"));
    setup();

}

void HgPushDialog::setOptions()
{
    m_optAllowNewBranch = new QCheckBox(xi18nc("@label:checkbox",
                "Allow pushing a new branch"));
    m_optInsecure = new QCheckBox(xi18nc("@label:checkbox",
                "Do not verify server certificate"));
    m_optForce = new QCheckBox(xi18nc("@label:checkbox",
                "Force Push"));
    m_optionGroup = new QGroupBox(xi18nc("@label:group",
                "Options"));

    m_options << m_optForce;
    m_options << m_optAllowNewBranch;
    m_options << m_optInsecure;
}

void HgPushDialog::createChangesGroup()
{
    m_changesGroup = new QGroupBox(xi18nc("@label:group",
                "Outgoing Changes"));    
    QHBoxLayout *hbox = new QHBoxLayout;
    m_outChangesList = new QTableWidget;
    m_changesetInfo = new KTextEdit;

    m_outChangesList->setColumnCount(3);
    m_outChangesList->verticalHeader()->hide();
    m_outChangesList->horizontalHeader()->hide();
    m_outChangesList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_outChangesList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_changesetInfo->setFontFamily(QLatin1String("Monospace"));

    hbox->addWidget(m_outChangesList);
    hbox->addWidget(m_changesetInfo);

    m_changesGroup->setLayout(hbox);
    m_changesGroup->setVisible(false);

    connect(m_outChangesList, &QTableWidget::itemSelectionChanged,
            this, &HgPushDialog::slotOutSelChanged);
    connect(this, &HgSyncBaseDialog::changeListAvailable,
            this, &HgPushDialog::slotUpdateChangesGeometry);
}

void HgPushDialog::slotOutSelChanged()
{
    if (m_hgw->isBusy()) {
        return;
    }

    QString changeset = m_outChangesList->item(m_outChangesList->currentRow(), 0)->text().split(QLatin1Char(' '), Qt::SkipEmptyParts).takeLast();

    const QStringList args{
        QStringLiteral("-r"),
        changeset,
        QStringLiteral("-v"),
        QStringLiteral("-p"),
    };

    QString output;
    m_hgw->executeCommand(QStringLiteral("log"), args, output);
    m_changesetInfo->clear();
    m_changesetInfo->setText(output);
}

void HgPushDialog::getHgChangesArguments(QStringList &args)
{
    args << QStringLiteral("outgoing");
    args << m_pathSelector->remote();
    args << QStringLiteral("--config");
    args << QStringLiteral("ui.verbose=False");
    args << QStringLiteral("--template");
    args << QStringLiteral("Commit: {rev}:{node|short}   "
                    "{date|isodate}   {desc|firstline}\n");
}

void HgPushDialog::parseUpdateChanges(const QString &input)
{
    QStringList list = input.split(QLatin1String("  "), Qt::SkipEmptyParts);
    QTableWidgetItem *changeset = new QTableWidgetItem;
    QTableWidgetItem *date = new QTableWidgetItem;
    QTableWidgetItem *summary = new QTableWidgetItem;

    changeset->setForeground(Qt::red);
    date->setForeground(Qt::blue);

    changeset->setText(list.takeFirst());
    date->setText(list.takeFirst());
    summary->setText(list.takeFirst());

    int rowCount = m_outChangesList->rowCount();
    m_outChangesList->insertRow(rowCount);
    m_outChangesList->setItem(rowCount, 0, changeset);
    m_outChangesList->setItem(rowCount, 1, date);
    m_outChangesList->setItem(rowCount, 2, summary);
}

void HgPushDialog::appendOptionArguments(QStringList &args)
{
    if (m_optForce->isChecked()) {
        args << QStringLiteral("--force");
    }
    if (m_optAllowNewBranch->isChecked()) {
        args << QStringLiteral("--new-branch");
    }
    if (m_optInsecure->isChecked()) {
        args << QStringLiteral("--insecure");
    }
}

void HgPushDialog::slotUpdateChangesGeometry()
{
    m_outChangesList->resizeColumnsToContents();
    m_outChangesList->resizeRowsToContents();
    m_outChangesList->horizontalHeader()->setStretchLastSection(true);
}

void HgPushDialog::readBigSize()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    m_bigSize = QSize(settings->pushDialogBigWidth(), settings->pushDialogBigHeight());
}

void HgPushDialog::writeBigSize()
{
    qDebug() << "Saving geometry";
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setPushDialogBigWidth(m_bigSize.width());
    settings->setPushDialogBigHeight(m_bigSize.height());
    settings->save();
}

void HgPushDialog::noChangesMessage()
{
    KMessageBox::information(this, xi18nc("@message:info",
                "No outgoing changes!"));
}



#include "moc_pushdialog.cpp"
