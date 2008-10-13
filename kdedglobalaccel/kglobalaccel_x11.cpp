/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

#include "kglobalaccel_x11.h"

#include <config.h>

#include <QtGui/QWidgetList>

#include "kaction.h"
#include "kdedglobalaccel.h"
#include "kkeyserver_x11.h"

#include <kapplication.h>
#include <kdebug.h>

#include <QtCore/QRegExp>
#include <QtGui/QWidget>
#include <QtCore/QMetaClassInfo>
#include <QtGui/QMenu>

#include <kxerrorhandler.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <fixx11h.h>

extern "C" {
  static int XGrabErrorHandler( Display *, XErrorEvent *e ) {
	if ( e->error_code != BadAccess ) {
	    kWarning() << "grabKey: got X error " << e->type << " instead of BadAccess\n";
	}
	return 1;
  }
}

// g_keyModMaskXAccel
//	mask of modifiers which can be used in shortcuts
//	(meta, alt, ctrl, shift)
// g_keyModMaskXOnOrOff
//	mask of modifiers where we don't care whether they are on or off
//	(caps lock, num lock, scroll lock)
static uint g_keyModMaskXAccel = 0;
static uint g_keyModMaskXOnOrOff = 0;

static void calculateGrabMasks()
{
	g_keyModMaskXAccel = KKeyServer::accelModMaskX();
	g_keyModMaskXOnOrOff =
			KKeyServer::modXLock() |
			KKeyServer::modXNumLock() |
			KKeyServer::modXScrollLock() |
			KKeyServer::modXModeSwitch();
	//kDebug() << "g_keyModMaskXAccel = " << g_keyModMaskXAccel
	//	<< "g_keyModMaskXOnOrOff = " << g_keyModMaskXOnOrOff << endl;
}

//----------------------------------------------------

KGlobalAccelImpl::KGlobalAccelImpl(KdedGlobalAccel *owner)
	: m_owner(owner)
{
	calculateGrabMasks();
}

bool KGlobalAccelImpl::grabKey( int keyQt, bool grab )
{
	if( !keyQt ) {
		kWarning(125) << "Tried to grab key with null code.";
		return false;
	}

	int keyCodeX;
	uint keyModX;
	KKeyServer::keyQtToCodeX(keyQt, &keyCodeX);
	KKeyServer::keyQtToModX(keyQt, &keyModX);

	keyModX &= g_keyModMaskXAccel; // Get rid of any non-relevant bits in mod
	
	// HACK: make Alt+Print work
	// only do this for the Xorg default keyboard keycodes,
	// other mappings (e.g. evdev) don't need or want it
	if( keyCodeX == XK_Sys_Req && XKeycodeToKeysym( QX11Info::display(), 111, 0 ) == XK_Print ) {
	    keyModX |= KKeyServer::modXAlt();
	    keyCodeX = 111;
	}

	if( !keyCodeX )
		return false;

    // kDebug(125) << "grabKey keyQt " << (keyQt & ~Qt::KeyboardModifierMask)
    //    << " mod " << (keyQt & Qt::KeyboardModifierMask) << " ( key: '" << QKeySequence(keyQt).toString()
    //    << "', grab: " << grab << " ): keyCodeX: " << keyCodeX << " keyModX: " << keyModX << endl;

	KXErrorHandler handler( XGrabErrorHandler );

	// We'll have to grab 8 key modifier combinations in order to cover all
	//  combinations of CapsLock, NumLock, ScrollLock.
	// Does anyone with more X-savvy know how to set a mask on QX11Info::appRootWindow so that
	//  the irrelevant bits are always ignored and we can just make one XGrabKey
	//  call per accelerator? -- ellis
#ifndef NDEBUG
	QString sDebug = QString("\tcode: 0x%1 state: 0x%2 | ").arg(keyCodeX,0,16).arg(keyModX,0,16);
#endif
	uint keyModMaskX = ~g_keyModMaskXOnOrOff;
	for( uint irrelevantBitsMask = 0; irrelevantBitsMask <= 0xff; irrelevantBitsMask++ ) {
		if( (irrelevantBitsMask & keyModMaskX) == 0 ) {
#ifndef NDEBUG
			sDebug += QString("0x%3, ").arg(irrelevantBitsMask, 0, 16);
#endif
			if( grab )
				XGrabKey( QX11Info::display(), keyCodeX, keyModX | irrelevantBitsMask,
					QX11Info::appRootWindow(), True, GrabModeAsync, GrabModeSync );
			else
				XUngrabKey( QX11Info::display(), keyCodeX, keyModX | irrelevantBitsMask, QX11Info::appRootWindow() );
		}
	}

#ifndef NDEBUG
	// kDebug(125) << sDebug;
#endif

	bool failed = false;
	if( grab ) {
		failed = handler.error( true ); // sync now
		if( failed ) {
			kDebug(125) << "grab failed!\n";
			for( uint m = 0; m <= 0xff; m++ ) {
				if(( m & keyModMaskX ) == 0 )
					XUngrabKey( QX11Info::display(), keyCodeX, keyModX | m, QX11Info::appRootWindow() );
				}
			}
	}
	
	return !failed;
}

