/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "generalconfig.h"

#include <KLocalizedString>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>

HgGeneralConfigWidget::HgGeneralConfigWidget(HgConfig::ConfigType type, QWidget *parent)
    : QWidget(parent)
    , m_configType(type)
{
    setupUI();
    loadConfig();
}

void HgGeneralConfigWidget::setupUI()
{
    m_userEdit = new QLineEdit;
    m_editorEdit = new QLineEdit;
    m_mergeEdit = new QLineEdit;
    m_verboseCheck = new QCheckBox(xi18nc("@label:checkbox", "Verbose Output"));

    QLabel *userLabel = new QLabel(xi18nc("@label", "Username"));
    QLabel *editorLabel = new QLabel(xi18nc("@label", "Default Editor"));
    QLabel *mergeLabel = new QLabel(xi18nc("@label", "Default Merge Tool"));

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

    QString verbose = hgc.property(QLatin1String("ui"), QLatin1String("verbose"));
    if (verbose.isEmpty() || verbose == QStringLiteral("False")) {
        m_verboseCheck->setChecked(false);
    } else if (verbose == QLatin1String("True")) {
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
        hgc.setProperty(QLatin1String("ui"), QLatin1String("verbose"), QLatin1String("True"));
    } else {
        hgc.setProperty(QLatin1String("ui"), QLatin1String("verbose"), QLatin1String("False"));
    }
}

#include "moc_generalconfig.cpp"
