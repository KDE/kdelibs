/*
    Copyright (c) 2001 Ellis Whitehead <ellis@kde.org>
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

#include "kaccelbase.h"

#include <qkeycode.h>
#include <qpopupmenu.h>

#include <kconfig.h>
#include <kckey.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kkeynative.h>
#include <klocale.h>

//----------------------------------------------------

class KAccelBasePrivate
{
 public:
	//QMap<KKeySequence, QString> m_mapKeyToActionName;
};

KAccelBase::KAccelBase( int fInitCode )
:	m_rgActions( this )
{
	kdDebug(125) << "KAccelBase(): this = " << this << endl;
	d = new KAccelBasePrivate;
	m_bNativeKeys = fInitCode & NATIVE_KEYS;
	m_bEnabled = true;
	m_sConfigGroup = "Shortcuts";
	m_bConfigIsGlobal = false;
	m_bAutoUpdate = false;
}

KAccelBase::~KAccelBase()
{
	kdDebug(125) << "~KAccelBase(): this = " << this << endl;
	delete d;
}

uint KAccelBase::actionCount() const { return m_rgActions.count(); }
KAccelActions& KAccelBase::actions() { return m_rgActions; }
bool KAccelBase::isEnabled() const { return m_bEnabled; }

KAccelAction* KAccelBase::actionPtr( const QString& sAction )
	{ return m_rgActions.actionPtr( sAction ); }

const KAccelAction* KAccelBase::actionPtr( const QString& sAction ) const
	{ return m_rgActions.actionPtr( sAction ); }

KAccelAction* KAccelBase::actionPtr( const KKey& spec )
{
	if( !m_mapKeyToAction.contains( spec ) )
		return 0;
	return m_mapKeyToAction[spec].pAction;
}

void KAccelBase::setConfigGroup( const QString& sConfigGroup )
	{ m_sConfigGroup = sConfigGroup; }

void KAccelBase::setConfigGlobal( bool global )
	{ m_bConfigIsGlobal = global; }

bool KAccelBase::setActionEnabled( const QString& sAction, bool bEnable )
{
	KAccelAction* pAction = actionPtr( sAction );
	if( pAction ) {
		if( pAction->m_bEnabled != bEnable ) {
			kdDebug(125) << "KAccelBase::setActionEnabled( " << sAction << ", " << bEnable << " )" << endl;
			pAction->m_bEnabled = bEnable;
			if( m_bAutoUpdate ) {
				if( bEnable )
					insertConnection( *pAction );
				else if( pAction->isConnected() )
					removeConnection( *pAction );
			}
		}
		return true;
	}
	return false;
}

/*void KAccelBase::removeDeletedMenu( QPopupMenu* pMenu )
{
	for( KAccelActions::iterator it = m_rgActions.begin(); it != m_rgActions.end(); ++it ) {
		if( (*it).m_pMenu == pMenu )
			(*it).m_pMenu = 0;
	}
}*/

bool KAccelBase::setAutoUpdate( bool bAuto )
{
	bool b = m_bAutoUpdate;
	if( !m_bAutoUpdate && bAuto )
		updateConnections();
	m_bAutoUpdate = bAuto;
	return b;
}

void KAccelBase::clearActions()
{
	m_rgActions.clear();
}

KAccelAction* KAccelBase::insert( const QString& sAction, const QString& sDesc, const QString& sHelp,
			const KShortcut& rgCutDefaults3, const KShortcut& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "KAccelBase::insert() begin" << endl;
	KAccelAction* pAction = m_rgActions.insert(
		sAction, sDesc, sHelp,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );

	if( pAction && m_bAutoUpdate )
		insertConnection( *pAction );

	//kdDebug(125) << "KAccelBase::insert() end" << endl;
	return pAction;
}

KAccelAction* KAccelBase::insert( const QString& sName, const QString& sDesc )
	{ return m_rgActions.insert( sName, sDesc ); }

bool KAccelBase::remove( const QString& sAction )
{
	kdDebug(125) << "KAccelBase::removeAction( \"" << sAction << "\" ) this = " << this << endl;
	return m_rgActions.remove( sAction );
}

void KAccelBase::slotRemoveAction( KAccelAction* pAction )
{
	kdDebug(125) << "KAccelBase::slotRemoveAction( \"" << pAction << "\" ) this = " << this << endl;
	removeConnection( *pAction );
}

