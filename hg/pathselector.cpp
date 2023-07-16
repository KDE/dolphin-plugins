/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pathselector.h"
#include "hgconfig.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <KComboBox>
#include <KLocalizedString>

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
    m_urlEdit = new QLineEdit;
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

    m_selectPathAlias->addItem(xi18nc("@label:combobox", "edit"));
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



#include "moc_pathselector.cpp"
