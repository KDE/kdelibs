/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KWINDOW_INTERFACE_H
#define KWINDOW_INTERFACE_H

#include <dcopobject.h>
#include <qstringlist.h>
#include <qcstring.h>

class KDCOPActionProxy;
class KMainWindow;
/**
This is the main interface to the @ref KMainWindow.  This will provide a consistant
dcop interface to all KDE applications that use it.
@short DCOP interface to @ref KMainWindow.
@author Ian Reinhart Geiser <geiseri@yahoo.com>
*/
class KMainWindowInterface : virtual public DCOPObject
{
K_DCOP

public:
	/**
	Construct a new interface object.
	@param mainWindow - The parent @ref KMainWindow object
	that will provide us with the @ref KAction objects.
	*/
	KMainWindowInterface( KMainWindow * mainWindow );
	/**
	Destructor
	Cleans up the dcop action proxy object.
	**/
	~KMainWindowInterface();
k_dcop:
	/**
	Return a list of actions available to the application's window.
	@return A QCStringList containing valid names actions.
	*/
	QCStringList actions();

	/**
	Activates the requested action.
	@param action The name of the action to activate.  The names of valid
	can actions can be found by calling @ref actions().
	@return The success of the operation.
	*/
	bool activate( QCString action);

	/**
	Disables the requested action.
	@param action The name of the action to activate.  The names of valid
	can actions can be found by calling @ref actions().
	@return The success of the operation.
	*/
	bool disable( QCString action);

	/**
	Enables the requested action.
	@param action The name of the action to activate.  The names of valid
	can actions can be found by calling @ref actions().
	@return The success of the operation.
	*/
	bool enable( QCString action);

	/**
	Returns the status of the requested action.
	@param action The name of the action to activate.  The names of valid
	can actions can be found by calling @ref actions().
	@returns The state of the current Actions, TRUE - enabled, FALSE - disabled.
	*/
	bool status( QCString action);

	/**
	Returns the tool tip text of the requested action.
	@param action The name of the action to activate.  The names of valid
	can actions can be found by calling @ref actions().
	@return A QCString containing the text of the actions tool tip.
	*/
	QCString tooltip( QCString action);
//	virtual QCStringList functions();
//	virtual QCStringList interfaces();
//	virtual bool process( const QCString &functionName, const QByteArray &argData, QCString &returnType, QByteArray &returnData);

private:
	KMainWindow * m_pMainWindow;
	KDCOPActionProxy *m_dcopActionProxy;
};

#endif


