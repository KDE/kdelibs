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

#include "config.h"

#include <qwindowdefs.h>
#ifdef Q_WS_X11

#include "kglobalaccel_x11.h"
#include "kglobalaccel.h"
#include "kkeyserver_x11.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kkeynative.h>

#include <qregexp.h>
#include <qwidget.h>
#include <qmetaobject.h>
#include <qlinkedlist.h>
#include <qmenu.h>

#ifdef Q_WS_X11
#include <kxerrorhandler.h>
#endif

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <fixx11h.h>

extern "C" {
  static int XGrabErrorHandler( Display *, XErrorEvent *e ) {
	if ( e->error_code != BadAccess ) {
	    kdWarning() << "grabKey: got X error " << e->type << " instead of BadAccess\n";
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
	//kdDebug() << "g_keyModMaskXAccel = " << g_keyModMaskXAccel
	//	<< "g_keyModMaskXOnOrOff = " << g_keyModMaskXOnOrOff << endl;
}

//----------------------------------------------------

static QLinkedList< KGlobalAccelPrivate* >* all_accels;

KGlobalAccelPrivate::KGlobalAccelPrivate()
: KAccelBase( KAccelBase::NATIVE_KEYS )
, m_blocked( false )
, m_blockingDisabled( false )
{
        if( all_accels == NULL )
            all_accels = new QLinkedList< KGlobalAccelPrivate* >;
        all_accels->append( this );
	m_sConfigGroup = "Global Shortcuts";
	kapp->installX11EventFilter( this );
}

KGlobalAccelPrivate::~KGlobalAccelPrivate()
{
	// TODO: Need to release all grabbed keys if the main window is not shutting down.
	//for( CodeModMap::ConstIterator it = m_rgCodeModToAction.begin(); it != m_rgCodeModToAction.end(); ++it ) {
	//	const CodeMod& codemod = it.key();
	//}
        all_accels->removeAll( this );
        if( all_accels->count() == 0 ) {
            delete all_accels;
            all_accels = NULL;
        }
}

void KGlobalAccelPrivate::setEnabled( bool bEnable )
{
	m_bEnabled = bEnable;
	updateConnections();
}

void KGlobalAccelPrivate::blockShortcuts( bool block )
{
        if( all_accels == NULL )
            return;
        for( QLinkedList< KGlobalAccelPrivate* >::ConstIterator it = all_accels->begin();
             it != all_accels->end();
             ++it ) {
            if( (*it)->m_blockingDisabled )
                continue;
            (*it)->m_blocked = block;
            (*it)->updateConnections();
        }
}

void KGlobalAccelPrivate::disableBlocking( bool block )
{
        m_blockingDisabled = block;
}

bool KGlobalAccelPrivate::isEnabledInternal() const
{
        return KAccelBase::isEnabled() && !m_blocked;
}

bool KGlobalAccelPrivate::emitSignal( Signal )
{
	return false;
}

bool KGlobalAccelPrivate::connectKey( KAccelAction& action, const KKeyServer::Key& key )
	{ return grabKey( key, true, &action ); }
bool KGlobalAccelPrivate::connectKey( const KKeyServer::Key& key )
	{ return grabKey( key, true, 0 ); }
bool KGlobalAccelPrivate::disconnectKey( KAccelAction& action, const KKeyServer::Key& key )
	{ return grabKey( key, false, &action ); }
bool KGlobalAccelPrivate::disconnectKey( const KKeyServer::Key& key )
	{ return grabKey( key, false, 0 ); }

bool KGlobalAccelPrivate::grabKey( const KKeyServer::Key& key, bool bGrab, KAccelAction* pAction )
{
	if( !key.code() ) {
		kdWarning(125) << "KGlobalAccelPrivate::grabKey( " << key.key().toStringInternal() << ", " << bGrab << ", \"" << (pAction ? pAction->name().latin1() : "(null)") << "\" ): Tried to grab key with null code." << endl;
		return false;
	}

	// Make sure that grab masks have been initialized.
	if( g_keyModMaskXOnOrOff == 0 )
		calculateGrabMasks();

	uchar keyCodeX = key.code();
	uint keyModX = key.mod() & g_keyModMaskXAccel; // Get rid of any non-relevant bits in mod
	// HACK: make Alt+Print work
	if( key.sym() == XK_Sys_Req ) {
	    keyModX |= KKeyServer::modXAlt();
	    keyCodeX = 111;
	}

#ifndef __osf__
// this crashes under Tru64 so .....
	kdDebug(125) << QString( "grabKey( key: '%1', bGrab: %2 ): keyCodeX: %3 keyModX: %4\n" )
		.arg( key.key().toStringInternal() ).arg( bGrab )
		.arg( keyCodeX, 0, 16 ).arg( keyModX, 0, 16 );
#endif
	if( !keyCodeX )
		return false;

#ifdef Q_WS_X11
        KXErrorHandler handler( XGrabErrorHandler );
#endif
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
			if( bGrab )
				XGrabKey( QX11Info::display(), keyCodeX, keyModX | irrelevantBitsMask,
					QX11Info::appRootWindow(), True, GrabModeAsync, GrabModeSync );
			else
				XUngrabKey( QX11Info::display(), keyCodeX, keyModX | irrelevantBitsMask, QX11Info::appRootWindow() );
		}
	}
#ifndef NDEBUG
	kdDebug(125) << sDebug << endl;
#endif

        bool failed = false;
        if( bGrab ) {
#ifdef Q_WS_X11
        	failed = handler.error( true ); // sync now
#endif
        	// If grab failed, then ungrab any grabs that could possibly succeed
		if( failed ) {
			kdDebug(125) << "grab failed!\n";
			for( uint m = 0; m <= 0xff; m++ ) {
				if( m & keyModMaskX == 0 )
					XUngrabKey( QX11Info::display(), keyCodeX, keyModX | m, QX11Info::appRootWindow() );
				}
                }
	}
        if( !failed )
        {
		CodeMod codemod;
		codemod.code = keyCodeX;
		codemod.mod = keyModX;
		if( key.mod() & KKeyServer::MODE_SWITCH )
			codemod.mod |= KKeyServer::MODE_SWITCH;

		if( bGrab )
			m_rgCodeModToAction.insert( codemod, pAction );
		else
			m_rgCodeModToAction.remove( codemod );
	}
	return !failed;
}

