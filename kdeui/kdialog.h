/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#ifndef __KDIALOG_H
#define __KDIALOG_H

#include <qdialog.h>

/**
 * Dialog with extended modeless support.
 *
 * So far the only extended functionality is that if the dialog is
 * modeless and has a parent the default keybindings (escape = reject(),
 * enter = accept() etc.) are disabled.
 *
 * @author Thomas Tanghus <tanghus@earthling.net>
 * @version 0.1.1
*/
class KDialog : public QDialog
{
	Q_OBJECT

public:
/**
* Constructor. Takes the same arguments as QDialog.
*/
	KDialog(QWidget *parent = 0, const char *name = 0, 
			bool modal = false, WFlags f = 0);
protected:

/**
* @internal
*/
	virtual void keyPressEvent(QKeyEvent*);
};

#endif // __KDIALOG_H


