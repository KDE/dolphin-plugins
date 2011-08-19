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

#include "pathselector.h"
#include "hgconfig.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>

HgPathSelector::HgPathSelector(QWidget *parent) :
    QWidget(parent)
{
    setupUI();
    reload();

    // connections
    connect(m_selectPathAlias, SIGNAL(currentIndexChanged(int)), 
            this, SLOT(slotChangeEditUrl(int)));
    connect(m_selectPathAlias, SIGNAL(highlighted(int)), 
            this, SLOT(slotChangeEditUrl(int)));
}

void HgPathSelector::setupUI()
{
    QHBoxLayout *urlLayout = new QHBoxLayout;
    m_selectPathAlias = new KComboBox;
    m_urlEdit = new KLineEdit;
    m_urlEdit->setReadOnly(true);

    urlLayout->addWidget(m_selectPathAlias);
    urlLayout->addWidget(m_urlEdit);

    setLayout(urlLayout);
}

void HgPathSelector::reload()
{
    HgConfig hgc(HgConfig::RepoConfig);
    m_pathList = hgc.repoRemotePathList();

    m_selectPathAlias->clear();

    QMutableMapIterator<QString, QString> it(m_pathList);
    while (it.hasNext()) {
        it.next();
        if (it.key() == QLatin1String("default")) {
            m_selectPathAlias->insertItem(0, it.key());
        }
        else {
            m_selectPathAlias->addItem(it.key());
        }
    }

    m_selectPathAlias->addItem(i18nc("@label:combobox", "<edit>"));
    slotChangeEditUrl(0);
}


void HgPathSelector::slotChangeEditUrl(int index)
{
    if (index == m_selectPathAlias->count() - 1) { ///enter URL manually
        m_urlEdit->setReadOnly(false);
        m_urlEdit->clear();
        m_urlEdit->setFocus();
    }
    else {
        QString url = m_pathList[m_selectPathAlias->itemText(index)];
        m_urlEdit->setText(url);
        m_urlEdit->setReadOnly(true);
    }
}

const QString HgPathSelector::remote() const
{
    return (m_selectPathAlias->currentIndex() == m_selectPathAlias->count()-1)?m_urlEdit->text():m_selectPathAlias->currentText();
}

#include "pathselector.moc"

