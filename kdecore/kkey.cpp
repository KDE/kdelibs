/*
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997-2000 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>

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

#include "kkey.h"
#include "kkey_x11.h"

#include <qaccel.h>
#include <qnamespace.h>
#include <qwindowdefs.h>
#include <kckey.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

//-------------------------------------------------------------------

KKey::KKey( const QKeyEvent *pEvent )	{ m_keyCombQt = KKey::keyEventQtToKeyQt( pEvent ); }
KKey::KKey( const QString& keyStr )	{ *this = KKey::stringToKey( keyStr ); }
QString KKey::toString()		{ return KKey::keyToString( m_keyCombQt ); }

//-------------------------------------------------------------------
// Config File Functions
//-------------------------------------------------------------------

// Indicate whether to default to the 3- or 4- modifier keyboard schemes
// This variable should also be moved into a class along with the
// X11-related key functions below.
static int g_bUseFourModifierKeys = -1;

bool KKey::useFourModifierKeys()
{
	if( g_bUseFourModifierKeys == -1 ) {
		// Read in whether to use 4 modifier keys
		KConfigGroupSaver cgs( KGlobal::config(), "Keyboard Layout" );
		bool fourMods = KGlobal::config()->readBoolEntry( "Use Four Modifier Keys",  false );
		g_bUseFourModifierKeys = fourMods && keyboardHasMetaKey();
	}
	return g_bUseFourModifierKeys == 1;
}

void KKey::useFourModifierKeys( bool b )
{
	if( g_bUseFourModifierKeys != (int)b ) {
		g_bUseFourModifierKeys = b && keyboardHasMetaKey();
		// If we're 'turning off' the meta key or, if we're turning it on,
		//  the keyboard must actually have a meta key.
		if( b && !keyboardHasMetaKey() )
			kdDebug(125) << "Tried to use four modifier keys on a keyboard layout without a Meta key.\n";
	}
	KConfigGroupSaver cgs( KGlobal::config(), "Keyboard Layout" );
	KGlobal::config()->writeEntry( "Use Four Modifier Keys", g_bUseFourModifierKeys, true, true);

	kdDebug(125) << "bUseFourModifierKeys = " << g_bUseFourModifierKeys << endl;
}

bool KKey::qtSupportsMetaKey()
{
	static int qtSupport = -1;
	if( qtSupport == -1 ) {
		qtSupport = QAccel::stringToKey("Meta+A") & (Qt::ALT<<1);
		kdDebug(125) << "Qt Supports Meta Key: " << qtSupport << endl;
	}
	return qtSupport == 1;
}

//-------------------------------------------------------------------

QString KKey::keyToString( int keyCombQt, bool bi18n )
{
#ifdef Q_WS_X11
	return KKeyX11::keyToString( keyCombQt, bi18n );
#else
	QString keyStr, keyModStr;
	uint keySymQt = keyCombQt & 0xffff;
	uint keyModQt = keyCombQt & ~0xffff;

	//FIXME: This is just the basics, needs to become more advanced
	if(keySymQt && (Qt::ALT<<1))
		keyModStr += ((bi18n) ? i18n("Meta") : "Meta") + "+";
	if(keySymQt & Qt::ALT)
		keyModStr += ((bi18n) ? i18n("Alt") : "Alt") + "+";
	if(keySymQt & Qt::CTRL)
		keyModStr += ((bi18n) ? i18n("Ctrl") : "Ctrl") + "+";
	if(keySymQt & Qt::SHIFT)
		keyModStr = ((bi18n) ? i18n("Shift") : "Shift") + "+";

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

	return !keyStr.isEmpty() ? (keyModStr + keyStr) : QString::null;
#endif
}

KKey KKey::stringToKey( const QString& key )
{
#ifdef Q_WS_X11
	uint keyCombQt = KKeyX11::stringToKey( key );
	return KKey( keyCombQt );
#else
	QString keyStr = key;

        if( key == "default" )  // old code used to write just "default"
            return 0;                           //  which is not enough
        if( key.startsWith( "default(" )) {
            int pos = key.findRev( ')' );
            if( pos >= 0 ) // this should be really done with regexp
                keyStr = key.mid( 8, pos - 8 );
        }

	kdDebug(125) << QString("stringToKey("+key+") = %1\n").arg(stringToKey( keyStr, 0, 0, 0 ), 0, 16);

	uint	keyCombQt = 0;
	QString sKeySym;
	QChar	c;

	if( keyStr.isNull() || keyStr.isEmpty() )
		return 0;

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

		//FIXME(E): This needs to be refined
		if(keyStr.contains("Shift"))
			keyCombQt |= Qt::SHIFT;
		if(keyStr.contains("Ctrl"))
			keyCombQt |= Qt::CTRL;
		if(keyStr.contains("Alt"))
			keyCombQt |= Qt::ALT;
		if(keyStr.contains("Meta"))
			keyCombQt |= (Qt::ALT<<1);

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
		if( c.isLetter() && (keyCombQt&Qt::SHIFT)!=Qt::SHIFT )
			c = c.lower();

		// For some reason, Qt always wants 'a-z' as 'A-Z'.
		if( c >= 'a' && c <= 'z' )
			c = c.upper();
		keyCombQt |= c.unicode();
	}

	return keyCombQt;
#endif
}

uint KKey::keyEventQtToKeyQt( const QKeyEvent* pke )
{
        uint keyCombQt;

	// Set the modifier bits.
	keyCombQt = (pke->state() & Qt::KeyButtonMask) * (Qt::SHIFT / Qt::ShiftButton);

	if( pke->key() )
		keyCombQt |= pke->key();
	// If key() == 0, then it may be a compose character, so we need to
	//  look at text() instead.
	else {
		QChar c = pke->text()[0];
		// Looks like Qt allows unicode character up to 0x0fff.
		if( pke->text().length() == 1 && c.unicode() < 0x1000 )
			keyCombQt |= c.unicode();
		else
			keyCombQt |= Qt::Key_unknown;
	}

	return keyCombQt;
}

uint KKey::accelModMaskQt()		{ return Qt::SHIFT | Qt::CTRL | Qt::ALT | (Qt::ALT<<1); }

bool KKey::keyboardHasMetaKey()
{
#ifdef Q_WS_X11
	return KKeyX11::keyboardHasMetaKey();
#else
	//FIXME(E): Is there any way to implement this in Qt/Embedded?
	return false;
#endif
}
