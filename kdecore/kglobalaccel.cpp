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
#include <qmessagebox.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>

// NOTE ABOUT CONFIGURATION CHANGES
// Test if keys enabled because these keys have made X server grabs

KGlobalAccel::KGlobalAccel(bool _do_not_grab)
 : QObject(), aKeyDict(100)
{
	aAvailableId = 1;
	bEnabled = true;
	aGroup = "Global Keys";
	do_not_grab =_do_not_grab;
}

KGlobalAccel::KGlobalAccel(QWidget * parent, const QString& name, bool _do_not_grab)
    : QObject(parent, name), aKeyDict(100) {
    	aAvailableId = 1;
	bEnabled = true;
	aGroup = "Global Keys";
	do_not_grab =_do_not_grab;

}

KGlobalAccel::~KGlobalAccel()
{
	setEnabled( false );
}

void KGlobalAccel::clear()
{
	setEnabled( false );
	aKeyDict.clear();
}

void KGlobalAccel::connectItem( const QString& action,
				const QObject* receiver, const QString& member,
				bool activate )
{
    KKeyEntry *pEntry = aKeyDict[ action ];
	if ( !pEntry ) {
		QString str = i18n("KGlobalAccel : Cannot connect action %1 "
				   "which is not in the object dictionary").arg(action);
		warning( str );
		return;
	}
	
	pEntry->receiver = receiver;
	pEntry->member = member;
	pEntry->aAccelId = aAvailableId;
	aAvailableId++;
	
	setItemEnabled( action, activate );

}

uint KGlobalAccel::count() const
{
	return aKeyDict.count();
}

uint KGlobalAccel::currentKey( const QString& action )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
		return 0;
	else
		return pEntry->aCurrentKeyCode;
}

uint KGlobalAccel::defaultKey( const QString& action )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
        return 0;
    else
        return pEntry->aDefaultKeyCode;
}

void KGlobalAccel::disconnectItem( const QString& action,
				   const QObject* /*receiver*/, const QString& /*member*/ )
{
    KKeyEntry *pEntry = aKeyDict[ action ];
    if ( !pEntry )
		return;
	
}

const QString KGlobalAccel::findKey( int key ) const
{
	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();
#define pE aKeyIt.current()
	while ( pE ) {
		if ( (unsigned int)key == pE->aCurrentKeyCode ) return aKeyIt.currentKey();
		++aKeyIt;
	}
#undef pE
	return QString::null;	
}

bool grabFailed;

static int XGrabErrorHandler( Display *, XErrorEvent *e ) {
	if ( e->error_code != BadAccess ) {
		warning( "grabKey: got X error %d instead of BadAccess", e->type );
	}
	grabFailed = true;
	return 0;
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

bool KGlobalAccel::insertItem(  const QString& descr, const QString& action, uint keyCode,
					   bool configurable )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( pEntry )
		removeItem( action );

	pEntry = new KKeyEntry;
	aKeyDict.insert( action, pEntry );
	
	pEntry->aDefaultKeyCode = keyCode;
	pEntry->aCurrentKeyCode = keyCode;
	pEntry->bConfigurable = configurable;
	pEntry->bEnabled = false;
	pEntry->aAccelId = 0;
	pEntry->receiver = 0;
	pEntry->member = QString::null;
	pEntry->descr = descr;

	return TRUE;
}

bool KGlobalAccel::insertItem( const QString& descr, const QString& action,
					   const QString& keyCode, bool configurable )
{
	uint iKeyCode = stringToKey( keyCode );
	return insertItem(descr, action, iKeyCode, configurable);
}

bool KGlobalAccel::isEnabled()
{
	return bEnabled;
}

bool KGlobalAccel::isItemEnabled( const QString& action )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
        return false;
    else
        return pEntry->bEnabled;
}

QDict<KKeyEntry> KGlobalAccel::keyDict()
{
	return aKeyDict;
}

