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
#include "kkey_x11.h"

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
#ifdef KeyPress
// defined by X11 headers
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#endif
#endif

//---------------------------------------------------------------------
struct ModInfo
{
	int modSpec, modQt, modX;
};

ModInfo g_rgModInfo[KKey::MOD_FLAG_COUNT] =
{
	{ KKey::SHIFT, Qt::SHIFT,   ShiftMask },
	{ KKey::CTRL,  Qt::CTRL,    ControlMask },
	{ KKey::ALT,   Qt::ALT,     Mod1Mask },
	{ KKey::WIN,   KKey::QtWIN, Mod4Mask }
};

static KKeyNative* gx_pkey = 0;

//---------------------------------------------------------------------
static bool modSpecToModX( int modSpec, int& modX );
static bool modXToModSpec( int modX, int& modSpec );
static bool modXToModQt( int modX, int& modQt );
static int getModsRequiredForSym( int sym );
static int getSym( int code, int mod );

//---------------------------------------------------------------------
// KKeyNative::Variations
//---------------------------------------------------------------------

KKeyNative::Variations::~Variations()
{
}

void KKeyNative::Variations::init( const KKey& key, bool bQt )
{
	KKeyNative::keyToVariations( key, *this );
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

KKeyNative::KKeyNative()                        { clear(); }
KKeyNative::KKeyNative( const KKey& key )       { init( key ); }
KKeyNative::KKeyNative( const KKeyNative& key ) { init( key ); }
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
	m_mod = pEvent->xkey.state & KKeyX11::accelModMaskX();
	XLookupString( (XKeyEvent*) pEvent, 0, 0, (KeySym*) &m_sym, 0 );
	return true;
}

bool KKeyNative::init( const KKey& key )
{
	modSpecToModX( key.modFlags(), m_mod );
	m_sym = key.key();

	m_mod |= getModsRequiredForSym( m_sym );
	m_code = XKeysymToKeycode( qt_xdisplay(), (uint) m_sym );
	m_sym = getSym( m_code, m_mod );

	return true;
}

bool KKeyNative::init( const KKeyNative& key )
{
	m_code = key.m_code;
	m_mod = key.m_mod;
	m_sym = key.m_sym;
	return true;
}

int KKeyNative::code() const { return m_code; }
int KKeyNative::mod() const  { return m_mod; }
int KKeyNative::sym() const  { return m_sym; }

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

bool KKeyNative::keyQtToSym( int keyQt, int& sym )
{
	KKeyX11::keyQtToKeyX( keyQt & 0xffff, 0, (unsigned*)&sym, 0 );
	return true;
}

bool KKeyNative::symToKeyQt( int sym, int& keyQt )
{
	keyQt = KKeyX11::keySymXToKeyQt( sym, 0 );
	return true;
}

KKey KKeyNative::key() const
{
	int modSpec;
	modXToModSpec( m_mod, modSpec );
	return KKey( m_sym, modSpec );
}

int KKeyNative::keyCodeQt() const
{
	int keyQt, modQt;

	if( symToKeyQt( m_sym, keyQt ) && modXToModQt( m_mod, modQt ) )
		return keyQt | modQt;

	return 0;
}

int KKeyNative::modX( int modSpec )
{
	for( uint i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( g_rgModInfo[i].modSpec == modSpec )
			return g_rgModInfo[i].modX;
	}
	return 0;
}

QString KKeyNative::symToString( int symNative )
{
	return KKeyX11::keySymXToString( symNative, 0, true );
}

QString KKeyNative::symToStringInternal( int symNative )
{
	return KKeyX11::keySymXToString( symNative, 0, false );
}

// Returns true if X has the Meta key assigned to a modifier bit
// FIXME: use the settings in kdeglobals for Meta/Super/Hyper
bool KKeyNative::keyboardHasWinKey()
{
	return KKeySequenceOld::keyboardHasMetaKey();
}

