/*
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997-2000 Nicolas Hadacek <hadacek@kde.org>
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
#include <qapplication.h>
#include <qdrawutil.h>
#include <qpopupmenu.h>

#include <kaccel.h>
#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kckey.h>

KAccel::KAccel( QWidget * parent, const char *name )
: QAccel(parent, name), aKeyDict(100, false){
	aAvailableId = 1;
	bEnabled = true;
	aGroup = "Keys";
	bGlobal = false;
}

void KAccel::clear()
{
	QAccel::clear();
	aKeyDict.clear();
}

void KAccel::connectItem( const QString& action,
			  const QObject* receiver, const char *member,
			  bool activate )
{
	if (action.isNull()) return;
    KKeyEntry *pEntry = aKeyDict[ action ];

	if ( !pEntry ) {
		kdWarning(125) << "cannot connect action " << action
					   << " which is not in the object dictionary" << endl;
	    return;
	}
	
	pEntry->receiver = receiver;
	pEntry->member = member;
	pEntry->aAccelId = aAvailableId;
	aAvailableId++;
	
	// Qt does strange things if a QAccel contains a accelerator
	// with key code 0, so leave it out here.
	if (pEntry->aCurrentKeyCode) {
		QAccel::insertItem( pEntry->aCurrentKeyCode, pEntry->aAccelId );
		QAccel::connectItem( pEntry->aAccelId, receiver, member );
	}
	
	if ( !activate ) setItemEnabled( action, false );
}

void KAccel::connectItem( KStdAccel::StdAccel accel,
			  const QObject* receiver, const char *member,
			  bool activate )
{
	QString action(KStdAccel::action(accel));
	if (!action.isNull() && !aKeyDict[ action ]){
		insertStdItem(accel);
	}
	connectItem(action, receiver, member, activate);
}

uint KAccel::count() const
{
	return aKeyDict.count();
}

int KAccel::currentKey( const QString& action ) const
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
		return 0;
	else
		return pEntry->aCurrentKeyCode;
}

void KAccel::setDescription(const QString &action,
							const QString &description)
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
		kdWarning(125) << "cannot set description for absent action "
					   << action << endl;
	else pEntry->descr = description;
}

QString KAccel::description( const QString& action ) const
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
		return QString::null;
	else
		return pEntry->descr;
}

int KAccel::defaultKey( const QString& action ) const
{
	KKeyEntry *pEntry = aKeyDict[ action ];
	
	if ( !pEntry )
        return 0;
    else
        return pEntry->aDefaultKeyCode;
}

void KAccel::disconnectItem( const QString& action,
			      const QObject* receiver, const char *member )
{
    KKeyEntry *pEntry = aKeyDict[ action ];
    if ( !pEntry )
		return;
	
	QAccel::disconnectItem( pEntry->aAccelId, receiver, member );
}

QString KAccel::findKey( int key ) const
{
	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();
	while ( aKeyIt.current() ) {
	    if ( (unsigned int)key == aKeyIt.current()->aCurrentKeyCode )
		return aKeyIt.currentKey();
	    ++aKeyIt;
	}
	return QString::null;
}

bool KAccel::insertItem( const QString& descr, const QString& action, int keyCode,
			 bool configurable )
{
	return insertItem( descr, action,  keyCode,
			 0, 0, configurable);
}

bool KAccel::insertItem( const QString& descr, const QString& action, int keyCode,
			 int id, QPopupMenu *qmenu, bool configurable)
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
	pEntry->menuId = id;
	pEntry->menu = qmenu;

	return true;
}

bool KAccel::insertItem( const QString& descr, const QString& action,
			 const QString& keyCode, bool configurable )
{
	int iKeyCode = stringToKey( keyCode );
	return insertItem( descr, action, iKeyCode, configurable );
}

bool KAccel::insertItem( const QString& descr, const QString& action,
			 const QString& keyCode, int id,
			 QPopupMenu *qmenu, bool configurable)
{
	int iKeyCode = stringToKey( keyCode );
	return insertItem( descr, action, iKeyCode, id, qmenu, configurable);
}

bool KAccel::insertItem( const QString& action, int keyCode,
			 bool configurable )
{
    return insertItem(action, action, keyCode, configurable);
}

bool KAccel::insertItem( const QString& action, int keyCode,
			 int id, QPopupMenu *qmenu, bool configurable)
{
    return insertItem(action, action, keyCode, id, qmenu, configurable);
}

void KAccel::changeMenuAccel ( QPopupMenu *menu, int id,
	const QString& action )
{
	QString s = menu->text( id );
	if ( s.isNull() ) return;
	if (action.isNull()) return;
	
	int i = s.find('\t');
	
	QString k = keyToString( currentKey( action), true );
	if( k.isNull() ) return;
	
	if ( i >= 0 )
		s.replace( i+1, s.length()-i, k );
	else {
		s += '\t';
		s += k;
	}

	QPixmap *pp = menu->pixmap(id);
	if(pp && !pp->isNull())
	  menu->changeItem( *pp, s, id );
	else
	  menu->changeItem( s, id );
}

void KAccel::changeMenuAccel( QPopupMenu *menu, int id,
							 KStdAccel::StdAccel accel )
{
	changeMenuAccel(menu, id, KStdAccel::action(accel));
}

bool KAccel::insertStdItem( KStdAccel::StdAccel id, const QString& descr )
{
	return insertItem(descr.isNull() ? KStdAccel::description(id) : descr,
					  KStdAccel::action(id), KStdAccel::key(id), false );
}

bool KAccel::isEnabled() const
{
	return bEnabled;
}

bool KAccel::isItemEnabled( const QString& action ) const
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

	KConfig *pConfig = config ? config : KGlobal::config();
	pConfig->setGroup( aGroup );
	
	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();
#define pE aKeyIt.current()
	while ( pE ) {
		s = pConfig->readEntry(aKeyIt.currentKey());
		
		if ( s.isNull() )
			pE->aConfigKeyCode = pE->aDefaultKeyCode;
		else
			pE->aConfigKeyCode = stringToKey( s );
	
		pE->aCurrentKeyCode = pE->aConfigKeyCode;
		if ( pE->aAccelId && pE->aCurrentKeyCode ) {
			QAccel::disconnectItem( pE->aAccelId, pE->receiver,
						pE->member );
		    QAccel::removeItem( pE->aAccelId );
			QAccel::insertItem( pE->aCurrentKeyCode, pE->aAccelId );
		    QAccel::connectItem( pE->aAccelId, pE->receiver,
					     pE->member);
		}
		if ( pE->menu ) {
		        changeMenuAccel(pE->menu, pE->menuId, aKeyIt.currentKey());
		}
		++aKeyIt;
	}
#undef pE

	emit keycodeChanged();
}

void KAccel::removeItem( const QString& action )
{
    KKeyEntry *pEntry = aKeyDict[ action ];
	
    if ( !pEntry )
		return;
	
	if ( pEntry->aAccelId ) {
		QAccel::disconnectItem( pEntry->aAccelId, pEntry->receiver,
					pEntry->member);
		QAccel::removeItem( pEntry->aAccelId );
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

void KAccel::setItemEnabled( const QString& action, bool activate )
{	
    KKeyEntry *pEntry = aKeyDict[ action ];
	if ( !pEntry ) {
		kdWarning(125) << "cannot enable action " << action
					   << " which is not in the object dictionary" << endl;
		return;
	}

	QAccel::setItemEnabled( pEntry->aAccelId, activate );
	pEntry->bEnabled = activate;
}

bool KAccel::setKeyDict( QDict<KKeyEntry> nKeyDict )
{

	kdDebug(125) << "Disconnect and remove" << endl;
	// Disconnect and remove all items in pAccel
	QDictIterator<KKeyEntry> *aKeyIt = new QDictIterator<KKeyEntry>( aKeyDict );
	aKeyIt->toFirst();
#define pE aKeyIt->current()
	while( pE ) {
		QString s;
		if ( pE->aAccelId && pE->aCurrentKeyCode ) {
			QAccel::disconnectItem( pE->aAccelId, pE->receiver,
						pE->member );
			QAccel::removeItem( pE->aAccelId );
		}
		++*aKeyIt;
	}
#undef pE
	
	kdDebug(125) << "Clear the dictionary" << endl;
	
	// Clear the dictionary
	aKeyDict.clear();
	
	kdDebug(125) << "Insert new items" << endl;
	
	// Insert the new items into the dictionary and reconnect if neccessary
	// Note also swap config and current key codes !!!!!!
	delete aKeyIt; // tanghus
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
		pEntry->bEnabled = pE->bEnabled;
		pEntry->aAccelId = pE->aAccelId;
		pEntry->receiver = pE->receiver;
		pEntry->member = pE->member;
		pEntry->descr = pE->descr; // tanghus
		pEntry->menuId = pE->menuId;
		pEntry->menu = pE->menu;
		
		if ( pEntry->aAccelId && pEntry->aCurrentKeyCode ) {
			QAccel::insertItem( pEntry->aCurrentKeyCode, pEntry->aAccelId );
			QAccel::setItemEnabled( pEntry->aAccelId, pEntry->bEnabled );
			QAccel::connectItem( pEntry->aAccelId, pEntry->receiver,
					     pEntry->member);
		}
		if ( pEntry->menu ) {
		  changeMenuAccel(pEntry->menu, pEntry->menuId,
				  aKeyIt->currentKey());
		}
		++*aKeyIt;
	}
#undef pE
		
	delete aKeyIt; // tanghus
	emit keycodeChanged();
	return true;
}

void KAccel::setConfigGroup( const QString& group )
{
	aGroup = group;
}

void KAccel::setConfigGlobal( bool global )
{
	bGlobal = global;
}

QString KAccel::configGroup() const
{
	return aGroup;
}

bool KAccel::configGlobal() const
{
	return bGlobal;
}

void KAccel::writeSettings(KConfig* config)
{
	KConfig *pConfig = config?config:KGlobal::config();
	pConfig->setGroup( aGroup );

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

bool KAccel::configurable( const QString &action ) const
{
  KKeyEntry *pEntry = aKeyDict[ action ];

  if ( !pEntry )
    return false;
  else
    return pEntry->bConfigurable;
}

void KAccel::clearItem(const QString &action)
{
    KKeyEntry *pEntry = aKeyDict[ action ];
    if (pEntry) {
      if ( pEntry->aAccelId  && pEntry->bConfigurable) {
		  QAccel::disconnectItem( pEntry->aAccelId, pEntry->receiver,
				pEntry->member);
		  QAccel::removeItem( pEntry->aAccelId );
		  pEntry->aAccelId = 0;
		  pEntry->aCurrentKeyCode = 0;
		  if ( pEntry->menu ) {
			  changeMenuAccel(pEntry->menu, pEntry->menuId, action);
		  }
      }
    }
}

bool KAccel::updateItem( const QString &action, int keyCode)
{
  KKeyEntry *pEntry = aKeyDict[ action ];
  if ( pEntry->aCurrentKeyCode==keyCode ) return true;
  if (pEntry) {
    if ( pEntry->aAccelId ) {
		QAccel::disconnectItem( pEntry->aAccelId, pEntry->receiver,
			      pEntry->member);
		QAccel::removeItem( pEntry->aAccelId );
    } else {
      pEntry->aAccelId = aAvailableId;
      aAvailableId++;
    }

    pEntry->aCurrentKeyCode = keyCode;
    if (pEntry->aCurrentKeyCode) {
		QAccel::insertItem( pEntry->aCurrentKeyCode, pEntry->aAccelId );
		QAccel::connectItem( pEntry->aAccelId, pEntry->receiver, pEntry->member );
    }
	emit keycodeChanged();
    return true;
  } else
    return false;
}

void KAccel::removeDeletedMenu(QPopupMenu *menu)
{
	QDictIterator<KKeyEntry> aKeyIt( aKeyDict );
	aKeyIt.toFirst();

#define pE aKeyIt.current()
	while ( pE ) {
		if ( pE->menu == menu )
		  pE->menu = 0;

		++aKeyIt;
	}
#undef pE
}



/*****************************************************************************/
QString KAccel::keyToString( int keyCode, bool i18_n )
{
	QString res = "";
	
	if ( keyCode == 0 ) return res;
	if ( keyCode & Qt::SHIFT ){
		if (i18_n) res += i18n("Shift");
	    else       res += "Shift";
	    res += "+";
	}
	if ( keyCode & Qt::CTRL ){
	   if (i18_n) res += i18n("Ctrl");
	   else       res += "Ctrl";
	    res += "+";
	}
	if ( keyCode & Qt::ALT ){
		if (i18_n) res += i18n("Alt");
		else       res += "Alt";
	    res += "+";
	}

	int kCode = keyCode & ~(Qt::SHIFT | Qt::CTRL | Qt::ALT);

	for (int i=0; i<NB_KEYS; i++) {
		if ( kCode == (int)KKEYS[i].code ) {
			res += KKEYS[i].name;
			return res;
		}
	}
	
	return QString::null;
}