bool KGlobalAccelPrivate::x11Event( XEvent* pEvent )
{
	//kdDebug(125) << "x11EventFilter( type = " << pEvent->type << " )" << endl;
	switch( pEvent->type ) {
	 case MappingNotify:
	        XRefreshKeyboardMapping( &pEvent->xmapping );
		x11MappingNotify();
		return false;
	 case XKeyPress:
		if( x11KeyPress( pEvent ) )
			return true;
	 default:
		return QWidget::x11Event( pEvent );
	}
}

void KGlobalAccelPrivate::x11MappingNotify()
{
	kdDebug(125) << "KGlobalAccelPrivate::x11MappingNotify()" << endl;
	// Maybe the X modifier map has been changed.
	KKeyServer::initializeMods();
	calculateGrabMasks();
	// Do new XGrabKey()s.
	updateConnections();
}

bool KGlobalAccelPrivate::x11KeyPress( const XEvent *pEvent )
{
	// do not change this line unless you really really know what you are doing (Matthias)
	if ( !QWidget::keyboardGrabber() && !QApplication::activePopupWidget() ) {
		XUngrabKeyboard( QX11Info::display(), pEvent->xkey.time );
                XFlush( QX11Info::display()); // avoid X(?) bug
        }

	if( !isEnabledInternal())
		return false;

	CodeMod codemod;
	codemod.code = pEvent->xkey.keycode;
	codemod.mod = pEvent->xkey.state & (g_keyModMaskXAccel | KKeyServer::MODE_SWITCH);

	// If numlock is active and a keypad key is pressed, XOR the SHIFT state.
	//  e.g., KP_4 => Shift+KP_Left, and Shift+KP_4 => KP_Left.
	if( pEvent->xkey.state & KKeyServer::modXNumLock() ) {
		// TODO: what's the xor operator in c++?
		uint sym = XKeycodeToKeysym( QX11Info::display(), codemod.code, 0 );
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
					if( codemod.mod & KKeyServer::modXShift() )
						codemod.mod &= ~KKeyServer::modXShift();
					else
						codemod.mod |= KKeyServer::modXShift();
			}
		}
	}

	KKeyNative keyNative( pEvent );
	KKey key = keyNative;

	kdDebug(125) << "x11KeyPress: seek " << key.toStringInternal()
		<< QString( " keyCodeX: %1 state: %2 keyModX: %3" )
			.arg( codemod.code, 0, 16 ).arg( pEvent->xkey.state, 0, 16 ).arg( codemod.mod, 0, 16 ) << endl;

	// Search for which accelerator activated this event:
	if( !m_rgCodeModToAction.contains( codemod ) ) {
#ifndef NDEBUG
		for( CodeModMap::ConstIterator it = m_rgCodeModToAction.begin(); it != m_rgCodeModToAction.end(); ++it ) {
			KAccelAction* pAction = *it;
			kdDebug(125) << "\tcode: " << QString::number(it.key().code, 16) << " mod: " << QString::number(it.key().mod, 16)
				<< (pAction ? QString(" name: \"%1\" shortcut: %2").arg(pAction->name()).arg(pAction->shortcut().toStringInternal()) : QString())
				<< endl;
		}
#endif
		return false;
	}
	KAccelAction* pAction = m_rgCodeModToAction[codemod];

	if( !pAction ) {
                static bool recursion_block = false;
                if( !recursion_block ) {
                        recursion_block = true;
		        QMenu* pMenu = createPopupMenu( 0, KKeySequence(key) );
		        connect( pMenu, SIGNAL(activated(int)), this, SLOT(slotActivated(int)) );
		        pMenu->exec( QPoint( 0, 0 ) );
		        disconnect( pMenu, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
		        delete pMenu;
                        recursion_block = false;
                }
	} else if( !pAction->objSlotPtr() || !pAction->isEnabled() )
		return false;
	else
		activate( pAction, KKeySequence(key) );

	return true;
}

