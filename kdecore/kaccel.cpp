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

// For the X11-related static functions
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <ctype.h>

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

KKey::KKey( const XEvent *pEvent )	{ m_keyCombQt = KAccel::keyEventXToKeyQt( pEvent ); }
KKey::KKey( const QKeyEvent *pEvent )	{ m_keyCombQt = KAccel::keyEventQtToKeyQt( pEvent ); }
KKey::KKey( const QString& keyStr )	{ m_keyCombQt = KAccel::stringToKey( keyStr ); }
QString KKey::toString()		{ return KAccel::keyToString( m_keyCombQt ); }

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

    // If this is an application shortcut, make sure that we are really
    //  using the right modifier combination by converting to it's X equivalent
    //  and back. Neccessary for punctuation keys on varying layouts.
    if( id ) {
        uint keySymX, keyModX;
        keyQtToKeyX( defaultKeyCode3.key(), 0, &keySymX, &keyModX );
        defaultKeyCode3 = keySymXToKeyQt( keySymX, keyModX );
    }

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
                QString keyStr = (*it).aConfigKeyCode ?
                                  KAccel::keyToString( (*it).aConfigKeyCode, false ) :
                                  QString::fromLatin1("none");
                pConfig->writeEntry( it.key(), keyStr, true, global );
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

// Indicate whether to default to the 3- or 4- modifier keyboard schemes
// This variable should also be moved into a class along with the
// X11-related key functions below.
static int g_bUseFourModifierKeys = -1;

bool KAccel::useFourModifierKeys()
{
	if( g_bUseFourModifierKeys == -1 ) {
		// Read in whether to use 4 modifier keys
		KConfigGroupSaver cgs( KGlobal::config(), "Keyboard Layout" );
		bool fourMods = KGlobal::config()->readBoolEntry( "Use Four Modifier Keys",  false );
		g_bUseFourModifierKeys = fourMods && keyboardHasMetaKey();
	}
	return g_bUseFourModifierKeys == 1;
}

void KAccel::useFourModifierKeys( bool b )
{
	if( g_bUseFourModifierKeys != (int)b ) {
		g_bUseFourModifierKeys = b && keyboardHasMetaKey();
		// If we're 'turning off' the meta key or, if we're turning it on,
		//  the keyboard must actually have a meta key.
		if( b && !keyboardHasMetaKey() )
			kdDebug(125) << "Tried to use four modifier keys on a keyboard layout without a Meta key.\n";
	}
	KConfigGroupSaver cgs( KGlobal::config(), "Keyboard Layout" );
	KGlobal::config()->writeEntry( "Use Four Modifier Keys", g_bUseFourModifierKeys, true, true);

	kdDebug(125) << "bUseFourModifierKeys = " << g_bUseFourModifierKeys << endl;
}

bool KAccel::qtSupportsMetaKey()
{
	static int qtSupport = -1;
	if( qtSupport == -1 ) {
		qtSupport = QAccel::stringToKey("Meta+A") & (Qt::ALT<<1);
		kdDebug(125) << "Qt Supports Meta Key: " << qtSupport << endl;
	}
	return qtSupport == 1;
}

/*****************************************************************************/

struct ModKeyXQt
{
	static bool	bInitialized;
	const char	*keyName;
	uint		keyModMaskQt;
	uint		keyModMaskX;
};
bool ModKeyXQt::bInitialized = false;

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
	{ "ModeSwitch",	0,		Mod3Mask },
	{ "Meta",	(Qt::ALT<<1), 	Mod4Mask },
	{ "ScrollLock", 0,		Mod5Mask },
	{ 0, 0, 0 }
};

static const TransKey g_aTransKeySyms[] = {
	{ Qt::Key_Backspace,	XK_BackSpace },
	{ Qt::Key_Backtab,	XK_ISO_Left_Tab },
	{ Qt::Key_Enter,	XK_KP_Enter },
	{ Qt::Key_SysReq,	XK_Sys_Req },
	{ Qt::Key_CapsLock,	XK_Caps_Lock },
	{ Qt::Key_NumLock,	XK_Num_Lock },
	{ Qt::Key_ScrollLock,	XK_Scroll_Lock }
};

