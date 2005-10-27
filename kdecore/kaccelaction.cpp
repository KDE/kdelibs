/*
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997-2000 Nicolas Hadacek <hadacek@kde.org>
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kaccelaction.h"
#include "kaccelbase.h"   // for KAccelBase::slotRemoveAction() & emitSignal()

#include <qnamespace.h>

#include <kconfig.h>
#include "kckey.h"
#include <kdebug.h>
#include <kglobal.h>
#include <kkeynative.h>
#include <klocale.h>
#include <kshortcutlist.h>

//---------------------------------------------------------------------
// KAccelAction
//---------------------------------------------------------------------

class KAccelActionPrivate
{
 public:
	uint m_nConnections;
};

KAccelAction::KAccelAction()
{
	//kdDebug(125) << "KAccelAction(): this = " << this << endl;
	d = new KAccelActionPrivate;
	m_pObjSlot = 0;
	m_psMethodSlot = 0;
	m_bConfigurable = true;
	m_bEnabled = true;
	m_nIDAccel = 0;
	d->m_nConnections = 0;
}

KAccelAction::KAccelAction( const KAccelAction& action )
{
	//kdDebug(125) << "KAccelAction( copy from \"" << action.m_sName << "\" ): this = " << this << endl;
	d = new KAccelActionPrivate;
	*this = action;
}

KAccelAction::KAccelAction( const QString& sName, const QString& sLabel, const QString& sWhatsThis,
			const KShortcut& cutDef, const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "KAccelAction( \"" << sName << "\" ): this = " << this << endl;
	d = new KAccelActionPrivate;
	init( sName, sLabel, sWhatsThis,	cutDef, pObjSlot, psMethodSlot, bConfigurable, bEnabled );
}


KAccelAction::~KAccelAction()
{
	//kdDebug(125) << "\t\t\tKAccelAction::~KAccelAction( \"" << m_sName << "\" ): this = " << this << endl;
	delete d;
}

void KAccelAction::clear()
{
	m_cut.clear();
	m_pObjSlot = 0;
	m_psMethodSlot = 0;
	m_bConfigurable = true;
	m_bEnabled = true;
	m_nIDAccel = 0;
	d->m_nConnections = 0;
}

bool KAccelAction::init( const QString& sName, const QString& sLabel, const QString& sWhatsThis,
			const KShortcut& rgCutDefaults, const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	m_sName = sName;
	m_sLabel = sLabel;
	m_sWhatsThis = sWhatsThis;
	m_cutDefault = rgCutDefaults;
	m_pObjSlot = pObjSlot;
	m_psMethodSlot = psMethodSlot;
	m_bConfigurable = bConfigurable;
	m_bEnabled = bEnabled;
	m_nIDAccel = 0;
	m_cut = shortcutDefault();
	d->m_nConnections = 0;
	if( !m_bEnabled )
		kdDebug(125) << "KAccelAction::init( \"" << sName << "\" ): created with enabled = false" << endl;
	return true;
}

KAccelAction& KAccelAction::operator =( const KAccelAction& action )
{
	m_sName          = action.m_sName;
	m_sLabel         = action.m_sLabel;
	m_sWhatsThis     = action.m_sWhatsThis;
	//m_cutDefault3    = action.m_cutDefault3;
	//m_cutDefault4    = action.m_cutDefault4;
	m_cutDefault     = action.m_cutDefault;
	m_pObjSlot       = action.m_pObjSlot;
	m_psMethodSlot   = action.m_psMethodSlot;
	m_bConfigurable  = action.m_bConfigurable;
	m_bEnabled       = action.m_bEnabled;
	m_nIDAccel       = action.m_nIDAccel;
	m_cut            = action.m_cut;
	d->m_nConnections = action.d->m_nConnections;

	return *this;
}

void KAccelAction::setName( const QString& s )
	{ m_sName = s; }
void KAccelAction::setLabel( const QString& s )
	{ m_sLabel = s; }
void KAccelAction::setWhatsThis( const QString& s )
	{ m_sWhatsThis = s; }

bool KAccelAction::setShortcut( const KShortcut& cut )
{
	m_cut = cut;
	return true;
}

void KAccelAction::setSlot( const QObject* pObjSlot, const char* psMethodSlot )
{
	m_pObjSlot = pObjSlot;
	m_psMethodSlot = psMethodSlot;
}

void KAccelAction::setConfigurable( bool b )
	{ m_bConfigurable = b; }
void KAccelAction::setEnabled( bool b )
	{ m_bEnabled = b; }

QString KAccelAction::toString() const
	{ return m_cut.toString(); }

QString KAccelAction::toStringInternal() const
	{ return m_cut.toStringInternal( &shortcutDefault() ); }

bool KAccelAction::setKeySequence( uint i, const KKeySequence& seq )
{
	if( i < m_cut.count() ) {
		m_cut.setSeq( i, seq );
		return true;
	} else if( i == m_cut.count() )
		return m_cut.append( seq );
	return false;
}

void KAccelAction::clearShortcut()
{
	m_cut.clear();
}

bool KAccelAction::contains( const KKeySequence& seq )
{
	return m_cut.contains( seq );
	for( uint i = 0; i < m_cut.count(); i++ ) {
		if( m_cut.seq(i) == seq )
			return true;
	}
	return false;
}

bool KAccelAction::isConnected() const
	{ return d->m_nConnections; }
void KAccelAction::incConnections()
	{ d->m_nConnections++; }
void KAccelAction::decConnections()
	{ if( d->m_nConnections > 0 ) d->m_nConnections--; }

//---------------------------------------------------------------------
// KAccelActions
//---------------------------------------------------------------------

class KAccelActionsPrivate
{
 public:
};

KAccelActions::KAccelActions()
{
	kdDebug(125) << "KAccelActions(): this = " << this << endl;
	initPrivate( 0 );
}

KAccelActions::KAccelActions( const KAccelActions& actions )
{
	kdDebug(125) << "KAccelActions( actions = " << &actions << " ): this = " << this << endl;
	initPrivate( 0 );
	init( actions );
}

KAccelActions::KAccelActions( KAccelBase* pKAccelBase )
{
	kdDebug(125) << "KAccelActions( KAccelBase = " << pKAccelBase << " ): this = " << this << endl;
	initPrivate( pKAccelBase );
}

KAccelActions::~KAccelActions()
{
	//kdDebug(125) << "KAccelActions::~KAccelActions(): this = " << this << endl;
	clear();
}

void KAccelActions::initPrivate( KAccelBase* pKAccelBase )
{
	m_pKAccelBase = pKAccelBase;
}

void KAccelActions::clear()
{
	while(!m_actions.empty())
		delete m_actions.takeFirst();
}

bool KAccelActions::init( const KAccelActions& actions )
{
	clear();

	for( int i = 0; i < actions.count() ; i++ ) {
		const KAccelAction *pAction = actions.actionPtr(i);
		if( pAction )
			m_actions[i] = new KAccelAction( *pAction );
		else
			m_actions[i] = 0;
	}

	return true;
}

bool KAccelActions::init( KConfigBase& config, const QString& sGroup )
{
	kdDebug(125) << "KAccelActions::init( " << sGroup << " )" << endl;
	QMap<QString, QString> mapEntry = config.entryMap( sGroup );
	resize( mapEntry.count() );

	QMap<QString, QString>::Iterator it( mapEntry.begin() );
	for( uint i = 0; it != mapEntry.end(); ++it, i++ ) {
		QString sShortcuts = *it;
		KShortcut cuts;

		kdDebug(125) << it.key() << " = " << sShortcuts << endl;
		if( !sShortcuts.isEmpty() && sShortcuts != "none" )
			cuts.init( sShortcuts );

		m_actions[i] = new KAccelAction( it.key(), it.key(), it.key(),
//			cuts, cuts,
			cuts,
			0, 0,          // pObjSlot, psMethodSlot,
			true, false ); // bConfigurable, bEnabled
	}

	return true;
}

void KAccelActions::resize( uint nSize )
{
	Q_UNUSED(nSize)
	/*
	if( nSize > m_nSizeAllocated ) {
		uint nSizeAllocated = ((nSize/10) + 1) * 10;
		KAccelAction** prgActions = new KAccelAction* [nSizeAllocated];

		// Copy pointers over to new array
		for( uint i = 0; i < m_nSizeAllocated; i++ )
			prgActions[i] = m_prgActions[i];

		// Null out new pointers
		for( uint i = m_nSizeAllocated; i < nSizeAllocated; i++ )
			prgActions[i] = 0;

		delete[] m_prgActions;
		m_prgActions = prgActions;
		m_nSizeAllocated = nSizeAllocated;
	}

	m_nSize = nSize;
	*/
}

