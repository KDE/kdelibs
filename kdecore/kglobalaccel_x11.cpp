#include <qwindowdefs.h>
#ifdef Q_WS_X11

#include "kglobalaccel_x11.h"
#include "kglobalaccel.h"

#include <qregexp.h>
#include <qwidget.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kkeysequence.h>
#include <kkey_x11.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#ifdef Q_WS_X11
#ifdef KeyPress
// defined by X11 headers
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#endif
#endif

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
// g_keyModMaskXAlwaysOff
//	mask of modifiers which should never bo on.  This is the inverse
//	of those that we know of.
// g_keyModMaskXOnOrOff
//	mask of modifiers where we don't care whether they are on or off
//	(caps lock, num lock, scroll lock)
static uint g_keyModMaskXAccel = 0;
static uint g_keyModMaskXAlwaysOff = 0;
static uint g_keyModMaskXOnOrOff = 0;

static void calculateGrabMasks()
{
	g_keyModMaskXAccel = KKeyX11::accelModMaskX();
	g_keyModMaskXAlwaysOff = ~(
			KKeyX11::keyModXShift() |
			KKeyX11::keyModXLock() |
			KKeyX11::keyModXCtrl() |
			KKeyX11::keyModXAlt() |
			KKeyX11::keyModXNumLock() |
			KKeyX11::keyModXModeSwitch() |
			KKeyX11::keyModXMeta() |
			KKeyX11::keyModXScrollLock() );
	g_keyModMaskXOnOrOff =
			KKeyX11::keyModXLock() |
			KKeyX11::keyModXNumLock() |
			KKeyX11::keyModXScrollLock();

	// X11 seems to treat the ModeSwitch bit differently than the others --
	//  namely, it won't grab anything if it's set, but both switched and
	//  unswiched keys if it's not.
	//  So we always need to XGrabKey with the bit set to 0.
	g_keyModMaskXAlwaysOff |= KKeyX11::keyModXModeSwitch();
}

//----------------------------------------------------

void KGlobalAccel::setKeyEventsEnabled( bool bEnabled )
{
	KGlobalAccelPrivate::gm_bKeyEventsEnabled = bEnabled;
}

bool KGlobalAccel::areKeyEventsEnabled()
{
	return KGlobalAccelPrivate::gm_bKeyEventsEnabled;
}

//----------------------------------------------------

bool KGlobalAccelPrivate::gm_bKeyEventsEnabled = true;

KGlobalAccelPrivate::KGlobalAccelPrivate()
{
	m_sConfigGroup = "Global Shortcuts";
	kapp->installX11EventFilter( this );
}

void KGlobalAccelPrivate::setEnabled( bool bEnable )
{
	KAccelBase::setEnabled( bEnable );
	updateConnections();
};

bool KGlobalAccelPrivate::connectKey( KAccelAction& action, KKeySequence key )
{
	kdDebug(125) << "KGlobalAccel::connectKey( " << action.m_sName << ", " << key.toString() << " )" << endl;
	return grabKey( key, true );
}

bool KGlobalAccelPrivate::disconnectKey( KAccelAction& action, KKeySequence key )
{
	kdDebug(125) << "KGlobalAccel::disconnectKey( " << action.m_sName << ", " << endl;
	kdDebug(125) << "KGlobalAccel::disconnectKey( " << action.m_sName << ", " << key.toString() << " )" << endl;
	return grabKey( key, false );
}

bool KGlobalAccelPrivate::grabKey( KKeySequence key, bool bGrab )
{
	if( key.m_keyMod == -1 || !key.m_keyCode )
		return false;

	// Make sure that grab masks have been initialized.
	if( g_keyModMaskXOnOrOff == 0 )
		calculateGrabMasks();

	uchar keyCodeX = key.m_keyCode;
	uint keyModX = key.m_keyMod;

	keyModX &= g_keyModMaskXAccel; // Get rid of any non-relevant bits in mod

#ifndef __osf__
// this crashes under Tru64 so .....
	kdDebug(125) << QString( "grabKey( key: 0x%1, bGrab: %2 ): keyCodeX: %3 keyModX: %4\n" )
		.arg( key.m_keyCombQt, 0, 16 ).arg( bGrab )
		.arg( keyCodeX, 0, 16 ).arg( keyModX, 0, 16 );
#endif

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
		// Do XUngrabKey()s.
		setEnabled( false );
		// Maybe the X modifier map has been changed.
		KKeyX11::init();
		calculateGrabMasks();
		// Do new XGrabKey()s.
		setEnabled( true );
	}
}

