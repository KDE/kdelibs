/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KWINDOW_INTERFACE_H
#define KWINDOW_INTERFACE_H

#include <dcopobject.h>
#include <qstringlist.h>
#include <qcstring.h>
#include <dcopref.h>

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
	bool activateAction( QCString action);

	/**
	Disables the requested action.
	@param action The name of the action to activate.  The names of valid
	can actions can be found by calling @ref actions().
	@return The success of the operation.
	*/
	bool disableAction( QCString action);

	/**
	Enables the requested action.
	@param action The name of the action to activate.  The names of valid
	can actions can be found by calling @ref actions().
	@return The success of the operation.
	*/
	bool enableAction( QCString action);

	/**
	Returns the status of the requested action.
	@param action The name of the action to activate.  The names of valid
	can actions can be found by calling @ref actions().
	@returns The state of the current Actions, TRUE - enabled, FALSE - disabled.
	*/
	bool actionIsEnabled( QCString action);

	/**
	Returns the tool tip text of the requested action.
	@param action The name of the action to activate.  The names of valid
	can actions can be found by calling @ref actions().
	@return A QCString containing the text of the actions tool tip.
	*/
	QCString actionToolTip( QCString action);

	/**
	Returns a dcop reference to the selected KAction
	@param action The name of the action to activate, The names of valid
	actions can be found by calling @actions().
	@return A @ref DCOPRef for the kaction.
	**/
	DCOPRef action( const QCString &name );

	/**
	Returns and action map
	**/
    	QMap<QCString,DCOPRef> actionMap();
	/**
	Returns the ID of the current main window.
	This is useful for automated screen captures or other evil 
	widget fun.
	@return A integer value of the main window's ID.
	**/
	int getWinID();
	/**
	Copies a pixmap representation of the current main window to
	the clipboard.
	**/
	void grabWindowToClipBoard();
	bool isHidden();
	void hide();
	bool isMaximized();
	void maximize();
	bool isMinimized();
	void minimize();
	int width();
	int height();
	void resize(int newWidth, int newHeight);
	int Xpos();
	int Ypos();
	void move(int newX, int newY);
	void setGeometry(int newX, int newY, int newWidth, int newHeight);
	void raise();
	void lower();
	void restore();
	void show();

private:
	KMainWindow *m_MainWindow;
	KDCOPActionProxy *m_dcopActionProxy;
};

#endif


