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

#include <iostream.h>	// Remove me -ellis

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

//-------------------------------------------------------------------
static void calcKeySym( KKeySequence& key );
static int calcKeyQt( int keySymX, int keyModX );

static void Initialize();
static void readModifierMapping();
static void readKeyMappingSub( uchar keyCodeX, uint keySymX, uint keyModX );
static void readKeyMapping();

//-------------------------------------------------------------------

#define SYS_REQ_CODE	92
#define PRINT_CODE	111
#define PAUSE_CODE	110
#define BREAK_CODE	114

const int _XMAX = 4;

struct KKeySymX
{
	uint keySymX;
	QString sName;
	int nCodes;
	uchar rgCodes[_XMAX];
	uint rgMods[_XMAX];

	KKeySymX()
	{
		keySymX = 0;
		nCodes = 0;
		for( int i = 0; i < _XMAX; i++ ) {
			rgCodes[i] = 0;
			rgMods[i] = 0;
		}
	}
};

class KSymToInfoMap : public QMap<uint, KKeySymX>
{
 public:
	void add( uint keySymX, QString sName, uchar keyCodeX, uint keyModX )
	{
		KKeySymX& info = operator[]( keySymX );
		int iCode = info.nCodes;

		if( iCode == _XMAX )
			return;
		info.keySymX = keySymX;
		info.sName = sName.lower();
		info.nCodes++;
		info.rgCodes[iCode] = keyCodeX;
		info.rgMods[iCode] = keyModX;
	}

	iterator search( QString sKeySym )
	{
		sKeySym = sKeySym.lower();
		for( iterator it = begin(); it != end(); ++it ) {
			if( (*it).sName == sKeySym )
				return it;
		}
		return end();
	}
	const_iterator search( QString sKeySym, const_iterator it )
	{
		sKeySym = sKeySym.lower();
		for( ; it != end(); ++it ) {
			if( (*it).sName == sKeySym )
				return it;
		}
		return end();
	}
};

typedef QMap<QChar, QString> KCharToNameMap;
KSymToInfoMap g_mapSymToInfo;
KCharToNameMap g_mapCharToName;

static bool g_bInitialized = false;

//-------------------------------------------------------------------

// Return value is Qt key code.
// Some X11 key syms have no Qt equivalent
//  (such as KF86XK_AudioMute = 0x1008FF12 in /usr/include/X11/XF86keysym.h)
// If this
// keyCode is the most specific you can get -- it is a specific physical
//  key on the keyboard.
// keySym
//
#define ToI18N( s ) \
((bi18n) ? i18n("QAccel", s) : QString(s))
QString KKeySequence::toString( KKeySequence::I18N bi18n ) const
{
	if( m_origin == OriginUnset )
		return (bi18n) ? i18n("Unknown Key", "Unset") : QString("Unset");
	// TODO: calculate X codes from Qt codes if they are not already set.
	if( m_keyMod == -1 )
		return (bi18n) ? i18n("Unknown Key", "Unset") : QString("Unset");

	QString sMods, sSym;
	uint keyModX = m_keyModExplicit;
	uint keySymX = m_keySymExplicit;

	if( keyModX & KKeyX11::keyModXMeta() ) sMods += ToI18N("Meta") + "+";
	if( keyModX & KKeyX11::keyModXAlt() )  sMods += ToI18N("Alt") + "+";
	if( keyModX & ControlMask )   sMods += ToI18N("Ctrl") + "+";
	if( keyModX & ShiftMask )     sMods += ToI18N("Shift") + "+";

	if( g_mapSymToInfo.contains( keySymX ) ) {
		const KKeySymX& sym = g_mapSymToInfo[keySymX];
		sSym = sym.sName[0].upper() + sym.sName.mid( 1 );
	} else {
		sSym = XKeysymToString( keySymX );
		if( sSym.isEmpty() )
			return (bi18n) ? i18n("Unknown Key", "Unknown") : QString("Unknown");
	}

	return sMods + sSym;
}

