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

#include "kkeysequenceold.h"

#include <qaccel.h>
#include <qnamespace.h>
#include <qwindowdefs.h>
#include <kckey.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

//-------------------------------------------------------------------

KKeySequenceOld::KKeySequenceOld()
	{ clear(); }
KKeySequenceOld::KKeySequenceOld( const KKeySequenceOld& k )
	{ *this = k; }
KKeySequenceOld::KKeySequenceOld( const QKeySequence& k )
	{ *this = (uint) (int) k; }

KKeySequenceOld::KKeySequenceOld( const QString& k )
{
	KKeySequenceOlds rg( k );
	if( rg.size() >= 1 )
		*this = rg[0];
	else
		clear();
}

KKeySequenceOld::KKeySequenceOld( uint keyCombQt )
	{ *this = keyCombQt; }

KKeySequenceOld::KKeySequenceOld( const QKeyEvent *pke )
{
	uint keyCombQt = 0;

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

	*this = keyCombQt;
}

void KKeySequenceOld::clear()
{
	m_origin = OriginUnset;
	m_keyCombQt = -1;
	m_keyMod = -1;
	m_keyCode = m_keySym = 0;
	m_keyCombQtExplicit = m_keySymExplicit = m_keyModExplicit = 0;
}

//KKeySequenceOld& KKeySequenceOld::operator =( KKeySequenceOld k )      { m_keyCombQt = k.m_keyCombQt; return *this; }
KKeySequenceOld& KKeySequenceOld::operator =( uint keyCombQt )
{
	m_origin = OriginQt;
	m_keyCombQtExplicit = keyCombQt;
	// TODO: decide whether to check if this key needs 'Shift'
	//  (Ctrl+% = Ctrl+Shift+5 in EN layout, for example)
	m_keyCombQt = keyCombQt;

	m_keyMod = -1;
	m_keyCode = m_keySym = 0;
	m_keySymExplicit = m_keyModExplicit = 0;


	return *this;
}

// Qt Key:
//  Enter == XK_Return, XK_KP_Enter, XK_ISO_Enter, XK_3270_Enter
//  Plus == Shift+'=', XK_KP_Add
// Key Sym:
//  XK_Return ==
// Key Code: XK_KP_Enter ==
//  we may initialize by key code in order to distinguish 'Enter'
//  from 'Keypad_Enter',

// A Qt key may have multiple Key Sym equivalents.
// A key sym may be associated with multiple key codes

int KKeySequenceOld::compare( const KKeySequenceOld& a, const KKeySequenceOld& b )
{
	if( a.m_origin == OriginUnset && b.m_origin == OriginUnset )
		return 0;
	if( a.m_origin == OriginUnset )
		return -1;
	if( b.m_origin == OriginUnset )
		return 1;

	if( a.m_origin != b.m_origin ) {
		//KKeySequenceOld k1( a ), k2( b );
		//k1.calcKeyQt();
		//k2.calcKeyQt();
		//return k1.m_keyCombQt - k2.m_keyCombQt;
		KKeySequenceOld* pA = (KKeySequenceOld*) &a;
		KKeySequenceOld* pB = (KKeySequenceOld*) &b;
		if( a.m_keyCombQt == -1 )
			pA->calcKeyQt();
		if( b.m_keyCombQt == -1 )
			pB->calcKeyQt();
		return a.m_keyCombQt - b.m_keyCombQt;
	}

	if( a.m_origin == OriginQt )
		return a.m_keyCombQt - b.m_keyCombQt;

	if( a.m_keyMod != b.m_keyMod )
		return a.m_keyMod - b.m_keyMod;

	/*if( a.m_origin == OriginNativeCode )
		return a.m_keyCode - b.m_keyCode;
	else*/
		return a.m_keySym - b.m_keySym;
}

KKeySequenceOld::operator QKeySequence()
{
	if( m_origin == OriginUnset )
		return QKeySequence();
	else if( m_keyCombQt == -1 )
		calcKeyQt();
	return QKeySequence( (int) m_keyCombQt );
}

int KKeySequenceOld::keyQt()
{
	if( m_keyCombQt == -1 )
		calcKeyQt();
	return m_keyCombQt;
}

/*
uint KKeySequenceOld::key() const                        { return m_keyCombQt; }
uint KKeySequenceOld::sym() const                        { return m_keyCombQt & 0xffff; }
uint KKeySequenceOld::mod() const                        { return m_keyCombQt & ~0xffff; }
uint KKeySequenceOld::state() const                      { return mod() >> 18; }
*/
bool KKeySequenceOld::isNull() const
{
	return m_origin == OriginUnset
		|| (m_origin == OriginQt && m_keyCombQt == 0)
		|| (m_origin == OriginNative && m_keySym == 0);
}

//-------------------------------------------------------------------
// Config File Functions
//-------------------------------------------------------------------

// Indicate whether to default to the 3- or 4- modifier keyboard schemes
// This variable should also be moved into a class along with the
// X11-related key functions below.
static int g_bUseFourModifierKeys = -1;

bool KKeySequenceOld::useFourModifierKeys()
{
	if( g_bUseFourModifierKeys == -1 ) {
		// Read in whether to use 4 modifier keys
		KConfigGroupSaver cgs( KGlobal::config(), "Keyboard Layout" );
		bool b = KGlobal::config()->readBoolEntry( "Use Four Modifier Keys",  false );
		g_bUseFourModifierKeys = b && keyboardHasMetaKey();
	}
	return g_bUseFourModifierKeys == 1;
}

void KKeySequenceOld::useFourModifierKeys( bool b )
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

bool KKeySequenceOld::qtSupportsMetaKey()
{
	static int qtSupport = -1;
	if( qtSupport == -1 ) {
		qtSupport = QAccel::stringToKey("Meta+A") & (Qt::ALT<<1);
		kdDebug(125) << "Qt Supports Meta Key: " << qtSupport << endl;
	}
	return qtSupport == 1;
}

//-------------------------------------------------------------------

uint KKeySequenceOld::accelModMaskQt()		{ return Qt::SHIFT | Qt::CTRL | Qt::ALT | (Qt::ALT<<1); }

//-------------------------------------------------------------------

KKeySequenceOlds::KKeySequenceOlds( const QString& sKey )
{
	*this = KKeySequenceOld::stringToKeys( sKey );
}

KKeySequenceOld KKeySequenceOlds::first() const
{
	if( size() > 0 )
		return operator[]( 0 );
	else
		return KKeySequenceOld();
}

bool KKeySequenceOlds::operator ==( KKeySequenceOlds& keys )
{
	if( size() != keys.size() )
		return false;

	for( uint i = 0; i < size(); i++ ) {
		if( operator[]( i ) != keys[i] )
			return false;
	}

	return true;
}
