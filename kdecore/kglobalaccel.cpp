/* This file is part of the KDE libraries
	Copyright (C) 1998 	Mark Donohoe <donohoe@kde.org>,
	Jani Jaakkola (jjaakkol@cs.helsinki.fi),
	Nicolas Hadacek <hadacek@via.ecp.fr>
	Matthias Ettrich (ettrich@kde.org)

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

#include "kglobalaccel.h"
#include <qkeycode.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qdrawutil.h>
#include <kdebug.h>

#include <qstringlist.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>

#ifdef KeyPress
const int XKeyPress = KeyPress;
#undef KeyPress
#endif

// NOTE ABOUT CONFIGURATION CHANGES
// Test if keys enabled because these keys have made X server grabs

struct KKeyNative {
    uint keyCode, keyMod;	// For storing the X11 codes (for global shortcuts)

    KKeyNative() : keyCode(0), keyMod(0) { }
};
typedef QMap<QString, KKeyNative> KKeyNativeMap;

class  KGlobalAccelPrivate : public QWidget
{
public:
    KGlobalAccelPrivate( KGlobalAccel* a, bool do_not_grab )
	:QWidget(), accel( a ), rawModeList( 0 )
    {
	if ( !do_not_grab )
	    kapp->installX11EventFilter( this );
    }
    ~KGlobalAccelPrivate() {
	delete rawModeList;
    }

protected:
    bool x11Event( XEvent * e )
    {
	return accel->x11EventFilter( e );
    }


private:
    KGlobalAccel* accel;
public:
    QStringList* rawModeList;
    static bool g_bKeyEventsEnabled;
    KKeyNativeMap keyNativeMap;
};

bool KGlobalAccelPrivate::g_bKeyEventsEnabled = true;


KGlobalAccel::KGlobalAccel(bool _do_not_grab)
 : QObject()
{
	aAvailableId = 1;
	bEnabled = true;
	aGroup = "Global Keys";
	do_not_grab =_do_not_grab;
	d = 0;
	d = new KGlobalAccelPrivate( this, do_not_grab );
}

KGlobalAccel::KGlobalAccel(QWidget * parent, const char *name, bool _do_not_grab)
    : QObject(parent, name) {
    	aAvailableId = 1;
	bEnabled = true;
	aGroup = "Global Keys";
	do_not_grab =_do_not_grab;
	d = 0;
	d = new KGlobalAccelPrivate( this, do_not_grab );
}

KGlobalAccel::~KGlobalAccel()
{
	setEnabled( false );
	delete d;
}

void KGlobalAccel::clear()
{
	setEnabled( false );
	aKeyMap.clear();
}

void KGlobalAccel::connectItem( const QString& action,
				const QObject* receiver, const char *member,
				bool activate )
{
    if (!aKeyMap.contains(action)) {
        kdDebug() << QString::fromLatin1("KGlobalAccel : Cannot connect action %1 "
                                         "which is not in the object dictionary\n").arg(action);
        return;
    }

    KKeyEntry entry = aKeyMap[ action ];
    entry.receiver = receiver;
    entry.member = member;
    entry.aAccelId = aAvailableId;
    aKeyMap.replace(action, entry);
    aAvailableId++;

    setItemEnabled( action, activate );

}

uint KGlobalAccel::count() const
{
  return aKeyMap.count();
}

int KGlobalAccel::currentKey( const QString& action ) const
{
  KKeyEntry entry = aKeyMap[ action ];
  return entry.aCurrentKeyCode;
}

int KGlobalAccel::defaultKey( const QString& action ) const
{
  KKeyEntry entry = aKeyMap[ action ];
  return entry.aDefaultKeyCode;
}

void KGlobalAccel::disconnectItem( const QString& action,
				   const QObject* /*receiver*/, const char */*member*/ )
{
  // TODO
  KKeyEntry entry = aKeyMap[ action ];

}

QString KGlobalAccel::findKey( int key ) const
{
  for (KKeyEntryMap::ConstIterator aKeyIt = aKeyMap.begin();
       aKeyIt != aKeyMap.end(); ++aKeyIt)
    if ( key == (*aKeyIt).aCurrentKeyCode )
      return aKeyIt.key();
  return QString::null;
}