void KAccel::readModifierMapping()
{
	XModifierKeymap* xmk = XGetModifierMapping( qt_xdisplay() );

	for( int i = Mod2MapIndex; i < 8; i++ )
		g_aModKeys[i].keyModMaskX = 0;

	// Qt assumes that Alt is always Mod1Mask, so start at Mod2Mask.
	for( int i = Mod2MapIndex; i < 8; i++ ) {
		uint keySymX = XKeycodeToKeysym( qt_xdisplay(), xmk->modifiermap[xmk->max_keypermod * i], 0 );
		int j = -1;
		switch( keySymX ) {
			//case XK_Alt_L:
			//case XK_Alt_R:		j = 3; break;	// Normally Mod1Mask
			case XK_Num_Lock:	j = 4; break;	// Normally Mod2Mask
			case XK_Mode_switch:	j = 5; break;	// Normally Mod3Mask
			case XK_Meta_L:
			case XK_Meta_R:		j = 6; break;	// Normally Mod4Mask
			case XK_Scroll_Lock:	j = 7; break;	// Normally Mod5Mask
		}
		if( j >= 0 ) {
			g_aModKeys[j].keyModMaskX = (1<<i);
			kdDebug(125) << QString( "%1 = mod%2, keySymX = %3\n" ).arg(g_aModKeys[j].keyName).arg(i-2).arg(keySymX, 0, 16);
		}
	}

	for( int i = Mod1MapIndex; i < 8; i++ )
		kdDebug(125) << QString( "%1: keyModMaskX = 0x%2\n" ).arg(g_aModKeys[i].keyName).arg(g_aModKeys[i].keyModMaskX, 0, 16);

	XFreeModifiermap(xmk);

	ModKeyXQt::bInitialized = true;
}

QString KAccel::keyToString( int keyCombQt, bool bi18n )
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
		// Make sure 'Backtab' print
		if( keySymQt == Qt::Key_Backtab ) {
			keySymQt = Qt::Key_Tab;
			keyModQt |= Qt::SHIFT;
		}
		if( keyModQt ) {
			// Should possibly remove SHIFT
			// i.e., in en_US: 'Exclam' instead of 'Shift+1'
			// But don't do it on the Tab key.
			if( (keyModQt & Qt::SHIFT) && keySymQt != Qt::Key_Tab ) {
				int	index = keySymXIndex( keySymX );
				int	indexUnshifted = (index / 2) * 2; // 0 & 1 => 0, 2 & 3 => 2
				uint	keySymX0 = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, indexUnshifted ),
					keySymX1 = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, indexUnshifted+1 );
				QString	s0 = XKeysymToString( keySymX0 ),
					s1 = XKeysymToString( keySymX1 );

				// If shifted value is not the same as unshifted,
				//  then we shouldn't print Shift.
				if( s0.lower() != s1.lower() ) {
					keyModQt &= ~Qt::SHIFT;
					keySymX = keySymX1;
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

		keyStr = (bi18n) ? i18n("Unknown Key", "Unknown") : QString("Unknown");
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
				     if (bi18n) 
              				keyStr = i18n("QAccel", KKEYS[i].name);
            			     else
              				keyStr = KKEYS[i].name;
			  	     break;
				}
			}
		}
	}

	return !keyStr.isEmpty() ? (keyModStr + keyStr) : QString::null;
}