// BCI: make virtual ASAP, and then make changes to KAccel::connectItem() ???
bool KAccelBase::setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
{
	kdDebug(125) << "KAccelBase::setActionSlot()\n";
	KAccelAction* pAction = m_rgActions.actionPtr( sAction );
	if( pAction ) {
		// If there was a previous connection, remove it.
		if( m_bAutoUpdate && pAction->isConnected() ) {
			kdDebug(125) << "\tm_pObjSlot = " << pAction->m_pObjSlot << " m_psMethodSlot = " << pAction->m_psMethodSlot << endl;
			removeConnection( *pAction );
		}

		pAction->m_pObjSlot = pObjSlot;
		pAction->m_psMethodSlot = psMethodSlot;

		// If we're setting a connection,
		if( m_bAutoUpdate && pObjSlot && psMethodSlot )
			insertConnection( *pAction );

		return true;
	} else
		return false;
}

/*bool KAccelBase::removeItem( const QString& sAction )
{
	KAccelActions::iterator it = actionIterator( sAction );
	if( it != m_rgActions.end() ) {
		KAccelAction& action = *it;
		QAccel::disconnectItem( action.m_nIDAccel, action.m_pObjSlot, action.m_psMethodSlot );
		QAccel::removeItem( action.m_nIDAccel );
		m_rgActions.erase( it );
		return true;
	} else
		return false;
}*/

/*bool KAccelBase::setItemEnabled( const QString& sAction, bool bActivate )
{
	KAccelAction* pAction = actionPtr( sAction );
	if( pAction ) {
		QAccel::setItemEnabled( pAction->m_nIDAccel, bActivate );
		pAction->m_bEnabled = bActivate;
		return true;
	} else
		return false;
}*/

/*
KAccelBase
	Execute Command=Meta+Enter;Alt+F2
	KAccelAction = "Execute Command"
		1) KAccelKeySeries = "Meta+Enter"
			1a) Meta+Enter
			1b) Meta+Keypad_Enter
		2) KAccelKeySeries = "Alt+F2"
			1a) Alt+F2

	Konqueror=Meta+I,I
	KAccelAction = "Konqueror"
		1) KAccelKeySeries = "Meta+I,I"
			1a) Meta+I
			2a) I

	Something=Meta+Asterisk,X
	KAccelAction = "Something"
		1) KAccelKeySeries = "Meta+Asterisk,X"
			1a) Meta+Shift+8
			1b) Meta+Keypad_8
			2a) X

read in a config entry
	split by ';'
	find key sequences to disconnect
	find new key sequences to connect
check for conflicts with implicit keys
	disconnect conflicting implicit keys
connect new key sequences
*/
// Check for double assignments
// Check for conflicts with implicit keys
/*
{
	For {
		for( KAccelAction::iterator itAction = m_rgActions.begin(); itAction != m_rgActions.end(); ++itAction ) {
			KAccelAction& action = *itAction;
			for( KAccelSeries::iterator itSeries = action.m_rgSeries.begin(); itSeries != action.m_rgSeries.end(); ++itSeries ) {
				KAccelSeries& series = *itSeries;
				if(
			}
		}
	}
	Sort by: iVariation, iSequence, iSeries, iAction

	1) KAccelAction = "Execute Command"
		1) KAccelKeySeries = "Meta+Enter"
			1a) Meta+Enter
			1b) Meta+Keypad_Enter
		2) KAccelKeySeries = "Alt+F2"
			1a) Alt+F2

	2) KAccelAction = "Enter Calculation"
		1) KAccelKeySeries = "Meta+Keypad_Enter"
			1a) Meta+Keypad_Enter

	List =
		Meta+Enter		-> 1, 1, 1a
		Meta+Keypad_Enter	-> 2, 1, 1a
		Alt+F2			-> 1, 2, 1a
		[Meta+Keypad_Enter]	-> [1, 1, 1b]

}
*/

struct X
{
	uint iAction, iSeq, iVari;
	KKey spec;

	X() {}
	X( uint _iAction, uint _iSeq, uint _iVari, const KKey& _spec )
		{ iAction = _iAction; iSeq = _iSeq; iVari = _iVari; spec = _spec; }

	int compare( const X& x )
	{
		int n = spec.compare( x.spec );
		if( n != 0 )           return n;
		if( iVari != x.iVari ) return iVari - x.iVari;
		if( iSeq != x.iSeq )   return iSeq - x.iSeq;
		return 0;
	}

	bool operator <( const X& x )  { return compare( x ) < 0; }
	bool operator >( const X& x )  { return compare( x ) > 0; }
	bool operator <=( const X& x ) { return compare( x ) <= 0; }
};