bool KGlobalAccel::insertItem(  const QString& descr, const QString& action,
				 KKey defaultKeyCode3, KKey /*defaultKeyCode4*/,
                                bool configurable )
{
  if (aKeyMap.contains(action))
    removeItem( action );

  KKeyEntry entry;
  entry.aDefaultKeyCode = defaultKeyCode3.key();
  //entry.aDefaultKeyCode4 = defaultKeyCode4.key();
  entry.aCurrentKeyCode = /*try.aConfigKeyCode = KAccel::useFourModifierKeys() ? defaultKeyCode4.key() : */ defaultKeyCode3.key();
  entry.bConfigurable = configurable;
  entry.bEnabled = false;
  entry.aAccelId = 0;
  entry.receiver = 0;
  entry.member = 0;
  entry.descr = descr;

  aKeyMap[action] = entry;
  return true;
}

bool KGlobalAccel::insertItem(  const QString& descr, const QString& action, int keyCode,
                                bool configurable )
{
	return insertItem( descr, action, keyCode, keyCode, configurable );
}

bool KGlobalAccel::insertItem( const QString& descr, const QString& action,
					   const QString& keyCode, bool configurable )
{
	return insertItem(descr, action, KKey(keyCode).key(), configurable);
}

bool KGlobalAccel::isEnabled() const
{
	return bEnabled;
}

bool KGlobalAccel::isItemEnabled( const QString& action ) const
{
  return aKeyMap[action].bEnabled;
}

KKeyEntryMap KGlobalAccel::keyDict() const
{
	return aKeyMap;
}

void KGlobalAccel::readSettings(KConfig* config)
{
	kdDebug(125) << "KGlobalAccel::readSettings()\n";
	QMap<int, QString> aKeysToGrab;
	int cKeysToGrab = 0;

	KConfigBase *pConfig = config ? config : KGlobal::config();
	KConfigGroupSaver cgs( pConfig, aGroup );

	// Read settings from config file, and if it differs
	//  from the current setting, release the key and save the
	//  new key for grabbing afterwards.
	for( KKeyEntryMap::Iterator it = aKeyMap.begin(); it != aKeyMap.end(); ++it ) {
		QString keyStr = pConfig->readEntry( it.key() );
		uint keyQt;
		uchar keyCodeX;
		uint keyModX;
		KKeyNative keyNative;

		if ( keyStr.isEmpty() || keyStr.startsWith( "default" ))
			keyQt = /*KAccel::useFourModifierKeys() ? (*it).aDefaultKeyCode4 :*/ (*it).aDefaultKeyCode;
		else
			keyQt = KAccel::stringToKey( keyStr );
		if( d->keyNativeMap.contains( it.key() ) )
			keyNative = d->keyNativeMap[ it.key() ];

		// Get X keycodes for current X keymap.
		KAccel::keyQtToKeyX( keyQt, &keyCodeX, 0, &keyModX );

		kdDebug(125) << QString( it.key()+" = "+keyStr+" key: 0x%1 curKey: 0x%2 enabled: %3\n" )
			.arg( keyQt, 0, 16 ). arg( (*it).aCurrentKeyCode, 0, 16 ).arg( (*it).bEnabled );

		// If the X codes have changed,
		if( (*it).bEnabled && (keyCodeX != keyNative.keyCode || keyModX != keyNative.keyMod) ) {
			if( keyNative.keyCode )
				grabKey( it.key(), false );
			aKeysToGrab[cKeysToGrab++] = it.key();
		}
		(*it).aConfigKeyCode = (*it).aCurrentKeyCode = keyQt;
	}

	// Grab the changed keys.
	for( int i = 0; i < cKeysToGrab; i++ )
		grabKey( aKeysToGrab[i], true );
}

void KGlobalAccel::readSettings()
{
    readSettings( NULL );
}

void KGlobalAccel::removeItem( const QString& action )
{
    aKeyMap.remove(action);
}

void KGlobalAccel::setConfigGroup( const QString& group )
{
	aGroup = group;
}

QString KGlobalAccel::configGroup() const
{
	return aGroup;
}

void KGlobalAccel::setKeyEventsEnabled( bool enabled )
{
	KGlobalAccelPrivate::g_bKeyEventsEnabled = enabled;
}

