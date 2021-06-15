/*
    SPDX-FileCopyrightText: 2015 Tomasz Bojczuk <seelook@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dialogbase.h"
#include <QBoxLayout>
#include <QKeyEvent>

DialogBase::DialogBase(QDialogButtonBox::StandardButtons buttons, QWidget* parent):
    QDialog(parent),
    m_okButton(nullptr),
    m_cancelButton(nullptr)
{
    m_buttonBox = new QDialogButtonBox(this);
    if (buttons & QDialogButtonBox::Ok) {
        m_okButton = m_buttonBox->addButton(QDialogButtonBox::Ok);
        m_okButton->setDefault(true);
    }
    if (buttons & QDialogButtonBox::Cancel) {
        m_cancelButton = m_buttonBox->addButton(QDialogButtonBox::Cancel);
    }

    m_layout = new QBoxLayout(QBoxLayout::TopToBottom);
    m_layout->addWidget(m_buttonBox);

    setLayout(m_layout);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void DialogBase::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return && event->modifiers() == Qt::ControlModifier) {
        done(Accepted);
    }
    else {
        QWidget::keyReleaseEvent(event);
    }
}



