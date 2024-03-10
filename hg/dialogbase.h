/*
    SPDX-FileCopyrightText: 2015 Tomasz Bojczuk <seelook@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIALOGBASE_H
#define DIALOGBASE_H

#include <QBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
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
    explicit DialogBase(QDialogButtonBox::StandardButtons buttons, QWidget *parent = nullptr);

    QPushButton *okButton()
    {
        return m_okButton;
    }
    QPushButton *cancelButton()
    {
        return m_cancelButton;
    }
    QDialogButtonBox *buttonBox()
    {
        return m_buttonBox;
    }
    /**
     * Layout of a dialog. By default vertical (@p QBoxLayout::TopToBottom)
     * Use  layout()->insertLayout(0, someLaayout) or
     * layout()->insertWidget(0, someWidget)
     * to keep buttonBox at the dialog bottom.
     */
    QBoxLayout *layout()
    {
        return m_layout;
    }

protected:
    void keyReleaseEvent(QKeyEvent *event) override; // to handle CTRL+Enter shortcut to accept dialog

private:
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QDialogButtonBox *m_buttonBox;
    QBoxLayout *m_layout;
};

#endif // DIALOGBASE_H
