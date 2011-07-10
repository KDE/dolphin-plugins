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

#ifndef HGCLONEDILAOG_H
#define HGCLONEDILAOG_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QCheckBox>
#include <QtGui/QStackedLayout>
#include <kaction.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <ktextedit.h>

//TODO: Read output of clone as soon as available. 
//TODO: Add buttons to cancel and go back during cloning.

class HgCloneDialog : public KDialog
{
    Q_OBJECT

public:
    HgCloneDialog(QWidget *parent = 0);

private slots:
    void saveGeometry();
    void slotUpdateOkButton();
    void slotBrowseDestClicked();
    void slotBrowseSourceClicked();

private:
    void done(int r);
    void browseDirectory(KLineEdit *dest);
    void appendOptionArguments(QStringList &args);

private:
    KLineEdit *m_source;
    KLineEdit *m_destination;
    KPushButton *m_browse_dest;
    KPushButton *m_browse_source;
    KTextEdit *m_outputEdit;
    QStackedLayout *m_stackLayout;

    // option checkboxes
    QCheckBox *m_optNoUpdate;
    QCheckBox *m_optUsePull;
    QCheckBox *m_optUseUncmprdTrans;
    QCheckBox *m_optNoVerifyServCert;

};

#endif // HGCLONEDILAOG_H