void KAccelActions::insertPtr( KAccelAction* pAction )
{
	m_actions.append(pAction);
}

void KAccelActions::updateShortcuts( KAccelActions& actions2 )
{
	kdDebug(125) << "KAccelActions::updateShortcuts()" << endl;
	bool bChanged = false;

	for( int i = 0; i < m_actions.size(); i++ ) {
		KAccelAction* pAction = m_actions[i];
		if( pAction && pAction->m_bConfigurable ) {
			KAccelAction* pAction2 = actions2.actionPtr( pAction->m_sName );
			if( pAction2 ) {
				QString sOld = pAction->m_cut.toStringInternal();
				pAction->m_cut = pAction2->m_cut;
				kdDebug(125) << "\t" << pAction->m_sName
					<< " found: " << sOld
					<< " => " << pAction2->m_cut.toStringInternal()
					<< " = " << pAction->m_cut.toStringInternal() << endl;
				bChanged = true;
			}
		}
	}

	if( bChanged )
		emitKeycodeChanged();
}

int KAccelActions::actionIndex( const QString& sAction ) const
{
	for( int i = 0; i < m_actions.size() ; i++ ) {
		if( m_actions.at(i) == 0 )
			kdWarning(125) << "KAccelActions::actionPtr( " << sAction << " ): encountered null pointer at m_prgActions[" << i << "]" << endl;
		else if( m_actions.at(i)->m_sName == sAction )
			return (int) i;
	}
	return -1;
}

