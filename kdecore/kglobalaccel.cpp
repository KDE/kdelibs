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
#include "kglobalaccel.moc"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>

#include <qkeycode.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qapp.h>
#include <qdrawutl.h>
#include <qmsgbox.h>

#include <kapp.h>

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

KGlobalAccel::~KGlobalAccel()
{
	setEnabled( false );
}

void KGlobalAccel::clear()
{
	setEnabled( false );
	aKeyDict.clear();
}

void KGlobalAccel::connectItem( const char * action,
				const QObject* receiver, const char* member,
				bool activate )
{
    KKeyEntry *pEntry = aKeyDict[ action ];
	if ( !pEntry ) {
		QString str;
		str.sprintf(
			"KGlobalAccel : Cannot connect action %s"\
			"which is not in the object dictionary", action );
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

uint KGlobalAccel::currentKey( const char * action )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
		return 0;
	else
		return pEntry->aCurrentKeyCode;
}

uint KGlobalAccel::defaultKey( const char * action )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
        return 0;
    else
        return pEntry->aDefaultKeyCode;
}

void KGlobalAccel::disconnectItem( const char * action,
				   const QObject* /*receiver*/, const char* /*member*/ )
{
    KKeyEntry *pEntry = aKeyDict[ action ];
    if ( !pEntry ) 
		return;
	
}

const char * KGlobalAccel::findKey( int key ) const
{
	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();
#define pE aKeyIt.current()
	while ( pE ) {
		if ( (unsigned int)key == pE->aCurrentKeyCode ) return aKeyIt.currentKey();
		++aKeyIt;
	}
#undef pE
	return 0;	
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

	debug("KGlobalAccel::grabKey");
	
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
	
	debug("Will grab key and variants with keyboard locks: %d, %d", keysym, mod);

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
	
	debug("       grabbed");
	
	if (grabFailed) {
		// FIXME: ungrab all successfull grabs!
		//warning("Global grab failed!");
   		return false;
	}
	return true;
}

bool KGlobalAccel::insertItem(  const char* descr, const char * action, uint keyCode,
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
	pEntry->member = 0;
	pEntry->descr = descr;

	return TRUE;
}

bool KGlobalAccel::insertItem( const char* descr, const char * action, 
					   const char * keyCode, bool configurable )
{
	uint iKeyCode = stringToKey( keyCode );
// 	if ( iKeyCode == 0 ) {
// 		QString str;
// 		str.sprintf(
// 			"KGlobalAccel : cannot insert item with invalid key string %s", keyCode );
// 		warning( str );
// 		return FALSE;
// 	}
	
	return insertItem(descr, action, iKeyCode, configurable);
}

bool KGlobalAccel::isEnabled()
{
	return bEnabled;
}

bool KGlobalAccel::isItemEnabled( const char *action )
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
	
void KGlobalAccel::removeItem( const char * action )
{
    KKeyEntry *pEntry = aKeyDict[ action ];
	
    if ( !pEntry ) 
		return;
	
	if ( pEntry->aAccelId ) {
	}
	
	aKeyDict.remove( action );
}

void KGlobalAccel::setConfigGroup( const char *group )
{
	aGroup = group;
}

const char *KGlobalAccel::configGroup()
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

void KGlobalAccel::setItemEnabled( const char * action, bool activate )
{	
	debug("KGlobalAccel::setItemEnabled");

    KKeyEntry *pEntry = aKeyDict[ action ];
	if ( !pEntry ) {
		QString str;
		str.sprintf( 
			"KGlobalAccel : cannont enable action %s"\
			"which is not in the object dictionary", action );
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
		pEntry->bEnabled = pE->bEnabled;
		
		if ( pEntry->bEnabled ) {
			uint keysym = keyToXSym( pEntry->aCurrentKeyCode );
			uint mod = keyToXMod( pEntry->aCurrentKeyCode );
			grabKey( keysym, mod );
		}
		
		++*aKeyIt;
	}
#undef pE
		
	return true;
}

