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

#include "pluginsettings.h"
#include "hgconfig.h"
#include <QtCore/QDir>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QGridLayout>
#include <klineedit.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <kpushbutton.h>


HgPluginSettingsWidget::HgPluginSettingsWidget(QWidget *parent) :
    QWidget(parent),
    m_config(0)
{
    setupUI();
    loadConfig();

    connect(m_diffBrowseButton, SIGNAL(clicked()), 
            this, SLOT(browse_diff()));
}

void HgPluginSettingsWidget::saveConfig()
{
    KConfigGroup group(m_config, QLatin1String("diff"));
    group.writeEntry(QLatin1String("exec"), m_diffProg->text());
    group.config()->sync();
}

void HgPluginSettingsWidget::loadConfig()
{
    KUrl url = KUrl::fromPath(QDir::homePath());
    url.addPath(".dolphin-hg");
    m_config = new KConfig(url.path(), KConfig::SimpleConfig);
    
    KConfigGroup group(m_config, QLatin1String("diff"));
    QString diffExec = group.readEntry(QLatin1String("exec"), QString()).trimmed();
    m_diffProg->setText(diffExec);
}

void HgPluginSettingsWidget::setupUI()
{
    m_diffProg = new KLineEdit;
    m_diffBrowseButton = new KPushButton(i18nc("@label", "Browse"));
    QLabel *diffProgLabel = new QLabel(i18nc("@label", 
                                "Visual Diff Executable"));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(diffProgLabel, 0, 0);
    layout->addWidget(m_diffProg, 0, 1);
    layout->addWidget(m_diffBrowseButton, 0, 2);
    layout->setRowStretch(layout->rowCount(), 1);

    setLayout(layout);
}

void HgPluginSettingsWidget::browse_diff()
{
    QString path = KFileDialog::getOpenFileName();
    if (path.isEmpty()) {
        return;
    }

    m_diffProg->setText(path);
}

#include "pluginsettings.moc"

