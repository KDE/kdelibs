#include <qnamespace.h>
#include <qwindowdefs.h>

#ifdef Q_WS_X11	// Only compile this module if we're compiling for X11

#include "kkeyserver_x11.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#undef NONE

#ifndef KeyPress // for --enable-final
#define KeyPress XKeyPress
#endif

namespace KKeyServer
{

//---------------------------------------------------------------------
// Data Structures
//---------------------------------------------------------------------

struct Mod
{
	int m_mod;
};

//---------------------------------------------------------------------
// Array Structures
//---------------------------------------------------------------------

struct ModInfo
{
	KKey::ModFlag mod;
	int modQt;
	uint modX;
	const char* psName;
	QString sLabel;
};

struct SymVariation
{
	uint sym, symVariation;
	bool bActive;
};

struct SymName
{
	uint sym;
	const char* psName;
};

struct TransKey {
	int keySymQt;
	uint keySymX;
};

//---------------------------------------------------------------------
// Arrays
//---------------------------------------------------------------------

static ModInfo g_rgModInfo[KKey::MOD_FLAG_COUNT] =
{
	{ KKey::SHIFT, Qt::SHIFT,   ShiftMask,   I18N_NOOP("Shift"), QString() },
	{ KKey::CTRL,  Qt::CTRL,    ControlMask, I18N_NOOP("Ctrl"), QString() },
	{ KKey::ALT,   Qt::ALT,     Mod1Mask,    I18N_NOOP("Alt"), QString() },
	{ KKey::WIN,   KKey::QtWIN, Mod4Mask,    I18N_NOOP("Win"), QString() }
};

static SymVariation g_rgSymVariation[] =
{
	{ '/', XK_KP_Divide, false },
	{ '*', XK_KP_Multiply, false },
	{ '-', XK_KP_Subtract, false },
	{ '+', XK_KP_Add, false },
	{ XK_Return, XK_KP_Enter, false },
	{ 0, 0, false }
};

// Special Names List
static const SymName g_rgSymNames[] = {
	{ XK_ISO_Left_Tab, "Backtab" },
	{ XK_BackSpace,    I18N_NOOP("Backspace") },
	{ XK_Sys_Req,      I18N_NOOP("SysReq") },
	{ XK_Caps_Lock,    I18N_NOOP("CapsLock") },
	{ XK_Num_Lock,     I18N_NOOP("NumLock") },
	{ XK_Scroll_Lock,  I18N_NOOP("ScrollLock") },
	{ 0, 0 }
};

// These are the X equivalents to the Qt keycodes 0x1000 - 0x1026
static const TransKey g_rgQtToSymX[] =
{
	{ Qt::Key_Escape,     XK_Escape },
	{ Qt::Key_Tab,        XK_Tab },
	{ Qt::Key_Backtab,    XK_ISO_Left_Tab },
	{ Qt::Key_Backspace,  XK_BackSpace },
	{ Qt::Key_Return,     XK_Return },
	{ Qt::Key_Enter,      XK_KP_Enter },
	{ Qt::Key_Insert,     XK_Insert },
	{ Qt::Key_Delete,     XK_Delete },
	{ Qt::Key_Pause,      XK_Pause },
	{ Qt::Key_Print,      XK_Print },
	{ Qt::Key_SysReq,     XK_Sys_Req },
	{ Qt::Key_Home,       XK_Home },
	{ Qt::Key_End,        XK_End },
	{ Qt::Key_Left,       XK_Left },
	{ Qt::Key_Up,         XK_Up },
	{ Qt::Key_Right,      XK_Right },
	{ Qt::Key_Down,       XK_Down },
	{ Qt::Key_Prior,      XK_Prior },
	{ Qt::Key_Next,       XK_Next },
	//{ Qt::Key_Shift,      0 },
	//{ Qt::Key_Control,    0 },
	//{ Qt::Key_Meta,       0 },
	//{ Qt::Key_Alt,        0 },
	{ Qt::Key_CapsLock,   XK_Caps_Lock },
	{ Qt::Key_NumLock,    XK_Num_Lock },
	{ Qt::Key_ScrollLock, XK_Scroll_Lock },
	{ Qt::Key_F1,         XK_F1 },
	{ Qt::Key_F2,         XK_F2 },
	{ Qt::Key_F3,         XK_F3 },
	{ Qt::Key_F4,         XK_F4 },
	{ Qt::Key_F5,         XK_F5 },
	{ Qt::Key_F6,         XK_F6 },
	{ Qt::Key_F7,         XK_F7 },
	{ Qt::Key_F8,         XK_F8 },
	{ Qt::Key_F9,         XK_F9 },
	{ Qt::Key_F10,        XK_F10 },
	{ Qt::Key_F11,        XK_F11 },
	{ Qt::Key_F12,        XK_F12 },
	{ Qt::Key_F13,        XK_F13 },
	{ Qt::Key_F14,        XK_F14 },
	{ Qt::Key_F15,        XK_F15 },
	{ Qt::Key_F16,        XK_F16 },
	{ Qt::Key_F17,        XK_F17 },
	{ Qt::Key_F18,        XK_F18 },
	{ Qt::Key_F19,        XK_F19 },
	{ Qt::Key_F20,        XK_F20 },
	{ Qt::Key_F21,        XK_F21 },
	{ Qt::Key_F22,        XK_F22 },
	{ Qt::Key_F23,        XK_F23 },
	{ Qt::Key_F24,        XK_F24 },
	{ Qt::Key_F25,        XK_F25 },
	{ Qt::Key_F26,        XK_F26 },
	{ Qt::Key_F27,        XK_F27 },
	{ Qt::Key_F28,        XK_F28 },
	{ Qt::Key_F29,        XK_F29 },
	{ Qt::Key_F30,        XK_F30 },
	{ Qt::Key_F31,        XK_F31 },
	{ Qt::Key_F32,        XK_F32 },
	{ Qt::Key_F33,        XK_F33 },
	{ Qt::Key_F34,        XK_F34 },
	{ Qt::Key_F35,        XK_F35 },
	{ Qt::Key_Super_L,    XK_Super_L },
	{ Qt::Key_Super_R,    XK_Super_R },
	{ Qt::Key_Menu,       XK_Menu },
	{ Qt::Key_Hyper_L,    XK_Hyper_L },
	{ Qt::Key_Hyper_R,    XK_Hyper_R },
	{ Qt::Key_Help,       XK_Help }
	//{ Qt::Key_Direction_L, XK_Direction_L,
	//{ Qt::Key_Direction_R, XK_Direction_R
};

//---------------------------------------------------------------------
// Initialization
//---------------------------------------------------------------------
static bool g_bInitializedMods, g_bInitializedVariations, g_bInitializedKKeyLabels;
static bool g_bMacLabels;
static uint g_modXNumLock, g_modXScrollLock;

bool initializeMods()
{
	XModifierKeymap* xmk = XGetModifierMapping( qt_xdisplay() );

	g_rgModInfo[3].modX = g_modXNumLock = g_modXScrollLock = 0;

	// Qt assumes that Alt is always Mod1Mask, so start at Mod2Mask.
	for( int i = Mod2MapIndex; i < 8; i++ ) {
		uint mask = (1 << i);
		uint keySymX = XKeycodeToKeysym( qt_xdisplay(), xmk->modifiermap[xmk->max_keypermod * i], 0 );
		switch( keySymX ) {
			case XK_Num_Lock:    g_modXNumLock = mask; break;     // Normally Mod2Mask
			case XK_Super_L:
			case XK_Super_R:     g_rgModInfo[3].modX = mask; break; // Win key, Normally Mod4Mask
			case XK_Meta_L:
			case XK_Meta_R:      if( !g_rgModInfo[3].modX ) g_rgModInfo[3].modX = mask; break; // Win alternate
			case XK_Scroll_Lock: g_modXScrollLock = mask; break;  // Normally Mod5Mask
		}
	}

	XFreeModifiermap( xmk );

	//KConfigGroupSaver cgs( KGlobal::config(), "Keyboard" );
	// read in mod that win should be attached to

	g_bInitializedMods = true;

	kdDebug(125) << "KKeyServer::initializeMods(): Win Mod = 0x" << QString::number(g_rgModInfo[3].modX, 16) << endl;
	return true;
}

static void initializeVariations()
{
	for( int i = 0; g_rgSymVariation[i].sym != 0; i++ )
		g_rgSymVariation[i].bActive = (XKeysymToKeycode( qt_xdisplay(), g_rgSymVariation[i].sym ) != 0);
	g_bInitializedVariations = true;
}

static void intializeKKeyLabels()
{
	KConfigGroupSaver cgs( KGlobal::config(), "Keyboard" );
	g_rgModInfo[0].sLabel = KGlobal::config()->readEntry( "Label Shift", i18n(g_rgModInfo[0].psName) );
	g_rgModInfo[1].sLabel = KGlobal::config()->readEntry( "Label Ctrl", i18n(g_rgModInfo[1].psName) );
	g_rgModInfo[2].sLabel = KGlobal::config()->readEntry( "Label Alt", i18n(g_rgModInfo[2].psName) );
	g_rgModInfo[3].sLabel = KGlobal::config()->readEntry( "Label Win", i18n(g_rgModInfo[3].psName) );
	g_bMacLabels = (g_rgModInfo[2].sLabel == "Command");
	g_bInitializedKKeyLabels = true;
}

//---------------------------------------------------------------------
// class Mod
//---------------------------------------------------------------------

/*void Mod::init( const QString& s )
{

}*/

//---------------------------------------------------------------------
// class Sym
//---------------------------------------------------------------------

bool Sym::initQt( int keyQt )
{
	int symQt = keyQt & 0xffff;

	if( (keyQt & Qt::UNICODE_ACCEL) || symQt < 0x1000 ) {
		m_sym = symQt;
		// For reasons unbeknownst to me, Qt converts 'a-z' to 'A-Z'.
		// So convert it back to lowercase if SHIFT is not held down.
		if( m_sym >= Qt::Key_A && m_sym <= Qt::Key_Z && !(keyQt & Qt::SHIFT) )
			m_sym = QChar(m_sym).lower();
		return true;
	}

	for( uint i = 0; i < sizeof(g_rgQtToSymX)/sizeof(TransKey); i++ ) {
		if( g_rgQtToSymX[i].keySymQt == symQt ) {
			m_sym = g_rgQtToSymX[i].keySymX;
			return true;
		}
	}

	m_sym = 0;
	kdWarning(125) << "Sym::initQt( " << QString::number(keyQt,16) << " ): failed to convert key." << endl;
	return false;
}

bool Sym::init( const QString& s )
{
	if( s.length() == 1 ) {
		m_sym = s[0].lower().unicode();
		return true;
	}

	// Look up in special names list
	for( int i = 0; g_rgSymNames[i].sym != 0; i++ ) {
		if( qstricmp( s.latin1(), g_rgSymNames[i].psName ) == 0 ) {
			m_sym = g_rgSymNames[i].sym;
			return true;
		}
	}

	// search X list: 's' as is, all lower, first letter in caps
	m_sym = XStringToKeysym( s.latin1() );
	if( !m_sym ) {
		m_sym = XStringToKeysym( s.lower().latin1() );
		if( !m_sym ) {
			QString s2 = s;
			s2[0] = s2[0].upper();
			m_sym = XStringToKeysym( s2.latin1() );
		}
	}

	return m_sym != 0;
}

int Sym::qt() const
{
	if( m_sym < 0x1000 ) {
		if( m_sym >= 'a' && m_sym <= 'z' )
			return QChar(m_sym).upper();
		return m_sym;
	}
	if( m_sym < 0x3000 )
		return m_sym | Qt::UNICODE_ACCEL;

	for( uint i = 0; i < sizeof(g_rgQtToSymX)/sizeof(TransKey); i++ )
		if( g_rgQtToSymX[i].keySymX == m_sym )
			return g_rgQtToSymX[i].keySymQt;
	return Qt::Key_unknown;
}

QString Sym::toString( bool bUserSpace ) const
{
	// If it's a unicode character,
	if( m_sym < 0x3000 ) {
		QChar c = QChar(m_sym).upper();
		// Print all unicode characters directly when output is user-visible.
		// Otherwise only print alphanumeric latin1 characters directly (A,B,C,1,2,3).
		if( bUserSpace || (c.latin1() && c.isLetterOrNumber()) )
				return c;
	}

	// Look up in special names list
	for( int i = 0; g_rgSymNames[i].sym != 0; i++ ) {
		if( m_sym == g_rgSymNames[i].sym )
			return bUserSpace ? i18n(g_rgSymNames[i].psName) : g_rgSymNames[i].psName;
	}

	// Get X-name
	QString s = XKeysymToString( m_sym );
	capitalizeKeyname( s );
	return bUserSpace ? i18n(s.latin1()) : s;
}

QString Sym::toStringInternal() const { return toString( false ); }
QString Sym::toString() const         { return toString( true ); }

uint Sym::getModsRequired() const
{
	// FIXME: This might not be true on all keyboard layouts!
	if( m_sym == XK_Sys_Req ) return KKey::ALT;
	if( m_sym == XK_Break ) return KKey::CTRL;

	if( m_sym < 0x3000 ) {
		QChar c(m_sym);
		if( c.isLetter() && c.lower() != c.upper() && m_sym == c.upper().unicode() )
			return KKey::SHIFT;
	}

	uchar code = XKeysymToKeycode( qt_xdisplay(), m_sym );
	if( m_sym == XKeycodeToKeysym( qt_xdisplay(), code, 0 )
	    || m_sym == XKeycodeToKeysym( qt_xdisplay(), code, 2 ) )
		return 0;
	else
		return KKey::SHIFT;
}

void Sym::capitalizeKeyname( QString& s )
{
	s[0] = s[0].upper();
	int len = s.length();
	if( s.endsWith( "left" ) )       s[len-4] = 'L';
	else if( s.endsWith( "right" ) ) s[len-5] = 'R';
	else if( s == "Sysreq" )         s[len-3] = 'R';
}

//---------------------------------------------------------------------
// class SymMod
//---------------------------------------------------------------------

// getModsRequiredForSym( int sym ) & g_rgModInfosymInfoPtr( sym );
/*void SymMod::init( int sym )
{
	m_sym = sym;
	...
}*/

// keyCodeXToKeySymX( (uchar) code, (uint) mod );
/*void SymMod::initX( uchar codeX, uint modX )
{
	XKeyPressedEvent event;

	event.type = KeyPress;
	event.display = qt_xdisplay();
	event.state = modX;
	event.keycode = codeX;

	XLookupString( &event, 0, 0, (KeySym*) &m_sym, 0 );
	modXToMod( modX, m_mod );
}
*/
// KKeySequenceOlds( QString )
//void SymMod::init( const QString& );
/*
void SymMod::initModQt( int keyQt )
{
	// Get modifiers
	m_mod = 0;
	for( uint i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( keyQt & g_rgModInfo[i].modQt )
			m_mod |= g_rgModInfo[i].modKDE;
	}
	return true;
}

bool SymMod::initSymQt( int keyQt )
{
	if( keyQt < 0x1000 || (keyQt & Qt::UNICODE_ACCEL) )
		m_sym = keyQt & 0xffff;
	else
}
*/
// keySymXToKeyQt( sym, 0 );
//int SymMod::qt() const;

// keySymXToString( symNative, 0, true );
//QString SymMod::toStringInternal() const;
//QString SymMod::toStringUser() const;

//---------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------

uint modX( KKey::ModFlag mod )
{
	if( mod == KKey::WIN && !g_bInitializedMods )
		initializeMods();

	for( uint i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( g_rgModInfo[i].mod == mod )
			return g_rgModInfo[i].modX;
	}
	return 0;
}

bool keyboardHasWinKey() { if( !g_bInitializedMods ) { initializeMods(); } return g_rgModInfo[3].modX != 0; }
uint modXShift()      { return ShiftMask; }
uint modXLock()       { return LockMask; }
uint modXCtrl()       { return ControlMask; }
uint modXAlt()        { return Mod1Mask; }
uint modXNumLock()    { if( !g_bInitializedMods ) { initializeMods(); } return g_modXNumLock; }
uint modXWin()        { if( !g_bInitializedMods ) { initializeMods(); } return g_rgModInfo[3].modX; }
uint modXScrollLock() { if( !g_bInitializedMods ) { initializeMods(); } return g_modXScrollLock; }

uint accelModMaskX()
{
	if( !g_bInitializedMods )
		initializeMods();
	return ShiftMask | ControlMask | Mod1Mask | g_rgModInfo[3].modX;
}

uint getSymVariation( uint sym )
{
	if( !g_bInitializedVariations )
		initializeVariations();

	for( int i = 0; g_rgSymVariation[i].sym != 0; i++ )
		if( g_rgSymVariation[i].sym == sym && g_rgSymVariation[i].bActive )
			return g_rgSymVariation[i].symVariation;
	return 0;
}


bool keyQtToSym( int keyQt, uint& keySym )
{
	Sym sym;
	if( sym.initQt( keyQt ) ) {
		keySym = sym.m_sym;
		return true;
	} else
		return false;
}

bool keyQtToMod( int keyQt, uint& mod )
{
	mod = 0;

	if( keyQt & Qt::SHIFT )    mod |= KKey::SHIFT;
	if( keyQt & Qt::CTRL )     mod |= KKey::CTRL;
	if( keyQt & Qt::ALT )      mod |= KKey::ALT;
	if( keyQt & (Qt::ALT<<1) ) mod |= KKey::WIN;

	return true;
}

bool symToKeyQt( uint keySym, int& keyQt )
{
	Sym sym( keySym );
	keyQt = sym.qt();
	return (keyQt != Qt::Key_unknown);
}

bool modToModQt( uint mod, int& modQt )
{
	modQt = 0;
	for( int i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( mod & g_rgModInfo[i].mod ) {
			if( !g_rgModInfo[i].modQt ) {
				modQt = 0;
				return false;
			}
			modQt |= g_rgModInfo[i].modQt;
		}
	}
	return true;
}

bool modToModX( uint mod, uint& modX )
{
	modX = 0;
	for( int i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( mod & g_rgModInfo[i].mod ) {
			if( !g_rgModInfo[i].modX ) {
				modX = 0;
				return false;
			}
			modX |= g_rgModInfo[i].modX;
		}
	}
	return true;
}

bool modXToModQt( uint modX, int& modQt )
{
	modQt = 0;
	for( int i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( modX & g_rgModInfo[i].modX ) {
			if( !g_rgModInfo[i].modQt ) {
				modQt = 0;
				return false;
			}
			modQt |= g_rgModInfo[i].modQt;
		}
	}
	return true;
}

bool modXToMod( uint modX, uint& mod )
{
	mod = 0;
	for( int i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( modX & g_rgModInfo[i].modX )
			mod |= g_rgModInfo[i].mod;
	}
	return true;
}

bool codeXToSym( uchar codeX, uint modX, uint& sym )
{
	XKeyPressedEvent event;

	event.type = KeyPress;
	event.display = qt_xdisplay();
	event.state = modX;
	event.keycode = codeX;

	XLookupString( &event, 0, 0, (KeySym*) &sym, 0 );
	return true;
}

//QString symToStringInternal( uint sym ) { return Sym(sym).toStringInternal(); }
//QString symToStringUser( uint sym ) { return Sym(sym).toStringInternal(); }

static QString modToString( uint mod, bool bUserSpace )
{
	if( bUserSpace && !g_bInitializedKKeyLabels )
		intializeKKeyLabels();

	QString s;
	for( int i = KKey::MOD_FLAG_COUNT-1; i >= 0; i-- ) {
		if( mod & g_rgModInfo[i].mod ) {
			if( !s.isEmpty() )
				s += '+';
			s += (bUserSpace)
			          ? g_rgModInfo[i].sLabel
				  : QString(g_rgModInfo[i].psName);
		}
	}
	return s;
}

QString modToStringInternal( uint mod ) { return modToString( mod, false ); }
QString modToStringUser( uint mod )     { return modToString( mod, true ); }

/*void keySymModToKeyX( uint sym, uint mod, unsigned char *pKeyCodeX, uint *pKeySymX, uint *pKeyModX )
{
...
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

		if( keySymX == 0 )
			keySymX = getSymXEquiv( keySymQt );
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
}*/

}; // end of namespace KKeyServer block
#undef KeyPress
#endif // Q_WS_X11