void KGlobalAccelPrivate::activate( KAccelAction* pAction, const KKeySequence& seq )
{
	kdDebug(125) << "KGlobalAccelPrivate::activate( \"" << pAction->name() << "\" ) " << endl;

	QRegExp rexPassIndex( "([ ]*int[ ]*)" );
	QRegExp rexPassInfo( " QString" );
	QRegExp rexIndex( " ([0-9]+)$" );

	// If the slot to be called accepts an integer index
	//  and an index is present at the end of the action's name,
	//  then send the slot the given index #.
	if( rexPassIndex.search( pAction->methodSlotPtr() ) >= 0 && rexIndex.search( pAction->name() ) >= 0 ) {
		int n = rexIndex.cap(1).toInt();
		kdDebug(125) << "Calling " << pAction->methodSlotPtr() << " int = " << n << endl;
                int slot_id = pAction->objSlotPtr()->metaObject()->indexOfSlot( QMetaObject::normalizedSignature( pAction->methodSlotPtr() ).data() + 1 );
                if( slot_id >= 0 ) {
                    QMetaObject::invokeMethod (const_cast< QObject* >( pAction->objSlotPtr()), QMetaObject::normalizedSignature( pAction->methodSlotPtr() ).data() + 1, Q_ARG(int, n));
                }
	} else if( rexPassInfo.search( pAction->methodSlotPtr() ) ) {
                int slot_id = pAction->objSlotPtr()->metaObject()->indexOfSlot( QMetaObject::normalizedSignature( pAction->methodSlotPtr() ).data() + 1 );
                if( slot_id >= 0 ) {
                    QMetaObject::invokeMethod (const_cast< QObject* >( pAction->objSlotPtr()), QMetaObject::normalizedSignature( pAction->methodSlotPtr() ).data() + 1
                         ,Q_ARG(QString, pAction->name())
                         ,Q_ARG(QString, pAction->label())
                         ,Q_ARG(const KKeySequence *, &seq)
                         );
                }
	} else {
                int slot_id = pAction->objSlotPtr()->metaObject()->indexOfSlot( QMetaObject::normalizedSignature( pAction->methodSlotPtr() ).data() + 1 );
                if( slot_id >= 0 )
                    QMetaObject::invokeMethod (const_cast< QObject* >( pAction->objSlotPtr()), QMetaObject::normalizedSignature( pAction->methodSlotPtr() ).data() + 1);
	}
}

void KGlobalAccelPrivate::slotActivated( int iAction )
{
	KAccelAction* pAction = KAccelBase::actions().actionPtr( iAction );
	if( pAction )
		activate( pAction, KKeySequence() );
}

#include "kglobalaccel_x11.moc"

#endif // !Q_WS_X11
