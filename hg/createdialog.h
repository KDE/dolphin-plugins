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

#ifndef HGCREATEDILAOG_H
#define HGCREATEDILAOG_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QCheckBox>
#include <kdialog.h>
#include <klineedit.h>
#include <kpushbutton.h>

//TODO: Read output of clone as soon as available. 
//TODO: Add buttons to cancel and go back during cloning.

class HgCreateDialog : public KDialog
{
    Q_OBJECT

public:
    HgCreateDialog(QWidget *parent = 0);

private:
    void done(int r);
    void appendOptionArguments(QStringList &args);

private:

};

#endif // HGCREATEDILAOG_H

