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

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>

// NOTE ABOUT CONFIGURATION CHANGES
// Test if keys enabled because these keys have made X server grabs

class  KGlobalAccelPrivate : public QWidget
{
public:
    KGlobalAccelPrivate( KGlobalAccel* a)
	:QWidget(), accel( a )
    {
	kapp->installX11EventFilter( this );
    }

protected:
    bool x11Event( XEvent * e )
    {
	return accel->x11EventFilter( e );
    }


private:
    KGlobalAccel* accel;
};

KGlobalAccel::KGlobalAccel(bool _do_not_grab)
 : QObject()
{
	aAvailableId = 1;
	bEnabled = true;
	aGroup = "Global Keys";
	do_not_grab =_do_not_grab;
	d = 0;
	if ( !do_not_grab )
	    d = new KGlobalAccelPrivate( this );
}

KGlobalAccel::KGlobalAccel(QWidget * parent, const char *name, bool _do_not_grab)
    : QObject(parent, name) {
    	aAvailableId = 1;
	bEnabled = true;
	aGroup = "Global Keys";
	do_not_grab =_do_not_grab;
	d = 0;
	if ( !do_not_grab )
	    d = new KGlobalAccelPrivate( this );
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

bool KGlobalAccel::grabKey( uint keysym, uint mod ) {
	// Most of this comes from kpanel/main.C
	// Copyright (C) 1996,97 Matthias Ettrich
	static int NumLockMask = 0;

	if (do_not_grab)
	  return true;

	if (!keysym || !XKeysymToKeycode(qt_xdisplay(), keysym)) return false;
	if (!NumLockMask){
		XModifierKeymap* xmk = XGetModifierMapping(qt_xdisplay());
		int i;
		for (i=0; i<8; i++){
		   if (xmk->modifiermap[xmk->max_keypermod * i] ==
		   		XKeysymToKeycode(qt_xdisplay(), XK_Num_Lock))
		   			NumLockMask = (1<<i);
		}
	}

	grabFailed = false;

	// We wan't to catch only our own errors
	XSync(qt_xdisplay(),0);
	XErrorHandler savedErrorHandler=XSetErrorHandler(XGrabErrorHandler);
	
	XGrabKey(qt_xdisplay(),
		XKeysymToKeycode(qt_xdisplay(), keysym), mod,
		qt_xrootwin(), True,
		GrabModeAsync, GrabModeSync);
	XGrabKey(qt_xdisplay(),
		XKeysymToKeycode(qt_xdisplay(), keysym), mod | LockMask,
		qt_xrootwin(), True,
		GrabModeAsync, GrabModeSync);
	XGrabKey(qt_xdisplay(),
		XKeysymToKeycode(qt_xdisplay(), keysym), mod | NumLockMask,
		qt_xrootwin(), True,
		GrabModeAsync, GrabModeSync);
	XGrabKey(qt_xdisplay(),
		XKeysymToKeycode(qt_xdisplay(), keysym), mod | LockMask | NumLockMask,
		qt_xrootwin(), True,
		GrabModeAsync, GrabModeSync);

	XSync(qt_xdisplay(),0);
	XSetErrorHandler(savedErrorHandler);
	
	if (grabFailed) {
		// FIXME: ungrab all successfull grabs!
		//warning("Global grab failed!");
   		return false;
	}
	return true;
}

bool KGlobalAccel::insertItem(  const QString& descr, const QString& action, int keyCode,
                                bool configurable )
{
  if (aKeyMap.contains(action))
    removeItem( action );
	
  KKeyEntry entry;
  entry.aDefaultKeyCode = keyCode;
  entry.aCurrentKeyCode = keyCode;
  entry.bConfigurable = configurable;
  entry.bEnabled = false;
  entry.aAccelId = 0;
  entry.receiver = 0;
  entry.member = 0;
  entry.descr = descr;

  aKeyMap[action] = entry;
  return true;
}

bool KGlobalAccel::insertItem( const QString& descr, const QString& action,
					   const QString& keyCode, bool configurable )
{
	int iKeyCode = KAccel::stringToKey( keyCode );
	return insertItem(descr, action, iKeyCode, configurable);
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

void KGlobalAccel::readSettings()
{
	QString s;
	KConfig *pConfig = KGlobal::config();

        KConfigGroupSaver cs(pConfig, aGroup);

        for (KKeyEntryMap::ConstIterator aKeyIt = aKeyMap.begin();
             aKeyIt != aKeyMap.end(); ++aKeyIt) {
          s = pConfig->readEntry( aKeyIt.key() );
          if ( (*aKeyIt).bEnabled ) {
            uint keysym = keyToXSym( (*aKeyIt).aCurrentKeyCode );
            uint mod = keyToXMod( (*aKeyIt).aCurrentKeyCode );
            ungrabKey( keysym, mod );
          }

	}
        for (KKeyEntryMap::Iterator aKeyIt = aKeyMap.begin();
             aKeyIt != aKeyMap.end(); ++aKeyIt) {
          s = pConfig->readEntry( aKeyIt.key() );
          if ( s.isNull() )
            (*aKeyIt).aConfigKeyCode =  (*aKeyIt).aDefaultKeyCode;
          else
            (*aKeyIt).aConfigKeyCode = KAccel::stringToKey( s );

          (*aKeyIt).aCurrentKeyCode =  (*aKeyIt).aConfigKeyCode;

          if (  (*aKeyIt).bEnabled ) {
            uint keysym = keyToXSym( (*aKeyIt).aCurrentKeyCode );
            uint mod = keyToXMod( (*aKeyIt).aCurrentKeyCode );
            grabKey( keysym, mod );
          }

	}
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

void KGlobalAccel::setEnabled( bool activate )
{
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

    KKeyEntry entry = aKeyMap[action];
    bool old = entry.bEnabled;
    aKeyMap[action].bEnabled = activate;
    if ( entry.bEnabled == old ) return;

    if ( entry.aCurrentKeyCode == 0 ) return;
	
    uint keysym = keyToXSym( entry.aCurrentKeyCode );
    uint mod = keyToXMod( entry.aCurrentKeyCode );

    if ( keysym == NoSymbol ) return;

    if ( entry.bEnabled )
        grabKey( keysym, mod );
    else
        ungrabKey( keysym, mod );

}

bool KGlobalAccel::setKeyDict( const KKeyEntryMap& nKeyMap )
{
    for (KKeyEntryMap::ConstIterator it = aKeyMap.begin();
         it != aKeyMap.end(); ++it) {
	// ungrab all connected and enabled keys
        QString s;
        if ( (*it).bEnabled ) {
            uint keysym = keyToXSym( (*it).aCurrentKeyCode );
            uint mod = keyToXMod( (*it).aCurrentKeyCode );
            ungrabKey( keysym, mod );
        }
    }
	
    // Clear the dictionary
    aKeyMap.clear();
	
    // Insert the new items into the dictionary and reconnect if neccessary
    // Note also swap config and current key codes !!!!!!
    for (KKeyEntryMap::ConstIterator it = nKeyMap.begin();
         it != nKeyMap.end(); ++it) {

        KKeyEntry entry = *it;

        // Not we write config key code to current key code !!
        entry.aCurrentKeyCode = (*it).aConfigKeyCode;
	
        aKeyMap[it.key()] = entry;
        if ( entry.bEnabled ) {
            uint keysym = keyToXSym( entry.aCurrentKeyCode );
            uint mod = keyToXMod( entry.aCurrentKeyCode );
            grabKey( keysym, mod );
        }
		
    }
    return true;
}

bool KGlobalAccel::ungrabKey( uint keysym, uint mod ) {
	// Most of this comes from kpanel/main.C
	// Copyright (C) 1996,97 Matthias Ettrich
	static int NumLockMask = 0;
	
	if (do_not_grab)
	  return true;

	if (!keysym||!XKeysymToKeycode(qt_xdisplay(), keysym)) return false;
	if (!NumLockMask){
		XModifierKeymap* xmk = XGetModifierMapping(qt_xdisplay());
		int i;
		for (i=0; i<8; i++){
		   if (xmk->modifiermap[xmk->max_keypermod * i] ==
		   		XKeysymToKeycode(qt_xdisplay(), XK_Num_Lock))
		   			NumLockMask = (1<<i);
		}
	}

	grabFailed = false;

	// We wan't to catch only our own errors
	XSync(qt_xdisplay(),0);
	XErrorHandler savedErrorHandler=XSetErrorHandler(XGrabErrorHandler);
	
	XUngrabKey(qt_xdisplay(),
		XKeysymToKeycode(qt_xdisplay(), keysym), mod,
		qt_xrootwin());
	XUngrabKey(qt_xdisplay(),
		XKeysymToKeycode(qt_xdisplay(), keysym), mod | LockMask,
		qt_xrootwin());
	XUngrabKey(qt_xdisplay(),
		XKeysymToKeycode(qt_xdisplay(), keysym), mod | NumLockMask,
		qt_xrootwin());
	XUngrabKey(qt_xdisplay(),
		XKeysymToKeycode(qt_xdisplay(), keysym), mod | LockMask | NumLockMask,
		qt_xrootwin());

	XSync(qt_xdisplay(),0);
	XSetErrorHandler(savedErrorHandler);
	if (grabFailed) {
		// FIXME: ungrab all successfull grabs!
		//warning("Global grab failed!");
   		return false;
	}
	return true;
}

void KGlobalAccel::writeSettings() const
{
    KConfig *pConfig = KGlobal::config();

    KConfigGroupSaver cs(pConfig, aGroup);

    for (KKeyEntryMap::ConstIterator it = aKeyMap.begin();
         it != aKeyMap.end(); ++it) {
        if ( (*it).bConfigurable ){
            pConfig->writeEntry( it.key(),
                                 KAccel::keyToString( (*it).aCurrentKeyCode),
                                 true, true);
        }
    }
    pConfig->sync();
}

bool KGlobalAccel::x11EventFilter( const XEvent *event_ ) {

	if ( aKeyMap.isEmpty() ) return false;
	if ( event_->type != KeyPress ) return false;
	
	uint mod=event_->xkey.state & (ControlMask | ShiftMask | Mod1Mask);
	uint keysym= XKeycodeToKeysym(qt_xdisplay(), event_->xkey.keycode, 0);
	
        KKeyEntry entry;

        for (KKeyEntryMap::ConstIterator it = aKeyMap.begin();
             it != aKeyMap.end(); ++it) {
            int kc = (*it).aCurrentKeyCode;
            if ( mod == keyToXMod( kc ) && keysym == keyToXSym( kc ) ) {
                entry = *it;
            }
	}
	
	if ( !entry.receiver )
            return false;
	

	XAllowEvents(qt_xdisplay(), AsyncKeyboard, CurrentTime);
	XUngrabKeyboard(qt_xdisplay(), CurrentTime);
	XSync(qt_xdisplay(), false);
	if ( !QWidget::keyboardGrabber() ) {
	    connect( this, SIGNAL( activated() ),
                     entry.receiver, entry.member);
	    emit activated();
	    disconnect( this, SIGNAL( activated() ), entry.receiver,
                        entry.member );
	}

	return true;
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
		
	return mod;
}

uint keyToXSym( int keyCode )
{
	char *toks[4], *next_tok;
        char sKey[100];
	int nb_toks = 0;

	uint keysym = 0;
	QString s = KAccel::keyToString( keyCode);
	
	if ( s.isEmpty() ) return keysym;

        qstrncpy(sKey, s.ascii(), sizeof(sKey));
	next_tok = strtok( sKey, "+" );

	if ( next_tok == 0L ) return 0;
	
	do {
		toks[nb_toks] = next_tok;
		nb_toks++;
		if ( nb_toks == 5 ) return 0;
		next_tok = strtok( 0L, "+" );
	} while ( next_tok != 0L );

	// Test for exactly one key (other tokens are accelerators)
	// Fill the keycode with infos
	bool  keyFound = false;
	for ( int i=0; i<nb_toks; i++ ) {
		if (stricmp(toks[i], "SHIFT") != 0 &&
		    stricmp(toks[i], "CTRL")  != 0 &&
		    stricmp(toks[i], "ALT")   != 0) {
		   if ( keyFound ) return 0;
		   keyFound = true;
		   QCString l = toks[i];
		   l = l.lower();
		   keysym = XStringToKeysym(l.data());
		   if (keysym == NoSymbol){
		     keysym = XStringToKeysym( toks[i] );
		   }
		   if ( keysym == NoSymbol ) {
			return 0;
		  }
		}
	}
	
	return keysym;
}

#include "kglobalaccel.moc"

