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

#include "clonedialog.h"
#include "fileviewhgpluginsettings.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QStackedLayout>
#include <QApplication>
#include <QCheckBox>
#include <QTextCodec>
#include <QFileDialog>
#include <QLineEdit>
#include <KTextEdit>
#include <KLocalizedString>
#include <KMessageBox>

HgCloneDialog::HgCloneDialog(const QString &directory, QWidget *parent):
    DialogBase(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, parent),
    m_cloned(false),
    m_terminated(true),
    m_workingDirectory(directory)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Clone"));
    okButton()->setText(xi18nc("@action:button", "Clone"));
    okButton()->setDisabled(true);


    //////////////
    // Setup UI //
    //////////////

    QGroupBox *urlGroup = new QGroupBox(i18n("URLs"));
    QGridLayout *urlLayout = new QGridLayout;
    QLabel *sourceLabel = new QLabel(xi18nc("@label", "Source"));
    QLabel *destLabel = new QLabel(xi18nc("@lobel", "Destination"));
    QPushButton *m_browse_dest = new QPushButton(xi18nc("@button", "Browse"));
    QPushButton *m_browse_source = new QPushButton(xi18nc("@button", "Browse"));
    m_source = new QLineEdit;
    m_destination = new QLineEdit;
    urlLayout->addWidget(sourceLabel, 0, 0);
    urlLayout->addWidget(m_source, 0, 1);
    urlLayout->addWidget(m_browse_source, 0, 2);
    urlLayout->addWidget(destLabel, 1, 0);
    urlLayout->addWidget(m_destination, 1, 1);
    urlLayout->addWidget(m_browse_dest, 1, 2);
    urlGroup->setLayout(urlLayout);

    // Options Group
    QGroupBox *optionGroup = new QGroupBox(i18nc("@label", "Options"));
    QVBoxLayout *optionLayout = new QVBoxLayout;

    m_optNoUpdate = new QCheckBox(i18n("Do not update the new working directory."));
    m_optUsePull = new QCheckBox(i18n("Use pull protocol to copy metadata."));
    m_optUseUncmprdTrans = new QCheckBox(i18n("Use uncompressed transfer."));
    m_optNoVerifyServCert = new QCheckBox(i18n("Do not verify server certificate (ignoring web.cacerts config)."));

    optionLayout->addWidget(m_optNoUpdate);
    optionLayout->addWidget(m_optUsePull);
    optionLayout->addWidget(m_optUseUncmprdTrans);
    optionLayout->addWidget(m_optNoVerifyServCert);
    optionGroup->setLayout(optionLayout);
    // end options

    QFrame *frame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(urlGroup);
    mainLayout->addWidget(optionGroup);
    mainLayout->addStretch();
    frame->setLayout(mainLayout);
    
    m_stackLayout = new  QStackedLayout;
    m_outputEdit = new KTextEdit;
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setFontFamily(QLatin1String("Monospace"));
    m_stackLayout->addWidget(frame);
    m_stackLayout->addWidget(m_outputEdit);
    m_stackLayout->setCurrentIndex(0);

    layout()->insertLayout(0, m_stackLayout);
    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->resize(QSize(settings->cloneDialogWidth(),
                               settings->cloneDialogHeight()));
    
    connect(this, SIGNAL(finished(int)), this, SLOT(saveGeometry()));
    connect(m_source, SIGNAL(textChanged(const QString&)), 
                this, SLOT(slotUpdateOkButton()));
    connect(m_browse_dest, SIGNAL(clicked()),
                this, SLOT(slotBrowseDestClicked()));
    connect(m_browse_source, SIGNAL(clicked()),
                this, SLOT(slotBrowseSourceClicked()));
    connect(&m_process, SIGNAL(started()), this, SLOT(slotCloningStarted()));
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(slotCloningFinished(int, QProcess::ExitStatus)));
    connect(&m_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(slotUpdateCloneOutput()));
}

void HgCloneDialog::browseDirectory(QLineEdit *dest)
{
    QString result = QFileDialog::getExistingDirectory(this);
    if (result.length() > 0) {
        dest->setText(result);
    }
}

void HgCloneDialog::slotBrowseDestClicked()
{
    browseDirectory(m_destination);
}

void HgCloneDialog::slotBrowseSourceClicked()
{
    browseDirectory(m_source);
}

void HgCloneDialog::done(int r)
{
    if (r == QDialog::Accepted && !m_cloned) {
        // Will execute 'stdbuf' command to make the output of
        // mercurial command line buffered and enable us to show 
        // output of cloning as soon as new line is available
        QStringList args;
        args << QLatin1String("-oL"); //argument for stdbuf. 
        args << QLatin1String("hg");
        args << QLatin1String("clone");
        args << QLatin1String("--verbose");
        appendOptionArguments(args);
        args << m_source->text();

        if (!m_destination->text().isEmpty()) {
            args << m_destination->text();
        }

        m_outputEdit->clear();
        m_stackLayout->setCurrentIndex(1);
        QApplication::processEvents();
        okButton()->setDisabled(true);

        m_process.setWorkingDirectory(m_workingDirectory);
        m_process.start(QLatin1String("stdbuf"), args);
    }
    else if (r == QDialog::Accepted && m_cloned) {
        QDialog::done(r);
    }
    else {
        if (m_process.state() == QProcess::Running) {
            KMessageBox::error(this, i18n("Terminating cloning!"));
            okButton()->setDisabled(false);
            m_terminated = true;
            m_process.terminate();
            m_stackLayout->setCurrentIndex(0);
        }
        else {
            QDialog::done(r);
        }
    }
}

void HgCloneDialog::slotCloningStarted()
{
    m_terminated = false;
}

void HgCloneDialog::slotUpdateCloneOutput()
{
    m_outputEdit->insertPlainText(QTextCodec::codecForLocale()->toUnicode(m_process.readAllStandardOutput()));
}

void HgCloneDialog::slotCloningFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        m_cloned = true;
        okButton()->setText(xi18nc("@action:button", "Close"));
        okButton()->setDisabled(false);
    }
    else if (!m_terminated) {
        KMessageBox::error(this, xi18nc("@message:error",
                                        "Error Cloning Repository!"));
    }
}

void HgCloneDialog::appendOptionArguments(QStringList &args)
{
    if (m_optNoUpdate->checkState() == Qt::Checked) {
        args << QLatin1String("-U");
    }
    if (m_optUsePull->checkState() == Qt::Checked) {
        args << QLatin1String("--pull");
    }
    if (m_optUseUncmprdTrans->checkState() == Qt::Checked) {
        args << QLatin1String("--uncompressed");
    }
    if (m_optNoVerifyServCert->checkState() == Qt::Checked) {
        args << QLatin1String("--insecure");
    }
}

void HgCloneDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setCloneDialogHeight(this->height());
    settings->setCloneDialogWidth(this->width());
    settings->save();
}

void HgCloneDialog::slotUpdateOkButton()
{
    if (m_source->text().length() > 0) {
        okButton()->setDisabled(false);
    }
    else {
        okButton()->setDisabled(true);
    }
}

#include "clonedialog.moc"

