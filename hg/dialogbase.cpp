/***************************************************************************
 *   Copyright (C) 2015 by Tomasz Bojczuk <seelook@gmail.com>              *
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
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include "dialogbase.h"
#include <QBoxLayout>
#include <QKeyEvent>

DialogBase::DialogBase(QDialogButtonBox::StandardButtons buttons, QWidget* parent):
    QDialog(parent),
    m_okButton(0),
    m_cancelButton(0)
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



