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

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h> // For the X11-related static functions
#include <qt_x11.h>
#include <ctype.h>

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

struct ModKeyXQt
{
	const char	*keyName;
	uint		keyModMaskQt;
	uint		keyModMaskX;
};

struct TransKey {
	uint keySymQt;
	uint	keySymX;
};

static ModKeyXQt g_aModKeys[] =
{
	{ "Shift",	Qt::SHIFT,	ShiftMask },
	{ "CapsLock",	0, 		LockMask },
	{ "Ctrl",	Qt::CTRL,	ControlMask },
	{ "Alt",	Qt::ALT,	Mod1Mask },
	{ "NumLock",	0,		Mod2Mask },
	{ "Alt-Gr",	0,		Mod3Mask },
	{ "Meta",	(Qt::ALT<<1), 	Mod4Mask },
	{ "ScrollLock", 0,		Mod5Mask },
	{ 0, 0, 0 }
};

static TransKey g_aTransKeySyms[] = {
	{ Qt::Key_Backspace,	XK_BackSpace },
	{ Qt::Key_Backtab,	XK_ISO_Left_Tab },
	{ Qt::Key_Enter,	XK_KP_Enter },
	{ Qt::Key_SysReq,	XK_Sys_Req },
	{ Qt::Key_CapsLock,	XK_Caps_Lock },
	{ Qt::Key_NumLock,	XK_Num_Lock },
	{ Qt::Key_ScrollLock,	XK_Scroll_Lock }
};

QString KAccel::keyToString( uint keyCombQt, bool bi18n )
{
	QString keyStr, keyModStr;
	uint keySymQt = keyCombQt & 0xffff;
	uint keyModQt = keyCombQt & ~0xffff;

	unsigned char keyCodeX;
	uint keySymX;
	uint keyModX;
	keyQtToKeyX( keyCombQt, &keyCodeX, &keySymX, &keyModX );

	// Letters should be displayed in upper-case.
	// If this is a unicode value (Qt special keys begin at 0x1000)
	if( keySymQt < 0x1000 )
		keySymQt = QChar( keySymQt ).upper().unicode();

	if( keySymQt ) {
		if( keyModQt ) {
			// Should possibly remove SHIFT
			// i.e., in en_US: 'Exclam' instead of 'Shift+1'
			// ***: Once i know how, i need to check for Mode_switch too.
			if( keyModQt & Qt::SHIFT ) {
				QString s0 = XKeysymToString( XKeycodeToKeysym( qt_xdisplay(), keyCodeX, 0 ) ),
					s1 = XKeysymToString( XKeycodeToKeysym( qt_xdisplay(), keyCodeX, 1 ) );

				// If shifted value is not the same as unshifted,
				//  then we shouldn't print Shift.
				if( s0.lower() != s1.lower() ) {
					keyModQt &= ~Qt::SHIFT;
					keySymX = XStringToKeysym( s1.ascii() );
				}
			}

			// Search for modifier flags.
			for( int i = MOD_KEYS-1; i >= 0; i-- ) {
				if( keyModQt & g_aModKeys[i].keyModMaskQt ) {
					keyModStr += (bi18n) ? i18n(g_aModKeys[i].keyName) : QString(g_aModKeys[i].keyName);
					keyModStr += "+";
				}
			}
		}

		keyStr = "Unknown";
		// Determine name of primary key.
		// If printable, non-space unicode character,
		//  then display it directly instead of by name
		//  (e.g. '!' instead of 'Exclam')
		// UNLESS we're not wanting internationalization.  Then all
		//  keys should be printed with their ASCII-7 name.
		if( bi18n && keySymQt < 0x1000 && QChar(keySymQt).isPrint() && !QChar(keySymQt).isSpace() )
			keyStr = QChar(keySymQt);
		else {
			for( int i = 0; i < NB_KEYS; i++ ) {
				if( keySymQt == (uint) KKEYS[i].code ) {
					keyStr = KKEYS[i].name;
					break;
				}
			}
		}
	}

	return keyModStr + keyStr;
}

uint KAccel::stringToKey(const QString& key)
{
	QString keyStr = key;

        if( key == "default" )  // old code used to write just "default"
            return 0;                           //  which is not enough
        if( key.startsWith( "default(" )) {
            int pos = key.findRev( ')' );
            if( pos >= 0 ) // this should be really done with regexp
                keyStr = key.mid( 8, pos - 8 );
        }

	return stringToKey( keyStr, 0, 0, 0 );
}