bool KGlobalAccel::areKeyEventsEnabled()
{
	return KGlobalAccelPrivate::g_bKeyEventsEnabled;
}

void KGlobalAccel::setEnabled( bool activate )
{
    kdDebug(125) << QString( "KGlobalAccel::setEnabled( %1 )\n" ).arg( activate );
    for (KKeyEntryMap::ConstIterator it = aKeyMap.begin();
         it != aKeyMap.end(); ++it)
        setItemEnabled( it.key(), activate );
    bEnabled = activate;
}

void KGlobalAccel::setItemEnabled( const QString& action, bool activate )
{
    if ( !aKeyMap.contains(action) ) {
	kdDebug() << QString::fromLatin1("KGlobalAccel : cannot enable action %1 "
					 "which is not in the object dictionary\n").arg(action);
        return;
    }

    KKeyEntry& entry = aKeyMap[action];
    if ( entry.bEnabled != activate ) {
        aKeyMap[action].bEnabled = activate;
        if ( entry.aCurrentKeyCode )
            grabKey( action, activate );
    }
}

bool KGlobalAccel::setKeyDict( const KKeyEntryMap& nKeyMap )
{
    for (KKeyEntryMap::ConstIterator it = aKeyMap.begin();
         it != aKeyMap.end(); ++it) {
	// ungrab all connected and enabled keys
        QString s;
        if ( (*it).bEnabled )
            grabKey( it.key(), false );
    }

    // Clear the dictionary
    aKeyMap.clear();
    d->keyNativeMap.clear();

    // Insert the new items into the dictionary and reconnect if neccessary
    // Note also swap config and current key codes !!!!!!
    for (KKeyEntryMap::ConstIterator it = nKeyMap.begin();
         it != nKeyMap.end(); ++it) {

        KKeyEntry entry = *it;

        // Not we write config key code to current key code !!
        entry.aCurrentKeyCode = (*it).aConfigKeyCode;

        aKeyMap[it.key()] = entry;
        if ( entry.bEnabled )
            grabKey( it.key(), true );
    }
    return true;
}

void KGlobalAccel::writeSettings(KConfig* config) const
{
    KAccel::writeKeyMap( aKeyMap, aGroup, config );
}

void KGlobalAccel::writeSettings() const
{
    KAccel::writeKeyMap( aKeyMap, aGroup, NULL );
}

bool grabFailed;

extern "C" {
  static int XGrabErrorHandler( Display *, XErrorEvent *e ) {
	if ( e->error_code != BadAccess ) {
	    kdWarning() << "grabKey: got X error " << e->type << " instead of BadAccess\n";
	}
	grabFailed = true;
	return 0;
  }
}

static uint g_keyModMaskXAccel = 0;
static uint g_keyModMaskXAlwaysOff = 0;
static uint g_keyModMaskXOnOrOff = 0;

static void calculateGrabMasks()
{
	KAccel::readModifierMapping();
	g_keyModMaskXAccel = KAccel::accelModMaskX();
	g_keyModMaskXAlwaysOff = ~(
			KAccel::keyModXShift() |
			KAccel::keyModXLock() |
			KAccel::keyModXCtrl() |
			KAccel::keyModXAlt() |
			KAccel::keyModXNumLock() |
			KAccel::keyModXModeSwitch() |
			KAccel::keyModXMeta() |
			KAccel::keyModXScrollLock() );
	g_keyModMaskXOnOrOff =
			KAccel::keyModXLock() |
			KAccel::keyModXNumLock() |
			KAccel::keyModXScrollLock();

	// X11 seems to treat the ModeSwitch bit differently than the others --
	//  namely, it won't grab anything if it's set, but both switched and
	//  unswiched keys if it's not.
	//  So we always need to XGrabKey with the bit set to 0.
	g_keyModMaskXAlwaysOff |= KAccel::keyModXModeSwitch();
}