bool g_bCrash = false;
KKeySequences KKeySequence::stringToKeys( QString sKey )
{
	KKeySequences rgKeys;
	KKeySequence key;

	if( g_bCrash ) {
		char* crash = 0;
		*crash = 0;
	}

	if( !g_bInitialized )
		Initialize();

	key.m_origin = OriginNative;
	sKey = sKey.lower().stripWhiteSpace();
	if( sKey.startsWith( "default(" ) && sKey.endsWith( ")" ) )
		sKey = sKey.mid( 8, sKey.length() - 9 );
	QStringList rgs = QStringList::split( '+', sKey, true );

	uint i;
	// Check for modifier keys first.
	key.m_keyModExplicit = 0;
	for( i = 0; i < rgs.size(); i++ ) {
		if( rgs[i] == "meta" )       key.m_keyModExplicit |= Mod4Mask;
		else if( rgs[i] == "alt" )   key.m_keyModExplicit |= Mod1Mask;
		else if( rgs[i] == "ctrl" )  key.m_keyModExplicit |= ControlMask;
		else if( rgs[i] == "shift" ) key.m_keyModExplicit |= ShiftMask;
		else break;
	}
	// If there are 1) one non-blank key left, or
	//  2) two keys left, but they are both blank (in the case of "Ctrl++"),
	if( (i == rgs.size() - 1 && !rgs[i].isEmpty())
		|| (i == rgs.size() - 2 && rgs[i].isEmpty() && rgs[i+1].isEmpty()) )
	{
		QString sKeySym = rgs[i];
		if( sKeySym.isEmpty() )
			sKeySym = "plus";

		// If this is a single character symbol (such as '!'),
		//  get it's name to use in lookup (such as 'exclam').
		if( sKeySym.length() == 1 && g_mapCharToName.contains( sKeySym[0] ) )
			sKeySym = g_mapCharToName[sKeySym[0]];

		KSymToInfoMap::const_iterator it = g_mapSymToInfo.search( sKeySym );
		if( it != g_mapSymToInfo.end() ) {
			do {
				key.m_keySymExplicit = (*it).keySymX;
				key.m_keyCombQtExplicit = ::calcKeyQt( key.m_keySymExplicit, key.m_keyModExplicit );
				for( int iCode = 0; iCode < (*it).nCodes; iCode++ ) {
					key.m_keyCode = (*it).rgCodes[iCode];
					key.m_keyMod = (*it).rgMods[iCode] | key.m_keyModExplicit;
					calcKeySym( key );
					if( key.m_keySym != key.m_keySymExplicit || key.m_keyMod != key.m_keyModExplicit )
						key.m_keyCombQt = ::calcKeyQt( key.m_keySym, key.m_keyMod );
					else
						key.m_keyCombQt = key.m_keyCombQtExplicit;
					rgKeys.push_back( key );
				}
				++it;
				it = g_mapSymToInfo.search( sKeySym, it );
			} while( it != g_mapSymToInfo.end() );
		}
		// Otherwise, key name may be valid, but we just don't have
		//  it on our keyboard (such as 'agrave' on an EN layout).
		else {
			key.m_keySymExplicit = XStringToKeysym( sKeySym.latin1() );
			key.m_keyCombQtExplicit = ::calcKeyQt( key.m_keySymExplicit, key.m_keyModExplicit );
			if( key.m_keySymExplicit )
				rgKeys.push_back( key );
		}
	}


	if( rgKeys.size() <= 1 ) {
		kdDebug(125) << "stringToKeys( " << sKey << " ):"
			<< " combqt: " << QString::number(key.m_keyCombQt, 16)
			<< "/" << QString::number(key.m_keyCombQtExplicit, 16)
			<< " c: " << QString::number(key.m_keyCode, 16)
			<< ", s: " << QString::number(key.m_keySym, 16)
			<< "/" << QString::number(key.m_keySymExplicit, 16)
			<< ", m: " << QString::number(key.m_keyMod, 16)
			<< "/" << QString::number(key.m_keyModExplicit, 16)
			<< " str: " << key.toString()
			<< endl;
	} else {
		kdDebug(125) << "stringToKeys( " << sKey << " ):" << endl;
		for( uint i = 0; i < rgKeys.size(); i++ ) {
			key = rgKeys[i];
			kdDebug(125)
			<< "\tcombqt: " << QString::number(key.m_keyCombQt, 16)
			<< "/" << QString::number(key.m_keyCombQtExplicit, 16)
			<< " c: " << QString::number(key.m_keyCode, 16)
			<< ", s: " << QString::number(key.m_keySym, 16)
			<< "/" << QString::number(key.m_keySymExplicit, 16)
			<< ", m: " << QString::number(key.m_keyMod, 16)
			<< "/" << QString::number(key.m_keyModExplicit, 16)
			<< " str: " << key.toString()
			<< endl;
		}
	}

	return rgKeys;
}