KAccelAction* KAccelActions::actionPtr( uint i )
{
	return m_actions[i];
}

const KAccelAction* KAccelActions::actionPtr( uint i ) const
{
	return m_actions.at(i);
}

KAccelAction* KAccelActions::actionPtr( const QString& sAction )
{
	int i = actionIndex( sAction );
	return (i >= 0) ? m_actions[i] : 0;
}

const KAccelAction* KAccelActions::actionPtr( const QString& sAction ) const
{
	int i = actionIndex( sAction );
	return (i >= 0) ? m_actions.at(i) : 0;
}

KAccelAction* KAccelActions::actionPtr( KKeySequence cut )
{
	for( int i = 0; i < m_actions.size(); i++ ) {
		if( m_actions[i] == 0 )
			kdWarning(125) << "KAccelActions::actionPtr( " << cut.toStringInternal() << " ): encountered null pointer at m_prgActions[" << i << "]" << endl;
		else if( m_actions[i]->contains( cut ) )
			return m_actions[i];
	}
	return 0;
}

KAccelAction& KAccelActions::operator []( uint i )
{
	return *actionPtr( i );
}

const KAccelAction& KAccelActions::operator []( uint i ) const
{
	return *actionPtr( i );
}

KAccelAction* KAccelActions::insert( const QString& sName, const QString& sLabel )
{
	if( actionPtr( sName ) ) {
		kdWarning(125) << "KAccelActions::insertLabel( " << sName << ", " << sLabel << " ): action with same name already present." << endl;
		return 0;
	}

	KAccelAction* pAction = new KAccelAction;
	pAction->m_sName = sName;
	pAction->m_sLabel = sLabel;
	pAction->m_bConfigurable = false;
	pAction->m_bEnabled = false;

	insertPtr( pAction );
	return pAction;
}