void KGlobalAccel::readSettings()
{
	QString s;
	//KConfig *pConfig = kapp->getConfig();
	KConfig globalConfig;// this way we are certain to get the global stuff!
	KConfig *pConfig = &globalConfig;

	pConfig->setGroup( aGroup.data() );

	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();
#define pE aKeyIt.current()
	// first ungrab
	while ( pE ) {
		s = pConfig->readEntry( aKeyIt.currentKey() );
		if ( pE->bEnabled ) {
			uint keysym = keyToXSym( pE->aCurrentKeyCode );
			uint mod = keyToXMod( pE->aCurrentKeyCode );
			ungrabKey( keysym, mod );
		}
		
		++aKeyIt;
	}
	// then grab
	aKeyIt.toFirst();
	while ( pE ) {
		s = pConfig->readEntry( aKeyIt.currentKey() );
		if ( s.isNull() )
			pE->aConfigKeyCode = pE->aDefaultKeyCode;
		else
			pE->aConfigKeyCode = stringToKey( s.data() );
		
		pE->aCurrentKeyCode = pE->aConfigKeyCode;
		
		if ( pE->bEnabled ) {
			uint keysym = keyToXSym( pE->aCurrentKeyCode );
			uint mod = keyToXMod( pE->aCurrentKeyCode );
			grabKey( keysym, mod );
		}
		
		++aKeyIt;
	}
#undef pE
}
	
void KGlobalAccel::removeItem( const QString& action )
{
    KKeyEntry *pEntry = aKeyDict[ action ];
	
    if ( !pEntry )
		return;
	
	if ( pEntry->aAccelId ) {
	}
	
	aKeyDict.remove( action );
}

void KGlobalAccel::setConfigGroup( const QString& group )
{
	aGroup = group;
}

const QString KGlobalAccel::configGroup()
{
	return aGroup.data();
}

void KGlobalAccel::setEnabled( bool activate )
{
	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();
#define pE aKeyIt.current()
	while ( pE ) {
		setItemEnabled( aKeyIt.currentKey(), activate );
		++aKeyIt;
	}
#undef pE
	bEnabled = activate;
}

void KGlobalAccel::setItemEnabled( const QString& action, bool activate )
{	

    KKeyEntry *pEntry = aKeyDict[ action ];
	if ( !pEntry ) {
	    QString str = i18n("KGlobalAccel : cannont enable action %1 "
			       "which is not in the object dictionary").arg(action);
	    warning( str );
	    return;
	}

	bool old = pEntry->bEnabled;
	pEntry->bEnabled = activate;
	if ( pEntry->bEnabled == old ) return;

	if ( pEntry->aCurrentKeyCode == 0 ) return;
	
	uint keysym = keyToXSym( pEntry->aCurrentKeyCode );
	uint mod = keyToXMod( pEntry->aCurrentKeyCode );
	
	if ( keysym == NoSymbol ) return;

	if ( pEntry->bEnabled ) {
    		grabKey( keysym, mod );
	} else {
		ungrabKey( keysym, mod );
	}

	return;
}

