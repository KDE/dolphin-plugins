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

#include "bundledialog.h"
#include "commitinfowidget.h"
#include "pathselector.h"
#include "fileviewhgpluginsettings.h"
#include "hgwrapper.h"

#include <QtGui/QWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include <klineedit.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

HgBundleDialog::HgBundleDialog(QWidget *parent) :
    KDialog(parent, Qt::Dialog)
{
    this->setCaption(i18nc("@title:window", 
                "<application>Hg</application> Bundle"));
    this->setButtons(KDialog::Ok | KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->setButtonText(KDialog::Ok, i18nc("@action:button", "Bundle"));

    // Load saved settings
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    this->setInitialSize(QSize(settings->bundleDialogWidth(),
                               settings->bundleDialogHeight()));
    //
    setupUI();

    // connections
    connect(this, SIGNAL(finished()), this, SLOT(saveGeometry()));
}

void HgBundleDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout;

    // main 
    m_pathSelect = new HgPathSelector;
    m_commitInfo = new HgCommitInfoWidget;

    QGridLayout *bodyLayout = new QGridLayout;
    bodyLayout->addWidget(m_pathSelect);
    bodyLayout->addWidget(m_commitInfo);

    m_mainGroup = new QGroupBox;
    m_mainGroup->setLayout(bodyLayout);

    layout->addWidget(m_mainGroup);

    // options
    m_optionGroup = new QGroupBox(i18nc("@label:group", "Options"));
    m_optForce = new QCheckBox(i18nc("@label:checkbox", 
                                     "Run even when the destination is "
                                     " unrelated (force)"));
    m_optInsecure = new QCheckBox(i18nc("@label:checkbox", 
                             "Do not verify server certificate"));
    
    QVBoxLayout *optionLayout = new QVBoxLayout;
    optionLayout->addWidget(m_optForce);
    optionLayout->addWidget(m_optInsecure);
    m_optionGroup->setLayout(optionLayout);

    layout->addWidget(m_optionGroup);
    //end options

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setMainWidget(widget);
}

void HgBundleDialog::done(int r)
{
    if (r == KDialog::Accepted) {

    }
    else {
        KDialog::done(r);
    }
}

void HgBundleDialog::saveGeometry()
{
    FileViewHgPluginSettings *settings = FileViewHgPluginSettings::self();
    settings->setBundleDialogHeight(this->height());
    settings->setBundleDialogWidth(this->width());
    settings->writeConfig();
}

#include "bundledialog.moc"