int KAccel::stringToKey(const QString& key)
{
	QString keyStr = key;

        if( key == "default" )  // old code used to write just "default"
            return 0;                           //  which is not enough
        if( key.startsWith( "default(" )) {
            int pos = key.findRev( ')' );
            if( pos >= 0 ) // this should be really done with regexp
                keyStr = key.mid( 8, pos - 8 );
        }

	kdDebug(125) << QString("stringToKey("+key+") = %1\n").arg(stringToKey( keyStr, 0, 0, 0 ), 0, 16);

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
	QChar	c;

	// Initialize
	if( pKeySymX )	*pKeySymX = 0;
	if( pKeyCodeX )	*pKeyCodeX = 0;
	if( pKeyModX )	*pKeyModX = 0;

	if( keyStr.isNull() || keyStr.isEmpty() )
		return 0;

	if( !ModKeyXQt::bInitialized )
		KAccel::readModifierMapping();

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

		// Check if this is a modifier key (Shift, Ctrl, Alt, Meta).
		for( i = 0; i < MOD_KEYS; i++ ) {
			if( g_aModKeys[i].keyModMaskQt && stricmp( sKeySym.ascii(), g_aModKeys[i].keyName ) == 0 ) {
				// If there is no X mod flag defined for this modifier,
				//  then all zeroes should be returned for the X-codes.
				// Ex: If string="Meta+F1", but X hasn't assigned Meta, don't return 'F1'.
				if( g_aModKeys[i].keyModMaskX == 0 ) {
					pKeyCodeX = 0;
					pKeySymX = 0;
					pKeyModX = 0;
				}
				keyCombQt |= g_aModKeys[i].keyModMaskQt;
				keyModX |= g_aModKeys[i].keyModMaskX;
				break;
			}
		}

		// If this was not a modifier key,
		//  search for 'normal' key.
		if( i == MOD_KEYS ) {
			// Abort if already found primary key.
			if( !c.isNull() || keySymX ) {
				c = QChar::null;
				keySymX = keyModX = keyCombQt = 0;
				break;
			}
			//if( keySymX ) { kdWarning(125) << "keystrToKey: Tried to set more than one key in key code." << endl; return 0; }

			if( sKeySym.length() == 1 )
				c = sKeySym[0];
			else {
				// Search for Qt keycode
				for( i = 0; i < NB_KEYS; i++ ) {
					if( stricmp( sKeySym.ascii(), KKEYS[i].name ) == 0 ) {
						keyCombQt |= KKEYS[i].code;
						keyQtToKeyX( KKEYS[i].code, 0, &keySymX, 0 );
						if( KKEYS[i].code < 0x1000 && QChar(KKEYS[i].code).isLetter() )
							c = KKEYS[i].code;
						break;
					}
				}

				//if( i == NB_KEYS ) { kdWarning(125) << "keystrToKey: Unknown key name " << sKeySym << endl; return 0; }
				if( i == NB_KEYS ) {
					c = QChar::null;
					keySymX = keyModX = keyCombQt = 0;
					break;
				}
			}
		}
	} while( (uint)iOffsetToken < keyStr.length() );

	if( !c.isNull() ) {
		if( c.isLetter() && !(keyModX & ShiftMask) )
			c = c.lower();
		keySymX = c.unicode();
		// For some reason, Qt always wants 'a-z' as 'A-Z'.
		if( c >= 'a' && c <= 'z' )
			c = c.upper();
		keyCombQt |= c.unicode();
	}

	if( keySymX ) {
		// Find X key code (code of key sent from keyboard)
		keyCodeX = XKeysymToKeycode( qt_xdisplay(), keySymX );

		// If 'Shift' has been explicitly give, i.e. 'Shift+1',
		if( keyModX & ShiftMask ) {
			int index = keySymXIndex( keySymX );
			// But symbol given is unshifted, i.e. '1'
			if( index == 0 || index == 2 ) {
				keySymX = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, index+1 );
				keyCombQt = keySymXToKeyQt( keySymX, keyModX );
			}
		}

		// If keySym requires Shift or ModeSwitch to activate,
		//  then add the flags.
		if( keySymX != XK_Sys_Req && keySymX != XK_Break )
			keySymXMods( keySymX, &keyCombQt, &keyModX );
	}

	// Take care of complications:
	//  The following keys will not have been correctly interpreted,
	//   because their shifted values are not activated with the
	//   Shift key, but rather something else.  They are also
	//   defined twice under different keycodes.
	//  keycode 111 & 92:  Print Sys_Req -> Sys_Req = Alt+Print
	//  keycode 110 & 114: Pause Break   -> Break = Ctrl+Pause
	if( (keyCodeX == 92 || keyCodeX == 111) &&
	    XKeycodeToKeysym( qt_xdisplay(), 92, 0 ) == XK_Print &&
	    XKeycodeToKeysym( qt_xdisplay(), 111, 0 ) == XK_Print )
	{
		// If Alt is pressed, then we need keycode 92, keysym XK_Sys_Req
		if( keyModX & keyModXAlt() ) {
			keyCodeX = 92;
			keySymX = XK_Sys_Req;
		}
		// Otherwise, keycode 111, keysym XK_Print
		else {
			keyCodeX = 111;
			keySymX = XK_Print;
		}
	}
	else if( (keyCodeX == 110 || keyCodeX == 114) &&
	    XKeycodeToKeysym( qt_xdisplay(), 110, 0 ) == XK_Pause &&
	    XKeycodeToKeysym( qt_xdisplay(), 114, 0 ) == XK_Pause )
	{
		if( keyModX & keyModXCtrl() ) {
			keyCodeX = 114;
			keySymX = XK_Break;
		} else {
			keyCodeX = 110;
			keySymX = XK_Pause;
		}
	}

	if( pKeySymX )	*pKeySymX = keySymX;
	if( pKeyCodeX )	*pKeyCodeX = keyCodeX;
	if( pKeyModX )	*pKeyModX = keyModX;

	return keyCombQt;
}

