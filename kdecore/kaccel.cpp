/*
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@via.ecp.fr>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>

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

#include <qkeycode.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qapp.h>
#include <qdrawutl.h>
#include <qmsgbox.h>

#include <kapp.h>

#include "kaccel.h"

KAccel::KAccel( QWidget * parent, const char * name ): 
  aKeyDict(100){
	aAvailableId = 1;
	bEnabled = true;
	aGroup = "Keys";
	pAccel = new QAccel( parent, name );
}

KAccel::~KAccel()
{
  delete pAccel;
}

void KAccel::clear()
{
	pAccel->clear();
	aKeyDict.clear();
}

void KAccel::connectItem( const char * action,
			  const QObject* receiver, const char* member,
			  bool activate )
{
    KKeyEntry *pEntry = aKeyDict[ action ];

	if ( !pEntry ) {
		QString str;
		str.sprintf( "KAccel : Cannot connect action %s ", action );
		str.append( "which is not in the object dictionary" );
		warning(str);
		return;
	}
	
	pEntry->receiver = receiver;
	pEntry->member = member;
	pEntry->aAccelId = aAvailableId;
	aAvailableId++;
	
	pAccel->insertItem( pEntry->aCurrentKeyCode, pEntry->aAccelId );
	pAccel->connectItem( pEntry->aAccelId, receiver, member );
	
	if ( !activate )
		setItemEnabled( action, FALSE );
}

uint KAccel::count() const
{
	return aKeyDict.count();
}

uint KAccel::currentKey( const char * action )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
		return 0;
	else
		return pEntry->aCurrentKeyCode;
}

uint KAccel::defaultKey( const char * action )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
        return 0;
    else
        return pEntry->aDefaultKeyCode;
}

void  KAccel::disconnectItem( const char * action,
			      const QObject* receiver, const char* member )
{
    KKeyEntry *pEntry = aKeyDict[ action ];
    if ( !pEntry ) 
		return;
	
	pAccel->disconnectItem( pEntry->aAccelId, receiver, member  );
}

const char * KAccel::findKey( int key ) const
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

bool KAccel::insertItem( const char* descr, const char * action, uint keyCode,
			 bool configurable )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( pEntry )
		removeItem( action );

	pEntry = new KKeyEntry;
	aKeyDict.insert( action, pEntry );
	
	pEntry->aDefaultKeyCode = keyCode;
	pEntry->aCurrentKeyCode = keyCode;
	pEntry->aConfigKeyCode = keyCode;
	pEntry->bConfigurable = configurable;
	pEntry->aAccelId = 0;
	pEntry->receiver = 0;
	pEntry->member = 0;
	pEntry->descr = descr;
	
	return TRUE;
}

bool KAccel::insertItem( const char* descr, const char * action, 
					   const char * keyCode, bool configurable )
{
	uint iKeyCode = stringToKey( keyCode );
// 	if ( iKeyCode == 0 ) {
// 		QString str;
// 		str.sprintf(
// 			"KAccel : cannot insert item with invalid key string %s", keyCode );
// 		warning(str);
// 		return FALSE;
// 	}
	
	return insertItem( descr, action, iKeyCode, configurable );
}

const char * KAccel::insertStdItem( StdAccel id )
{
	const char* action=0, *key=0, *name = 0;
	switch( id ) {
		case Open:
			name=i18n("Open") ;
			key = "CTRL+O";
			break;
		case New:
			name=i18n("New") ;
			key = "CTRL+N";
			break;
		case Close:
			name=i18n("Close") ;
			key = "CTRL+W";
			break;
		case Save:
			name=i18n("Save") ;
			key = "CTRL+S";
			break;
		case Print:
			name=i18n("Print") ;
			key = "CTRL+P";
			break;
		case Quit:
			name=i18n("Quit") ;
			key = "CTRL+Q";
			break;
		case Cut:
			name=i18n("Cut") ;
			key = "CTRL+X";
			break;
		case Copy:
			name=i18n("Copy") ;
			key = "CTRL+C";
			break;
		case Paste:
			name=i18n("Paste") ;
			key = "CTRL+V";
			break;
		case Undo:
			name=i18n("Undo") ;
			key = "CTRL+Z";
			break;
		case Find:
			name=i18n("Find") ;
			key = "CTRL+F";
			break;
		case Replace:
			name=i18n("Replace") ;
			key = "CTRL+R";
			break;
		case Insert:
			name=i18n("Insert") ;
			key = "CTRL+Insert";
			break;
		case Home:
			name=i18n("Home") ;
			key = "CTRL+Home";
			break;
		case End:
			name=i18n("End") ;
			key = "CTRL+End";
			break;
		case Prior:
			name=i18n("Prior") ;
			key = "Prior";
			break;
		case Next:
			name=i18n("Next") ;
			key = "Next";
			break;
		case Help:
			name=i18n("Help") ;
			key = "F1";
			break;
		default:
			return 0;
			break;
	}
	insertItem( name, stdAction(id), key, false );
	return action;
}

bool KAccel::isEnabled()
{
	return bEnabled;
}

bool KAccel::isItemEnabled( const char *action )
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
        return false;
    else
        return pEntry->bEnabled;
}

QDict<KKeyEntry> KAccel::keyDict()
{
	return aKeyDict;
}

void KAccel::readSettings(KConfig* config)
{
	QString s;

	KConfig *pConfig = config?config:kapp->getConfig();
	pConfig->setGroup( aGroup.data() );
	
	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();
#define pE aKeyIt.current()
	while ( pE ) {
		s = pConfig->readEntry( aKeyIt.currentKey() );
		
		if ( s.isNull() )
			pE->aConfigKeyCode = pE->aDefaultKeyCode;
		else
			pE->aConfigKeyCode = stringToKey( s.data() );
	
		pE->aCurrentKeyCode = pE->aConfigKeyCode;
		if ( pE->aAccelId && pE->aCurrentKeyCode ) {
			pAccel->disconnectItem( pE->aAccelId, pE->receiver, 
						pE->member );
			pAccel->removeItem( pE->aAccelId );
			pAccel->insertItem( pE->aCurrentKeyCode, pE->aAccelId );
			pAccel->connectItem( pE->aAccelId, pE->receiver, 
					     pE->member);
		}
		++aKeyIt;
	}
#undef pE
}

void KAccel::removeItem( const char * action )
{

    KKeyEntry *pEntry = aKeyDict[ action ];
	
    if ( !pEntry ) 
		return;
	
	if ( pEntry->aAccelId ) {
		pAccel->disconnectItem( pEntry->aAccelId, pEntry->receiver, 
					pEntry->member);
		pAccel->removeItem( pEntry->aAccelId );
	}
	
	aKeyDict.remove( action );
}

void KAccel::setEnabled( bool activate )
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

void KAccel::setItemEnabled( const char * action, bool activate )
{	
    KKeyEntry *pEntry = aKeyDict[ action ];
	if ( !pEntry ) {
		QString str;
		str.sprintf( 
			"KAccel : cannont enable action %s"\
			"which is not in the object dictionary", action );
		warning( str );
		return;
	}

	pAccel->setItemEnabled( pEntry->aAccelId, activate );
}

bool KAccel::setKeyDict( QDict<KKeyEntry> nKeyDict )
{

	debug("Disconenct and remove");
	// Disconnect and remove all items in pAccel
	QDictIterator<KKeyEntry> *aKeyIt = new QDictIterator<KKeyEntry>( aKeyDict );
	aKeyIt->toFirst();
#define pE aKeyIt->current()
	while( pE ) {
		QString s;
		if ( pE->aAccelId && pE->aCurrentKeyCode ) {
			pAccel->disconnectItem( pE->aAccelId, pE->receiver, 
						pE->member );
			pAccel->removeItem( pE->aAccelId );
		}
		++*aKeyIt;
	}
#undef pE
	
	debug("Clear the dictionary");
	
	// Clear the dictionary
	aKeyDict.clear();
	
	debug("Insert new items");
	
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
		// Note we write config key code to current key code !!
		pEntry->aCurrentKeyCode = pE->aConfigKeyCode;
		pEntry->aConfigKeyCode = pE->aConfigKeyCode;
		pEntry->bConfigurable = pE->bConfigurable;
		pEntry->aAccelId = pE->aAccelId;
		pEntry->receiver = pE->receiver;
		pEntry->member = pE->member;
		
		if ( pEntry->aAccelId && pEntry->aCurrentKeyCode ) {
			pAccel->insertItem( pEntry->aCurrentKeyCode, pEntry->aAccelId );
			pAccel->connectItem( pEntry->aAccelId, pEntry->receiver, 
					     pEntry->member);
		}
		++*aKeyIt;
	}
#undef pE
		
	return true;
}

const char * KAccel::stdAction( StdAccel id ) {
	const char* action = 0;
	switch( id ) {
		case Open:
			action = "Open";
			break;
		case New:
			action = "New";
			break;
		case Close:
			action = "Close";
			break;
		case Save:
			action = "Save";
			break;
		case Print:
			action = "Print";
			break;
		case Quit:
			action = "Quit";
			break;
		case Cut:
			action = "Cut";
			break;
		case Copy:
			action = "Copy";
			break;
		case Paste:
			action = "Paste";
			break;
		case Undo:
			action = "Undo";
			break;
		case Find:
			action = "Find";
			break;
		case Replace:
			action = "Replace";
			break;
		case Insert:
			action = "Insert";
			break;
		case Home:
			action = "Home";
			break;
		case End:
			action = "End";
			break;
		case Prior:
			action = "Prior";
			break;
		case Next:
			action = "Next";
			break;
		case Help:
			action = "Help";
			break;
		default:
			return 0;
			break;
	}
	return action;
}

void KAccel::setConfigGroup( const char *group )
{
	aGroup = group;
}

void KAccel::setConfigGlobal( bool global )
{
	bGlobal = global;
}

const char *KAccel::configGroup()
{
	return aGroup.data();
}

bool KAccel::configGlobal()
{
	return bGlobal;
}

void KAccel::writeSettings(KConfig* config)
{
	KConfig *pConfig = config?config:kapp->getConfig();
	pConfig->setGroup( aGroup.data() );

	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();
	while ( aKeyIt.current() ) {
		if ( aKeyIt.current()->bConfigurable ) {
			if ( bGlobal )
				pConfig->writeEntry( aKeyIt.currentKey(),
					keyToString( aKeyIt.current()->aCurrentKeyCode),
					true, true );
			 else
				pConfig->writeEntry( aKeyIt.currentKey(),
					keyToString( aKeyIt.current()->aCurrentKeyCode ));
		}
		++aKeyIt;
	}
	pConfig->sync();
}

/*****************************************************************************/

