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

#include <qnamespace.h>

#ifdef Q_WS_X11
#ifndef _KKEY_X11_H
#define _KKEY_X11_H

#include "kkeysequence.h"

typedef union  _XEvent XEvent;

class KKeyX11 : public KKeySequence
{
public:
	KKeyX11()                                  { m_keyCombQt = 0; }
	KKeyX11( const KKeyX11& k )  : KKeySequence( (KKeySequence&)k ) {}
	KKeyX11( uint keyCombQt )                  { m_keyCombQt = keyCombQt; }
	KKeyX11( const QKeyEvent * );
	KKeyX11( const QString& );
	//KKeyX11( const XEvent * );

	KKeyX11& operator =( KKeyX11 k )           { m_keyCombQt = k.m_keyCombQt; return *this; }
	KKeyX11& operator =( uint keyCombQt )      { m_keyCombQt = keyCombQt; return *this; }

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
	//static uint stringToKey( const QString& keyStr, uchar *pKeyCodeX, uint *pKeySymX, uint *pKeyModX );
	static void init();
	static uint keyCodeXToKeySymX( uchar keyCodeX, uint keyModX );

	enum ModKeysIndex {
		ModShiftIndex, ModCapsLockIndex, ModCtrlIndex, ModAltIndex,
		ModNumLockIndex, ModModeSwitchIndex, ModMetaIndex, ModScrollLockIndex,
		MOD_KEYS
	};

	static KKeySequence keyEventXToKey( const XEvent* pEvent );
	static void keyEventXToKeyX( const XEvent* pEvent, uchar *pKeyCodeX, uint *pKeySymX, uint *pKeyModX );
	static uint keyEventXToKeyQt( const XEvent* pEvent );
	static int keySymXIndex( uint keySym );
	static void keySymXMods( uint keySym, uint *pKeyModQt, uint *pKeyModX );
	static uint keyCodeXToKeyQt( uchar keyCodeX, uint keyModX );
	static uint keySymXToKeyQt( uint keySymX, uint keyModX );
	static void keyQtToKeyX( uint keyCombQt, uchar *pKeyCodeX, uint *pKeySymX, uint *pKeyModX );

	static QString keyCodeXToString( uchar keyCodeX, uint keyModX, bool bi18n );
	static QString keySymXToString( uint keySymX, uint keyModX, bool bi18n );

	// Return the keyModX containing just the bit set for the given modifier.
	static uint keyModXShift();		// ShiftMask
	static uint keyModXLock();		// LockMask
	static uint keyModXCtrl();		// ControlMask
	static uint keyModXAlt();		// Normally Mod1Mask
	static uint keyModXNumLock();		// Normally Mod2Mask
	static uint keyModXModeSwitch();	// Normally Mod3Mask
	static uint keyModXMeta();		// Normally Mod4Mask
	static uint keyModXScrollLock();	// Normally Mod5Mask

	static uint accelModMaskX();		// Normally ShiftMask | ControlMask | Mod1Mask | Mod3Mask
};

#endif // !_KKEY_X11_H
#endif // Q_WS_X11