KAccelAction* KAccelActions::insert( const QString& sAction, const QString& sLabel, const QString& sWhatsThis,
			const KShortcut& rgCutDefaults, const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "KAccelActions::insert()2 begin" << endl;
	if( actionPtr( sAction ) ) {
		kdWarning(125) << "KAccelActions::insert( " << sAction << " ): action with same name already present." << endl;
		return 0;
	}

	KAccelAction* pAction = new KAccelAction(
		sAction, sLabel, sWhatsThis,
		rgCutDefaults,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
	insertPtr( pAction );

	//kdDebug(125) << "KAccelActions::insert()2 end" << endl;
	return pAction;
}

bool KAccelActions::remove( const QString& sAction )
{
	kdDebug(125) << "KAccelActions::remove( \"" << sAction << "\" ): this = " << this << " m_pKAccelBase = " << m_pKAccelBase << endl;

	int iAction = actionIndex( sAction );
	if( iAction < 0 )
		return false;

	if( m_pKAccelBase )
		m_pKAccelBase->slotRemoveAction( m_actions[iAction] );
	
	delete m_actions.takeAt(iAction);
	return true;
}

bool KAccelActions::readActions( const QString& sConfigGroup, KConfigBase* pConfig )
{
	KAccelShortcutList accelList(*this, false);
	return accelList.readSettings( sConfigGroup, pConfig );
}

/*
	1) KAccelAction = "Something"
		1) KKeySequence = "Meta+X,Asterisk"
			1) KAccelSequence = "Meta+X"
				1) KKeySequence = Meta+X
			2) KAccelSequence = "Asterisk"
				1) KKeySequence = Shift+8 (English layout)
				2) KKeySequence = Keypad_Asterisk
		2) KKeySequence = "Alt+F2"
			1) KAccelSequence = "Alt+F2"
				1) KKeySequence = Alt+F2
	-> "Something=Meta+X,Asterisk;Alt+F2"
*/
bool KAccelActions::writeActions( const QString &sGroup, KConfigBase* pConfig,
			bool bWriteAll, bool bGlobal ) const
{
	kdDebug(125) << "KAccelActions::writeActions( " << sGroup << ", " << pConfig << ", " << bWriteAll << ", " << bGlobal << " )" << endl;
	if( !pConfig )
		pConfig = KGlobal::config();
	KConfigGroup cg( pConfig, sGroup );

	for( int i = 0; i < m_actions.size() ; i++ ) {
		if( m_actions.at(i) == 0 ) {
			kdWarning(125) << "KAccelActions::writeActions(): encountered null pointer at m_prgActions[" << i << "]" << endl;
			continue;
		}
		const KAccelAction& action = *m_actions.at(i);

		QString s;
		bool bConfigHasAction = !cg.readEntry( action.m_sName ).isEmpty();
		bool bSameAsDefault = true;
		bool bWriteAction = false;

		if( action.m_bConfigurable ) {
			s = action.toStringInternal();
			bSameAsDefault = (action.m_cut == action.shortcutDefault());

			//if( bWriteAll && s.isEmpty() )
			if( s.isEmpty() )
				s = "none";

			// If we're using a global config or this setting
			//  differs from the default, then we want to write.
			if( bWriteAll || !bSameAsDefault )
				bWriteAction = true;

			if( bWriteAction ) {
				kdDebug(125) << "\twriting " << action.m_sName << " = " << s << endl;
				// Is passing bGlobal irrelevant, since if it's true,
				//  then we're using the global config anyway? --ellis
				cg.writeEntry( action.m_sName, s, true, bGlobal );
			}
			// Otherwise, this key is the same as default
			//  but exists in config file.  Remove it.
			else if( bConfigHasAction ) {
				kdDebug(125) << "\tremoving " << action.m_sName << " because == default" << endl;
				cg.deleteEntry( action.m_sName, bGlobal );
			}

		}
	}

	pConfig->sync();
	return true;
}

void KAccelActions::emitKeycodeChanged()
{
	if( m_pKAccelBase )
		m_pKAccelBase->emitSignal( KAccelBase::KEYCODE_CHANGED );
}

int KAccelActions::count() const
{ 
	return m_actions.size(); 
}