uint KAccel::keyCodeXToKeySymX( uchar keyCodeX, uint keyModX )
{
	uint keySymX = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, 0 );

	// Alt+Print = Sys_Req
	if( keySymX == XK_Print ) {
		if( keyModX & keyModXAlt() && XKeycodeToKeysym( qt_xdisplay(), keyCodeX, 1 ) == XK_Sys_Req )
			keySymX = XK_Sys_Req;
	}
	// Ctrl+Pause = Break
	else if( keySymX == XK_Pause ) {
		if( keyModX & keyModXCtrl() && XKeycodeToKeysym( qt_xdisplay(), keyCodeX, 1 ) == XK_Break )
			keySymX = XK_Break;
	} else {
		// I don't know where it's documented, but Mode_shift sometimes sets the 13th bit in 'state'.
		int index = ((keyModX & ShiftMask) ? 1 : 0) +
			((keyModX & (0x2000 | keyModXModeSwitch())) ? 2 : 0);
		keySymX = XKeycodeToKeysym( qt_xdisplay(), keyCodeX, index );
	}

	return keySymX;
}

void KAccel::keyEventXToKeyX( const XEvent *pEvent, uchar *pKeyCodeX, uint *pKeySymX, uint *pKeyModX )
{
	if( pKeyCodeX )	*pKeyCodeX = pEvent->xkey.keycode;
	if( pKeySymX )	*pKeySymX = keyCodeXToKeySymX( pEvent->xkey.keycode, pEvent->xkey.state );
	if( pKeyModX )	*pKeyModX = pEvent->xkey.state;
}

uint KAccel::keyEventXToKeyQt( const XEvent *pEvent )
{
	uint keySymX, keyModX;
	keyEventXToKeyX( pEvent, 0, &keySymX, &keyModX );
	return keySymXToKeyQt( keySymX, keyModX );
}

int KAccel::keySymXIndex( uint keySym )
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
	int i = keySymXIndex( keySym );

	if( i == 1 || i == 3 ) {
		keyModQt |= Qt::SHIFT;
		keyModX |= ShiftMask;
	}
	if( i == 2 || i == 3 ) {
		keyModX |= keyModXModeSwitch();
	}

	if( pKeyModQt )	*pKeyModQt |= keyModQt;
	if( pKeyModX )	*pKeyModX |= keyModX;
}

uint KAccel::keyCodeXToKeyQt( uchar keyCodeX, uint keyModX )
{
	return keySymXToKeyQt( keyCodeXToKeySymX( keyCodeX, keyModX ), keyModX );
}

uint KAccel::keySymXToKeyQt( uint keySymX, uint keyModX )
{
	uint	keyCombQt = 0;

	if( !ModKeyXQt::bInitialized )
		KAccel::readModifierMapping();

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

	if( !keyCombQt ) {
		if( keySymX == XK_Sys_Req )
			keyCombQt = Qt::Key_Print | Qt::ALT;
		else if( keySymX == XK_Break )
			keyCombQt = Qt::Key_Pause | Qt::CTRL;
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
	uint	keySymX = 0;
	unsigned char	keyCodeX = 0;
	uint	keyModX = 0;

	const char *psKeySym = 0;

	if( !ModKeyXQt::bInitialized )
		KAccel::readModifierMapping();

	// Get code of just the primary key
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
				//kdDebug(125) << " symbol found: \"" << psKeySym << "\"" << endl;
				break;
			}
		}

		// Get X key symbol.  Only works if Qt name is same as X name.
		if( psKeySym ) {
			QString sKeySym = psKeySym;

			// Check for lower-case equalent first because most
			//  X11 names are all lower-case.
			keySymX = XStringToKeysym( sKeySym.lower().ascii() );
			if( keySymX == 0 )
				keySymX = XStringToKeysym( psKeySym );
		}

		if( keySymX == 0 ) {
			for( uint i = 0; i < sizeof(g_aTransKeySyms)/sizeof(TransKey); i++ )
			{
				if( keySymQt == g_aTransKeySyms[i].keySymQt ) {
					keySymX = g_aTransKeySyms[i].keySymX;
					break;
				}
			}
		}
	}

	if( keySymX != 0 ) {
		// Get X keyboard code
		keyCodeX = XKeysymToKeycode( qt_xdisplay(), keySymX );
		// Add ModeSwitch modifier bit, if necessary
		keySymXMods( keySymX, 0, &keyModX );

		// Get X modifier flags
		for( int i = 0; i < MOD_KEYS; i++ ) {
			if( keyCombQt & g_aModKeys[i].keyModMaskQt ) {
				if( g_aModKeys[i].keyModMaskX )
					keyModX |= g_aModKeys[i].keyModMaskX;
				// Qt key calls for a modifier which the current
				//  X modifier map doesn't support.
				else {
					keySymX = 0;
					keyCodeX = 0;
					keyModX = 0;
					break;
				}
			}
		}
	}

	// Take care of complications:
	//  The following keys will not have been correctly interpreted,
	//   because their shifted values are not activated with the
	//   Shift key, but rather something else.  They are also
	//   defined twice under different keycodes.
	//  keycode 111 & 92:  Print Sys_Req -> Sys_Req = Alt+Print
	//  keycode 110 & 114: Pause Break   -> Break = Ctrl+Pause
	if( (keyCodeX == 92 || keyCodeX == 111) &&
	    XKeycodeToKeysym( qt_xdisplay(), 92, 0 ) == XK_Print &&
	    XKeycodeToKeysym( qt_xdisplay(), 111, 0 ) == XK_Print )
	{
		// If Alt is pressed, then we need keycode 92, keysym XK_Sys_Req
		if( keyModX & keyModXAlt() ) {
			keyCodeX = 92;
			keySymX = XK_Sys_Req;
		}
		// Otherwise, keycode 111, keysym XK_Print
		else {
			keyCodeX = 111;
			keySymX = XK_Print;
		}
	}
	else if( (keyCodeX == 110 || keyCodeX == 114) &&
	    XKeycodeToKeysym( qt_xdisplay(), 110, 0 ) == XK_Pause &&
	    XKeycodeToKeysym( qt_xdisplay(), 114, 0 ) == XK_Pause )
	{
		if( keyModX & keyModXCtrl() ) {
			keyCodeX = 114;
			keySymX = XK_Break;
		} else {
			keyCodeX = 110;
			keySymX = XK_Pause;
		}
	}

	if( pKeySymX )	*pKeySymX = keySymX;
	if( pKeyCodeX ) *pKeyCodeX = keyCodeX;
	if( pKeyModX )  *pKeyModX = keyModX;
}