static void calcKeySym( KKeySequence& key )
{
	// Alt+Print = Sys_Req
	if( key.m_keySymExplicit == XK_Print && key.m_keyMod & KKeyX11::keyModXAlt() && key.m_keyCode == PRINT_CODE ) {
		key.m_keyCode = SYS_REQ_CODE;
		key.m_keySym = XK_Sys_Req;
	}
	// Ctrl+Pause = Break
	else if( key.m_keySymExplicit == XK_Pause && key.m_keyMod & ControlMask && key.m_keyCode == PAUSE_CODE ) {
		key.m_keyCode = BREAK_CODE;
		key.m_keySym = XK_Break;
	}
	// Otherwise get correct KeySym considering Shift & Mode_switch states.
	else {
		XKeyPressedEvent event;
		char rgc[3];

		event.type = KeyPress;
		event.display = qt_xdisplay();
		event.state = key.m_keyMod;
		if( event.state & KKeyX11::keyModXModeSwitch() )
			event.state = (event.state & ~KKeyX11::keyModXModeSwitch()) | 0x2000;
		event.keycode = key.m_keyCode;

		XLookupString( &event, rgc, sizeof(rgc)-1, (KeySym*) &key.m_keySym, 0 );
	}
}

static int calcKeyQt( int keySymX, int keyModX )
{
	int keyCombQt = 0;

	if( !keySymX )
		return 0;

	// Qt's own key definitions begin at 0x1000
	if( keySymX < 0x1000 ) {
		// For some reason, Qt wants 'a-z' converted to 'A-Z'
		if( keySymX >= 'a' && keySymX <= 'z' )
			keyCombQt = toupper( keySymX );
		else
			keyCombQt = keySymX;
	} else {
		if( g_mapSymToInfo.contains( keySymX ) ) {
			const QString& sName = g_mapSymToInfo[keySymX].sName;
			for( int i = 0; i < NB_KEYS; i++ ) {
				if( qstricmp( sName.latin1(), KKEYS[i].name ) == 0 ) {
					keyCombQt = KKEYS[i].code;
					break;
				}
			}
			if( !keyCombQt )
				kdWarning(125) << "Unable to find Qt equivalent of X's '" << sName << "'" << endl;
		} else
			kdWarning(125) << "g_mapSymToInfo does not contain keySym 0x" << QString::number(keySymX, 16) << endl;
	}

	if( keyCombQt ) {
		if( keyModX & KKeyX11::keyModXMeta() ) keyCombQt |= (Qt::ALT<<1);
		if( keyModX & KKeyX11::keyModXAlt() )  keyCombQt |= Qt::ALT;
		if( keyModX & ControlMask )            keyCombQt |= Qt::CTRL;
		if( keyModX & ShiftMask )              keyCombQt |= Qt::SHIFT;
	}

	return keyCombQt;
}

void KKeySequence::calcKeyQt()
{
	if( m_origin == OriginNative ) {
		m_keyCombQtExplicit = ::calcKeyQt( m_keySymExplicit, m_keyModExplicit );
		m_keyCombQt = ::calcKeyQt( m_keySym, m_keyMod );
	}
}

