/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backoutdialog.h"
#include "commitinfowidget.h"
#include "fileviewhgpluginsettings.h"
#include "hgwrapper.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QCheckBox>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QProcess>
#include <QVBoxLayout>
#include <QWidget>

HgBackoutDialog::HgBackoutDialog(QWidget *parent)
    : DialogBase(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, parent)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window", "<application>Hg</application> Backout"));

    okButton()->setText(xi18nc("@action:button", "Backout"));
    okButton()->setDisabled(true);

    //
    setupUI();

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->resize(QSize(settings->backoutDialogWidth(), settings->backoutDialogHeight()));

    // connections
    connect(this, SIGNAL(finished(int)), this, SLOT(saveGeometry()));
    connect(m_selectBaseCommitButton, &QAbstractButton::clicked, this, &HgBackoutDialog::slotSelectBaseChangeset);
    connect(m_selectParentCommitButton, &QAbstractButton::clicked, this, &HgBackoutDialog::slotSelectParentChangeset);
    connect(m_baseRevision, &QLineEdit::textChanged, this, &HgBackoutDialog::slotUpdateOkButton);
}

void HgBackoutDialog::setupUI()
{
    m_mainGroup = new QGroupBox;
    m_baseRevision = new QLineEdit;
    m_parentRevision = new QLineEdit;
    m_optMerge = new QCheckBox(xi18nc("@label:checkbox", "Merge with old dirstate parent after backout"));
    m_selectParentCommitButton = new QPushButton(xi18nc("@label:button", "Select Changeset"));
    m_selectBaseCommitButton = new QPushButton(xi18nc("@label:button", "Select Changeset"));
    QGridLayout *mainGroupLayout = new QGridLayout;

    mainGroupLayout->addWidget(new QLabel(xi18nc("@label", "Revision to Backout: ")), 0, 0);
    mainGroupLayout->addWidget(m_baseRevision, 0, 1);
    mainGroupLayout->addWidget(m_selectBaseCommitButton, 0, 2);

    mainGroupLayout->addWidget(new QLabel(xi18nc("@label", "Parent Revision (optional): ")), 1, 0);
    mainGroupLayout->addWidget(m_parentRevision, 1, 1);
    mainGroupLayout->addWidget(m_selectParentCommitButton, 1, 2);

    mainGroupLayout->addWidget(m_optMerge, 2, 0, 1, 0);

    m_mainGroup->setLayout(mainGroupLayout);

    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(m_mainGroup);
    layout()->insertLayout(0, lay);
}

void HgBackoutDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setBackoutDialogHeight(this->height());
    settings->setBackoutDialogWidth(this->width());
    settings->save();
}

void HgBackoutDialog::loadCommits()
{
    HgWrapper *hgWrapper = HgWrapper::instance();

    // update heads list
    QProcess process;
    process.setWorkingDirectory(hgWrapper->getBaseDir());

    QStringList args;
    args << QLatin1String("log");
    args << QLatin1String("--template");
    args << QLatin1String(
        "{rev}\n{node|short}\n{branch}\n"
        "{author}\n{desc|firstline}\n");

    process.start(QLatin1String("hg"), args);
    process.waitForFinished();
    m_commitInfo->clear();

    const int FINAL = 5;
    char buffer[FINAL][1024];
    int count = 0;
    while (process.readLine(buffer[count], sizeof(buffer[count])) > 0) {
        if (count == FINAL - 1) {
            QString rev = QString::fromLocal8Bit(buffer[0]).trimmed();
            QString changeset = QString::fromLocal8Bit(buffer[1]).trimmed();
            QString branch = QString::fromLocal8Bit(buffer[2]).trimmed();
            QString author = QString::fromLocal8Bit(buffer[3]).trimmed();
            QString log = QString::fromLocal8Bit(buffer[4]).trimmed();

            QListWidgetItem *item = new QListWidgetItem;
            item->setData(Qt::DisplayRole, changeset);
            item->setData(Qt::UserRole + 1, rev);
            item->setData(Qt::UserRole + 2, branch);
            item->setData(Qt::UserRole + 3, author);
            item->setData(Qt::UserRole + 4, log);
            m_commitInfo->addItem(item);
        }
        count = (count + 1) % FINAL;
    }
}

QString HgBackoutDialog::selectChangeset()
{
    DialogBase diag(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    diag.setWindowTitle(xi18nc("@title:window", "Select Changeset"));
    diag.okButton()->setText(xi18nc("@action:button", "Select"));

    diag.setMinimumWidth(700);

    m_commitInfo = new HgCommitInfoWidget;
    loadCommits();
    diag.layout()->insertWidget(0, m_commitInfo);

    if (diag.exec() == QDialog::Accepted) {
        return m_commitInfo->selectedChangeset();
    }
    return QString();
}

void HgBackoutDialog::slotSelectBaseChangeset()
{
    QString changeset = selectChangeset();
    if (!changeset.isEmpty()) {
        m_baseRevision->setText(changeset);
    }
}

void HgBackoutDialog::slotSelectParentChangeset()
{
    QString changeset = selectChangeset();
    if (!changeset.isEmpty()) {
        m_parentRevision->setText(changeset);
    }
}

void HgBackoutDialog::done(int r)
{
    if (r == QDialog::Accepted) {
        HgWrapper *hgw = HgWrapper::instance();
        QStringList args;
        args << QLatin1String("--rev");
        args << m_baseRevision->text();

        if (!m_parentRevision->text().isEmpty()) {
            args << QLatin1String("--parent");
            args << m_parentRevision->text();
        }

        if (m_optMerge->checkState() == Qt::Checked) {
            args << QLatin1String("--merge");
        }

        if (hgw->executeCommandTillFinished(QLatin1String("backout"), args)) {
            KMessageBox::information(this, hgw->readAllStandardOutput());
            QDialog::done(r);
        } else {
            KMessageBox::error(this, hgw->readAllStandardError());
        }
    } else {
        QDialog::done(r);
    }
}

void HgBackoutDialog::slotUpdateOkButton(const QString &text)
{
    qDebug() << "text" << text;
    okButton()->setEnabled(!text.isEmpty());
}

#include "moc_backoutdialog.cpp"