// TODO: allow for sym to have variations, such as Plus => { Plus, KP_Add }
bool KKeyNative::keyToVariations( const KKey& key, Variations& var )
{
	if( key.isNull() ) {
		var.m_nVariations = 0;
		return true;
	}

	var.m_nVariations = 1;
	var.m_rgkey[0].init( key );

	return true;
}

// mod  All set bits will be preserved.  Some new ones may be set.
bool KKeyNative::stringToSym( const QString& sKey, int& sym, int& mod )
{
	KKeySequenceOlds seqs( sKey );
	if( seqs.size() > 0 ) {
		sym = seqs[0].m_keySym;
		mod = seqs[0].m_keyMod;
		return true;
	} else {
		sym = 0;
		mod = 0;
		return true;
	}
/*	sym = 0;

	if( !g_bInitialized )
		Initialize();

	QString sKeySym = sKey.lower().stripWhiteSpace();

	// If this is a single character symbol (such as '!'),
	//  get it's name to use in lookup (such as 'exclam').
	if( sKeySym.length() == 1 && g_mapCharToName.contains( sKeySym[0] ) )
		sKeySym = g_mapCharToName[sKeySym[0]];

	KSymToInfoMap::const_iterator it = g_mapSymToInfo.search( sKeySym );
	if( it != g_mapSymToInfo.end() ) {
		sym = (*it).keySymX;
		int modRequired;
		if( modXToModSpec( (*it).rgMods[iCode], modRequired ) ) {
			mod |= modRequired;
		else
			sym = 0;
	}
	// Otherwise, key name may be valid, but we just don't have
	//  it on our keyboard (such as 'agrave' on an EN layout).
	else {
		sym = XStringToKeysym( sKeySym.latin1() );
		if( !sym )
			key.m_keySymExplicit = XStringToKeysym( (sKeySym[0].upper() + sKeySym.mid(1)).latin1() );
		if( !sym ) {
			for( int i = 0; i < NB_KEYS; i++ ) {
				if( KKEYS[i].code < 0x1000 && qstricmp( KKEYS[i].name, sKeySym.latin1() ) == 0 ) {
					sym = KKEYS[i].code;
					break;
				}
			}
		}
	}

	kdDebug(125) << "stringToKeys( " << sKey << " ):"
		<< " sym: " << QString::number(sym, 16)
		<< ", mod: " << QString::number(mod, 16)
		<< ", str: " << key.toString()
		<< endl;

	return sym != 0;
*/
}

//---------------------------------------------------------------------
// KKeyNative helper functions
//---------------------------------------------------------------------

static bool modSpecToModX( int modSpec, int& modX )
{
	modX = 0;
	for( int i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( modSpec & g_rgModInfo[i].modSpec ) {
			if( !g_rgModInfo[i].modX )
				return false;
			modX |= g_rgModInfo[i].modX;
		}
	}

	return true;
}

static bool modXToModSpec( int modX, int& modSpec )
{
	modSpec = 0;
	for( int i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( modX & g_rgModInfo[i].modX ) {
			if( !g_rgModInfo[i].modSpec )
				return false;
			modSpec |= g_rgModInfo[i].modSpec;
		}
	}

	return true;
}

static bool modXToModQt( int modX, int& modQt )
{
	modQt = 0;
	for( int i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( modX & g_rgModInfo[i].modX ) {
			if( !g_rgModInfo[i].modQt )
				return false;
			modQt |= g_rgModInfo[i].modQt;
		}
	}

	return true;
}

static int getModsRequiredForSym( int sym )
{
	KKeySymX* pInfo = KKeyX11::symInfoPtr( sym );
	return (pInfo) ? pInfo->rgMods[0] : 0;
}

static int getSym( int code, int mod )
{
	return KKeyX11::keyCodeXToKeySymX( (uchar) code, (uint) mod );
}

#endif // Q_WS_X11