// Return value is Qt key code.
uint KAccel::stringToKey( const QString& keyStr, unsigned char *pKeyCodeX, uint *pKeySymX, uint *pKeyModX )
{
	uint	keySymX = 0;
	unsigned char	keyCodeX = 0;
	uint	keyModX = 0;
	uint	keyCombQt = 0;
	QString sKeySym;

	QString t;
	for( int i = 0; i < keyStr.length(); i++ ) {
		t += QString( "[%1]" ).arg( keyStr[i] );
	}
	kdDebug() << t << endl;

	if( keyStr.isNull() || keyStr.isEmpty() )
		return 0;

	int iOffset = 0, iOffsetToken;
	do {
		int i;

		// Find next token.
		iOffsetToken = keyStr.find( '+', iOffset );
		// If no more '+'s are found, set to end of string.
		if( iOffsetToken < 0 )
			iOffsetToken = keyStr.length();
		// Allow a '+' to be the keysym if it's the last character.
		else if( iOffsetToken == iOffset && iOffset + 1 == (int)keyStr.length() )
			iOffsetToken++;
		sKeySym = keyStr.mid( iOffset, iOffsetToken - iOffset ).stripWhiteSpace();
		iOffset = iOffsetToken + 1;

		//if( sKeySym.isEmpty() ) { kdWarning(125) << "stringToKey::Empty token" << endl; return 0; }

		// Check if this is a modifier key.
		for( i = 0; i < MOD_KEYS; i++ ) {
			if( stricmp( sKeySym.ascii(), g_aModKeys[i].keyName ) == 0 )
				break;
		}

		// If a mod key was found (Shift, Ctrl, Alt, Meta),
		if( i < MOD_KEYS ) {
			keyCombQt |= g_aModKeys[i].keyModMaskQt;
			keyModX |= g_aModKeys[i].keyModMaskX;
		}
		// Otherwise, look key up,
		else {
			// Abort if already found primary key.
			if( keySymX ) {
				keySymX = keyModX = keyCodeX = keyCombQt = 0;
				break;
			}
			//if( keySymX ) { kdWarning(125) << "keystrToKey: Tried to set more than one key in key code." << endl; return 0; }

			if( sKeySym.length() == 1 ) {
				QChar c = sKeySym[0];
				keyCombQt |= c.unicode();
				keySymX = c.unicode();

				// If this is an ASCII alpha without the SHIFT-key held down,
				//  then use lower-case;
				if( keySymX >= 'A' && keySymX <= 'Z' && !(keyCombQt & Qt::SHIFT) )
					keySymX = QChar(int(keySymX)).lower().unicode();
			}
			else {
				// Search for Qt keycode
				for( i = 0; i < NB_KEYS; i++ ) {
					if( sKeySym == KKEYS[i].name ) {
						keyCombQt |= KKEYS[i].code;
						keyQtToKeyX( KKEYS[i].code, 0, &keySymX, 0 );
						break;
					}
				}

				//if( i == NB_KEYS ) { kdWarning(125) << "keystrToKey: Unknown key name " << sKeySym << endl; return 0; }
				if( i == NB_KEYS ) {
					keySymX = keyModX = keyCodeX = keyCombQt = 0;
					break;
				}
			}

			// Find X key code (code of key send from keyboard)
			keyCodeX = XKeysymToKeycode( qt_xdisplay(), keySymX );

			// Find which index this key symbol is at for the given key code.
			for( i = 0; i < 4; i++ ) {
				if( keySymX == XKeycodeToKeysym( qt_xdisplay(), keyCodeX, i ) )
					break;
			}
			// If this symbol is 'Shifted',
			//  ***: What to do about index 2 and 3 and the Mode_switch key?
			if( i == 1 || i == 3 ) {
				keyCombQt |= Qt::SHIFT;
				keyModX |= ShiftMask;
			}
		}
	} while( (uint)iOffsetToken < keyStr.length() );

	if( pKeySymX )	*pKeySymX = keySymX;
	if( pKeyCodeX )	*pKeyCodeX = keyCodeX;
	if( pKeyModX )	*pKeyModX = keyModX;

	kdDebug() << "KAccel::stringToKey( " << keyStr << " ) = " << QString().setNum( keyCombQt, 16 ) << endl;

	return keyCombQt;
}

void KAccel::setupMasks()
{
	XModifierKeymap* xmk = XGetModifierMapping( qt_xdisplay() );

	for( int i = 3; i < 8; i++ ) {
		int j = -1;
		switch( xmk->modifiermap[xmk->max_keypermod * i + j] ) {
			case XK_Alt_L:
			case XK_Alt_R:		j = 3; break;	// Normally Mod1Mask
			case XK_Num_Lock:	j = 4; break;	// Normally Mod2Mask
			case XK_Mode_switch:	j = 5; break;	// Normally Mod3Mask
			case XK_Meta_L:
			case XK_Meta_R:		j = 6; break;	// Normally Mod4Mask
			case XK_Scroll_Lock:	j = 7; break;	// Normally Mod5Mask
		}
		if( j >= 0 )
			g_aModKeys[j].keyModMaskX = (1<<i);
	}

	XFreeModifiermap(xmk);
}

int KAccel::keyMapXIndex( uint keySym )
{
	unsigned char keyCode = XKeysymToKeycode( qt_xdisplay(), keySym );

	if( keyCode ) {
		for( int i = 0; i < 4; i++ ) {
			uint keySym2 = XKeycodeToKeysym( qt_xdisplay(), keyCode, i );
			if( keySym == keySym2 )
				return i;
		}
	}

	return -1;
}

