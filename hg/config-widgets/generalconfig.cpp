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

#include "generalconfig.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtCore/QString>
#include <QtGui/QCheckBox>
#include <klineedit.h>
#include <kpushbutton.h>
#include <klocale.h>

HgGeneralConfigWidget::HgGeneralConfigWidget(HgConfig::ConfigType type, QWidget *parent):
    QWidget(parent),
    m_configType(type)
{
    setupUI();
    loadConfig();
}

void HgGeneralConfigWidget::setupUI()
{
    m_userEdit = new KLineEdit;
    m_editorEdit = new KLineEdit;
    m_mergeEdit = new KLineEdit;
    m_verboseCheck = new QCheckBox(i18nc("@label:checkbox", "Verbose Output"));
    
    QLabel *userLabel = new QLabel(i18nc("@label", "Username"));
    QLabel *editorLabel = new QLabel(i18nc("@label", "Default Editor"));
    QLabel *mergeLabel = new QLabel(i18nc("@label", "Default Merge Tool"));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(userLabel, 0, 0);
    mainLayout->addWidget(m_userEdit, 0, 1);
    mainLayout->addWidget(editorLabel, 1, 0);
    mainLayout->addWidget(m_editorEdit, 1, 1);
    mainLayout->addWidget(mergeLabel, 2, 0);
    mainLayout->addWidget(m_mergeEdit, 2, 1);
    mainLayout->addWidget(m_verboseCheck, 3, 0, 2, 0);
    mainLayout->setRowStretch(mainLayout->rowCount(), 1);

    setLayout(mainLayout);
}

void HgGeneralConfigWidget::loadConfig()
{
    HgConfig hgc(m_configType);

    m_userEdit->setText(hgc.username());
    m_editorEdit->setText(hgc.editor());
    m_mergeEdit->setText(hgc.merge());

    QString verbose = hgc.property(QLatin1String("ui"), QLatin1String("verobose"));
    if (verbose.isEmpty() || verbose == "False") {
        m_verboseCheck->setChecked(false);
    }
    else if (verbose == "True") {
        m_verboseCheck->setChecked(true);
    }
}

void HgGeneralConfigWidget::saveConfig()
{
    HgConfig hgc(m_configType);
    hgc.setUsername(m_userEdit->text());
    hgc.setEditor(m_editorEdit->text());
    hgc.setMerge(m_mergeEdit->text());

    if (m_verboseCheck->checkState() == Qt::Checked) {
        hgc.setProperty(QLatin1String("ui"), QLatin1String("verbose"),
                QLatin1String("True"));
    }
    else {
        hgc.setProperty(QLatin1String("ui"), QLatin1String("verbose"),
                QLatin1String("False"));
    }
}

#include "generalconfig.moc"

