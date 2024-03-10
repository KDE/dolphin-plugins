/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pluginsettings.h"
#include "hgconfig.h"
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

HgPluginSettingsWidget::HgPluginSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_config(nullptr)
{
    setupUI();
    loadConfig();

    connect(m_diffBrowseButton, SIGNAL(clicked()), this, SLOT(browse_diff()));
}

HgPluginSettingsWidget::~HgPluginSettingsWidget()
{
    delete m_config;
}

void HgPluginSettingsWidget::saveConfig()
{
    KConfigGroup group(m_config, QStringLiteral("diff"));
    group.writeEntry(QLatin1String("exec"), m_diffProg->text());
    group.config()->sync();
}

void HgPluginSettingsWidget::loadConfig()
{
    QString oldPath = QDir::homePath() + QLatin1String("/.dolphin-hg");
    if (QFile::exists(oldPath)) {
        // Copy old config file into user .config directory
        QFile::copy(oldPath, QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/dolphin-hg"));
        QFile::remove(oldPath);
    }
    m_config = new KConfig(QLatin1String("dolphin-hg"), KConfig::SimpleConfig, QStandardPaths::GenericConfigLocation);

    KConfigGroup group(m_config, QStringLiteral("diff"));
    QString diffExec = group.readEntry(QLatin1String("exec"), QString()).trimmed();
    m_diffProg->setText(diffExec);
}

void HgPluginSettingsWidget::setupUI()
{
    m_diffProg = new QLineEdit;
    m_diffBrowseButton = new QPushButton(xi18nc("@label", "Browse"));
    QLabel *diffProgLabel = new QLabel(xi18nc("@label", "Visual Diff Executable"));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(diffProgLabel, 0, 0);
    layout->addWidget(m_diffProg, 0, 1);
    layout->addWidget(m_diffBrowseButton, 0, 2);
    layout->setRowStretch(layout->rowCount(), 1);

    setLayout(layout);
}

void HgPluginSettingsWidget::browse_diff()
{
    QString path = QFileDialog::getOpenFileName();
    if (path.isEmpty()) {
        return;
    }

    m_diffProg->setText(path);
}

#include "moc_pluginsettings.cpp"
