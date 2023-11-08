/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "createdialog.h"
#include "fileviewhgpluginsettings.h"

#include <QHBoxLayout>
#include <QProcess>
#include <QLabel>
#include <KLocalizedString>
#include <QLineEdit>
#include <KMessageBox>

HgCreateDialog::HgCreateDialog(const QString &directory, QWidget *parent):
    DialogBase(QDialogButtonBox::Ok | QDialogButtonBox::Cancel ,parent),
    m_workingDirectory(directory)
{
    // dialog properties
    this->setWindowTitle(xi18nc("@title:window",
                "<application>Hg</application> Initialize Repository"));
    okButton()->setText(xi18nc("@action:button", "Initialize Repository"));


    //////////////
    // Setup UI //
    //////////////
    
    m_directory = new QLabel(QLatin1String("<b>") + m_workingDirectory + QLatin1String("</b>"));
    m_repoNameEdit = new QLineEdit;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_directory);
    mainLayout->addWidget(m_repoNameEdit);

    layout()->insertLayout(0, mainLayout);
    m_repoNameEdit->setFocus();
}

void HgCreateDialog::done(int r)
{
    if (r == QDialog::Accepted) {
        QProcess process;
        QStringList args;
        args << QLatin1String("init");
        if (!m_repoNameEdit->text().isEmpty()) {
            args << m_repoNameEdit->text();
        }
        process.setWorkingDirectory(m_workingDirectory);
        process.start(QLatin1String("hg"), args);
        process.waitForFinished();
        
        if (process.exitCode() == 0 && process.exitStatus() == QProcess::NormalExit) {
            QDialog::done(r);
        }
        else {
            KMessageBox::error(this, xi18nc("error message", "Error creating repository!"));
        }
    }
    else {
        QDialog::done(r);
    }
}



#include "moc_createdialog.cpp"