void KAccel::keySymXMods( uint keySym, uint *pKeyModQt, uint *pKeyModX )
{
	uint keyModQt = 0, keyModX = 0;
	int i = keyMapXIndex( keySym );

	if( i == 1 || i == 3 ) {
		keyModQt |= Qt::SHIFT;
		keyModX |= ShiftMask;
	}
	if( i == 2 || i == 3 ) {
		keyModX |= g_aModKeys[ModAltGrIndex].keyModMaskX;
	}

	if( pKeyModQt )	*pKeyModQt |= keyModQt;
	if( pKeyModX )	*pKeyModX |= keyModX;
}

uint KAccel::keyXToKeyQt( uint keySymX, uint keyModX )
{
	uint	keyCombQt = 0;

	// Qt's own key definitions begin at 0x1000
	if( keySymX < 0x1000 ) {
		// For some reason, Qt wants 'a-z' converted to 'A-Z'
		if( keySymX >= 'a' && keySymX <= 'z' )
			keyCombQt = toupper( keySymX );
		else
			keyCombQt = keySymX;
	}

	if( !keyCombQt ) {
		// Find name of key, and assign its code to keyCombQt.
		const char *psKeySym = XKeysymToString( keySymX );
		for( int i = 0; i < NB_KEYS; i++ ) {
			if( stricmp( psKeySym, KKEYS[i].name ) == 0 ) {
				keyCombQt = KKEYS[i].code;
				break;
			}
		}
	}

	if( !keyCombQt ) {
		for( uint i = 0; i < sizeof(g_aTransKeySyms)/sizeof(TransKey); i++ ) {
			if( keySymX == g_aTransKeySyms[i].keySymX ) {
				keyCombQt = g_aTransKeySyms[i].keySymQt;
				break;
			}
		}
	}

	if( keyCombQt ) {
		// Get Qt modifier flags
		for( int i = 0; i < MOD_KEYS; i++ ) {
			if( keyModX & g_aModKeys[i].keyModMaskX )
				keyCombQt |= g_aModKeys[i].keyModMaskQt;
		}
	}

	return keyCombQt;
}

void KAccel::keyQtToKeyX( uint keyCombQt, unsigned char *pKeyCodeX, uint *pKeySymX, uint *pKeyModX )
{
	uint	keySymQt;
	uint	keySymX = NoSymbol;
	unsigned char	keyCodeX = 0;
	uint	keyModX = 0;

	const char *psKeySym = 0;

	// Find name of key
	keySymQt = keyCombQt & 0xffff;

	// If unicode value beneath 0x1000 (special Qt codes begin thereafter),
	if( keySymQt < 0x1000 ) {
		// For reasons unbeknownst to me, Qt converts 'a-z' to 'A-Z'.
		// So convert it back to lowercase if SHIFT isn't held down.
		if( keySymQt >= Qt::Key_A && keySymQt <= Qt::Key_Z && !(keyCombQt & Qt::SHIFT) )
			keySymQt = tolower( keySymQt );
		keySymX = keySymQt;
	}
	// Else, special key (e.g. Delete, F1, etc.)
	else {
		for( int i = 0; i < NB_KEYS; i++ ) {
			if( keySymQt == (uint) KKEYS[i].code ) {
				psKeySym = KKEYS[i].name;
				//kdDebug() << " symbol found: \"" << psKeySym << "\"" << endl;
				break;
			}
		}

		// Get X key symbol.  Only works if Qt name is same as X name.
		if( psKeySym ) {
			QString sKeySym = psKeySym;

			// Check for lower-case equalent first because most
			//  X11 names are all lower-case.
			keySymX = XStringToKeysym( sKeySym.lower().ascii() );
			if( keySymX == NoSymbol )
				keySymX = XStringToKeysym( psKeySym );
		}

		if( keySymX == NoSymbol ) {
			for( uint i = 0; i < sizeof(g_aTransKeySyms)/sizeof(TransKey); i++ )
			{
				if( keySymQt == g_aTransKeySyms[i].keySymQt ) {
					keySymX = g_aTransKeySyms[i].keySymX;
					break;
				}
			}
		}
	}

	if( keySymX != NoSymbol ) {
		// Get X keyboard code
		keyCodeX = XKeysymToKeycode( qt_xdisplay(), keySymX );

		// Get X modifier flags
		for( int i = 0; i < MOD_KEYS; i++ ) {
			if( keyCombQt & g_aModKeys[i].keyModMaskQt )
				keyModX |= g_aModKeys[i].keyModMaskX;
		}
	}

	if( pKeySymX )	*pKeySymX = keySymX;
	if( pKeyCodeX ) *pKeyCodeX = keyCodeX;
	if( pKeyModX )  *pKeyModX = keyModX;
}

QString KAccel::keyXToString( unsigned char keyCodeX, uint keyModX, bool bi18n )
{
	uint keySymX = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, (keyModX & ShiftMask) );
	uint keyCombQt = keyXToKeyQt( keySymX, keyModX );
	return keyToString( keyCombQt, bi18n );
}

QString KAccel::keyXToString( uint keySymX, uint keyModX, bool bi18n )
{
	uint keyCombQt = keyXToKeyQt( keySymX, keyModX );
	return keyToString( keyCombQt, bi18n );
}

#include "kaccel.moc"
