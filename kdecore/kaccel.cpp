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


void KKeyEntry::operator=(const KKeyEntry& e) {
    aCurrentKeyCode = e.aCurrentKeyCode;
    aDefaultKeyCode = e.aDefaultKeyCode;
    aConfigKeyCode = e.aConfigKeyCode;
    bConfigurable = e.bConfigurable;
    bEnabled = e.bEnabled;
    aAccelId = e.aAccelId;
    receiver = e.receiver;
    member = e.member;
    descr = e.descr;
    menuId = e.menuId;
    menu = e.menu;
}

KKeyEntry::KKeyEntry() {
    aCurrentKeyCode = 0;
    aDefaultKeyCode = 0;
    aConfigKeyCode = 0;
    bConfigurable = false;
    bEnabled = false;
    aAccelId = 0;
    receiver = 0;
    member = 0;
    menuId = 0;
    menu = 0;
}

KKeyEntry::KKeyEntry(const KKeyEntry& e) {
    *this = e;
}

KAccel::KAccel( QWidget * parent, const char *name )
    : QAccel(parent, name)
{
    aAvailableId = 1;
    bEnabled = true;
    aGroup = "Keys";
    bGlobal = false;
}

void KAccel::clear()
{
    QAccel::clear();
    aKeyMap.clear();
}

void KAccel::connectItem( const QString& action,
			  const QObject* receiver, const char *member,
			  bool activate )
{
    if (action.isNull()) return;
    if (!aKeyMap.contains(action)) {
        kdWarning(125) << "cannot connect action " << action
                       << " which is not in the object dictionary" << endl;
        return;
    }

    KKeyEntry entry = aKeyMap[action];
    entry.receiver = receiver;
    entry.member = member;
    entry.aAccelId = aAvailableId;
    aKeyMap[action] = entry; // reassign
    aAvailableId++;

    // Qt does strange things if a QAccel contains a accelerator
    // with key code 0, so leave it out here.
    if (entry.aCurrentKeyCode) {
        QAccel::insertItem( entry.aCurrentKeyCode, entry.aAccelId );
        QAccel::connectItem( entry.aAccelId, receiver, member );
    }

    if ( !activate ) setItemEnabled( action, false );
}

void KAccel::connectItem( KStdAccel::StdAccel accel,
			  const QObject* receiver, const char *member,
			  bool activate )
{
	QString action(KStdAccel::action(accel));
	if (!action.isNull() && !aKeyMap.contains(action))
		insertStdItem(accel);

	connectItem(action, receiver, member, activate);
}

uint KAccel::count() const
{
	return aKeyMap.count();
}

int KAccel::currentKey( const QString& action ) const
{
    return aKeyMap[action].aCurrentKeyCode;
}

void KAccel::setDescription(const QString &action,
                            const QString &description)
{
    aKeyMap[action].descr = description;
}

QString KAccel::description( const QString& action ) const
{
    return aKeyMap[action].descr;
}

int KAccel::defaultKey( const QString& action ) const
{
    return aKeyMap[action].aDefaultKeyCode;
}

void KAccel::disconnectItem( const QString& action,
			      const QObject* receiver, const char *member )
{
    if (aKeyMap.contains(action))
        QAccel::disconnectItem( aKeyMap[action].aAccelId, receiver, member );
}

QString KAccel::findKey( int key ) const
{
    for (KKeyEntryMap::ConstIterator it = aKeyMap.begin();
         it != aKeyMap.end(); ++it)
        if ( key == (*it).aCurrentKeyCode )
            return it.key();

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
    if (aKeyMap.contains(action))
        removeItem( action );

    KKeyEntry entry;

    entry.aDefaultKeyCode = keyCode;
    entry.aCurrentKeyCode = keyCode;
    entry.aConfigKeyCode = keyCode;
    entry.bConfigurable = configurable;
    entry.descr = descr;
    entry.menuId = id;
    entry.menu = qmenu;
    entry.bEnabled = true;

    aKeyMap[action] = entry;

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
    return aKeyMap[action].bEnabled;
}