/*
#1 Ctrl+A
#2 Ctrl+A
#3 Ctrl+B
   ------
   Ctrl+A => Null
   Ctrl+B => #3

#1 Ctrl+A
#1 Ctrl+B;Ctrl+A
   ------
   Ctrl+A => #1
   Ctrl+B => #2

#1 Ctrl+A
#1 Ctrl+B,C
#1 Ctrl+B,D
   ------
   Ctrl+A => #1
   Ctrl+B => Null

#1 Ctrl+A
#2 Ctrl+Plus(Ctrl+KP_Add)
   ------
   Ctrl+A => #1
   Ctrl+Plus => #2
   Ctrl+KP_Add => #2

#1 Ctrl+Plus(Ctrl+KP_Add)
#2 Ctrl+KP_Add
   ------
   Ctrl+Plus => #1
   Ctrl+KP_Add => #2

#1 Ctrl+Plus(Ctrl+KP_Add)
#2 Ctrl+A;Ctrl+KP_Add
   ------
   Ctrl+A => #2
   Ctrl+Plus => #1
   Ctrl+KP_Add => #2
*/

bool KAccelBase::updateConnections()
{
	kdDebug(125) << "KAccelBase::updateConnections()  this = " << this << endl;
	// Retrieve the list of keys to be connected, sorted by priority.
	//  (key, variation, seq)
	QValueVector<X> rgKeys;
	createKeyList( rgKeys );

	KKeyToActionMap mapKeyToAction;
	for( uint i = 0; i < rgKeys.size(); i++ ) {
		X& x = rgKeys[i];
		KKey& spec = x.spec;
		ActionInfo info;
		bool bMultiKey = false;

		info.pAction = m_rgActions.actionPtr( x.iAction );
		info.iSeq = x.iSeq;
		info.iVariation = x.iVari;

		// If this is a multi-key shortcut,
		if( info.pAction->shortcut().seq(info.iSeq).count() > 1 )
			bMultiKey = true;
		// If this key is requested by more than one action,
		else if( i < rgKeys.size() - 1 && spec == rgKeys[i+1].spec ) {
			kdDebug(125) << "spec = " << spec.toString()
				<< " action1 = " << info.pAction->name()
				<< " action2 = " << m_rgActions.actionPtr( rgKeys[i+1].iAction )->name() << endl;
			// If multiple actions requesting this key
			//  have the same priority as the first one,
			if( info.iVariation == rgKeys[i+1].iVari && info.iSeq == rgKeys[i+1].iSeq )
				bMultiKey = true;

			// Skip over the other records with this same key.
			while( i < rgKeys.size() - 1 && spec == rgKeys[i+1].spec )
				i++;
		}

		if( bMultiKey ) {
			// Remove connection to single action if there is one
			KAccelAction* pAction = actionPtr( spec );
			if( pAction ) {
				m_mapKeyToAction.remove( spec );
				disconnectKey( *pAction, spec );
				pAction->decConnections();
			}
			// Indicate that no single action is associated with this key.
			info.pAction = 0;
		}

		mapKeyToAction[spec] = info;
	}

	// Disconnect keys which no longer have bindings:
	for( KKeyToActionMap::iterator it = m_mapKeyToAction.begin(); it != m_mapKeyToAction.end(); ++it ) {
		const KKey& spec = it.key();
		if( !mapKeyToAction.contains( spec ) ) {
			KAccelAction* pAction = (*it).pAction;
			if( pAction ) {
				disconnectKey( *pAction, spec );
				pAction->decConnections();
			} else
				disconnectKey( spec );
		}
	}

	// Connect any unconnected keys:
	// In other words, connect any keys which are present in the
	//  new action map, but which are _not_ present in the old one.
	for( KKeyToActionMap::iterator it = mapKeyToAction.begin(); it != mapKeyToAction.end(); ++it ) {
		if( !m_mapKeyToAction.contains( it.key() ) ) {
			// TODO: Decide what to do if connect fails.
			//  Probably should remove this item from map.
			KAccelAction* pAction = (*it).pAction;
			if( pAction ) {
				if( connectKey( *pAction, it.key() ) )
					pAction->incConnections();
			} else
				connectKey( it.key() );
		}
	}

	// Store new map.
	m_mapKeyToAction = mapKeyToAction;

	for( KKeyToActionMap::iterator it = m_mapKeyToAction.begin(); it != m_mapKeyToAction.end(); ++it ) {
		kdDebug(125) << "Key: " << it.key().toString() << " => '"
			<< (((*it).pAction) ? (*it).pAction->name() : QString::null) << "'" << endl;
	}

	return true;
}

