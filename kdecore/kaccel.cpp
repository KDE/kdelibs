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
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kckey.h>
#include <kkey.h>

#ifdef Q_WS_X11
#include "kkey_x11.h"
#endif
//#include <ctype.h>

//-------------------------------------------------------------------

// BCI: KAccel doesn't have a destructor, so we need to delete it's 'd' another way.
// So in this case, we'll make KAccelPrivate a child of QObject, and set it's QObject
//  parent to KAccel.
class KAccelPrivate : public QObject
{
public:
	KKeyMapOrder aKeyMapOrder;	// A list preserving the original insertItem order.

	KAccelPrivate( QObject *parent )
		: QObject( parent )
		{ };
};

//-------------------------------------------------------------------

void KKeyEntry::operator=(const KKeyEntry& e) {
    aCurrentKeyCode = e.aCurrentKeyCode;
    aDefaultKeyCode = e.aDefaultKeyCode;
    //aDefaultKeyCode4 = e.aDefaultKeyCode4;
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
    //aDefaultKeyCode4 = 0;
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
    d = new KAccelPrivate( this ); // BCI: we'll need a destructor function too.
}

KAccel::~KAccel()
{
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
	return insertItem( descr, action, keyCode, keyCode,
			 0, 0, configurable);
}

bool KAccel::insertItem( const QString& descr, const QString& action,
			 KKey defaultKeyCode3, KKey defaultKeyCode4,
			 bool configurable )
{
	//kdDebug(125) << QString( "insertItem("+action+", %1, %2)\n" ).arg(defaultKeyCode3).arg(defaultKeyCode4);
	return insertItem( descr, action, defaultKeyCode3, defaultKeyCode4,
			 0, 0, configurable);
}

bool KAccel::insertItem( const QString& descr, const QString& action,
			 KKey defaultKeyCode3, KKey defaultKeyCode4,
			 int id, QPopupMenu *qmenu, bool configurable)
{
    kdDebug(125) << QString( "insertItem("+action+", 0x%1, 0x%2)\n" ).arg(defaultKeyCode3.key(),0,16).arg(defaultKeyCode4.key(),0,16);

    if (aKeyMap.contains(action))
        removeItem( action );

    KKeyEntry entry;

#ifdef Q_WS_X11
    // If this is an application shortcut, make sure that we are really
    //  using the right modifier combination by converting to it's X equivalent
    //  and back. Neccessary for punctuation keys on varying layouts.
    if( id ) {
        uint keySymX, keyModX;
        KKeyX11::keyQtToKeyX( defaultKeyCode3.key(), 0, &keySymX, &keyModX );
        defaultKeyCode3 = KKeyX11::keySymXToKeyQt( keySymX, keyModX );
    }
#endif

    entry.aDefaultKeyCode = defaultKeyCode3.key();
    //entry.aDefaultKeyCode4 = defaultKeyCode4.key();
    entry.aCurrentKeyCode = /*entry.aConfigKeyCode = useFourModifierKeys() ? defaultKeyCode4.key() : */ defaultKeyCode3.key();
    //kdDebug(125) << "useFourModifierKeys() = " << useFourModifierKeys() << " entry.aCurrentKeyCode = " << entry.aCurrentKeyCode << endl;
    entry.bConfigurable = configurable;
    entry.descr = descr;
    entry.menuId = id;
    entry.menu = qmenu;
    entry.bEnabled = true;

    aKeyMap[action] = entry;

    // Hack to get ordering and labeling working in kcontrol -- this will be replaced as
    //  soon as the 2.2beta testing phase is over.
    d->aKeyMapOrder[d->aKeyMapOrder.count()] = action;
    // Program:XXX and Group:XXX labels should be disabled.
    if( action.contains( ':' ) )
    	entry.bEnabled = entry.bConfigurable = false;

    return true;
}

bool KAccel::insertItem( const QString& descr, const QString& action, int keyCode,
			 int id, QPopupMenu *qmenu, bool configurable)
{
	return insertItem( descr, action, keyCode, keyCode, id, qmenu, configurable );
}

bool KAccel::insertItem( const QString& descr, const QString& action,
			 const QString& keyCode, bool configurable )
{
	int iKeyCode = KKey::stringToKeyQt( keyCode );
	return insertItem( descr, action, iKeyCode, configurable );
}

bool KAccel::insertItem( const QString& descr, const QString& action,
			 const QString& keyCode, int id,
			 QPopupMenu *qmenu, bool configurable)
{
	int iKeyCode = KKey::stringToKeyQt( keyCode );
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

	QString k = KKey::keyToString( currentKey( action), true );
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

const KKeyMapOrder& KAccel::keyInsertOrder() const
{
	return d->aKeyMapOrder;
}

KKeyMapOrder& KAccel::keyInsertOrder()
{
	return d->aKeyMapOrder;
}

void KAccel::readSettings( KConfig* config )
{
    kdDebug(125) << "KAccel::readSettings(...)\n";
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

    kdDebug(125) << "readKeyMap start" << endl;

    KConfigBase *pConfig = config ? config : KGlobal::config();
    KConfigGroupSaver cgs(pConfig, group);

    for (KKeyEntryMap::Iterator it = map.begin();
         it != map.end(); ++it) {
        s = pConfig->readEntry(it.key());

	if ( s.isNull() || s.startsWith( "default" ))
            (*it).aConfigKeyCode = /*useFourModifierKeys() ? (*it).aDefaultKeyCode4 :*/ (*it).aDefaultKeyCode;
        else if( s == "none" )
            (*it).aConfigKeyCode = 0;
        else
            (*it).aConfigKeyCode = KKey::stringToKeyQt( s );

        (*it).aCurrentKeyCode = (*it).aConfigKeyCode;

	kdDebug(125) << "\t" << it.key() << " = " << s << " = " << QString::number((*it).aCurrentKeyCode,16) << endl;
    }

    kdDebug(125) << "readKeyMap done" << endl;
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
            kdDebug(125) << "writing " <<  KKey::keyToString( (*it).aCurrentKeyCode, false) << " " <<
                KKey::keyToString( (*it).aConfigKeyCode, false) << endl;

            if ( ( *it ).aConfigKeyCode != ( *it ).aDefaultKeyCode )
            {
                QString keyStr = (*it).aConfigKeyCode ?
                                  KKey::keyToString( (*it).aConfigKeyCode, false ) :
                                  "none";
                pConfig->writeEntry( it.key(), keyStr, true, global );
            }
            else
            { // global ones must be written
            if ( global || !pConfig->readEntry( it.key() ).isNull() )
                pConfig->writeEntry( it.key(), QString( "default(%1)" )
                    .arg( KKey::keyToString( (*it).aDefaultKeyCode, false )), true, global );
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

QString KAccel::keyToString( uint key )
{
	kdDebug(125) << "Obsolete KAccel::keyToString() called" << endl;
	return KKey::keyToString( key );
}

uint KAccel::stringToKey( const QString& s )
{
	kdDebug(125) << "Obsolete KAccel::stringToKey() called" << endl;
	return KKey::stringToKey( s ).key();
}

#include "kaccel.moc"