bool KGlobalAccel::setKeyDict( QDict<KKeyEntry> nKeyDict )
{
	// ungrab all connected and enabled keys
	QDictIterator<KKeyEntry> *aKeyIt = new QDictIterator<KKeyEntry>( aKeyDict );
	aKeyIt->toFirst();
#define pE aKeyIt->current()
	while( pE ) {
		QString s;
		if ( pE->bEnabled ) {
			uint keysym = keyToXSym( pE->aCurrentKeyCode );
			uint mod = keyToXMod( pE->aCurrentKeyCode );
			ungrabKey( keysym, mod );
		}
		++*aKeyIt;
	}
#undef pE
	
	// Clear the dictionary
	aKeyDict.clear();
	
	// Insert the new items into the dictionary and reconnect if neccessary
	// Note also swap config and current key codes !!!!!!
	aKeyIt = new QDictIterator<KKeyEntry>( nKeyDict );
	aKeyIt->toFirst();
#define pE aKeyIt->current()
	KKeyEntry *pEntry;
	while( pE ) {
		pEntry = new KKeyEntry;
		aKeyDict.insert( aKeyIt->currentKey(), pEntry );

		pEntry->aDefaultKeyCode = pE->aDefaultKeyCode;
		// Not we write config key code to current key code !!
		pEntry->aCurrentKeyCode = pE->aConfigKeyCode;
		pEntry->aConfigKeyCode = pE->aConfigKeyCode;
		pEntry->bConfigurable = pE->bConfigurable;
		pEntry->aAccelId = pE->aAccelId;
		pEntry->receiver = pE->receiver;
		pEntry->member = pE->member;
		pEntry->descr = pE->descr; // tanghus
		pEntry->bEnabled = pE->bEnabled;
		
		if ( pEntry->bEnabled ) {
			uint keysym = keyToXSym( pEntry->aCurrentKeyCode );
			uint mod = keyToXMod( pEntry->aCurrentKeyCode );
			grabKey( keysym, mod );
		}
		
		++*aKeyIt;
	}
#undef pE
	delete aKeyIt;
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

void KGlobalAccel::writeSettings()
{
	// KConfig *pConfig = kapp->getConfig();
	KConfig globalConfig;// this way we are certain to get the global stuff!
	KConfig *pConfig = &globalConfig;

	pConfig->setGroup( aGroup.data() );

	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();
	while ( aKeyIt.current() ) {
	  if ( aKeyIt.current()->bConfigurable ){
		  pConfig->writeEntry( aKeyIt.currentKey(),
				       QString(keyToString( aKeyIt.current()->aCurrentKeyCode)),
				       true, true);
	  }
		++aKeyIt;
	}
	pConfig->sync();
}

bool KGlobalAccel::x11EventFilter( const XEvent *event_ ) {

	if ( aKeyDict.isEmpty() ) return false;
	if ( event_->type != KeyPress ) return false;
	
	uint mod=event_->xkey.state & (ControlMask | ShiftMask | Mod1Mask);
	uint keysym= XKeycodeToKeysym(qt_xdisplay(), event_->xkey.keycode, 0);
	

	QDictIterator<KKeyEntry> *aKeyIt = new QDictIterator<KKeyEntry>( aKeyDict );
	aKeyIt->toFirst();
#define pE aKeyIt->current()
	while( pE ) { 
		int kc = pE->aCurrentKeyCode;
		if ( mod == keyToXMod( kc ) && keysym == keyToXSym( kc ) ) {
		break;
		}
		++*aKeyIt;
	}
	
	if ( !pE ) {
		return false;
	}
	
	if ( !pE ) {
		return false;
	}


	XAllowEvents(qt_xdisplay(), AsyncKeyboard, CurrentTime);
	XUngrabKeyboard(qt_xdisplay(), CurrentTime);
	XSync(qt_xdisplay(), false);
	connect( this, SIGNAL( activated() ), pE->receiver, pE->member);
	emit activated();
	disconnect( this, SIGNAL( activated() ), pE->receiver, pE->member );

	return true;
}

/*****************************************************************************/

uint keyToXMod( uint keyCode )
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

uint keyToXSym( uint keyCode )
{
	char *toks[4], *next_tok;
	int nb_toks = 0;

	uint keysym = 0;
	QString s = keyToString( keyCode);
	
	if ( s.isEmpty() ) return keysym;

	char *buffer = strdup( s.ascii() );
	next_tok = strtok( buffer, "+" );
	delete [] buffer;

	if ( next_tok == 0L ) return 0;
	
	do {
		toks[nb_toks] = next_tok;
		nb_toks++;
		if ( nb_toks == 5 ) return 0;
		next_tok = strtok( 0L, "+" );
	} while ( next_tok != 0L );

	// Test for exactly one key (other tokens are accelerators)
	// Fill the keycode with infos
	bool  keyFound = FALSE;
	for ( int i=0; i<nb_toks; i++ ) {
		if ( strcmp( toks[i], "SHIFT" ) != 0 &&
			 strcmp( toks[i], "CTRL" ) != 0 &&
			 strcmp( toks[i], "ALT" ) != 0 ) {
		   if ( keyFound ) return 0;
		   keyFound = TRUE;
		   QString l = toks[i];
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