uint KAccel::keyEventQtToKeyQt( const QKeyEvent* pke )
{
        uint keyCombQt;

	// Set the modifier bits.
	keyCombQt = (pke->state() & Qt::KeyButtonMask) * (Qt::SHIFT / Qt::ShiftButton);

	if( pke->key() )
		keyCombQt |= pke->key();
	// If key() == 0, then it may be a compose character, so we need to
	//  look at text() instead.
	else {
		QChar c = pke->text()[0];
		// Looks like Qt allows unicode character up to 0x0fff.
		if( pke->text().length() == 1 && c.unicode() < 0x1000 )
			keyCombQt |= c.unicode();
		else
			keyCombQt |= Qt::Key_unknown;
	}

	return keyCombQt;
}

QString KAccel::keyCodeXToString( uchar keyCodeX, uint keyModX, bool bi18n )
	{ return keyToString( keyCodeXToKeyQt( keyCodeX, keyModX ), bi18n ); }
QString KAccel::keySymXToString( uint keySymX, uint keyModX, bool bi18n )
	{ return keyToString( keySymXToKeyQt( keySymX, keyModX ), bi18n ); }

uint KAccel::keyModXShift()		{ return ShiftMask; }
uint KAccel::keyModXLock()		{ return LockMask; }
uint KAccel::keyModXCtrl()		{ return ControlMask; }

uint KAccel::keyModXAlt()
{
	if( !ModKeyXQt::bInitialized )
		KAccel::readModifierMapping();
	return g_aModKeys[ModAltIndex].keyModMaskX;
}

uint KAccel::keyModXNumLock()
{
	if( !ModKeyXQt::bInitialized )
		KAccel::readModifierMapping();
	return g_aModKeys[ModNumLockIndex].keyModMaskX;
}

uint KAccel::keyModXModeSwitch()
{
	if( !ModKeyXQt::bInitialized )
		KAccel::readModifierMapping();
	return g_aModKeys[ModModeSwitchIndex].keyModMaskX;
}

uint KAccel::keyModXMeta()
{
	if( !ModKeyXQt::bInitialized )
		KAccel::readModifierMapping();
	return g_aModKeys[ModMetaIndex].keyModMaskX;
}

uint KAccel::keyModXScrollLock()
{
	if( !ModKeyXQt::bInitialized )
		KAccel::readModifierMapping();
	return g_aModKeys[ModScrollLockIndex].keyModMaskX;
}

uint KAccel::accelModMaskQt()		{ return Qt::SHIFT | Qt::CTRL | Qt::ALT | (Qt::ALT<<1); }
uint KAccel::accelModMaskX()		{ return ShiftMask | ControlMask | keyModXAlt() | keyModXMeta(); }

bool KAccel::keyboardHasMetaKey()
{
	if( !ModKeyXQt::bInitialized )
		KAccel::readModifierMapping();
	return keyModXMeta() != 0;
}

#include "kaccel.moc"