bool KGlobalAccelPrivate::x11KeyPress( const XEvent *pEvent )
{
	// What's QWidget::keyboardGrabber() here for? -- ellis
	// Will this cause a locking up of the keyboard if
	//  gm_bKeyEventsEnabled == true?  Should we ungrab keyboard before
	//  returning?
	if( !gm_bKeyEventsEnabled ) //|| QWidget::keyboardGrabber() )
		return false;

	XUngrabKeyboard( qt_xdisplay(), pEvent->xkey.time );

	// TODO: Don't do conversion -- search in m_mapKeyToAction directly.
	KKeySequence key = KKeyX11::keyEventXToKey( pEvent );
	key.m_keyMod &= g_keyModMaskXAccel;
	uint keySymX = key.m_keySym;
	uint keyModX = key.m_keyMod;

	kdDebug(125) << "x11KeyPress: seek " << key.toString()
		<< QString( " keyCodeX: %1 state: %2 keySym: %3 keyMod: %4\n" )
			.arg( pEvent->xkey.keycode, 0, 16 ).arg( pEvent->xkey.state, 0, 16 ).arg( keySymX, 0, 16 ).arg( keyModX, 0, 16 );
	if( keySymX == 0 )
		return false;

	// Search for which accelerator activated this event:
	if( !m_mapKeyToAction.contains( key ) )
		return false;
	KAccelAction* pAction = m_mapKeyToAction[key];

	kdDebug(125) << "received action " << pAction->m_sName << endl;
	if( !pAction->m_pObjSlot || !pAction->m_bEnabled ) {
		kdDebug(125) << "KGlobalAccel::x11EventFilter(): Key has been grabbed(" << KKeyX11::keySymXToString( keySymX, keyModX, false ) << ") which doesn't have an associated action or was disabled.\n";
		return false;
	} else {
		QRegExp rexPassIndex( "([ ]*int[ ]*)" );
		QRegExp rexPassInfo( " QString" );
		QRegExp rexIndex( " ([0-9]+)$" );
		// If the slot to be called accepts an integer index
		//  and an index is present at the end of the action's name,
		//  then send the slot the given index #.
		if( rexPassIndex.search( pAction->m_psMethodSlot ) >= 0 && rexIndex.search( pAction->m_sName ) >= 0 ) {
			int n = rexIndex.cap(1).toInt();
			kdDebug(125) << "Calling " << pAction->m_psMethodSlot << " int = " << n << endl;
			connect( this, SIGNAL(activated(int)), pAction->m_pObjSlot, pAction->m_psMethodSlot );
			emit activated( n );
			disconnect( this, SIGNAL(activated(int)), pAction->m_pObjSlot, pAction->m_psMethodSlot );
		} else if( rexPassInfo.search( pAction->m_psMethodSlot ) ) {
			connect( this, SIGNAL(activated(const QString&, const QString&, int)), pAction->m_pObjSlot, pAction->m_psMethodSlot );
			emit activated( pAction->m_sName, pAction->m_sDesc, key.keyQt() );
			disconnect( this, SIGNAL(activated(const QString&, const QString&, int)), pAction->m_pObjSlot, pAction->m_psMethodSlot );
		} else {
			connect( this, SIGNAL(activated()), pAction->m_pObjSlot, pAction->m_psMethodSlot );
			emit activated();
			disconnect( this, SIGNAL(activated()), pAction->m_pObjSlot, pAction->m_psMethodSlot );
		}
	}

	return true;
}

#include "kglobalaccel_x11.moc"

#endif // !Q_WS_X11