bool KGlobalAccel::grabKey( const QString &action, bool bGrab )
{
	if( action.isEmpty() )
		return false;
	if( bGrab ) {
		if( do_not_grab )
			return true;
		if( !aKeyMap.contains( action ) )
			return false;
	} else if( !d->keyNativeMap.contains( action ) ) {
		kdDebug(125) << "Tried to ungrab an action (" << action <<") which is not is d->keyNativeMap." << endl;
		return false;
	}

	// Make sure that grab masks have been initialized.
	if( g_keyModMaskXOnOrOff == 0 )
		calculateGrabMasks();

	// Get the X equivalents.
	KKey key;
	KKeyNative keyNative;
	uchar keyCodeX;
	uint keyModX;

	if( bGrab ) {
		key = aKeyMap[action].aCurrentKeyCode;
		KAccel::keyQtToKeyX( key.key(), &keyCodeX, 0, &keyModX );
		keyNative.keyCode = keyCodeX;
		keyNative.keyMod = keyModX;
		d->keyNativeMap[action] = keyNative;
	} else {
		key = aKeyMap[action].aCurrentKeyCode; // for kdDebug output only
		keyCodeX = d->keyNativeMap[action].keyCode;
		keyModX = d->keyNativeMap[action].keyMod;
	}

	keyModX &= g_keyModMaskXAccel; // Get rid of any non-relevant bits in mod

#ifndef __osf__
// this crashes under Tru64 so .....
	kdDebug(125) << QString( "grabKey( \"" + action + "\" key: 0x%1, bGrab: %2 ): %3 keyCodeX: %4 keyModX: %5\n" )
		.arg( key.key(), 0, 16 ).arg( bGrab ).arg( action )
		.arg( keyCodeX, 0, 16 ).arg( keyModX, 0, 16 );
#endif

	if( !keyCodeX )
		return false;

	// We want to catch only our own errors
	grabFailed = false;
	XSync(qt_xdisplay(),0);
	XErrorHandler savedErrorHandler=XSetErrorHandler(XGrabErrorHandler);

	// We'll have to grab 8 key modifier combinations in order to cover all
	//  combinations of CapsLock, NumLock, ScrollLock.
	// Does anyone with more X-savvy know how to set a mask on qt_xrootwin so that
	//  the irrelevant bits are always ignored and we can just make one XGrabKey
	//  call per accelerator?
	uint keyModMaskX = ~g_keyModMaskXOnOrOff;
	for( uint irrelevantBitsMask = 0; irrelevantBitsMask <= 0xff; irrelevantBitsMask++ ) {
		if( (irrelevantBitsMask & keyModMaskX) == 0 ) {
			kdDebug(125) << QString( "code: 0x%1 state: 0x%2 | 0x%3\n" )
					.arg( keyCodeX, 0, 16 ).arg( keyModX, 0, 16 ).arg( irrelevantBitsMask, 0, 16 );
			if( bGrab ) {
				XGrabKey( qt_xdisplay(), keyCodeX, keyModX | irrelevantBitsMask,
					qt_xrootwin(), True, GrabModeAsync, GrabModeSync );

				// If grab failed, then ungrab any previously successful grabs.
				if( grabFailed ) {
					kdDebug(125) << "grab failed!\n";
					d->keyNativeMap.remove( action );
					for( uint m = 0; m < irrelevantBitsMask; m++ ) {
						if( m & keyModMaskX == 0 )
							XUngrabKey( qt_xdisplay(), keyCodeX, keyModX | m, qt_xrootwin() );
					}
					break;
				}
			} else {
				d->keyNativeMap.remove( action );
				XUngrabKey( qt_xdisplay(), keyCodeX, keyModX | irrelevantBitsMask, qt_xrootwin() );
			}
		}
	}

	XSync(qt_xdisplay(),0);
	XSetErrorHandler(savedErrorHandler);

	return !grabFailed;
}