KKeyEntryMap KAccel::keyDict() const
{
	return aKeyMap;
}

void KAccel::readSettings( KConfig* config )
{
    readKeyMap( aKeyMap, aGroup, config );

    for (KKeyEntryMap::Iterator it = aKeyMap.begin();
         it != aKeyMap.end(); ++it) {

        if ( (*it).aAccelId && (*it).aCurrentKeyCode ) {
            kdDebug(125) << "insert " << (*it).descr << " " << (*it).bEnabled << endl;
            QAccel::disconnectItem( (*it).aAccelId, (*it).receiver,
                                           (*it).member );
            QAccel::removeItem( (*it).aAccelId );
            QAccel::insertItem( (*it).aCurrentKeyCode, (*it).aAccelId );
            QAccel::connectItem( (*it).aAccelId, (*it).receiver,
                                        (*it).member);
        }
        if ( (*it).menu )
            changeMenuAccel((*it).menu, (*it).menuId, it.key());
    }

    emit keycodeChanged();

}

void KAccel::readKeyMap( KKeyEntryMap &map, const QString& group, KConfigBase *config )
{
    QString s;

    KConfigBase *pConfig = config ? config : KGlobal::config();
    KConfigGroupSaver cgs(pConfig, group);

    for (KKeyEntryMap::Iterator it = map.begin();
         it != map.end(); ++it) {
        s = pConfig->readEntry(it.key());

        if ( s.isNull() || s.startsWith( "default" ))
            (*it).aConfigKeyCode = (*it).aDefaultKeyCode;
        else
            (*it).aConfigKeyCode = stringToKey( s );

        (*it).aCurrentKeyCode = (*it).aConfigKeyCode;
    }
}

void KAccel::removeItem( const QString& action )
{
    if (!aKeyMap.contains(action))
        return;
    KKeyEntry entry = aKeyMap[ action ];

    if ( entry.aAccelId ) {
        QAccel::disconnectItem( entry.aAccelId, entry.receiver,
                                entry.member);
        QAccel::removeItem( entry.aAccelId );
    }

    aKeyMap.remove( action );
}

void KAccel::setEnabled( bool activate )
{
    kdDebug(125) << "setEnabled " << activate << endl;
    for (KKeyEntryMap::ConstIterator it = aKeyMap.begin();
         it != aKeyMap.end(); ++it)
        setItemEnabled( it.key(), activate );

    bEnabled = activate;
}

void KAccel::setItemEnabled( const QString& action, bool activate )
{
    QAccel::setItemEnabled( aKeyMap[action].aAccelId, activate );
    aKeyMap[action].bEnabled = activate;
}

