/*
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

    Win32 port:
    Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <qnamespace.h>
#include <qwindowdefs.h>
#include <QFlags>

#if defined(Q_WS_X11) || defined(Q_WS_WIN) || defined(Q_WS_MACX) // Only compile this module if we're compiling for X11, mac or win32

#include "kkeyserver_x11.h"
#include "kkeynative.h"
#include "kshortcut.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#ifdef Q_WS_X11
#include <QX11Info>
# define XK_MISCELLANY
# define XK_XKB_KEYS
# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/keysymdef.h>
# define X11_ONLY(arg) arg, //allows to omit an argument
#else
# include <kckey.h>
# define X11_ONLY(arg)
# define XK_ISO_Left_Tab Qt::Key_Backtab
# define XK_BackSpace Qt::Key_Backspace
# define XK_Sys_Req Qt::Key_SysReq
# define XK_Caps_Lock Qt::Key_CapsLock
# define XK_Num_Lock Qt::Key_NumLock
# define XK_Scroll_Lock Qt::Key_ScrollLock
# define XK_Prior Qt::Key_PageUp
# define XK_Next Qt::Key_PageDown
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
#ifdef Q_WS_X11
	uint modX;
#endif
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
	{ KKey::SHIFT, Qt::SHIFT,   X11_ONLY(ShiftMask)   I18N_NOOP("Shift"), QString() },
	{ KKey::CTRL,  Qt::CTRL,    X11_ONLY(ControlMask) I18N_NOOP("Ctrl"), QString() },
	{ KKey::ALT,   Qt::ALT,     X11_ONLY(Mod1Mask)    I18N_NOOP("Alt"), QString() },
	{ KKey::WIN,   Qt::META,    X11_ONLY(Mod4Mask)    I18N_NOOP("Win"), QString() }
};

// Special Names List
static const SymName g_rgSymNames[] = {
	{ XK_ISO_Left_Tab, "Backtab" },
	{ XK_BackSpace,    I18N_NOOP("Backspace") },
	{ XK_Sys_Req,      I18N_NOOP("SysReq") },
	{ XK_Caps_Lock,    I18N_NOOP("CapsLock") },
	{ XK_Num_Lock,     I18N_NOOP("NumLock") },
	{ XK_Scroll_Lock,  I18N_NOOP("ScrollLock") },
	{ XK_Prior,        I18N_NOOP("PageUp") },
	{ XK_Next,         I18N_NOOP("PageDown") },
#ifdef sun
	{ XK_F11,          I18N_NOOP("Stop") },
	{ XK_F12,          I18N_NOOP("Again") },
	{ XK_F13,          I18N_NOOP("Props") },
	{ XK_F14,          I18N_NOOP("Undo") },
	{ XK_F15,          I18N_NOOP("Front") },
	{ XK_F16,          I18N_NOOP("Copy") },
	{ XK_F17,          I18N_NOOP("Open") },
	{ XK_F18,          I18N_NOOP("Paste") },
	{ XK_F19,          I18N_NOOP("Find") },
	{ XK_F20,          I18N_NOOP("Cut") },
	{ XK_F22,          I18N_NOOP("Print") },
#endif
	{ 0, 0 }
};

#ifdef Q_WS_X11
static SymVariation g_rgSymVariation[] =
{
	{ '/', XK_KP_Divide, false },
	{ '*', XK_KP_Multiply, false },
	{ '-', XK_KP_Subtract, false },
	{ '+', XK_KP_Add, false },
	{ XK_Return, XK_KP_Enter, false },
	{ 0, 0, false }
};

// TODO: Add Mac key names list: Key_Backspace => "Delete", Key_Delete => "Del"

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
#ifdef sun
	{ Qt::Key_Print,      XK_F22 },
#else
	{ Qt::Key_Print,      XK_Print },
#endif
	{ Qt::Key_SysReq,     XK_Sys_Req },
	{ Qt::Key_Home,       XK_Home },
	{ Qt::Key_End,        XK_End },
	{ Qt::Key_Left,       XK_Left },
	{ Qt::Key_Up,         XK_Up },
	{ Qt::Key_Right,      XK_Right },
	{ Qt::Key_Down,       XK_Down },
	{ Qt::Key_PageUp,      XK_Prior },
	{ Qt::Key_PageDown,       XK_Next },
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
	{ Qt::Key_Help,       XK_Help },
	//{ Qt::Key_Direction_L, XK_Direction_L }, These keys don't exist in X11
	//{ Qt::Key_Direction_R, XK_Direction_R },

	{ '/',                XK_KP_Divide },
	{ '*',                XK_KP_Multiply },
	{ '-',                XK_KP_Subtract },
	{ '+',                XK_KP_Add },
	{ Qt::Key_Return,     XK_KP_Enter }

// the next lines are taken from XFree > 4.0 (X11/XF86keysyms.h), defining some special
// multimedia keys. They are included here as not every system has them.
#define XF86XK_Standby		0x1008FF10
#define XF86XK_AudioLowerVolume	0x1008FF11
#define XF86XK_AudioMute	0x1008FF12
#define XF86XK_AudioRaiseVolume	0x1008FF13
#define XF86XK_AudioPlay	0x1008FF14
#define XF86XK_AudioStop	0x1008FF15
#define XF86XK_AudioPrev	0x1008FF16
#define XF86XK_AudioNext	0x1008FF17
#define XF86XK_HomePage		0x1008FF18
#define XF86XK_Calculator	0x1008FF1D
#define XF86XK_Mail		0x1008FF19
#define XF86XK_Start		0x1008FF1A
#define XF86XK_Search		0x1008FF1B
#define XF86XK_AudioRecord	0x1008FF1C
#define XF86XK_Back		0x1008FF26
#define XF86XK_Forward		0x1008FF27
#define XF86XK_Stop		0x1008FF28
#define XF86XK_Refresh		0x1008FF29
#define XF86XK_Favorites	0x1008FF30
#define XF86XK_AudioPause	0x1008FF31
#define XF86XK_AudioMedia	0x1008FF32
#define XF86XK_MyComputer	0x1008FF33
#define XF86XK_OpenURL		0x1008FF38
#define XF86XK_Launch0		0x1008FF40
#define XF86XK_Launch1		0x1008FF41
#define XF86XK_Launch2		0x1008FF42
#define XF86XK_Launch3		0x1008FF43
#define XF86XK_Launch4		0x1008FF44
#define XF86XK_Launch5		0x1008FF45
#define XF86XK_Launch6		0x1008FF46
#define XF86XK_Launch7		0x1008FF47
#define XF86XK_Launch8		0x1008FF48
#define XF86XK_Launch9		0x1008FF49
#define XF86XK_LaunchA		0x1008FF4A
#define XF86XK_LaunchB		0x1008FF4B
#define XF86XK_LaunchC		0x1008FF4C
#define XF86XK_LaunchD		0x1008FF4D
#define XF86XK_LaunchE		0x1008FF4E
#define XF86XK_LaunchF		0x1008FF4F
// end of XF86keysyms.h
        ,
	{ Qt::Key_Standby,    XF86XK_Standby },
	{ Qt::Key_VolumeDown, XF86XK_AudioLowerVolume },
	{ Qt::Key_VolumeMute, XF86XK_AudioMute },
	{ Qt::Key_VolumeUp,   XF86XK_AudioRaiseVolume },
	{ Qt::Key_MediaPlay,  XF86XK_AudioPlay },
	{ Qt::Key_MediaStop,  XF86XK_AudioStop },
	{ Qt::Key_MediaPrevious,  XF86XK_AudioPrev },
	{ Qt::Key_MediaNext,  XF86XK_AudioNext },
	{ Qt::Key_HomePage,   XF86XK_HomePage },
	{ Qt::Key_LaunchMail, XF86XK_Mail },
	{ Qt::Key_Search,     XF86XK_Search },
	{ Qt::Key_MediaRecord, XF86XK_AudioRecord },
	{ Qt::Key_LaunchMedia, XF86XK_AudioMedia },
	{ Qt::Key_Launch1,    XF86XK_Calculator },
	{ Qt::Key_Back,       XF86XK_Back },
	{ Qt::Key_Forward,    XF86XK_Forward },
	{ Qt::Key_Stop,       XF86XK_Stop },
	{ Qt::Key_Refresh,    XF86XK_Refresh },
	{ Qt::Key_Favorites,  XF86XK_Favorites },
	{ Qt::Key_Launch0,    XF86XK_MyComputer },
	{ Qt::Key_OpenUrl,    XF86XK_OpenURL },
	{ Qt::Key_Launch2,    XF86XK_Launch0 },
	{ Qt::Key_Launch3,    XF86XK_Launch1 },
	{ Qt::Key_Launch4,    XF86XK_Launch2 },
	{ Qt::Key_Launch5,    XF86XK_Launch3 },
	{ Qt::Key_Launch6,    XF86XK_Launch4 },
	{ Qt::Key_Launch7,    XF86XK_Launch5 },
	{ Qt::Key_Launch8,    XF86XK_Launch6 },
	{ Qt::Key_Launch9,    XF86XK_Launch7 },
	{ Qt::Key_LaunchA,    XF86XK_Launch8 },
	{ Qt::Key_LaunchB,    XF86XK_Launch9 },
	{ Qt::Key_LaunchC,    XF86XK_LaunchA },
	{ Qt::Key_LaunchD,    XF86XK_LaunchB },
	{ Qt::Key_LaunchE,    XF86XK_LaunchC },
	{ Qt::Key_LaunchF,    XF86XK_LaunchD },
};
#endif //Q_WS_X11

//---------------------------------------------------------------------
// Initialization
//---------------------------------------------------------------------
static bool g_bInitializedMods, g_bInitializedVariations, g_bInitializedKKeyLabels;
static bool g_bMacLabels;
#ifdef Q_WS_X11
static uint g_modXNumLock, g_modXScrollLock, g_modXModeSwitch, g_alt_mask, g_meta_mask;

bool initializeMods()
{
	XModifierKeymap* xmk = XGetModifierMapping( QX11Info::display() );

	g_rgModInfo[3].modX = g_modXNumLock = g_modXScrollLock = g_modXModeSwitch = 0; 

	int min_keycode, max_keycode;	
	int keysyms_per_keycode = 0;
	
	XDisplayKeycodes( QX11Info::display(), &min_keycode, &max_keycode );
	XFree( XGetKeyboardMapping( QX11Info::display(), min_keycode, 1, &keysyms_per_keycode ));
	
	// Defaults
	g_alt_mask = Mod1Mask;
	g_meta_mask = Mod4Mask;

	for( int i = Mod1MapIndex; i < 8; i++ ) {
		uint mask = (1 << i);
		uint keySymX = NoSymbol;

                // This used to be only XKeycodeToKeysym( ... , 0 ), but that fails with XFree4.3.99
                // and X.org R6.7 , where for some reason only ( ... , 1 ) works. I have absolutely no
                // idea what the problem is, but searching all posibilities until something valid is
                // found fixes the problem.
                for( int j = 0; j < xmk->max_keypermod && keySymX == NoSymbol; ++j )
                    for( int k = 0; k < keysyms_per_keycode && keySymX == NoSymbol; ++k )
                        keySymX = XKeycodeToKeysym( QX11Info::display(), xmk->modifiermap[xmk->max_keypermod * i + j], k );
		switch( keySymX ) {
			case XK_Alt_L:
			case XK_Alt_R:     g_alt_mask = mask; break; // Alt key, Normally Mod1Mask

			case XK_Super_L:
			case XK_Super_R:     g_meta_mask = mask; break; // Win key, Normally Mod4Mask

			case XK_Meta_L:
			case XK_Meta_R:      if( !g_meta_mask ) g_meta_mask = mask; break; // Win alternate

			case XK_Num_Lock:    g_modXNumLock = mask; break;     // Normally Mod2Mask
			case XK_Scroll_Lock: g_modXScrollLock = mask; break;  // Normally Mod5Mask
			case XK_Mode_switch: g_modXModeSwitch = mask; break; 
		}
	}

	g_rgModInfo[3].modX = g_meta_mask;
	g_rgModInfo[2].modX = g_alt_mask;

	XFreeModifiermap( xmk );

	//KConfigGroup cg( KGlobal::config(), "Keyboard" );
	// read in mod that win should be attached to

	g_bInitializedMods = true;

	kdDebug(125) << "KKeyServer::initializeMods(): Win Mod = 0x" << QString::number(g_rgModInfo[3].modX, 16) << endl;
	return true;
}

static void initializeVariations()
{
	for( int i = 0; g_rgSymVariation[i].sym != 0; i++ )
		g_rgSymVariation[i].bActive = (XKeysymToKeycode( QX11Info::display(), g_rgSymVariation[i].symVariation ) != 0);
	g_bInitializedVariations = true;
}
#endif //Q_WS_X11

static void intializeKKeyLabels()
{
	KConfigGroup cg( KGlobal::config(), "Keyboard" );
	g_rgModInfo[0].sLabel = cg.readEntry( "Label Shift", i18n(g_rgModInfo[0].psName) );
	g_rgModInfo[1].sLabel = cg.readEntry( "Label Ctrl", i18n(g_rgModInfo[1].psName) );
	g_rgModInfo[2].sLabel = cg.readEntry( "Label Alt", i18n(g_rgModInfo[2].psName) );
	g_rgModInfo[3].sLabel = cg.readEntry( "Label Win", i18n(g_rgModInfo[3].psName) );
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
	int symQt = keyQt & ~Qt::KeyboardModifierMask; 

	if( symQt < 0x1000 ) {
		m_sym = QChar(symQt).toLower().unicode();
		return true;
	}

#ifdef Q_WS_WIN
	m_sym = symQt;
	return true;
#elif defined(Q_WS_MACX)
	m_sym = symQt;
	return true;
#elif defined(Q_WS_X11)
	for( uint i = 0; i < sizeof(g_rgQtToSymX)/sizeof(TransKey); i++ ) {
		if( g_rgQtToSymX[i].keySymQt == symQt ) {
			m_sym = g_rgQtToSymX[i].keySymX;
			return true;
		}
	}

	m_sym = 0;
	if( symQt != Qt::Key_Shift && symQt != Qt::Key_Control && symQt != Qt::Key_Alt &&
	    symQt != Qt::Key_Meta && symQt != Qt::Key_Direction_L && symQt != Qt::Key_Direction_R )
		kdDebug(125) << "Sym::initQt( " << QString::number(keyQt,16) << " ): failed to convert key." << endl;
	return false;

#endif
}

bool Sym::init( const QString& s )
{
	// If it's a single character, get unicode value.
	if( s.length() == 1 ) {
		m_sym = s[0].toLower().unicode();
		return true;
	}

	// Look up in special names list
	for( int i = 0; g_rgSymNames[i].sym != 0; i++ ) {
		if( qstricmp( s.latin1(), g_rgSymNames[i].psName ) == 0 ) {
			m_sym = g_rgSymNames[i].sym;
			return true;
		}
	}

#ifdef Q_WS_WIN
	// search for name in KKeys array
	for ( KKeys const *pKey  = kde_KKEYS; pKey->code != 0xffff; pKey++) {
		if( qstricmp( s.latin1(), pKey->name ) == 0 ) {
			m_sym = pKey->code;
			return true;
		}
	}
	m_sym = 0;
#elif defined(Q_WS_X11)
	// search X list: 's' as is, all lower, first letter in caps
	m_sym = XStringToKeysym( s.latin1() );
	if( !m_sym ) {
		m_sym = XStringToKeysym( s.toLower().latin1() );
		if( !m_sym ) {
			QString s2 = s;
			s2[0] = s2[0].toUpper();
			m_sym = XStringToKeysym( s2.latin1() );
		}
	}
#endif
	return m_sym != 0;
}

int Sym::qt() const
{
	if( m_sym < 0x1000 ) {
		if( m_sym >= 'a' && m_sym <= 'z' )
			return QChar(m_sym).toUpper().unicode();
		return m_sym;
	}
#ifdef Q_WS_WIN
	if( m_sym < 0x3000 )
		return m_sym;
#elif defined(Q_WS_X11)
	if( m_sym < 0x3000 )
		return m_sym;

	for( uint i = 0; i < sizeof(g_rgQtToSymX)/sizeof(TransKey); i++ )
		if( g_rgQtToSymX[i].keySymX == m_sym )
			return g_rgQtToSymX[i].keySymQt;
#endif
	return Qt::Key_unknown;
}

QString Sym::toString( bool bUserSpace ) const
{
	if( m_sym == 0 )
		return QString();

	// If it's a unicode character,
#ifdef Q_WS_WIN
	else if( m_sym < 0x1000 ) {
#else
	else if( m_sym < 0x3000 ) {
#endif
		QChar c = QChar(m_sym).toUpper();
		// Print all non-space characters directly when output is user-visible.
		// Otherwise only print alphanumeric latin1 characters directly (A,B,C,1,2,3).
		if( (c.latin1() && c.isLetterOrNumber())
		    || (bUserSpace && !c.isSpace()) )
				return QString( c );
	}

	// Look up in special names list
	for( int i = 0; g_rgSymNames[i].sym != 0; i++ ) {
		if( m_sym == g_rgSymNames[i].sym )
			return bUserSpace ? i18n(g_rgSymNames[i].psName) : QString(g_rgSymNames[i].psName);
	}

	QString s;
#ifdef Q_WS_WIN
	s = QString(QKeySequence(m_sym)); 
#elif defined(Q_WS_X11)
	// Get X-name
	s = XKeysymToString( m_sym );
#endif
	capitalizeKeyname( s );
	return bUserSpace ? i18n("QAccel", s.latin1()) : s;
}

QString Sym::toStringInternal() const { return toString( false ); }
QString Sym::toString() const         { return toString( true ); }

uint Sym::getModsRequired() const
{
	uint mod = 0;
#ifdef Q_WS_X11
	// FIXME: This might not be true on all keyboard layouts!
	if( m_sym == XK_Sys_Req ) return KKey::ALT;
	if( m_sym == XK_Break ) return KKey::CTRL;

	if( m_sym < 0x3000 ) {
		QChar c(m_sym);
		if( c.isLetter() && c.toLower() != c.toUpper() && m_sym == c.toUpper().unicode() )
			return KKey::SHIFT;
	}

	uchar code = XKeysymToKeycode( QX11Info::display(), m_sym );
	if( code ) {
		// need to check index 0 before the others, so that a null-mod
		//  can take precedence over the others, in case the modified
		//  key produces the same symbol.
		if( m_sym == XKeycodeToKeysym( QX11Info::display(), code, 0 ) )
			;
		else if( m_sym == XKeycodeToKeysym( QX11Info::display(), code, 1 ) )
			mod = KKey::SHIFT;
		else if( m_sym == XKeycodeToKeysym( QX11Info::display(), code, 2 ) )
			mod = KKeyServer::MODE_SWITCH;
		else if( m_sym == XKeycodeToKeysym( QX11Info::display(), code, 3 ) )
			mod = KKey::SHIFT | KKeyServer::MODE_SWITCH;
	}
#endif
	return mod;
}

uint Sym::getSymVariation() const
{
#ifdef Q_WS_X11
	if( !g_bInitializedVariations )
		initializeVariations();
	for( int i = 0; g_rgSymVariation[i].sym != 0; i++ )
		if( g_rgSymVariation[i].sym == m_sym && g_rgSymVariation[i].bActive )
			return g_rgSymVariation[i].symVariation;
#endif
	return 0;
}

void Sym::capitalizeKeyname( QString& s )
{
	s[0] = s[0].toUpper();
	int len = s.length();
	if( s.endsWith( "left" ) )       s[len-4] = 'L';
	else if( s.endsWith( "right" ) ) s[len-5] = 'R';
	else if( s == "Sysreq" )         s[len-3] = 'R';
}

//---------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------

#ifdef Q_WS_X11
#ifdef KDE3_SUPPORT
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
#endif //KDE3_SUPPORT

uint modXShift()      { return ShiftMask; }
uint modXCtrl()       { return ControlMask; }
uint modXAlt()        { return g_alt_mask; }
uint modXMeta()       { if( !g_bInitializedMods ) { initializeMods(); } return g_meta_mask; }
// KDE 3 compatibility
uint modXWin()        { return modXMeta(); }

uint modXNumLock()    { if( !g_bInitializedMods ) { initializeMods(); } return g_modXNumLock; }
uint modXLock()       { return LockMask; }
uint modXScrollLock() { if( !g_bInitializedMods ) { initializeMods(); } return g_modXScrollLock; }
uint modXModeSwitch() { if( !g_bInitializedMods ) { initializeMods(); } return g_modXModeSwitch; } 

bool keyboardHasWinKey() { return modXWin() != 0; }

uint accelModMaskX()
{
	return modXShift() | modXCtrl() | modXAlt() | modXMeta();
}
#endif //Q_WS_X11

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
	if( keyQt & Qt::META ) mod |= KKey::WIN;

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

#ifdef Q_WS_WIN
//wrapped
bool modXToModQt( uint modX, int& modQt )
{
	return modToModQt( modX, modQt );
}

KDECORE_EXPORT int qtButtonStateToMod( Qt::KeyboardModifiers s )
{
	int modQt = 0;
	if (s & Qt::ShiftModifier) modQt |= KKey::SHIFT;
	if (s & Qt::ControlModifier) modQt |= KKey::CTRL;
	if (s & Qt::AltModifier) modQt |= KKey::ALT;
	return modQt;
}

bool keyboardHasWinKey() { 
//! TODO
  return true;
}

#elif defined(Q_WS_MACX)

bool modXToModQt(uint modX, int& modQt)
{
    return modToModQt( modX, modQt );
}

bool keyboardHasWinKey() {
//! TODO - A win key on the Mac...?
  return false;
}

bool modXToMod( uint , uint& )
{
    return false;
}
#elif defined(Q_WS_X11)

bool modToModX( uint mod, uint& modX )
{
	if( !g_bInitializedMods )
		initializeMods();

	modX = 0;
	for( int i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( mod & g_rgModInfo[i].mod ) {
			if( !g_rgModInfo[i].modX ) {
				kdDebug(125) << "Invalid modifier flag." << endl;
				modX = 0;
				return false;
			}
			modX |= g_rgModInfo[i].modX;
		}
	}
	// TODO: document 0x2000 flag
	if( mod & 0x2000 )
	  modX |= 0x2000;
	return true;
}

bool modXToModQt( uint modX, int& modQt )
{
	if( !g_bInitializedMods )
		initializeMods();
	
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
	if( !g_bInitializedMods )
		initializeMods();
	
	mod = 0;
	for( int i = 0; i < KKey::MOD_FLAG_COUNT; i++ ) {
		if( modX & g_rgModInfo[i].modX )
			mod |= g_rgModInfo[i].mod;
	}
	return true;
}

bool codeXToSym( uchar codeX, uint modX, uint& sym )
{
	KeySym keySym;
	XKeyPressedEvent event;

	event.type = KeyPress;
	event.display = QX11Info::display();
	event.state = modX;
	event.keycode = codeX;

	XLookupString( &event, 0, 0, &keySym, 0 );
	sym = (uint) keySym;
	return true;
}
#endif //!Q_WS_WIN

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

uint stringUserToMod( const QString& mod )
{
	if( !g_bInitializedKKeyLabels )
		intializeKKeyLabels();

	QString s;
	for( int i = KKey::MOD_FLAG_COUNT-1; i >= 0; i-- ) {
		if( mod.toLower() == g_rgModInfo[i].sLabel.toLower())
			return g_rgModInfo[i].mod;
	}
	return 0;
}

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
			keySymX = XStringToKeysym( sKeySym.toLower().ascii() );
			if( keySymX == 0 )
				keySymX = XStringToKeysym( psKeySym );
		}

		if( keySymX == 0 )
			keySymX = getSymXEquiv( keySymQt );
	}

	if( keySymX != 0 ) {
		// Get X keyboard code
		keyCodeX = XKeysymToKeycode( QX11Info::display(), keySymX );
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
	    XKeycodeToKeysym( QX11Info::display(), 92, 0 ) == XK_Print &&
	    XKeycodeToKeysym( QX11Info::display(), 111, 0 ) == XK_Print )
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
	    XKeycodeToKeysym( QX11Info::display(), 110, 0 ) == XK_Pause &&
	    XKeycodeToKeysym( QX11Info::display(), 114, 0 ) == XK_Pause )
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

//---------------------------------------------------------------------
// Key
//---------------------------------------------------------------------

bool Key::init( const KKey& key, bool bQt )
{
	if( bQt ) {
		m_code = CODE_FOR_QT;
		m_sym = key.keyCodeQt();
	} else {
		KKeyNative keyNative( key );
		*this = keyNative;
	}
	return true;
}

KKey Key::key() const
{
	if( m_code == CODE_FOR_QT )
		return KKey( keyCodeQt() );
	else {
#if defined(Q_WS_WIN) || defined(Q_WS_MACX)
		return KKey();
#else
		uint mod;
		modXToMod( m_mod, mod );
		return KKey( m_sym, mod );
#endif
	}
}

Key& Key::operator =( const KKeyNative& key )
{
	m_code = key.code(); m_mod = key.mod(); m_sym = key.sym();
	return *this;
}

int Key::compare( const Key& b ) const
{
	if( m_code == CODE_FOR_QT )
		return m_sym - b.m_sym;
	if( m_sym != b.m_sym )	return m_sym - b.m_sym;
	if( m_mod != b.m_mod )	return m_mod - b.m_mod;
	return m_code - b.m_code;
}

//---------------------------------------------------------------------
// Variations
//---------------------------------------------------------------------

// TODO: allow for sym to have variations, such as Plus => { Plus, KP_Add }
void Variations::init( const KKey& key, bool bQt )
{
	if( key.isNull() ) {
		m_nVariations = 0;
		return;
	}

	m_nVariations = 1;
	m_rgkey[0] = KKeyNative(key);
	uint symVar = Sym(key.sym()).getSymVariation();
	if( symVar ) {
		uint modReq = Sym(m_rgkey[0].sym()).getModsRequired();
		uint modReqVar = Sym(symVar).getModsRequired();
		// If 'key' doesn't require any mods that are inherent in
		//  the primary key but not required for the alternate,
		if( (key.modFlags() & modReq) == (key.modFlags() & modReqVar) ) {
			m_rgkey[1] = KKeyNative(KKey(symVar, key.modFlags()));
			m_nVariations = 2;
		}
	}

	if( bQt ) {
		uint nVariations = 0;
		for( uint i = 0; i < m_nVariations; i++ ) {
			int keyQt = KKeyNative( m_rgkey[i].code(), m_rgkey[i].mod(), m_rgkey[i].sym() ).keyCodeQt();
			if( keyQt )
				m_rgkey[nVariations++].setKeycodeQt( keyQt );
		}
		m_nVariations = nVariations;

		// Two different native codes may produce a single
		//  Qt code.  Search for duplicates.
		for( uint i = 1; i < m_nVariations; i++ ) {
			for( uint j = 0; j < i; j++ ) {
				// If key is already present in list, then remove it.
				if( m_rgkey[i].keyCodeQt() == m_rgkey[j].keyCodeQt() ) {
					for( uint k = i; k < m_nVariations - 1; k++ )
						m_rgkey[k].setKeycodeQt( m_rgkey[k+1].keyCodeQt() );
					m_nVariations--;
					i--;
					break;
				}
			}
		}
	}
}

} // end of namespace KKeyServer block

// FIXME: This needs to be moved to kshortcut.cpp, and create a 
//  KKeyServer::method which it will call.
// Alt+SysReq => Alt+Print
// Ctrl+Shift+Plus => Ctrl+Plus (en)
// Ctrl+Shift+Equal => Ctrl+Plus
// Ctrl+Pause => Ctrl+Break
void KKey::simplify()
{
#ifdef Q_WS_X11
	if( m_sym == XK_Sys_Req ) {
		m_sym = XK_Print;
		m_mod |= Qt::ALT;
	} else if( m_sym == XK_ISO_Left_Tab ) {
		m_sym = XK_Tab;
		m_mod |= Qt::SHIFT;
	} else {
		// Shift+Equal => Shift+Plus (en)
		m_sym = KKeyNative(*this).sym();
	}

	// If this is a letter, don't remove any modifiers.
	if( m_sym < 0x3000 && QChar(m_sym).isLetter() )
		m_sym = QChar(m_sym).toLower().unicode();

	// Remove modifers from modifier list which are implicit in the symbol.
	// Ex. Shift+Plus => Plus (en)
	m_mod &= ~KKeyServer::Sym(m_sym).getModsRequired();
#endif
}

#endif //Q_WS_X11 || Q_WS_WIN

