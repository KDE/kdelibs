/* This file is part of the KDE libraries
    Copyright (C) 2001 Ellis Whitehead <kde@ellisw.net>

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

#ifndef _KKEY_H
#define _KKEY_H

#include <qstring.h>

class QKeyEvent;

class KKey
{
protected:
	uint	m_keyCombQt;
public:
	KKey()                                  { m_keyCombQt = 0; }
	KKey( const KKey& k )                   { m_keyCombQt = k.m_keyCombQt; }
	KKey( uint keyCombQt )                  { m_keyCombQt = keyCombQt; }
	KKey( const QKeyEvent * );
	KKey( const QString& );

	KKey& operator =( KKey k )              { m_keyCombQt = k.m_keyCombQt; return *this; }
	KKey& operator =( uint keyCombQt )      { m_keyCombQt = keyCombQt; return *this; }

	uint key() const                        { return m_keyCombQt; }
	uint sym() const                        { return m_keyCombQt & 0xffff; }
	uint mod() const                        { return m_keyCombQt & ~0xffff; }
	uint state() const                      { return mod() >> 18; }

	QString toString();

	// Config Functions

	// When bUseFourModifierKeys is on (setting: Global|Keyboard|Use Four Modifier Keys = true | false)
	//  calls to insertItem will set the current key to aDefaultKeyCode4.
	static bool useFourModifierKeys();
	static void useFourModifierKeys( bool b );
	static bool qtSupportsMetaKey();

	/**
	 * Retrieve the key code corresponding to the string @p sKey or
	 * zero if the string is not recognized.
	 *
	 * The string must be something like "Shift+A",
	 * "F1+Ctrl+Alt" or "Backspace" for example. That is, the string
	 * must consist of a key name plus a combination of
	 * the modifiers Shift, Ctrl and Alt.
	 *
	 * N.B.: @p sKey must @em not be @ref i18n()'d!
	 */
	static KKey stringToKey( const QString& keyStr );
	static uint stringToKeyQt( const QString& keyStr )
		{ return stringToKey( keyStr ).key(); }

	// Retrieve a string corresponding to the key code @p keyCode,
	//  which is empty if @p keyCode is not recognized or zero.
	static QString keyToString( int keyCode, bool i18_n = false );

	// X11-Related Functions
	// Naming Proceedure:
	//  -CodeX	the index of the physical key pressed (keyboard dependent)
	//  -Sym-	key symbol. Either unicode (like 'A') or special key (like delete)
	//  -Mod-	contains bits for modifier flags
	//  -X		Formatted for/by the X sever
	//  -Qt		Formatted for/by Qt
	//  keyQt	Qt shortcut key value containing both Qt Sym and Qt Mod.
	//  keyEvent-	An X or Qt key event
	// Example:
	//  keyCodeXToKeyQt() converts the X11 key code & mod into a Qt shortcut key
	static uint keyEventQtToKeyQt( const QKeyEvent* );

	// Return the keyMod mask containing the bits set for the modifiers
	//  which may be used in accelerator shortcuts.
	static uint accelModMaskQt();		// Normally Qt::SHIFT | Qt::CTRL | Qt::ALT | (Qt::ALT<<1)

	// Returns true if X has the Meta key assigned to a modifier bit
	static bool keyboardHasMetaKey();
};

#endif // _KKEY_H