// Construct a list of keys to be connected, sorted highest priority first.
void KAccelBase::createKeyList( QValueVector<X>& rgKeys )
{
	if( !m_bEnabled )
		return;

	// create the list
	// For each action
	for( uint iAction = 0; iAction < m_rgActions.count(); iAction++ ) {
		KAccelAction* pAction = m_rgActions.actionPtr( iAction );
		if( pAction && pAction->m_bEnabled && pAction->m_pObjSlot && pAction->m_psMethodSlot ) {
			// For each key sequence associated with action
			for( uint iSeq = 0; iSeq < pAction->sequenceCount(); iSeq++ ) {
				const KKeySequence& seq = pAction->seq(iSeq);
				if( seq.count() > 0 ) {
					KKeyNative::Variations vars;
					vars.init( seq.key(0), !m_bNativeKeys );
					for( uint iVari = 0; iVari < vars.count(); iVari++ )
						rgKeys.push_back( X( iAction, iSeq, iVari, vars.key( iVari ) ) );
				}
			}
		}
	}

	// sort by priority: iVariation[of first key], iSequence, iAction
	qHeapSort( rgKeys.begin(), rgKeys.end() );
}

bool KAccelBase::insertConnection( KAccelAction& action )
{
	if( !action.m_bEnabled || !action.m_pObjSlot || !action.m_psMethodSlot )
		return true;

	kdDebug(125) << "KAccelBase::insertConnection( " << &action << "=\"" << action.m_sName << "\"; shortcut = " << action.shortcut().toString() << " )  this = " << this << endl;

	// For each sequence associated with the given action:
	for( uint iSeq = 0; iSeq < action.sequenceCount(); iSeq++ ) {
		// Get the first key of the sequence.
		KKeyNative::Variations vars;
		vars.init( action.seq(iSeq).key(0), !m_bNativeKeys );
		for( uint iVari = 0; iVari < vars.count(); iVari++ ) {
			KKey key = vars.key( iVari );

			if( !key.isNull() ) {
				if( !m_mapKeyToAction.contains( key ) ) {
					if( action.seq(iSeq).count() == 1 ) {
						m_mapKeyToAction[key] = ActionInfo( &action, iSeq, iVari );
						if( connectKey( action, key ) )
							action.incConnections();
					} else {
						m_mapKeyToAction[key] = ActionInfo( 0, iSeq, iVari );
						if( connectKey( key ) )
							action.incConnections();
					}
				} else {
					// There is a key conflict.  A full update
					//  check is necessary.
					// TODO: make this more efficient where possible.
					if( m_mapKeyToAction[key].pAction != &action
					    && m_mapKeyToAction[key].pAction != 0 ) {
						kdDebug(125) << "Key conflict: call updateConnections()" << endl;
						return updateConnections();
					}
				}
			}
		}
	}

	//kdDebug(125) << "\tActions = " << m_rgActions.size() << endl;
	//for( KAccelActions::const_iterator it = m_rgActions.begin(); it != m_rgActions.end(); ++it ) {
	//	kdDebug(125) << "\t" << &(*it) << " '" << (*it).m_sName << "'" << endl;
	//}

	//kdDebug(125) << "\tKeys = " << m_mapKeyToAction.size() << endl;
	//for( KKeyToActionMap::iterator it = m_mapKeyToAction.begin(); it != m_mapKeyToAction.end(); ++it ) {
	//	//kdDebug(125) << "\tKey: " << it.key().toString() << " => '" << (*it)->m_sName << "'" << endl;
	//	kdDebug(125) << "\tKey: " << it.key().toString() << " => '" << *it << "'" << endl;
	//	kdDebug(125) << "\t\t'" << (*it)->m_sName << "'" << endl;
	//}

	return true;
}

bool KAccelBase::removeConnection( KAccelAction& action )
{
	kdDebug(125) << "KAccelBase::removeConnection( " << &action << " = " << action.m_sName << " )  this = " << this << endl;
	kdDebug(125) << "\tkeys = " << action.m_cut.toString() << endl;

	//for( KKeyToActionMap::iterator it = m_mapKeyToAction.begin(); it != m_mapKeyToAction.end(); ++it )
	//	kdDebug(125) << "\tKey: " << it.key().toString() << " => '" << (*it)->m_sName << "'" << " " << *it << endl;

	// For each sequence associated with the given action:
	for( uint iSeq = 0; iSeq < action.sequenceCount(); iSeq++ ) {
		// Get the first key of the sequence.
		KKeyNative::Variations vars;
		vars.init( action.seq(iSeq).key(0), !m_bNativeKeys );
		for( uint iVari = 0; iVari < vars.count(); iVari++ ) {
			const KKey& key = vars.key( iVari );

			if( m_mapKeyToAction.contains( key ) ) {
				if( m_mapKeyToAction[key].pAction == &action ) {
					m_mapKeyToAction.remove( key );
					disconnectKey( action, key );
					action.decConnections();
				} else if( m_mapKeyToAction[key].pAction == 0 )
					return updateConnections();
			}
		}
	}

	return true;
}

