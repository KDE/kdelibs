/*
    Copyright (C) 1997-2000 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (c) 2001,2002 Ellis Whitehead <ellis@kde.org>

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
#include <qlabel.h>
#include <qpopupmenu.h>

#include <kconfig.h>
#include <kckey.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kkeynative.h>
#include <kkeyserver_x11.h>
#include <klocale.h>
#include <kshortcutmenu.h>

//---------------------------------------------------------------------
// class KAccelBase::ActionInfo
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// class KAccelBase
//---------------------------------------------------------------------

KAccelBase::KAccelBase( int fInitCode )
:	m_rgActions( this )
{
	kdDebug(125) << "KAccelBase(): this = " << this << endl;
	m_bNativeKeys = fInitCode & NATIVE_KEYS;
	m_bEnabled = true;
	m_sConfigGroup = "Shortcuts";
	m_bConfigIsGlobal = false;
	m_bAutoUpdate = false;
	mtemp_pActionRemoving = 0;
}

KAccelBase::~KAccelBase()
{
	kdDebug(125) << "~KAccelBase(): this = " << this << endl;
}

uint KAccelBase::actionCount() const { return m_rgActions.count(); }
KAccelActions& KAccelBase::actions() { return m_rgActions; }
bool KAccelBase::isEnabled() const { return m_bEnabled; }

KAccelAction* KAccelBase::actionPtr( const QString& sAction )
	{ return m_rgActions.actionPtr( sAction ); }

const KAccelAction* KAccelBase::actionPtr( const QString& sAction ) const
	{ return m_rgActions.actionPtr( sAction ); }

KAccelAction* KAccelBase::actionPtr( const KKeyServer::Key& key )
{
	if( !m_mapKeyToAction.contains( key ) )
		return 0;
	// Note: If more than one action is connected to a single key, nil will be returned.
	return m_mapKeyToAction[key].pAction;
}

KAccelAction* KAccelBase::actionPtr( const KKey& key )
{
	KKeyServer::Key k2;
	k2.init( key, !m_bNativeKeys );
	return actionPtr( k2 );
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
				// FIXME: the action may already have it's connections inserted!
				if( bEnable )
					insertConnection( pAction );
				else if( pAction->isConnected() )
					removeConnection( pAction );
			}
		}
		return true;
	}
	return false;
}

bool KAccelBase::setAutoUpdate( bool bAuto )
{
	kdDebug(125) << "KAccelBase::setAutoUpdate( " << bAuto << " ): m_bAutoUpdate on entrance = " << m_bAutoUpdate << endl;
	bool b = m_bAutoUpdate;
	if( !m_bAutoUpdate && bAuto )
		updateConnections();
	m_bAutoUpdate = bAuto;
	return b;
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
		insertConnection( pAction );

	//kdDebug(125) << "KAccelBase::insert() end" << endl;
	return pAction;
}

KAccelAction* KAccelBase::insert( const QString& sName, const QString& sDesc )
	{ return m_rgActions.insert( sName, sDesc ); }

bool KAccelBase::remove( const QString& sAction )
{
	return m_rgActions.remove( sAction );
}

void KAccelBase::slotRemoveAction( KAccelAction* pAction )
{
	removeConnection( pAction );
}

bool KAccelBase::setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
{
	kdDebug(125) << "KAccelBase::setActionSlot( " << sAction << ", " << pObjSlot << ", " << psMethodSlot << " )\n";
	KAccelAction* pAction = m_rgActions.actionPtr( sAction );
	if( pAction ) {
		// If there was a previous connection, remove it.
		if( m_bAutoUpdate && pAction->isConnected() ) {
			kdDebug(125) << "\tm_pObjSlot = " << pAction->m_pObjSlot << " m_psMethodSlot = " << pAction->m_psMethodSlot << endl;
			removeConnection( pAction );
		}

		pAction->m_pObjSlot = pObjSlot;
		pAction->m_psMethodSlot = psMethodSlot;

		// If we're setting a connection,
		if( m_bAutoUpdate && pObjSlot && psMethodSlot )
			insertConnection( pAction );

		return true;
	} else
		return false;
}

/*
KAccelBase
	Run Command=Meta+Enter;Alt+F2
	KAccelAction = "Run Command"
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

	1) KAccelAction = "Run Command"
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
	KKeyServer::Key key;

	X() {}
	X( uint _iAction, uint _iSeq, uint _iVari, const KKeyServer::Key& _key )
		{ iAction = _iAction; iSeq = _iSeq; iVari = _iVari; key = _key; }

	int compare( const X& x )
	{
		int n = key.compare( x.key );
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
	m_rgActionsNonUnique.clear();

	KKeyToActionMap mapKeyToAction;
	for( uint i = 0; i < rgKeys.size(); i++ ) {
		X& x = rgKeys[i];
		KKeyServer::Key& key = x.key;
		ActionInfo info;
		bool bNonUnique = false;

		info.pAction = m_rgActions.actionPtr( x.iAction );
		info.iSeq = x.iSeq;
		info.iVariation = x.iVari;

		// If this is a multi-key shortcut,
		if( info.pAction->shortcut().seq(info.iSeq).count() > 1 )
			bNonUnique = true;
		// If this key is requested by more than one action,
		else if( i < rgKeys.size() - 1 && key == rgKeys[i+1].key ) {
			// If multiple actions requesting this key
			//  have the same priority as the first one,
			if( info.iVariation == rgKeys[i+1].iVari && info.iSeq == rgKeys[i+1].iSeq )
				bNonUnique = true;

			kdDebug(125) << "key conflict = " << key.key().toStringInternal()
				<< " action1 = " << info.pAction->name()
				<< " action2 = " << m_rgActions.actionPtr( rgKeys[i+1].iAction )->name() 
				<< " non-unique = " << bNonUnique << endl;
			
			// Skip over the other records with this same key.
			while( i < rgKeys.size() - 1 && key == rgKeys[i+1].key )
				i++;
		}

		if( bNonUnique ) {
			// Remove connection to single action if there is one
			if( m_mapKeyToAction.contains( key ) ) {
				KAccelAction* pAction = m_mapKeyToAction[key].pAction;
				if( pAction ) {
					m_mapKeyToAction.remove( key );
					disconnectKey( *pAction, key );
					pAction->decConnections();
					m_rgActionsNonUnique.append( pAction );
				}
			}
			// Indicate that no single action is associated with this key.
			m_rgActionsNonUnique.append( info.pAction );
			info.pAction = 0;
		}

		//kdDebug(125) << "mapKeyToAction[" << key.toStringInternal() << "] = " << info.pAction << endl;
		mapKeyToAction[key] = info;
	}

	// Disconnect keys which no longer have bindings:
	for( KKeyToActionMap::iterator it = m_mapKeyToAction.begin(); it != m_mapKeyToAction.end(); ++it ) {
		const KKeyServer::Key& key = it.key();
		KAccelAction* pAction = (*it).pAction;
		// If this key is longer used or it points to a different action now,
		if( !mapKeyToAction.contains( key ) || mapKeyToAction[key].pAction != pAction ) {
			if( pAction ) {
				disconnectKey( *pAction, key );
				pAction->decConnections();
			} else
				disconnectKey( key );
		}
	}

	// Connect any unconnected keys:
	// In other words, connect any keys which are present in the
	//  new action map, but which are _not_ present in the old one.
	for( KKeyToActionMap::iterator it = mapKeyToAction.begin(); it != mapKeyToAction.end(); ++it ) {
		const KKeyServer::Key& key = it.key();
		KAccelAction* pAction = (*it).pAction;
		if( !m_mapKeyToAction.contains( key ) || m_mapKeyToAction[key].pAction != pAction ) {
			// TODO: Decide what to do if connect fails.
			//  Probably should remove this item from map.
			if( pAction ) {
				if( connectKey( *pAction, key ) )
					pAction->incConnections();
			} else
				connectKey( key );
		}
	}

	// Store new map.
	m_mapKeyToAction = mapKeyToAction;

#ifndef NDEBUG
	for( KKeyToActionMap::iterator it = m_mapKeyToAction.begin(); it != m_mapKeyToAction.end(); ++it ) {
		kdDebug(125) << "Key: " << it.key().key().toStringInternal() << " => '"
			<< (((*it).pAction) ? (*it).pAction->name() : QString::null) << "'" << endl;
	}
#endif
	return true;
}

// Construct a list of keys to be connected, sorted highest priority first.
void KAccelBase::createKeyList( QValueVector<struct X>& rgKeys )
{
	//kdDebug(125) << "KAccelBase::createKeyList()" << endl;
	if( !m_bEnabled )
		return;

	// create the list
	// For each action
	for( uint iAction = 0; iAction < m_rgActions.count(); iAction++ ) {
		KAccelAction* pAction = m_rgActions.actionPtr( iAction );
		if( pAction && pAction->m_pObjSlot && pAction->m_psMethodSlot && pAction != mtemp_pActionRemoving ) {
			// For each key sequence associated with action
			for( uint iSeq = 0; iSeq < pAction->shortcut().count(); iSeq++ ) {
				const KKeySequence& seq = pAction->shortcut().seq(iSeq);
				if( seq.count() > 0 ) {
					KKeyServer::Variations vars;
					vars.init( seq.key(0), !m_bNativeKeys );
					for( uint iVari = 0; iVari < vars.count(); iVari++ ) {
						if( vars.key(iVari).code() && vars.key(iVari).sym() )
							rgKeys.push_back( X( iAction, iSeq, iVari, vars.key( iVari ) ) );
						//kdDebug(125) << "\t" << pAction->name() << ": " << vars.key(iVari).toStringInternal() << endl;
					}
				}
				//else
				//	kdDebug(125) << "\t*" << pAction->name() << ":" << endl;
			}
		}
	}

	// sort by priority: iVariation[of first key], iSequence, iAction
	qHeapSort( rgKeys.begin(), rgKeys.end() );
}

bool KAccelBase::insertConnection( KAccelAction* pAction )
{
	if( !pAction->m_pObjSlot || !pAction->m_psMethodSlot )
		return true;

	kdDebug(125) << "KAccelBase::insertConnection( " << pAction << "=\"" << pAction->m_sName << "\"; shortcut = " << pAction->shortcut().toStringInternal() << " )  this = " << this << endl;

	// For each sequence associated with the given action:
	for( uint iSeq = 0; iSeq < pAction->shortcut().count(); iSeq++ ) {
		// Get the first key of the sequence.
		KKeyServer::Variations vars;
		vars.init( pAction->shortcut().seq(iSeq).key(0), !m_bNativeKeys );
		for( uint iVari = 0; iVari < vars.count(); iVari++ ) {
			const KKeyServer::Key& key = vars.key( iVari );

			//if( !key.isNull() ) {
			if( key.sym() ) {
				if( !m_mapKeyToAction.contains( key ) ) {
					// If this is a single-key shortcut,
					if( pAction->shortcut().seq(iSeq).count() == 1 ) {
						m_mapKeyToAction[key] = ActionInfo( pAction, iSeq, iVari );
						if( connectKey( *pAction, key ) )
							pAction->incConnections();
					}
					// Else this is a multi-key shortcut,
					else {
						m_mapKeyToAction[key] = ActionInfo( 0, 0, 0 );
						// Insert into non-unique list if it's not already there.
						if( m_rgActionsNonUnique.findIndex( pAction ) == -1 )
							m_rgActionsNonUnique.append( pAction );
						if( connectKey( key ) )
							pAction->incConnections();
					}
				} else {
					// There is a key conflict.  A full update
					//  check is necessary.
					// TODO: make this more efficient where possible.
					if( m_mapKeyToAction[key].pAction != pAction
					    && m_mapKeyToAction[key].pAction != 0 ) {
						kdDebug(125) << "Key conflict: call updateConnections():" 
							<< " key = " << key.key().toStringInternal() << endl;
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

bool KAccelBase::removeConnection( KAccelAction* pAction )
{
	kdDebug(125) << "KAccelBase::removeConnection( " << pAction << " = \"" << pAction->m_sName << "\"; shortcut = " << pAction->m_cut.toStringInternal() << " ): this = " << this << endl;

	//for( KKeyToActionMap::iterator it = m_mapKeyToAction.begin(); it != m_mapKeyToAction.end(); ++it )
	//	kdDebug(125) << "\tKey: " << it.key().toString() << " => '" << (*it)->m_sName << "'" << " " << *it << endl;

	if( m_rgActionsNonUnique.findIndex( pAction ) >= 0 ) {
		mtemp_pActionRemoving = pAction;
		bool b = updateConnections();
		mtemp_pActionRemoving = 0;
		return b;
	}

	KKeyToActionMap::iterator it = m_mapKeyToAction.begin();
	while( it != m_mapKeyToAction.end() ) {
		KKeyServer::Key key = it.key();
		ActionInfo* pInfo = &(*it);

		// If the given action is connected to this key,
		if( pAction == pInfo->pAction ) {
			disconnectKey( *pAction, key );
			pAction->decConnections();

			KKeyToActionMap::iterator itRemove = it++;
			m_mapKeyToAction.remove( itRemove );
		} else
			it++;
	}
	return true;
}

bool KAccelBase::setShortcut( const QString& sAction, const KShortcut& cut )
{
	KAccelAction* pAction = actionPtr( sAction );
	if( pAction ) {
		if( m_bAutoUpdate )
			removeConnection( pAction );

		pAction->setShortcut( cut );

		if( m_bAutoUpdate && !pAction->shortcut().isNull() )
			insertConnection( pAction );
		return true;
	} else
		return false;
}

void KAccelBase::readSettings( KConfigBase* pConfig )
{
	m_rgActions.readActions( m_sConfigGroup, pConfig );
	if( m_bAutoUpdate )
		updateConnections();
}

void KAccelBase::writeSettings( KConfigBase* pConfig ) const
{
	m_rgActions.writeActions( m_sConfigGroup, pConfig, m_bConfigIsGlobal, m_bConfigIsGlobal );
}

QPopupMenu* KAccelBase::createPopupMenu( QWidget* pParent, const KKeySequence& seq )
{
	KShortcutMenu* pMenu = new KShortcutMenu( pParent, &actions(), seq );
	
	bool bActionInserted = false;
	bool bInsertSeparator = false;
	for( uint i = 0; i < actionCount(); i++ ) {
		const KAccelAction* pAction = actions().actionPtr( i );

		if( !pAction->isEnabled() )
			continue;

		// If an action has already been inserted into the menu
		//  and we have a label instead of an action here,
		//  then indicate that we should insert a separator before the next menu entry.
		if( bActionInserted && !pAction->isConfigurable() && pAction->name().contains( ':' ) )
			bInsertSeparator = true;

		for( uint iSeq = 0; iSeq < pAction->shortcut().count(); iSeq++ ) {
			const KKeySequence& seqAction = pAction->shortcut().seq(iSeq);
			if( seqAction.startsWith( seq ) ) {
				if( bInsertSeparator ) {
					pMenu->insertSeparator();
					bInsertSeparator = false;
				}

				pMenu->insertAction( i, seqAction );

				//kdDebug(125) << "sLabel = " << sLabel << ", seq = " << (QString)seqMenu.qt() << ", i = " << i << endl;
				//kdDebug(125) << "pMenu->accel(" << i << ") = " << (QString)pMenu->accel(i) << endl;
				bActionInserted = true;
				break;
			}
		}
	}
	pMenu->updateShortcuts();
	return pMenu;
}

void KAccelBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }
