/*
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

#include <qnamespace.h>
#include <qwindowdefs.h>

#ifdef Q_WS_X11	// Only compile this module if we're compiling for X11

#include "kkey_x11.h"

#include <kckey.h>
#include <kdebug.h>
#include <klocale.h>

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <ctype.h>
#undef NONE

//-------------------------------------------------------------------

KKeyX11::KKeyX11( const XEvent *pEvent )	{ m_keyCombQt = KKeyX11::keyEventXToKeyQt( pEvent ); }
KKeyX11::KKeyX11( const QKeyEvent *pEvent )	{ m_keyCombQt = KKeyX11::keyEventQtToKeyQt( pEvent ); }
KKeyX11::KKeyX11( const QString& keyStr )
{
*this = KKeyX11::stringToKey( keyStr );
}
QString KKeyX11::toString()		{ return KKeyX11::keyToString( m_keyCombQt ); }

//-------------------------------------------------------------------

struct ModKeyXQt
{
	static bool	bInitialized;
	const char	*keyName;
	uint		keyModMaskQt;
	uint		keyModMaskX;
};
bool ModKeyXQt::bInitialized = false;

struct TransKey {
	uint keySymQt;
	uint	keySymX;
};

static ModKeyXQt g_aModKeys[] =
{
	{ "Shift",	Qt::SHIFT,	ShiftMask },
	{ "CapsLock",	0, 		LockMask },
	{ "Ctrl",	Qt::CTRL,	ControlMask },
	{ "Alt",	Qt::ALT,	Mod1Mask },
	{ "NumLock",	0,		Mod2Mask },
	{ "ModeSwitch",	0,		Mod3Mask },
	{ "Meta",	(Qt::ALT<<1), 	Mod4Mask },
	{ "ScrollLock", 0,		Mod5Mask },
	{ 0, 0, 0 }
};

static const TransKey g_aTransKeySyms[] = {
	{ Qt::Key_Backspace,	XK_BackSpace },
	{ Qt::Key_Backtab,	XK_ISO_Left_Tab },
	{ Qt::Key_Enter,	XK_KP_Enter },
	{ Qt::Key_SysReq,	XK_Sys_Req },
	{ Qt::Key_CapsLock,	XK_Caps_Lock },
	{ Qt::Key_NumLock,	XK_Num_Lock },
	{ Qt::Key_ScrollLock,	XK_Scroll_Lock }
};

void KKeyX11::readModifierMapping()
{
	XModifierKeymap* xmk = XGetModifierMapping( qt_xdisplay() );

	for( int i = Mod2MapIndex; i < 8; i++ )
		g_aModKeys[i].keyModMaskX = 0;

	// Qt assumes that Alt is always Mod1Mask, so start at Mod2Mask.
	for( int i = Mod2MapIndex; i < 8; i++ ) {
		uint keySymX = XKeycodeToKeysym( qt_xdisplay(), xmk->modifiermap[xmk->max_keypermod * i], 0 );
		int j = -1;
		switch( keySymX ) {
			//case XK_Alt_L:
			//case XK_Alt_R:		j = 3; break;	// Normally Mod1Mask
			case XK_Num_Lock:	j = 4; break;	// Normally Mod2Mask
			case XK_Mode_switch:	j = 5; break;	// Normally Mod3Mask
			case XK_Meta_L:
			case XK_Meta_R:		j = 6; break;	// Normally Mod4Mask
			case XK_Scroll_Lock:	j = 7; break;	// Normally Mod5Mask
		}
		if( j >= 0 ) {
			g_aModKeys[j].keyModMaskX = (1<<i);
			kdDebug(125) << QString( "%1 = mod%2, keySymX = %3\n" ).arg(g_aModKeys[j].keyName).arg(i-2).arg(keySymX, 0, 16);
		}
	}

	for( int i = Mod1MapIndex; i < 8; i++ )
		kdDebug(125) << QString( "%1: keyModMaskX = 0x%2\n" ).arg(g_aModKeys[i].keyName).arg(g_aModKeys[i].keyModMaskX, 0, 16);

	XFreeModifiermap(xmk);

	ModKeyXQt::bInitialized = true;
}

QString KKeyX11::keyToString( int keyCombQt, bool bi18n )
{
	QString keyStr, keyModStr;
	uint keySymQt = keyCombQt & 0xffff;
	uint keyModQt = keyCombQt & ~0xffff;

	unsigned char keyCodeX;
	uint keySymX;
	uint keyModX;
	keyQtToKeyX( keyCombQt, &keyCodeX, &keySymX, &keyModX );

	// Letters should be displayed in upper-case.
	// If this is a unicode value (Qt special keys begin at 0x1000)
	if( keySymQt < 0x1000 )
		keySymQt = QChar( keySymQt ).upper().unicode();

	if( keySymQt ) {
		// Make sure 'Backtab' print
		if( keySymQt == Qt::Key_Backtab ) {
			keySymQt = Qt::Key_Tab;
			keyModQt |= Qt::SHIFT;
		}
		if( keyModQt ) {
			// Should possibly remove SHIFT
			// i.e., in en_US: 'Exclam' instead of 'Shift+1'
			// But don't do it on the Tab key.
			if( (keyModQt & Qt::SHIFT) && keySymQt != Qt::Key_Tab ) {
				int	index = keySymXIndex( keySymX );
				int	indexUnshifted = (index / 2) * 2; // 0 & 1 => 0, 2 & 3 => 2
				uint	keySymX0 = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, indexUnshifted ),
					keySymX1 = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, indexUnshifted+1 );
				QString	s0 = XKeysymToString( keySymX0 ),
					s1 = XKeysymToString( keySymX1 );

				// If shifted value is not the same as unshifted,
				//  then we shouldn't print Shift.
				if( s0.lower() != s1.lower() ) {
					keyModQt &= ~Qt::SHIFT;
					keySymX = keySymX1;
				}
			}

			// Search for modifier flags.
			for( int i = MOD_KEYS-1; i >= 0; i-- ) {
				if( keyModQt & g_aModKeys[i].keyModMaskQt ) {
					keyModStr += (bi18n) ? i18n(g_aModKeys[i].keyName) : QString(g_aModKeys[i].keyName);
					keyModStr += "+";
				}
			}
		}

		keyStr = (bi18n) ? i18n("Unknown Key", "Unknown") : QString("Unknown");
		// Determine name of primary key.
		// If printable, non-space unicode character,
		//  then display it directly instead of by name
		//  (e.g. '!' instead of 'Exclam')
		// UNLESS we're not wanting internationalization.  Then all
		//  keys should be printed with their ASCII-7 name.
		if( bi18n && keySymQt < 0x1000 && QChar(keySymQt).isPrint() && !QChar(keySymQt).isSpace() )
			keyStr = QChar(keySymQt);
		else {
			for( int i = 0; i < NB_KEYS; i++ ) {
				if( keySymQt == (uint) KKEYS[i].code ) {
				     if (bi18n)
              				keyStr = i18n("QAccel", KKEYS[i].name);
            			     else
              				keyStr = KKEYS[i].name;
			  	     break;
				}
			}
		}
	}

	return !keyStr.isEmpty() ? (keyModStr + keyStr) : QString::null;
}

uint KKeyX11::stringToKey( const QString& key )
{
	QString keyStr = key;

	if( key == "default" )  // old code used to write just "default"
		return 0;                           //  which is not enough
	if( key.startsWith( "default(" )) {
		int pos = key.findRev( ')' );
		if( pos >= 0 ) // this should be really done with regexp
			keyStr = key.mid( 8, pos - 8 );
	}

	kdDebug(125) << QString("stringToKey("+key+") = %1\n").arg(stringToKey( keyStr, 0, 0, 0 ), 0, 16);

	return stringToKey( keyStr, 0, 0, 0 );
}

// Return value is Qt key code.
uint KKeyX11::stringToKey( const QString& keyStr, unsigned char *pKeyCodeX, uint *pKeySymX, uint *pKeyModX )
{
	uint	keySymX = 0;
	unsigned char	keyCodeX = 0;
	uint	keyModX = 0;
	uint	keyCombQt = 0;
	QString sKeySym;
	QChar	c;

	// Initialize
	if( pKeySymX )	*pKeySymX = 0;
	if( pKeyCodeX )	*pKeyCodeX = 0;
	if( pKeyModX )	*pKeyModX = 0;

	if( keyStr.isNull() || keyStr.isEmpty() )
		return 0;

	if( !ModKeyXQt::bInitialized )
		KKeyX11::readModifierMapping();

	int iOffset = 0, iOffsetToken;
	do {
		int i;

		// Find next token.
		iOffsetToken = keyStr.find( '+', iOffset );
		// If no more '+'s are found, set to end of string.
		if( iOffsetToken < 0 )
			iOffsetToken = keyStr.length();
		// Allow a '+' to be the keysym if it's the last character.
		else if( iOffsetToken == iOffset && iOffset + 1 == (int)keyStr.length() )
			iOffsetToken++;
		sKeySym = keyStr.mid( iOffset, iOffsetToken - iOffset ).stripWhiteSpace();
		iOffset = iOffsetToken + 1;

		// Check if this is a modifier key (Shift, Ctrl, Alt, Meta).
		for( i = 0; i < MOD_KEYS; i++ ) {
			if( g_aModKeys[i].keyModMaskQt && qstricmp( sKeySym.ascii(), g_aModKeys[i].keyName ) == 0 ) {
				// If there is no X mod flag defined for this modifier,
				//  then all zeroes should be returned for the X-codes.
				// Ex: If string="Meta+F1", but X hasn't assigned Meta, don't return 'F1'.
				if( g_aModKeys[i].keyModMaskX == 0 ) {
					if( sKeySym == "Meta" )
						kdDebug(125) << "\t\tNo keyModMaskX for Meta" << endl;
					pKeyCodeX = 0;
					pKeySymX = 0;
					pKeyModX = 0;
				}
				kdDebug(125) << "\t\tMeta has Qt " << QString::number(g_aModKeys[i].keyModMaskQt,16) << " and X " << QString::number(g_aModKeys[i].keyModMaskX,16) << endl;
				keyCombQt |= g_aModKeys[i].keyModMaskQt;
				keyModX |= g_aModKeys[i].keyModMaskX;
				break;
			}
		}

		// If this was not a modifier key,
		//  search for 'normal' key.
		if( i == MOD_KEYS ) {
			// Abort if already found primary key.
			if( !c.isNull() || keySymX ) {
				c = QChar::null;
				keySymX = keyModX = keyCombQt = 0;
				break;
			}
			//if( keySymX ) { kdWarning(125) << "keystrToKey: Tried to set more than one key in key code." << endl; return 0; }

			if( sKeySym.length() == 1 )
				c = sKeySym[0];
			else {
				// Search for Qt keycode
				for( i = 0; i < NB_KEYS; i++ ) {
					if( qstricmp( sKeySym.ascii(), KKEYS[i].name ) == 0 ) {
						keyCombQt |= KKEYS[i].code;
						keyQtToKeyX( KKEYS[i].code, 0, &keySymX, 0 );
						if( KKEYS[i].code < 0x1000 && QChar(KKEYS[i].code).isLetter() )
							c = KKEYS[i].code;
						break;
					}
				}

				//if( i == NB_KEYS ) { kdWarning(125) << "keystrToKey: Unknown key name " << sKeySym << endl; return 0; }
				if( i == NB_KEYS ) {
					c = QChar::null;
					keySymX = keyModX = keyCombQt = 0;
					break;
				}
			}
		}
	} while( (uint)iOffsetToken < keyStr.length() );

	if( !c.isNull() ) {
		if( c.isLetter() && !(keyModX & ShiftMask) )
			c = c.lower();
		keySymX = c.unicode();
		// For some reason, Qt always wants 'a-z' as 'A-Z'.
		if( c >= 'a' && c <= 'z' )
			c = c.upper();
		keyCombQt |= c.unicode();
	}

	if( keySymX ) {
		// Find X key code (code of key sent from keyboard)
		keyCodeX = XKeysymToKeycode( qt_xdisplay(), keySymX );

		// If 'Shift' has been explicitly give, i.e. 'Shift+1',
		if( keyModX & ShiftMask ) {
			int index = keySymXIndex( keySymX );
			// But symbol given is unshifted, i.e. '1'
			if( index == 0 || index == 2 ) {
				keySymX = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, index+1 );
				keyCombQt = keySymXToKeyQt( keySymX, keyModX );
			}
		}

		// If keySym requires Shift or ModeSwitch to activate,
		//  then add the flags.
		if( keySymX != XK_Sys_Req && keySymX != XK_Break )
			keySymXMods( keySymX, &keyCombQt, &keyModX );
	}

	// Take care of complications:
	//  The following keys will not have been correctly interpreted,
	//   because their shifted values are not activated with the
	//   Shift key, but rather something else.  They are also
	//   defined twice under different keycodes.
	//  keycode 111 & 92:  Print Sys_Req -> Sys_Req = Alt+Print
	//  keycode 110 & 114: Pause Break   -> Break = Ctrl+Pause
	if( (keyCodeX == 92 || keyCodeX == 111) &&
	    XKeycodeToKeysym( qt_xdisplay(), 92, 0 ) == XK_Print &&
	    XKeycodeToKeysym( qt_xdisplay(), 111, 0 ) == XK_Print )
	{
		// If Alt is pressed, then we need keycode 92, keysym XK_Sys_Req
		if( keyModX & keyModXAlt() ) {
			keyCodeX = 92;
			keySymX = XK_Sys_Req;
		}
		// Otherwise, keycode 111, keysym XK_Print
		else {
			keyCodeX = 111;
			keySymX = XK_Print;
		}
	}
	else if( (keyCodeX == 110 || keyCodeX == 114) &&
	    XKeycodeToKeysym( qt_xdisplay(), 110, 0 ) == XK_Pause &&
	    XKeycodeToKeysym( qt_xdisplay(), 114, 0 ) == XK_Pause )
	{
		if( keyModX & keyModXCtrl() ) {
			keyCodeX = 114;
			keySymX = XK_Break;
		} else {
			keyCodeX = 110;
			keySymX = XK_Pause;
		}
	}

	if( pKeySymX )	*pKeySymX = keySymX;
	if( pKeyCodeX )	*pKeyCodeX = keyCodeX;
	if( pKeyModX )	*pKeyModX = keyModX;

	return keyCombQt;
}

uint KKeyX11::keyCodeXToKeySymX( uchar keyCodeX, uint keyModX )
{
	uint keySymX = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, 0 );

	// Alt+Print = Sys_Req
	if( keySymX == XK_Print ) {
		if( keyModX & keyModXAlt() && XKeycodeToKeysym( qt_xdisplay(), keyCodeX, 1 ) == XK_Sys_Req )
			keySymX = XK_Sys_Req;
	}
	// Ctrl+Pause = Break
	else if( keySymX == XK_Pause ) {
		if( keyModX & keyModXCtrl() && XKeycodeToKeysym( qt_xdisplay(), keyCodeX, 1 ) == XK_Break )
			keySymX = XK_Break;
	} else {
		// I don't know where it's documented, but Mode_shift sometimes sets the 13th bit in 'state'.
		int index = ((keyModX & ShiftMask) ? 1 : 0) +
			((keyModX & (0x2000 | keyModXModeSwitch())) ? 2 : 0);
		keySymX = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, index );
	}

	return keySymX;
}

void KKeyX11::keyEventXToKeyX( const XEvent *pEvent, uchar *pKeyCodeX, uint *pKeySymX, uint *pKeyModX )
{
	if( pKeyCodeX )	*pKeyCodeX = pEvent->xkey.keycode;
	if( pKeySymX )	*pKeySymX = keyCodeXToKeySymX( pEvent->xkey.keycode, pEvent->xkey.state );
	if( pKeyModX )	*pKeyModX = pEvent->xkey.state;
}

uint KKeyX11::keyEventXToKeyQt( const XEvent *pEvent )
{
	uint keySymX, keyModX;
	keyEventXToKeyX( pEvent, 0, &keySymX, &keyModX );
	return keySymXToKeyQt( keySymX, keyModX );
}

int KKeyX11::keySymXIndex( uint keySym )
{
	unsigned char keyCode = XKeysymToKeycode( qt_xdisplay(), keySym );

	if( keyCode ) {
		for( int i = 0; i < 4; i++ ) {
			uint keySym2 = XKeycodeToKeysym( qt_xdisplay(), keyCode, i );
			if( keySym == keySym2 )
				return i;
		}
	}

	return -1;
}

void KKeyX11::keySymXMods( uint keySym, uint *pKeyModQt, uint *pKeyModX )
{
	uint keyModQt = 0, keyModX = 0;
	int i = keySymXIndex( keySym );

	if( i == 1 || i == 3 ) {
		keyModQt |= Qt::SHIFT;
		keyModX |= ShiftMask;
	}
	if( i == 2 || i == 3 ) {
		keyModX |= keyModXModeSwitch();
	}

	if( pKeyModQt )	*pKeyModQt |= keyModQt;
	if( pKeyModX )	*pKeyModX |= keyModX;
}

uint KKeyX11::keyCodeXToKeyQt( uchar keyCodeX, uint keyModX )
{
	return keySymXToKeyQt( keyCodeXToKeySymX( keyCodeX, keyModX ), keyModX );
}

uint KKeyX11::keySymXToKeyQt( uint keySymX, uint keyModX )
{
	uint	keyCombQt = 0;

	if( !ModKeyXQt::bInitialized )
		KKeyX11::readModifierMapping();

	// Qt's own key definitions begin at 0x1000
	if( keySymX < 0x1000 ) {
		// For some reason, Qt wants 'a-z' converted to 'A-Z'
		if( keySymX >= 'a' && keySymX <= 'z' )
			keyCombQt = toupper( keySymX );
		else
			keyCombQt = keySymX;
	}

	if( !keyCombQt ) {
		// Find name of key, and assign its code to keyCombQt.
		const char *psKeySym = XKeysymToString( keySymX );
		for( int i = 0; i < NB_KEYS; i++ ) {
			if( qstricmp( psKeySym, KKEYS[i].name ) == 0 ) {
				keyCombQt = KKEYS[i].code;
				break;
			}
		}
	}

	if( !keyCombQt ) {
		for( uint i = 0; i < sizeof(g_aTransKeySyms)/sizeof(TransKey); i++ ) {
			if( keySymX == g_aTransKeySyms[i].keySymX ) {
				keyCombQt = g_aTransKeySyms[i].keySymQt;
				break;
			}
		}
	}

	if( !keyCombQt ) {
		if( keySymX == XK_Sys_Req )
			keyCombQt = Qt::Key_Print | Qt::ALT;
		else if( keySymX == XK_Break )
			keyCombQt = Qt::Key_Pause | Qt::CTRL;
	}

	if( keyCombQt ) {
		// Get Qt modifier flags
		for( int i = 0; i < MOD_KEYS; i++ ) {
			if( keyModX & g_aModKeys[i].keyModMaskX )
				keyCombQt |= g_aModKeys[i].keyModMaskQt;
		}
	}

	return keyCombQt;
}

void KKeyX11::keyQtToKeyX( uint keyCombQt, unsigned char *pKeyCodeX, uint *pKeySymX, uint *pKeyModX )
{
	uint	keySymQt;
	uint	keySymX = 0;
	unsigned char	keyCodeX = 0;
	uint	keyModX = 0;

	const char *psKeySym = 0;

	if( !ModKeyXQt::bInitialized )
		KKeyX11::readModifierMapping();

	// Get code of just the primary key
	keySymQt = keyCombQt & 0xffff;

	// If unicode value beneath 0x1000 (special Qt codes begin thereafter),
	if( keySymQt < 0x1000 ) {
		// For reasons unbeknownst to me, Qt converts 'a-z' to 'A-Z'.
		// So convert it back to lowercase if SHIFT isn't held down.
		if( keySymQt >= Qt::Key_A && keySymQt <= Qt::Key_Z && !(keyCombQt & Qt::SHIFT) )
			keySymQt = tolower( keySymQt );
		keySymX = keySymQt;
	}
	// Else, special key (e.g. Delete, F1, etc.)
	else {
		for( int i = 0; i < NB_KEYS; i++ ) {
			if( keySymQt == (uint) KKEYS[i].code ) {
				psKeySym = KKEYS[i].name;
				//kdDebug(125) << " symbol found: \"" << psKeySym << "\"" << endl;
				break;
			}
		}

		// Get X key symbol.  Only works if Qt name is same as X name.
		if( psKeySym ) {
			QString sKeySym = psKeySym;

			// Check for lower-case equalent first because most
			//  X11 names are all lower-case.
			keySymX = XStringToKeysym( sKeySym.lower().ascii() );
			if( keySymX == 0 )
				keySymX = XStringToKeysym( psKeySym );
		}

		if( keySymX == 0 ) {
			for( uint i = 0; i < sizeof(g_aTransKeySyms)/sizeof(TransKey); i++ )
			{
				if( keySymQt == g_aTransKeySyms[i].keySymQt ) {
					keySymX = g_aTransKeySyms[i].keySymX;
					break;
				}
			}
		}
	}

	if( keySymX != 0 ) {
		// Get X keyboard code
		keyCodeX = XKeysymToKeycode( qt_xdisplay(), keySymX );
		// Add ModeSwitch modifier bit, if necessary
		keySymXMods( keySymX, 0, &keyModX );

		// Get X modifier flags
		for( int i = 0; i < MOD_KEYS; i++ ) {
			if( keyCombQt & g_aModKeys[i].keyModMaskQt ) {
				if( g_aModKeys[i].keyModMaskX )
					keyModX |= g_aModKeys[i].keyModMaskX;
				// Qt key calls for a modifier which the current
				//  X modifier map doesn't support.
				else {
					keySymX = 0;
					keyCodeX = 0;
					keyModX = 0;
					break;
				}
			}
		}
	}

	// Take care of complications:
	//  The following keys will not have been correctly interpreted,
	//   because their shifted values are not activated with the
	//   Shift key, but rather something else.  They are also
	//   defined twice under different keycodes.
	//  keycode 111 & 92:  Print Sys_Req -> Sys_Req = Alt+Print
	//  keycode 110 & 114: Pause Break   -> Break = Ctrl+Pause
	if( (keyCodeX == 92 || keyCodeX == 111) &&
	    XKeycodeToKeysym( qt_xdisplay(), 92, 0 ) == XK_Print &&
	    XKeycodeToKeysym( qt_xdisplay(), 111, 0 ) == XK_Print )
	{
		// If Alt is pressed, then we need keycode 92, keysym XK_Sys_Req
		if( keyModX & keyModXAlt() ) {
			keyCodeX = 92;
			keySymX = XK_Sys_Req;
		}
		// Otherwise, keycode 111, keysym XK_Print
		else {
			keyCodeX = 111;
			keySymX = XK_Print;
		}
	}
	else if( (keyCodeX == 110 || keyCodeX == 114) &&
	    XKeycodeToKeysym( qt_xdisplay(), 110, 0 ) == XK_Pause &&
	    XKeycodeToKeysym( qt_xdisplay(), 114, 0 ) == XK_Pause )
	{
		if( keyModX & keyModXCtrl() ) {
			keyCodeX = 114;
			keySymX = XK_Break;
		} else {
			keyCodeX = 110;
			keySymX = XK_Pause;
		}
	}

	if( pKeySymX )	*pKeySymX = keySymX;
	if( pKeyCodeX ) *pKeyCodeX = keyCodeX;
	if( pKeyModX )  *pKeyModX = keyModX;
}

QString KKeyX11::keyCodeXToString( uchar keyCodeX, uint keyModX, bool bi18n )
	{ return keyToString( keyCodeXToKeyQt( keyCodeX, keyModX ), bi18n ); }
QString KKeyX11::keySymXToString( uint keySymX, uint keyModX, bool bi18n )
	{ return keyToString( keySymXToKeyQt( keySymX, keyModX ), bi18n ); }

uint KKeyX11::keyModXShift()		{ return ShiftMask; }
uint KKeyX11::keyModXLock()		{ return LockMask; }
uint KKeyX11::keyModXCtrl()		{ return ControlMask; }

uint KKeyX11::keyModXAlt()
{
	if( !ModKeyXQt::bInitialized )
		KKeyX11::readModifierMapping();
	return g_aModKeys[ModAltIndex].keyModMaskX;
}

uint KKeyX11::keyModXNumLock()
{
	if( !ModKeyXQt::bInitialized )
		KKeyX11::readModifierMapping();
	return g_aModKeys[ModNumLockIndex].keyModMaskX;
}

uint KKeyX11::keyModXModeSwitch()
{
	if( !ModKeyXQt::bInitialized )
		KKeyX11::readModifierMapping();
	return g_aModKeys[ModModeSwitchIndex].keyModMaskX;
}

uint KKeyX11::keyModXMeta()
{
	if( !ModKeyXQt::bInitialized )
		KKeyX11::readModifierMapping();
	return g_aModKeys[ModMetaIndex].keyModMaskX;
}

uint KKeyX11::keyModXScrollLock()
{
	if( !ModKeyXQt::bInitialized )
		KKeyX11::readModifierMapping();
	return g_aModKeys[ModScrollLockIndex].keyModMaskX;
}

uint KKeyX11::accelModMaskX()		{ return ShiftMask | ControlMask | keyModXAlt() | keyModXMeta(); }

bool KKeyX11::keyboardHasMetaKey()
{
	if( !ModKeyXQt::bInitialized )
		KKeyX11::readModifierMapping();
	return keyModXMeta() != 0;
}

#endif // Q_WS_X11
