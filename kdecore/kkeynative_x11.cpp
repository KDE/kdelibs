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

#include "kkeynative.h"
#include "kkeyserver_x11.h"

#include <qmap.h>
#include <qstringlist.h>
#include <kckey.h>
#include <kdebug.h>
#include <klocale.h>

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <ctype.h>
#undef NONE

#ifdef Q_WS_X11
#ifndef KDE_USE_FINAL
// defined by X11 headers
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#endif
#endif

//---------------------------------------------------------------------

static KKeyNative* gx_pkey = 0;

//---------------------------------------------------------------------
// KKeyNative::Variations
//---------------------------------------------------------------------

KKeyNative::Variations::~Variations()
{
}

// TODO: allow for sym to have variations, such as Plus => { Plus, KP_Add }
void KKeyNative::Variations::init( const KKey& key, bool bQt )
{
	if( key.isNull() ) {
		m_nVariations = 0;
		return;
	}

	m_nVariations = 1;
	m_rgkey[0].init( key );

	if( bQt ) {
		for( uint i = 0; i < m_nVariations; i++ )
			m_rgkey[i].init( m_rgkey[i].keyCodeQt() );

		// Two different native codes may produce a single
		//  Qt code.  Search for duplicates.
		for( uint i = 1; i < m_nVariations; i++ ) {
			for( uint j = 0; j < i; j++ ) {
				// If key is already present in list, then remove it.
				if( m_rgkey[i] == m_rgkey[j] ) {
					for( uint k = i; k < m_nVariations - 1; k++ )
						m_rgkey[k] = m_rgkey[k+1];
					m_nVariations--;
					i--;
					break;
				}
			}
		}
	}
}

//---------------------------------------------------------------------
// KKeyNative
//---------------------------------------------------------------------

KKeyNative::KKeyNative()                           { clear(); }
KKeyNative::KKeyNative( const KKey& key )          { init( key ); }
KKeyNative::KKeyNative( const KKeyNative& key )    { init( key ); }
KKeyNative::KKeyNative( const XEvent* pEvent )  { init( pEvent ); }
KKeyNative::~KKeyNative()
	{ }

void KKeyNative::clear()
{
	m_code = 0;
	m_mod = 0;
	m_sym = 0;
}

bool KKeyNative::init( const XEvent* pEvent )
{
	m_code = pEvent->xkey.keycode;
	m_mod = pEvent->xkey.state & KKeyServer::accelModMaskX();
	XLookupString( (XKeyEvent*) pEvent, 0, 0, (KeySym*) &m_sym, 0 );
	return true;
}

// TODO: Avoid call to XKeysymToKeycode here by creating a method in KKeyServer.
bool KKeyNative::init( const KKey& key )
{
	// Get any extra mods required by the sym.
	//  E.g., XK_Plus requires SHIFT on the en layout.
	m_sym = key.key();
	uint modExtra = KKeyServer::Sym(m_sym).getModsRequired();
	// Get the X modifier equivalent.
	if( !KKeyServer::modToModX( key.modFlags() | modExtra, m_mod ) ) {
		m_sym = m_mod = 0;
		return false;
	}

	m_code = XKeysymToKeycode( qt_xdisplay(), m_sym );
	// Now get the true sym formed by the modifiers
	//  E.g., Shift+Equal => Plus on the en layout.
	if( key.modFlags() )
		KKeyServer::codeXToSym( m_code, m_mod, m_sym );

	return true;
}

bool KKeyNative::init( const KKeyNative& key )
{
	m_code = key.m_code;
	m_mod = key.m_mod;
	m_sym = key.m_sym;
	return true;
}

uint KKeyNative::code() const { return m_code; }
uint KKeyNative::mod() const  { return m_mod; }
uint KKeyNative::sym() const  { return m_sym; }

bool KKeyNative::isNull() const
{
	return m_sym == 0;
}

int KKeyNative::compare( const KKeyNative& key ) const
{
	if( m_sym != key.m_sym )   return m_sym - key.m_sym;
	if( m_mod != key.m_mod )   return m_mod - key.m_mod;
	if( m_code != key.m_code ) return m_code - key.m_code;
	return 0;
}

KKeyNative& KKeyNative::null()
{
	if( !gx_pkey )
		gx_pkey = new KKeyNative;
	if( !gx_pkey->isNull() )
		gx_pkey->clear();
	return *gx_pkey;
}

KKey KKeyNative::key() const
{
	uint modSpec;
	if( KKeyServer::modXToMod( m_mod, modSpec ) )
		return KKey( m_sym, modSpec );
	else
		return KKey();
}

int KKeyNative::keyCodeQt() const
{
	int keyQt = KKeyServer::Sym(m_sym).qt(), modQt;

	if( keyQt != Qt::Key_unknown && KKeyServer::modXToModQt( m_mod, modQt ) )
		return keyQt | modQt;

	return 0;
}

uint KKeyNative::modX( KKey::ModFlag modFlag )
{
	return KKeyServer::modX( modFlag );
}

// Returns true if X has the Meta key assigned to a modifier bit
// FIXME: use the settings in kdeglobals for Meta/Super/Hyper
bool KKeyNative::keyboardHasWinKey()
{
	return KKeyServer::keyboardHasWinKey();
}

//---------------------------------------------------------------------
// KKeyNative helper functions
//---------------------------------------------------------------------

#endif // Q_WS_X11
