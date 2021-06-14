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

#ifndef DIALOGBASE_H
#define DIALOGBASE_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QPushButton>

/**
 * QDialog subclass with common method used in the plugin classes.
 * It has already initialized layout (@class QBoxLayout), with @p buttonBox(),
 * so adding other widgets/layout should be invoked by:
 * @p layout()->insert() to keep buttonBox at the bottom.
 * It also handles CTRL+Enter shortcut to accept - as old Kdialog did.
 */
class DialogBase : public QDialog
{

    Q_OBJECT

public:
    /**
     * Creates Dialog window with given buttons types.
     * But only OK and Cancel are supported so far
     * and available through @p okButton() and @p cancelButton()
     */
    explicit DialogBase(QDialogButtonBox::StandardButtons buttons, QWidget* parent = nullptr);

    QPushButton* okButton() { return m_okButton; }
    QPushButton* cancelButton() { return m_cancelButton; }
    QDialogButtonBox* buttonBox() { return m_buttonBox; }
    /**
     * Layout of a dialog. By default vertical (@p QBoxLayout::TopToBottom)
     * Use  layout()->insertLayout(0, someLaayout) or
     * layout()->insertWidget(0, someWidget)
     * to keep buttonBox at the dialog bottom.
     */
    QBoxLayout* layout() { return m_layout; }

protected:
    void keyReleaseEvent(QKeyEvent* event) override; // to handle CTRL+Enter shortcut to accept dialog

private:
    QPushButton               *m_okButton;
    QPushButton               *m_cancelButton;
    QDialogButtonBox          *m_buttonBox;
    QBoxLayout                *m_layout;

};

#endif // DIALOGBASE_H