// Returns true if X has the Meta key assigned to a modifier bit
bool KKeySequence::keyboardHasMetaKey()
{
	if( !g_bInitialized )
		Initialize();
	return KKeyX11::keyModXMeta() != 0;
}

//-------------------------------------------------------------------

//-------------------------------------------------------------------

struct ModKeyXQt
{
	const char	*keyName;
	uint		keyModMaskQt;
	uint		keyModMaskX;
};

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

static void Initialize()
{
	readModifierMapping();
	g_bInitialized = true;
	readKeyMapping();
}

static void readModifierMapping()
{
	XModifierKeymap* xmk = XGetModifierMapping( qt_xdisplay() );

	for( int i = Mod2MapIndex; i < 8; i++ )
		g_aModKeys[i].keyModMaskX = 0;

	// Qt assumes that Alt is always Mod1Mask, so start at Mod2Mask.
	for( int i = Mod2MapIndex; i < 8; i++ ) {
		// TODO: Try to avoid X-Server calls -- get this from m_mapSymToInfo
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
}

static void readKeyMappingSub( uchar keyCodeX, uint keySymX, uint keyModX )
{
	QString sName = XKeysymToString( keySymX );
	bool bInsert = true;

	// If this is a unicode character,
	if( keySymX < 0x3000 ) {
		QChar c( keySymX );
		// If the key has a special name (such as "exclam" for '!'),
		if( !c.isNull() && sName != QString(c) )
			g_mapCharToName[c] = sName;
		// Don't insert upper-case letters into Sym->Info map.
		if( c.isLetter() && c == c.upper() )
			bInsert = false;
	}

	// Keycode 92 => XK_Sys_Req + Alt
	// Keycode 111 => XK_Print
	// Keycode 110 => XK_Pause
	// Keycode 114 => XK_Break + Ctrl
	if( keyCodeX == SYS_REQ_CODE ) {
		if( keySymX == XK_Print )
			bInsert = false;
		else if( keySymX == XK_Sys_Req )
			keyModX = KKeyX11::keyModXAlt();
	} else if( keyCodeX == PRINT_CODE ) {
		if( keySymX == XK_Sys_Req )
			bInsert = false;
	} else if( keyCodeX == PAUSE_CODE ) {
		if( keySymX == XK_Break )
			bInsert = false;
	} else if( keyCodeX == BREAK_CODE ) {
		if( keySymX == XK_Pause )
			bInsert = false;
		else if( keySymX == XK_Break )
			keyModX = ControlMask;
	}

	if( bInsert )
		g_mapSymToInfo.add( keySymX, sName, keyCodeX, keyModX );

	//cout << QString::number(keySymX, 16) << '\t';
	//if( !qChar.isNull() && qChar.isPrint() && !qChar.isSpace() )
	//	cout << qChar << " ";
	//else
	//	cout << "  ";
	//cout << sName.leftJustify( 16, ' ' );
}

static void readKeyMapping()
{
	int keyCodeMin, keyCodeMax;
	XDisplayKeycodes( qt_xdisplay(), &keyCodeMin, &keyCodeMax );
	int nKeyCodes = keyCodeMax - keyCodeMin + 1;

	g_mapSymToInfo.clear();
	int nSymsPerCode;
	KeySym* rgKeySyms = XGetKeyboardMapping( qt_xdisplay(), keyCodeMin, nKeyCodes, &nSymsPerCode );
	if( rgKeySyms ) {
		for( int iKeyCode = 0; iKeyCode < nKeyCodes; iKeyCode++ ) {
			if( rgKeySyms[iKeyCode * nSymsPerCode] ) {
				//cout << "Keycode " << (keyCodeMin + iKeyCode) << ":\t";
				for( int iSym = 0; iSym < nSymsPerCode; iSym++ ) {
					int i = iKeyCode * nSymsPerCode + iSym;
					uint keySymX = rgKeySyms[i];
					uchar keyCodeX = keyCodeMin + iKeyCode;
					if( keySymX ) {
						uint keyModX = ((iSym % 2) == 1 ? ShiftMask : 0) | ((iSym >= 2) ? Mod3Mask : 0);
						readKeyMappingSub( keyCodeX, keySymX, keyModX );
					}
					// else
					//	cout << "\t  \t\t";
				}
				//cout << endl;
			}
		}
		XFree( rgKeySyms );
	}

	//for( KCharToNameMap::iterator it = g_mapCharToName.begin(); it != g_mapCharToName.end(); ++it )
	//	kdDebug(125) << it.key() << '\t' << (*it) << endl;

	//for( KSymToInfoMap::iterator it = g_mapSymToInfo.begin(); it != g_mapSymToInfo.end(); ++it ) {
	//	KKeySymX& sym = *it;
	//	cerr << it.key() << '\t' << sym.sName.latin1();
	//	for( int i = 0; i < sym.nCodes; i++ )
	//		cerr << '\t' << QString::number(sym.rgCodes[i],16).local8Bit() << '/' << QString::number(sym.rgMods[i],16).local8Bit();
	//	cerr << endl;
	//}
}

KKeySequence KKeyX11::keyEventXToKey( const XEvent* pEvent )
{
	KKeySequence key;
	key.m_origin = KKeySequence::OriginNative;
	key.m_keyCode = pEvent->xkey.keycode;
	// TODO: Do we need to check that state has ModeSwitch mask in right place?
	key.m_keyMod = pEvent->xkey.state;

	calcKeySym( key );
	key.m_keySymExplicit = key.m_keySym;
	key.m_keyModExplicit = key.m_keyMod;
	return key;
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

	if( !g_bInitialized )
		Initialize();

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

	if( !g_bInitialized )
		Initialize();

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

	if( pKeySymX )  *pKeySymX = keySymX;
	if( pKeyCodeX ) *pKeyCodeX = keyCodeX;
	if( pKeyModX )  *pKeyModX = keyModX;
}

QString KKeyX11::keyCodeXToString( uchar keyCodeX, uint keyModX, bool bi18n )
{
	KKeySequence key;
	key.m_origin = OriginNative;
	key.m_keyCode = keyCodeX;
	key.m_keyMod = keyModX;
	calcKeySym( key );

	key.m_keySymExplicit = key.m_keySymExplicit;
	key.m_keyModExplicit = keyModX;
	return key.toString( bi18n ? I18N_Yes : I18N_No );
}
QString KKeyX11::keySymXToString( uint keySymX, uint keyModX, bool bi18n )
{
	KKeySequence key;
	key.m_origin = OriginNative;
	key.m_keySymExplicit = keySymX;
	key.m_keyModExplicit = keyModX;
	return key.toString( bi18n ? I18N_Yes : I18N_No );
}

uint KKeyX11::keyModXShift()		{ return ShiftMask; }
uint KKeyX11::keyModXLock()		{ return LockMask; }
uint KKeyX11::keyModXCtrl()		{ return ControlMask; }

uint KKeyX11::keyModXAlt()
{
	if( !g_bInitialized )
		Initialize();
	return g_aModKeys[ModAltIndex].keyModMaskX;
}

uint KKeyX11::keyModXNumLock()
{
	if( !g_bInitialized )
		Initialize();
	return g_aModKeys[ModNumLockIndex].keyModMaskX;
}

uint KKeyX11::keyModXModeSwitch()
{
	if( !g_bInitialized )
		Initialize();
	return g_aModKeys[ModModeSwitchIndex].keyModMaskX;
}

uint KKeyX11::keyModXMeta()
{
	if( !g_bInitialized )
		Initialize();
	return g_aModKeys[ModMetaIndex].keyModMaskX;
}

uint KKeyX11::keyModXScrollLock()
{
	if( !g_bInitialized )
		Initialize();
	return g_aModKeys[ModScrollLockIndex].keyModMaskX;
}

uint KKeyX11::accelModMaskX()		{ return ShiftMask | ControlMask | keyModXAlt() | keyModXMeta(); }

#endif // Q_WS_X11
