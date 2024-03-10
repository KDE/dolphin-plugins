/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "importdialog.h"
#include "fileviewhgpluginsettings.h"
#include "commititemdelegate.h"
#include "hgwrapper.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <KLocalizedString>
#include <KMessageBox>

HgImportDialog::HgImportDialog(QWidget *parent) :
    DialogBase(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, parent)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Import"));
    okButton()->setText(xi18nc("@action:button", "Import"));

    //
    setupUI();

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->resize(QSize(settings->importDialogWidth(),
                               settings->importDialogHeight()));

    //
    connect(this, SIGNAL(finished(int)), this, SLOT(saveGeometry()));
    connect(m_addPatches, &QAbstractButton::clicked,
            this, &HgImportDialog::slotAddPatches);
    connect(m_removePatches, &QAbstractButton::clicked,
            this, &HgImportDialog::slotRemovePatches);
}

void HgImportDialog::setupUI()
{
    QGroupBox *mainGroup = new QGroupBox;
    QGridLayout *mainLayout = new QGridLayout;
    m_patchList = new QListWidget;

    m_patchList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_patchList->setItemDelegate(new CommitItemDelegate);
    mainLayout->addWidget(m_patchList);
    mainGroup->setLayout(mainLayout);

    // options
    m_optionGroup = new QGroupBox(xi18nc("@label:group", "Options"));
    m_optNoCommit = new QCheckBox(xi18nc("@label",
                      "Do not commit, just update the working directory"));
    m_optForce = new QCheckBox(xi18nc("@label",
                      "Skip test for outstanding uncommitted changes"));
    m_optExact = new QCheckBox(xi18nc("@label",
                   "Apply patch to the nodes from which it was generated"));
    m_optBypass = new QCheckBox(xi18nc("@label",
                      "Apply patch without touching working directory"));

    QVBoxLayout *optionLayout = new QVBoxLayout;
    optionLayout->addWidget(m_optNoCommit);
    optionLayout->addWidget(m_optForce);
    optionLayout->addWidget(m_optExact);
    optionLayout->addWidget(m_optBypass);
    m_optionGroup->setLayout(optionLayout);

    // top buttons
    QHBoxLayout *topButtons = new QHBoxLayout;
    m_addPatches = new QPushButton(xi18nc("@label:button",
                        "Add Patches"));
    m_removePatches = new QPushButton(xi18nc("@label:button",
                        "Remove Patches"));
    topButtons->addWidget(m_addPatches);
    topButtons->addWidget(m_removePatches);
    topButtons->addStretch();

    //setup main dialog widget
    QVBoxLayout *lay = new QVBoxLayout;
    lay->addLayout(topButtons);
    lay->addWidget(mainGroup);
    lay->addWidget(m_optionGroup);
    layout()->insertLayout(0, lay);
}

void HgImportDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setImportDialogHeight(this->height());
    settings->setImportDialogWidth(this->width());
    settings->save();
}

void HgImportDialog::done(int r)
{
    if (r == QDialog::Accepted) {
        QStringList args;
        if (m_optForce->checkState() == Qt::Checked) {
            args << QLatin1String("--force");
        }

        if (m_optBypass->checkState() == Qt::Checked) {
            args << QLatin1String("--bypass");
        }

        if (m_optNoCommit->checkState() == Qt::Checked) {
            args << QLatin1String("--no-commit");
        }
        if (m_optExact->checkState() == Qt::Checked) {
            args << QLatin1String("--exact");
        }

        int countRows = m_patchList->count();
        for (int i=0; i<countRows; i++) {
            QListWidgetItem *item = m_patchList->item(i);
            args << item->data(Qt::UserRole + 5).toString();
        }

        HgWrapper *hgw = HgWrapper::instance();
        if (hgw->executeCommandTillFinished(QLatin1String("import"), args)) {
            QDialog::done(r);
        }
        else {
            KMessageBox::error(this, hgw->readAllStandardError());
        }
    }
    else {
        QDialog::done(r);
    }
}

void HgImportDialog::getPatchInfo(const QString &fileName)
{
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QTextStream fileStream(&file);

    QListWidgetItem *item = new QListWidgetItem;
    item->setData(Qt::UserRole + 1, QString());
    item->setData(Qt::UserRole + 2, QString());
    item->setData(Qt::UserRole + 5, fileName);

    bool gotInfo = false;

    do {
        QString line = fileStream.readLine();
        if (line.startsWith(QLatin1String("diff"))) {
            break;
        }
        else if (line.startsWith(QLatin1String("# User"))) {
            item->setData(Qt::UserRole + 3, 
                    line.remove(QLatin1String("# User")).trimmed());
        }
        else if (line.startsWith(QLatin1String("# Node ID"))) {
            QString node = line.remove(QLatin1String("# Node ID")).trimmed();
            if (!m_patchList->findItems(node, Qt::MatchExactly).empty()) {
                return;
            }
            item->setData(Qt::DisplayRole, node);
        }
        else if (line.startsWith(QLatin1String("# Parent"))) {
            gotInfo = true;
        }
        else if (gotInfo) {
            item->setData(Qt::UserRole + 4, line.trimmed());
            break;
        }
    } while (!fileStream.atEnd());

    m_patchList->addItem(item);

    file.close();
}

void HgImportDialog::slotAddPatches()
{
    const QStringList patches = QFileDialog::getOpenFileNames(this);
    for (const QString &fileName : patches) {
        getPatchInfo(fileName);
    }
}

void HgImportDialog::slotRemovePatches()
{
    int count = m_patchList->count();
    for (int i=0; i<count; i++) {
        m_patchList->takeItem(i);
    }
}



#include "moc_importdialog.cpp"
