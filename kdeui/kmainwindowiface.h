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
#include <dcopref.h>

#include <qmap.h>

class KDCOPActionProxy;
class KDCOPPropertyProxy;
class KMainWindow;

/**
 * @short DCOP interface to KMainWindow.
 *
 * This is the main interface to the KMainWindow.  This will provide a consistent
 * DCOP interface to all KDE applications that use it.
 *
 * @author Ian Reinhart Geiser <geiseri@yahoo.com>
 */
class KDEUI_EXPORT KMainWindowInterface : virtual public DCOPObject
{
K_DCOP

public:
	/**
	Construct a new interface object.
	@param mainWindow - The parent KMainWindow object
	that will provide us with the KAction objects.
	*/
	KMainWindowInterface( KMainWindow * mainWindow );
	/**
	Destructor
	Cleans up the dcop action proxy object.
	**/
	~KMainWindowInterface();

	DCOPCStringList functionsDynamic();
	bool processDynamic(const DCOPCString &fun, const QByteArray &data, DCOPCString& replyType, QByteArray &replyData);


k_dcop:
	/**
	Return a list of actions available to the application's window.
	@return A QCStringList containing valid names actions.
	*/
	DCOPCStringList actions();

	/**
	Activates the requested action.
	@param action The name of the action to activate.  The names of valid
	actions can be found by calling actions().
	@return The success of the operation.
	*/
	bool activateAction( const DCOPCString& action);

	/**
	Disables the requested action.
	@param action The name of the action to disable.  The names of valid
	actions can be found by calling actions().
	@return The success of the operation.
	*/
	bool disableAction( const DCOPCString& action);

	/**
	Enables the requested action.
	@param action The name of the action to enable.  The names of valid
	actions can be found by calling actions().
	@return The success of the operation.
	*/
	bool enableAction( const DCOPCString& action);

	/**
	Returns the status of the requested action.
	@param action The name of the action.  The names of valid
	actions can be found by calling actions().
	@returns The state of the action, true - enabled, false - disabled.
	*/
	bool actionIsEnabled( const DCOPCString& action);

	/**
	Returns the tool tip text of the requested action.
	@param action The name of the action to activate.  The names of valid
	actions can be found by calling actions().
	@return A QCString containing the text of the action's tool tip.
	*/
	DCOPCString actionToolTip( const DCOPCString& action);

	/**
	Returns a dcop reference to the selected KAction
	@param name The name of the action.  The names of valid
	actions can be found by calling actions().
	@return A DCOPRef for the kaction.
	**/
	DCOPRef action( const DCOPCString& name );

	/**
	Returns and action map
	**/
  	QMap<DCOPCString,DCOPRef> actionMap();
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
//	bool isHidden();
	void hide();
//	bool isMaximized();
	void maximize();
//	bool isMinimized();
	void minimize();
//	int width();
//	int height();
	void resize(int newWidth, int newHeight);
//	int Xpos();
//	int Ypos();
	void move(int newX, int newY);
	void setGeometry(int newX, int newY, int newWidth, int newHeight);
	void raise();
	void lower();
	void restore();
	void show();

//	QCStringList getQTProperties();

private:
	KMainWindow *m_MainWindow;
	KDCOPActionProxy *m_dcopActionProxy;
	KDCOPPropertyProxy *m_dcopPropertyProxy;
};

#endif