bool KGlobalAccel::x11EventFilter( const XEvent *event_ ) {
    uint keyModX, keyModX2;
    uint keySymX, keySymX2;

    if ( event_->type == MappingNotify ) {
	kdDebug(125) << "Caught MappingNotify" << endl;
	// Do XUngrabKey()s.
	setEnabled( false );
	// Maybe the X modifier map has been changed.
	calculateGrabMasks();
	// Do new XGrabKey()s.
	setEnabled( true );
	return true;
    }

    if ( aKeyMap.isEmpty() ) return false;
    if ( event_->type != XKeyPress ) return false;
    if ( !KGlobalAccelPrivate::g_bKeyEventsEnabled ) return false;

    KAccel::keyEventXToKeyX( event_, 0, &keySymX, &keyModX );
    keyModX &= g_keyModMaskXAccel;

    kdDebug(125) << "x11EventFilter: seek " << KAccel::keySymXToString( keySymX, keyModX, false )
    	<< QString( " keyCodeX: %1 state: %2 keySym: %3 keyMod: %4\n" )
    		.arg( event_->xkey.keycode, 0, 16 ).arg( event_->xkey.state, 0, 16 ).arg( keySymX, 0, 16 ).arg( keyModX, 0, 16 );
    if( keySymX == 0 )
    	return false;

    // Search for which accelerator activated this event:
    KKeyEntry entry;
    QString sConfigKey;
    for (KKeyEntryMap::ConstIterator it = aKeyMap.begin(); it != aKeyMap.end(); ++it) {
	KAccel::keyQtToKeyX( (*it).aCurrentKeyCode, 0, &keySymX2, &keyModX2 );
	//kdDebug() << "x11EventFilter: inspecting " << KAccel::keyToString( (*it).aCurrentKeyCode )
	//	<< QString( " keySym: %1 keyMod: %2\n" ).arg( keySymX2, 0, 16 ).arg( keyModX2, 0, 16 );
	if ( keySymX == keySymX2 && keyModX == (keyModX2 & g_keyModMaskXAccel) ) {
	    entry = *it;
	    sConfigKey = it.key();
	    break;
	}
    }

    if ( sConfigKey.isNull() )
	return false;
    
    if ( !QWidget::keyboardGrabber() ) {
	kdDebug(125) << "received action " << sConfigKey << endl;
	if ( !d->rawModeList || !d->rawModeList->contains( sConfigKey ) ) {
	    XUngrabKeyboard(qt_xdisplay(), event_->xkey.time );
	} else {
	    kdDebug(125) << "in raw mode !" << endl;
	}
	if ( !entry.receiver || !entry.bEnabled ) {
		kdDebug(125) << "KGlobalAccel::x11EventFilter(): Key has been grabbed (" << KAccel::keySymXToString( keySymX, keyModX, false ) << ") which doesn't have an associated action or was disabled.\n";
		return false;
	} else {
		QRegExp r1( "([ ]*int[ ]*)" ), r2( " [0-9]+$" );
		if( r1.match( entry.member ) >= 0 && r2.match( sConfigKey ) >= 0 ) {
			int n = sConfigKey.mid( sConfigKey.findRev(' ')+1 ).toInt();
			kdDebug(125) << "Calling " << entry.member << " int = " << n << endl;
			connect( this, SIGNAL( activated( int ) ),
				entry.receiver, entry.member);
			emit activated( n );
			disconnect( this, SIGNAL( activated( int ) ), entry.receiver,
				entry.member );
		} else {
			connect( this, SIGNAL( activated() ),
				entry.receiver, entry.member);
			emit activated();
			disconnect( this, SIGNAL( activated() ), entry.receiver,
				entry.member );
		}
	}
    }

    return true;
}


void KGlobalAccel::setItemRawModeEnabled( const QString& action, bool activate )
{
    if ( !d->rawModeList )
	d->rawModeList = new QStringList;
    if ( activate ) {
	if ( !d->rawModeList->contains( action ) )
	    d->rawModeList->append( action );
    } else {
	d->rawModeList->remove( action );
    }
}

/*****************************************************************************/

uint keyToXMod( int keyCode )
{
	uint mod = 0;

	if ( keyCode == 0 ) return mod;

	if ( keyCode & Qt::SHIFT )
		 mod |= ShiftMask;
	if ( keyCode & Qt::CTRL )
		 mod |= ControlMask;
	if ( keyCode & Qt::ALT )
		 mod |= Mod1Mask;
	if ( keyCode & (Qt::ALT<<1) )
		 mod |= Mod4Mask;

	return mod;
}

uint keyToXSym( int keyCode )
{
	uint keySymX;
	KAccel::keyQtToKeyX( keyCode, 0, &keySymX, 0 );
	return keySymX;
}

#include "kglobalaccel.moc"