bool KAccelBase::setShortcut( const QString& sAction, const KShortcut& cut )
{
	KAccelAction* pAction = actionPtr( sAction );
	if( pAction ) {
		if( m_bAutoUpdate )
			removeConnection( *pAction );

		pAction->setShortcut( cut );

		if( m_bAutoUpdate )
			insertConnection( *pAction );

		return true;
	} else
		return false;
}

// Create list of extended shortcuts

/*QString KAccelBase::configGroup() const
{
	return aGroup;
}

bool KAccelBase::configGlobal() const
{
	return bGlobal;
}*/

/*bool KAccelBase::isConfigurable( const QString& sAction ) const
{
	const KAccelAction* pAction = actionPtr( sAction );
	return (pAction) ? pAction->m_bConfigurable : false;
}

void KAccelBase::clearItem( const QString& sAction )
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

bool KAccelBase::updateItem( const QString &action, int keyCode)
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
*/
/*
void KAccelBase::readActions( KAccelActions& rgActions, const QString& sGroup, KConfigBase* pConfig )
{
	kdDebug(125) << "readActions start" << endl;

	if( !pConfig )
		pConfig = KGlobal::config();
	KConfigGroupSaver cgs( pConfig, sGroup );

	for( KAccelActions::iterator it = rgActions.begin(); it != rgActions.end(); ++it ) {
		KAccelAction& action = *it;

		QString s = pConfig->readEntry( action.m_sName );
		QStringList rgs = QStringList::split( ';', s );

		if( rgs.size() ) {
			action.m_rgShortcuts.resize( rgs.size() );

			uint i = 0;
			for( ; i < rgs.size(); i++ ) {
				KAccelShortcut& cut = action.m_rgShortcuts[i];
				QString sKey = !rgs[i].startsWith( "default(" ) ? rgs[i] : rgs[i].mid( 8, rgs[i].length() - 9 );
				KKeySequence key;

				if( sKey.isEmpty() )
					key = cut.getKeyDefault();
				else if( s != "none" )
					key = KKey::stringToKey( rgs[i] );
				seq.setKeySequence( key );
			}
		}

		kdDebug(125) << "\t" << action.m_sName << " = '" << s << "' = " << QString::number(action.getPrimaryKey().key(),16) << endl;
	}

	kdDebug(125) << "readActions done" << endl;
}
*/

void KAccelBase::readSettings( KConfig* pConfig )
{
	m_rgActions.readActions( m_sConfigGroup, pConfig );
	if( m_bAutoUpdate )
		updateConnections();
}

void KAccelBase::writeSettings( KConfig* pConfig ) const
{
	m_rgActions.writeActions( m_sConfigGroup, pConfig, m_bConfigIsGlobal, m_bConfigIsGlobal );
}

QPopupMenu* KAccelBase::createPopupMenu( QWidget* pParent, const KKeySequence& seq )
{
	QPopupMenu* pMenu = new QPopupMenu( pParent, "KAccelBase-QPopupMenu" );
	pMenu->setFont( KGlobalSettings::menuFont() );

	bool bActionInserted = false;
	bool bInsertSeparator = false;
	for( uint i = 0; i < actionCount(); i++ ) {
		const KAccelAction* pAction = actions().actionPtr( i );
		// If an action has already been inserted into the menu
		//  and we have a label instead of an action here,
		//  then indicate that we should insert a separator before the next menu entry.
		if( bActionInserted && !pAction->isConfigurable() && pAction->name().contains( ':' ) )
			bInsertSeparator = true;

		for( uint iSeq = 0; iSeq < pAction->sequenceCount(); iSeq++ ) {
			const KKeySequence& seqAction = pAction->seq(iSeq);
			if( seqAction.startsWith( seq ) ) {
				if( bInsertSeparator ) {
					pMenu->insertSeparator();
					bInsertSeparator = false;
				}

				QString sLabel = pAction->label();
				if( seq.count() < seqAction.count() ) {
					sLabel += "\t&";
					for( uint iKey = seq.count(); iKey < seqAction.count(); iKey++ ) {
						sLabel += seqAction.key(iKey).toString();
						if( iKey < seqAction.count() - 1 )
							sLabel += '+';
					}
				}

				pMenu->insertItem( sLabel, i );
				bActionInserted = true;
				break;
			}
		}
	}

	return pMenu;
}