int KAccel::stringToKey(const QString& key)
{
        // Empty string is interpreted as code zero, which is
        // consistent with the behaviour of other KAccel methods

	if ( key.isNull() ) {
//		kdWarning(125) << "stringToKey::Null key" << endl;
		return 0;
	} else if ( key.isEmpty() ) {
//		kdWarning(125) << "stringToKey::Empty key" << endl;
		return 0;
	}

	// break the string in tokens separated by "+"
	int k = 0;
	QArray<int> tokens;
	int plus = -1;
	do {
		tokens.resize(k+1);
		tokens[k] = plus+1;
		plus = key.find('+', plus+1);
		k++;
	} while ( plus!=-1 );
	tokens.resize(k+1);
	tokens[k] = key.length() + 1;
	
	// we have k tokens.
	// find a keycode (only one)
	// the other tokens are accelerators (SHIFT, CTRL & ALT)
	// the order is unimportant
	bool codeFound = false;
	QString str;
	int keyCode = 0;
	for (int i=0; i<k; i++) {
		str = key.mid(tokens[i], tokens[i+1]-tokens[i]-1);
		str.stripWhiteSpace();
		if ( str.isEmpty() ) {
			kdWarning(125) << "stringToKey::Empty token" << endl;
			return 0;
		}

		if ( k!=1 ) { // for e.g. "Shift" can be a modifier or a key
			if ( str.upper()=="SHIFT" )     { keyCode |= Qt::SHIFT; continue; }
			if ( str.upper()=="CTRL" )      { keyCode |= Qt::CTRL;  continue; }
			if ( str.upper()=="ALT" )       { keyCode |= Qt::ALT;   continue; }
		}

		if (codeFound) {
			kdWarning(125) << "stringToKey::Duplicate keycode" << endl;
			return 0;
		}
		// search for keycode
		int j;
		for(j=0; j<NB_KEYS; j++) {
			if ( str==KKEYS[j].name ) {
				keyCode |= KKEYS[j].code;
				break;
			}
		}
		if ( j==NB_KEYS ) {
			kdWarning(125) << "stringToKey::Unknown key name " << str << endl;
			return 0;
		}
	}

	return keyCode;
}

#include "kaccel.moc"