bool KGlobalAccel::ungrabKey( uint keysym, uint mod ) {
	// Most of this comes from kpanel/main.C
	// Copyright (C) 1996,97 Matthias Ettrich
	static int NumLockMask = 0;
	
	if (do_not_grab)
	  return true;

	debug("KGlobalAccel::ungrabKey");
	
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
	
	debug("Will ungrab key and variants with keyboard locks: %d, %d", keysym, mod);

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
				       keyToString( aKeyIt.current()->aCurrentKeyCode),
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
	
	//	debug("Key press event :: mod = %d, sym =%d", mod, keysym );
   
// 	KKeyEntry *pEntry =0;
// 	const char *action;
   
	QDictIterator<KKeyEntry> *aKeyIt = new QDictIterator<KKeyEntry>( aKeyDict );
	aKeyIt->toFirst();
#define pE aKeyIt->current()
	while( pE ) { //&& 
		//( mod != keyToXMod( pE->aCurrentKeyCode ) 
		//	|| keysym != keyToXSym( pE->aCurrentKeyCode ) ) ) {
		int kc = pE->aCurrentKeyCode;
		// 		if( mod == keyToXMod( kc ) ) debug("That's the right modifier");
		// 		if( keysym == keyToXSym( kc ) ) debug("That's the right symbol");
		if ( mod == keyToXMod( kc ) && keysym == keyToXSym( kc ) ) {
		//pEntry = pE;
		//action = aKeyIt->currentKey();
		//break;
		debug("Found key in dictionary for action %s", aKeyIt->currentKey());
		break;
		}
		++*aKeyIt;
	}
	
	if ( !pE ) {
	  //		debug("Null KeyEntry object");
		return false; 
	}
	
	if ( !pE ) {
		debug("KeyEntry object not enabled");
		return false; 
	}

	debug("KGlobalAccel:: event action %s", aKeyIt->currentKey() );

	XAllowEvents(qt_xdisplay(), AsyncKeyboard, CurrentTime);
	XUngrabKeyboard(qt_xdisplay(), CurrentTime);
	XSync(qt_xdisplay(), false);
	connect( this, SIGNAL( activated() ), pE->receiver, pE->member);
	emit activated();
	disconnect( this, SIGNAL( activated() ), pE->receiver, pE->member );

//    warning("Signal has been sent!");
	return true;
}

/*****************************************************************************/

uint keyToXMod( uint keyCode )
{
	uint mod = 0;
	
	if ( keyCode == 0 ) return mod;
	
	if ( keyCode & SHIFT )
		 mod |= ShiftMask;
	if ( keyCode & CTRL )
		 mod |= ControlMask;
	if ( keyCode & ALT )
		 mod |= Mod1Mask;
		 
	return mod;
}

uint keyToXSym( uint keyCode )
{
  //	debug("keyToXSym");
	char *toks[4], *next_tok;
	int nb_toks = 0;
	char sKey[200];

	uint keysym = 0;
	QString s = keyToString( keyCode);
	//s= s.lower();
	
	strncpy(sKey, (const char *)s.data(), 200);
	
	//	debug("Find key %s", sKey ); 
	
	if ( s.isEmpty() ) return keysym;
	
	next_tok = strtok( sKey, "+" );
	
	if ( next_tok == NULL ) return 0;
	
	do {
		toks[nb_toks] = next_tok;
		nb_toks++;
		if ( nb_toks == 5 ) return 0;
		next_tok = strtok( NULL, "+" );
	} while ( next_tok != NULL );

	// Test for exactly one key (other tokens are accelerators)
	// Fill the keycode with infos
	bool  keyFound = FALSE;
	for ( int i=0; i<nb_toks; i++ ) {
	  //		debug("%d", i);
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
		   //		   debug("Key = %s, sym = %d", toks[i], keysym );
		   if ( keysym == NoSymbol ) {
		     //		   	debug("Keysym no symbol (%d)", NoSymbol);
			return 0;
		  }
		}
	}
	
	//debug("Return %d", keysym);
	return keysym;
}
