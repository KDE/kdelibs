#include <qwindowdefs.h>
#ifdef Q_WS_X11

#include "kglobalaccel_x11.h"
#include "kglobalaccel.h"
#include "kkeyserver_x11.h"

#include <qpopupmenu.h>
#include <qregexp.h>
#include <qwidget.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kkeynative.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#ifdef KeyPress
// defined by X11 headers
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#endif

// this is the flag in a keypress event's state variable which indicates mode_switch (AltGr).
#define MODE_SWITCH 0x2000

static bool g_bGrabFailed;

extern "C" {
  static int XGrabErrorHandler( Display *, XErrorEvent *e ) {
	if ( e->error_code != BadAccess ) {
	    kdWarning() << "grabKey: got X error " << e->type << " instead of BadAccess\n";
	}
	g_bGrabFailed = true;
	return 0;
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
			KKeyServer::modXScrollLock();
	//kdDebug() << "g_keyModMaskXAccel = " << g_keyModMaskXAccel
	//	<< "g_keyModMaskXOnOrOff = " << g_keyModMaskXOnOrOff << endl;
}

//----------------------------------------------------

KGlobalAccelPrivate::KGlobalAccelPrivate()
: KAccelBase( KAccelBase::NATIVE_KEYS )
{
	m_sConfigGroup = "Global Shortcuts";
	kapp->installX11EventFilter( this );
}

KGlobalAccelPrivate::~KGlobalAccelPrivate()
{
	// TODO: Need to release all grabbed keys if the main window is not shutting down.
	//for( CodeModMap::ConstIterator it = m_rgCodeModToAction.begin(); it != m_rgCodeModToAction.end(); ++it ) {
	//	const CodeMod& codemod = it.key();
	//}
}

void KGlobalAccelPrivate::setEnabled( bool bEnable )
{
	m_bEnabled = bEnable;
	//updateConnections();
};

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
	uint keyModX = key.mod();

	keyModX &= g_keyModMaskXAccel; // Get rid of any non-relevant bits in mod

#ifndef __osf__
// this crashes under Tru64 so .....
	kdDebug(125) << QString( "grabKey( key: '%1', bGrab: %2 ): keyCodeX: %3 keyModX: %4\n" )
		.arg( key.key().toStringInternal() ).arg( bGrab )
		.arg( keyCodeX, 0, 16 ).arg( keyModX, 0, 16 );
#endif
	if( !keyCodeX )
		return false;

	// We want to catch only our own errors
	g_bGrabFailed = false;
	XSync( qt_xdisplay(), 0 );
	XErrorHandler savedErrorHandler = XSetErrorHandler( XGrabErrorHandler );

	// We'll have to grab 8 key modifier combinations in order to cover all
	//  combinations of CapsLock, NumLock, ScrollLock.
	// Does anyone with more X-savvy know how to set a mask on qt_xrootwin so that
	//  the irrelevant bits are always ignored and we can just make one XGrabKey
	//  call per accelerator? -- ellis
	QString sDebug = QString("\tcode: 0x%1 state: 0x%2 | ").arg(keyCodeX,0,16).arg(keyModX,0,16);
	uint keyModMaskX = ~g_keyModMaskXOnOrOff;
	for( uint irrelevantBitsMask = 0; irrelevantBitsMask <= 0xff; irrelevantBitsMask++ ) {
		if( (irrelevantBitsMask & keyModMaskX) == 0 ) {
			sDebug += QString("0x%3, ").arg(irrelevantBitsMask, 0, 16);
			if( bGrab ) {
				XGrabKey( qt_xdisplay(), keyCodeX, keyModX | irrelevantBitsMask,
					qt_xrootwin(), True, GrabModeAsync, GrabModeSync );

				// If grab failed, then ungrab any previously successful grabs.
				if( g_bGrabFailed ) {
					kdDebug(125) << "grab failed!\n";
					for( uint m = 0; m < irrelevantBitsMask; m++ ) {
						if( m & keyModMaskX == 0 )
							XUngrabKey( qt_xdisplay(), keyCodeX, keyModX | m, qt_xrootwin() );
					}
					break;
				}
			} else
				XUngrabKey( qt_xdisplay(), keyCodeX, keyModX | irrelevantBitsMask, qt_xrootwin() );
		}
	}
	kdDebug(125) << sDebug << endl;

	XSync( qt_xdisplay(), 0 );
	XSetErrorHandler( savedErrorHandler );

	if( !g_bGrabFailed ) {
		CodeMod codemod;
		codemod.code = keyCodeX;
		codemod.mod = key.mod() & (g_keyModMaskXAccel | MODE_SWITCH);
		if( bGrab )
			m_rgCodeModToAction.insert( codemod, pAction );
		else
			m_rgCodeModToAction.remove( codemod );
	}
	return !g_bGrabFailed;
}

bool KGlobalAccelPrivate::x11Event( XEvent* pEvent )
{
	//kdDebug(125) << "x11EventFilter( type = " << pEvent->type << " )" << endl;
	switch( pEvent->type ) {
	 case MappingNotify:
		x11MappingNotify();
		return true;
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
	if( m_bEnabled ) {
		// Maybe the X modifier map has been changed.
		KKeyServer::initializeMods();
		calculateGrabMasks();
		// Do new XGrabKey()s.
		updateConnections();
	}
}

bool KGlobalAccelPrivate::x11KeyPress( const XEvent *pEvent )
{
	// do not change this line unless you really really know what you are doing (Matthias)
	if ( !QWidget::keyboardGrabber() && !QApplication::activePopupWidget() )
		XUngrabKeyboard( qt_xdisplay(), pEvent->xkey.time );

	if( !m_bEnabled )
		return false;

	CodeMod codemod;
	codemod.code = pEvent->xkey.keycode;
	codemod.mod = pEvent->xkey.state & (g_keyModMaskXAccel | MODE_SWITCH);

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
			<< (pAction ? QString(" name: \"%1\" shortcut: %2").arg(pAction->name()).arg(pAction->shortcut().toStringInternal()) : QString::null)
			<< endl;
		}
#endif
		return false;
	}
	KAccelAction* pAction = m_rgCodeModToAction[codemod];

	if( !pAction ) {
		QPopupMenu* pMenu = createPopupMenu( 0, KKeySequence(key) );
		connect( pMenu, SIGNAL(activated(int)), this, SLOT(slotActivated(int)) );
		pMenu->exec( QPoint( 0, 0 ) );
		disconnect( pMenu, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
		delete pMenu;
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
		connect( this, SIGNAL(activated(int)), pAction->objSlotPtr(), pAction->methodSlotPtr() );
		emit activated( n );
		disconnect( this, SIGNAL(activated(int)), pAction->objSlotPtr(), pAction->methodSlotPtr() );
	} else if( rexPassInfo.search( pAction->methodSlotPtr() ) ) {
		connect( this, SIGNAL(activated(const QString&, const QString&, const KKeySequence&)), pAction->objSlotPtr(), pAction->methodSlotPtr() );
		emit activated( pAction->name(), pAction->label(), seq );
		disconnect( this, SIGNAL(activated(const QString&, const QString&, const KKeySequence&)), pAction->objSlotPtr(), pAction->methodSlotPtr() );
	} else {
		connect( this, SIGNAL(activated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
		emit activated();
		disconnect( this, SIGNAL(activated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
	}
}

void KGlobalAccelPrivate::slotActivated( int iAction )
{
	KAccelAction* pAction = actions().actionPtr( iAction );
	if( pAction )
		activate( pAction, KKeySequence() );
}

#include "kglobalaccel_x11.moc"

#endif // !Q_WS_X11
