/* This file is part of the KDE libraries
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

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

#ifndef _KGLOBALACCEL_H_
#define _KGLOBALACCEL_H_

#include <qobject.h>
#include <kshortcut.h>

class QPopupMenu;
class QWidget;
class KAccelAction;
class KAccelActions;
class KConfig;

class KGlobalAccel : public QObject
{
	Q_OBJECT
 public:
	KGlobalAccel( QObject* pParent, const char* psName = 0 );
	virtual ~KGlobalAccel();

	void clearActions();

	KAccelActions& actions();
	const KAccelActions& actions() const;

	KAccelAction* insertAction( const QString& sAction, const QString& sDesc, const QString& sHelp,
	                 const KShortcut& cutDef3, const KShortcut& cutDef4,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );
	KAccelAction* insertAction( const QString& sAction, const QString& sDesc, const QString& sHelp,
	                 const char* cutDef3, const char* cutDef4,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );
	bool insertLabel( const QString& sName, const QString& sDesc );

	bool updateConnections();

	const KShortcut& shortcut( const QString& sAction ) const;

	bool setShortcut( const QString& sAction, const KShortcut& );
	bool setSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );

	void readSettings( KConfig* pConfig = 0 );
	void writeSettings( KConfig* pConfig = 0 ) const;

	// Functions which mimic QAccel somewhat:
	//virtual bool insertItem( const QString& sDesc, const QString& sAction,
	//                 KShortcuts rgCutDefaults3,
	//                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );

 protected:
	// Attempts to make a passive X server grab/ungrab of the specified key.
	//  Return true if successful.
	// Modifications with NumLock, CapsLock, ScrollLock, and ModeSwitch are
	//  also grabbed.
	bool grabKey( const QString& action, bool bGrab );

 private:
	class KGlobalAccelPrivate* d;

 public:
	// Setting this to false shuts off processing of KeyPress events in
	//  x11EventFilter(). It will still be called, but won't act on them.
	// This is a more efficient means for briefly suspending processing
	//  than setEnabled(false) ... setEnabled(true).
	// These functions should be implemented in kglobalaccel_x11/emb.cpp
	static void setKeyEventsEnabled( bool enabled );
	static bool areKeyEventsEnabled();

	friend class KGlobalAccelPrivate;
};

#endif // _KGLOBALACCEL_H_