bool KGlobalAccelImpl::x11Event( XEvent* event )
{
	switch( event->type ) {
		case MappingNotify:
			XRefreshKeyboardMapping(&event->xmapping);
			x11MappingNotify();
			return true;

		 case XKeyPress:
			if( x11KeyPress( event ) )
				return true;
			break;
	}
	return false;
}

void KGlobalAccelImpl::x11MappingNotify()
{
	kDebug(125) << "KGlobalAccelImpl::x11MappingNotify()";
	// Maybe the X modifier map has been changed.
	uint oldKeyModMaskXAccel = g_keyModMaskXAccel;
	uint oldKeyModMaskXOnOrOff = g_keyModMaskXOnOrOff;

	KKeyServer::initializeMods();
	calculateGrabMasks();
	
#if 0   //### investigate!
	if (oldKeyModMaskXAccel != g_keyModMaskXAccel || oldKeyModMaskXOnOrOff != g_keyModMaskXOnOrOff)
		// Do new XGrabKey()s.
		m_owner->regrabKeys();
#endif
}

bool KGlobalAccelImpl::x11KeyPress( const XEvent *pEvent )
{
    kDebug();
	// Keyboard needs to be ungrabed after XGrabKey() activates the grab, otherwise
        // it becomes frozen. There is a chance this will ungrab even when it should
        // not, if some code calls XGrabKeyboard() directly, but doing so in kded
        // should be very unlikely, and probably stupid.
        // If this code is again moved out of kded for some reason, this needs
        // to be revisited (I'm pretty sure this used to break KWin).
	if( !QWidget::keyboardGrabber() && !QApplication::activePopupWidget()) {
		XUngrabKeyboard( QX11Info::display(), pEvent->xkey.time );
		XFlush( QX11Info::display()); // avoid X(?) bug
	}

	uchar keyCodeX = pEvent->xkey.keycode;
	uint keyModX = pEvent->xkey.state & (g_keyModMaskXAccel | KKeyServer::MODE_SWITCH);

	KeySym keySym;
	XLookupString( (XKeyEvent*) pEvent, 0, 0, &keySym, 0 );
	uint keySymX = (uint)keySym;

	// If numlock is active and a keypad key is pressed, XOR the SHIFT state.
	//  e.g., KP_4 => Shift+KP_Left, and Shift+KP_4 => KP_Left.
	if( pEvent->xkey.state & KKeyServer::modXNumLock() ) {
		uint sym = XKeycodeToKeysym( QX11Info::display(), keyCodeX, 0 );
		// If this is a keypad key,
		if( sym >= XK_KP_Space && sym <= XK_KP_9 ) {
			switch( sym ) {
				// Leave the following keys unaltered
				// FIXME: The proper solution is to see which keysyms don't change when shifted.
				case XK_KP_Multiply:
				case XK_KP_Add:
				case XK_KP_Subtract:
				case XK_KP_Divide:
					break;
				default:
					keyModX ^= KKeyServer::modXShift();
			}
		}
	}

	int keyCodeQt;
	int keyModQt;
	KKeyServer::symXToKeyQt(keySymX, &keyCodeQt);
	KKeyServer::modXToQt(keyModX, &keyModQt);
	
	int keyQt = keyCodeQt | keyModQt;
	
	kDebug(125) << "Qt " << keyQt << " [Key: " << keyCodeQt << " Mod: " << keyModQt << "] X [Key: " << keySymX << " Mod: " << keyModX << "]";

	// All that work for this hey... argh...
	if (m_owner->keyPressed(keyQt))
		return true;

	return false;
}

void KGlobalAccelImpl::setEnabled( bool enable )
{
	if (enable) {
		kapp->installX11EventFilter( this );
	} else
		kapp->removeX11EventFilter( this );
}


#include "kglobalaccel_x11.moc"