bool KAccel::setKeyDict( const KKeyEntryMap& nKeyDict )
{

	kdDebug(125) << "Disconnect and remove" << endl;
	// Disconnect and remove all items in pAccel
        for (KKeyEntryMap::ConstIterator it = aKeyMap.begin();
             it != aKeyMap.end(); ++it) {
            QString s;
            if ( (*it).aAccelId && (*it).aCurrentKeyCode ) {
                QAccel::disconnectItem( (*it).aAccelId, (*it).receiver,
                                        (*it).member );
                QAccel::removeItem( (*it).aAccelId );
            }
	}

	// Clear the dictionary
	aKeyMap = nKeyDict;

	kdDebug(125) << "Insert new items" << endl;

	// Insert the new items into the dictionary and reconnect if neccessary
	// Note also swap config and current key codes !!!!!!
        for (KKeyEntryMap::Iterator it = aKeyMap.begin();
             it != aKeyMap.end(); ++it)
        {
            // Note we write config key code to current key code !!
            (*it).aCurrentKeyCode = (*it).aConfigKeyCode;

            if ( (*it).aAccelId && (*it).aCurrentKeyCode ) {
                kdDebug(125) << "insert " << (*it).descr << " " << (*it).bEnabled << endl;
                QAccel::insertItem( (*it).aCurrentKeyCode, (*it).aAccelId );
                QAccel::setItemEnabled( (*it).aAccelId, (*it).bEnabled );
                QAccel::connectItem( (*it).aAccelId, (*it).receiver,
                                     (*it).member);
            }
            if ( (*it).menu ) {
                changeMenuAccel((*it).menu, (*it).menuId,
                                it.key());
            }
	}

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

void KAccel::writeKeyMap( const KKeyEntryMap &map, const QString &group, KConfig *config,
    bool global )
{
    KConfig *pConfig = config ? config : KGlobal::config();
    KConfigGroupSaver cs(pConfig, group);

    for (KKeyEntryMap::ConstIterator it = map.begin();
         it != map.end(); ++it)
    {
        if ( (*it).bConfigurable )
        {
            kdDebug(125) << "writing " <<  KAccel::keyToString( (*it).aCurrentKeyCode, false) << " " <<
                KAccel::keyToString( (*it).aConfigKeyCode, false) << endl;

            if ( ( *it ).aConfigKeyCode != ( *it ).aDefaultKeyCode )
            {
                pConfig->writeEntry( it.key(),
                                     KAccel::keyToString( (*it).aConfigKeyCode, false),
                                     true, global );
            }
            else
            { // global ones must be written
            if ( global || !pConfig->readEntry( it.key() ).isNull() )
                pConfig->writeEntry( it.key(), QString( "default(%1)" )
                    .arg( KAccel::keyToString( (*it).aDefaultKeyCode, false )), true, global );
            }
        }
    }
    pConfig->sync();
}

void KAccel::writeSettings(KConfig* config) const
{
    writeKeyMap( aKeyMap, aGroup, config, bGlobal );
}

bool KAccel::configurable( const QString &action ) const
{
    return aKeyMap[action].bConfigurable;
}

void KAccel::clearItem(const QString &action)
{
    if (!aKeyMap.contains(action))
        return;

    KKeyEntry entry = aKeyMap[ action ];
    if ( entry.aAccelId  && entry.bConfigurable) {
        QAccel::disconnectItem( entry.aAccelId, entry.receiver,
				entry.member);
        QAccel::removeItem( entry.aAccelId );
        entry.aAccelId = 0;
        entry.aCurrentKeyCode = 0;
        aKeyMap[action] = entry; // reassign
        if ( entry.menu ) {
            changeMenuAccel(entry.menu, entry.menuId, action);
        }
    }
}

bool KAccel::updateItem( const QString &action, int keyCode)
{
    if (!aKeyMap.contains(action))
        return false;
    KKeyEntry entry = aKeyMap[ action ];
    if ( entry.aCurrentKeyCode==keyCode )
        return true;
    if ( entry.aAccelId ) {
        QAccel::disconnectItem( entry.aAccelId, entry.receiver,
                                entry.member);
        QAccel::removeItem( entry.aAccelId );
    } else {
        entry.aAccelId = aKeyMap[action].aAccelId = aAvailableId;
        aAvailableId++;
    }

    aKeyMap[action].aCurrentKeyCode = keyCode;
    if (keyCode) {
        QAccel::insertItem( keyCode, entry.aAccelId );
        QAccel::connectItem( entry.aAccelId, entry.receiver, entry.member );
    }
    emit keycodeChanged();
    return true;

}

void KAccel::removeDeletedMenu(QPopupMenu *menu)
{
    for (KKeyEntryMap::Iterator it = aKeyMap.begin();
         it != aKeyMap.end(); ++it)
        if ( (*it).menu == menu )
            (*it).menu = 0;
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
	    if (i18_n)
	      res += i18n("QAccel", KKEYS[i].name);
	    else
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
        if( key == "default" )  // old code used to write just "default"
            return 0;                           //  which is not enough
        if( key.startsWith( "default(" )) {
            int pos = key.findRev( ')' );
            if( pos >= 0 ) // this should be really done with regexp
                return stringToKey( key.mid( 8, pos - 8 ));
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