const QString keyToString( uint keyCode, bool i18_n )
{
	QString res;
	
	if ( keyCode == 0 ) {
		res.sprintf( "" );
		return res;
	}
	if (!i18_n){
	  if ( keyCode & SHIFT ){
	    res += ("SHIFT"); 
	    res += "+";
	  }
	  if ( keyCode & CTRL ){
	    res +=("CTRL"); 
	    res += "+";
	  }
	  if ( keyCode & ALT ){
	    res +=("ALT");
	    res += "+";
	  }
	}
	else {
	  if ( keyCode & SHIFT ){
	    res = i18n("SHIFT"); 
	    res += "+";
	  }
	  if ( keyCode & CTRL ){
	    res += i18n("CTRL"); 
	    res += "+";
	  }
	  if ( keyCode & ALT ){
	    res += i18n("ALT");
	    res += "+";
	  }
	}

	uint kCode = keyCode & ~(SHIFT | CTRL | ALT);

	for (int i=0; i<NB_KEYS; i++) {
		if ( kCode == (uint)KKeys[i].code ) {
			res += KKeys[i].name;
			return res;
		}
	}
	
	return 0;
}

uint stringToKey(const char * key )
{
	char *toks[4], *next_tok;
	uint keyCode = 0;
	int j, nb_toks = 0;
	char sKey[200];
	
	//printf("string to key %s\n", key);
	if ( key == 0 ) { debug("stringToKey::Null key");return 0; }
	if( strcmp( key, "" ) == -1 ) { debug("stringToKey::Empty key");return 0; }
	
	strncpy(sKey, (const char *)key, 200);
	next_tok = strtok(sKey,"+");
	
	if ( next_tok==NULL ) return 0;
	
	do {
		toks[nb_toks] = next_tok;
		nb_toks++;
		if ( nb_toks==5 ) return 0;
		next_tok = strtok(NULL, "+");
	} while ( next_tok!=NULL );
	
	/* we test if there is one and only one key (the others tokens
	   are accelerators) ; we also fill the keycode with infos */
	bool  keyFound = FALSE;
	for (int i=0; i<nb_toks; i++) {
		if ( strcmp(toks[i], "SHIFT")==0 )
		  keyCode |= SHIFT;
		else if ( strcmp(toks[i], "CTRL")==0 )
		  keyCode |= CTRL;
		else if ( strcmp(toks[i], "ALT")==0 )
		  keyCode |= ALT;
		else if ( strcmp(toks[i], "Umschalt")==0 )
		  keyCode |= SHIFT;
		else if ( strcmp(toks[i], "Strg")==0 )
		  keyCode |= CTRL;
		else if ( strcmp(toks[i], "Alt")==0 )
		  keyCode |= ALT;
		else if ( strcmp(toks[i], i18n("SHIFT"))==0 )
		  keyCode |= SHIFT;
		else if ( strcmp(toks[i], i18n("CTRL"))==0 )
		  keyCode |= CTRL;
		else if ( strcmp(toks[i], i18n("ALT"))==0 )
		  keyCode |= ALT;
	    else {
			/* key already found ? */
			if ( keyFound ) return 0;
			keyFound = TRUE;
			
			/* search key */
			for(j=0; j<NB_KEYS; j++) {
				if ( strcmp(toks[i], KKeys[j].name)==0 ) {
				    keyCode |= KKeys[j].code;
					break;
				}
			}
			
			/* key name ot found ? */
			if ( j==NB_KEYS ) return 0;
		}
	}
	
	return keyCode;
}
