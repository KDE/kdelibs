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
//#include <qlayout.h>
//#include <qpainter.h>
//#include <qdrawutil.h>
//#include <qpopupmenu.h>

#include <kconfig.h>
#include <kckey.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kkeysequence.h>
#include <klocale.h>
#include <kshortcuts.h>

#ifdef Q_WS_X11
//#include "kkey_x11.h"
#endif

//----------------------------------------------------

KAccelSequence::KAccelSequence()
	{ }
KAccelSequence::KAccelSequence( const KAccelSequence& seq )
	{ m_rgKeys = seq.m_rgKeys; }
KAccelSequence::KAccelSequence( KKeySequence key )
	{ setKey( key ); }
	
KAccelSequence::KAccelSequence( const QString& s )
{
	m_rgKeys = KKeySequence::stringToKeys( s );
}

KAccelSequence::~KAccelSequence()
	{ }

uint KAccelSequence::count() const { return m_rgKeys.size(); }
QString KAccelSequence::toString( KKeySequence::I18N bi18n ) const { return getKey().toString( bi18n ); }
KKeySequences::iterator KAccelSequence::begin() { return m_rgKeys.begin(); }
KKeySequences::iterator KAccelSequence::end() { return m_rgKeys.end(); }

KKeySequence KAccelSequence::getKey( uint i ) const
{
	KKeySequence key;
	if( i < m_rgKeys.size() )
		key = m_rgKeys[i];
	return key;
}

void KAccelSequence::setKey( const KKeySequence& key )
{
	m_rgKeys.resize( 1 );
	m_rgKeys[0] = key;
}

// FIXME: this is acually equal() and not compare()
int KAccelSequence::compare( KAccelSequence& a, KAccelSequence& b )
{
	//kdDebug(125) << "KAccelSequence::compare()" << endl;
	return !(a.m_rgKeys == b.m_rgKeys);
}

bool KAccelSequence::operator ==( KAccelSequence& seq )
{
	return m_rgKeys == seq.m_rgKeys;
}

bool KAccelSequence::operator ==( KAccelSequence& seq ) const
{
	KKeySequences a = m_rgKeys;
	return a == seq.m_rgKeys;
}

bool KAccelSequence::operator ==( const KAccelSequence& seq )
{
	KKeySequences b = seq.m_rgKeys;
	return m_rgKeys == b;
}

bool KAccelSequence::operator ==( const KAccelSequence& seq ) const
{
	KKeySequences a = m_rgKeys;
	KKeySequences b = seq.m_rgKeys;
	return a == b;
}

//----------------------------------------------------

KAccelShortcut::KAccelShortcut()
{
}

KAccelShortcut::KAccelShortcut( const KAccelShortcut& cut )
{
	m_rgSequences = cut.m_rgSequences;
}

KAccelShortcut::KAccelShortcut( const QString& s )
{
	init( s );
}

KAccelShortcut::~KAccelShortcut()
{
}

bool KAccelShortcut::init( const KAccelShortcut& cut )
{
	//m_rgSequences.resize( cut.m_rgSequences.size() );
	m_rgSequences = cut.m_rgSequences;
	return true;
}

bool KAccelShortcut::init( KKeySequence key )
{
	m_rgSequences.resize( 1 );
	m_rgSequences[0] = KAccelSequence( key );
	return true;
}

bool KAccelShortcut::init( const QString& s )
{
	QStringList rgs = QStringList::split( ',', s );
	m_rgSequences.resize( rgs.size() );
	for( uint i = 0; i < m_rgSequences.size(); i++ )
		m_rgSequences[i] = rgs[i];
	return true;
}

uint KAccelShortcut::count() const
{
	return m_rgSequences.size();
}

QString KAccelShortcut::toString( KKeySequence::I18N bi18n ) const
{
	QString s;

	for( uint i = 0; i < count(); i++ ) {
		s += m_rgSequences[i].toString( bi18n );
		if( i < m_rgSequences.size() - 1 )
			s += ',';
	}

	return s;
}

KAccelSequence KAccelShortcut::getSequence( uint i ) const
{
	return (i < count()) ? m_rgSequences[i] : KKeySequence();
}

KAccelSequences::iterator KAccelShortcut::begin()  { return m_rgSequences.begin(); }
KAccelSequences::iterator KAccelShortcut::end()    { return m_rgSequences.end(); }
KAccelSequences::reference KAccelShortcut::front() { return m_rgSequences.front(); }

bool KAccelShortcut::equals( KAccelShortcut& cut )
{
	//kdDebug(125) << "== KAccelShortcut::equals() <non-const>" << endl;
	if( m_rgSequences.size() != cut.m_rgSequences.size() )
		return false;

	for( uint i = 0; i < m_rgSequences.size(); i++ ) {
		if( KAccelSequence::compare( m_rgSequences[i], cut.m_rgSequences[i] ) != 0 )
			return false;
	}
	return true;
	//KAccelSequences seqs( m_rgSequences );
	//return seqs == cut.m_rgSequences;
}

bool KAccelShortcut::equals( const KAccelShortcut& cut ) const
{
	//kdDebug(125) << "KAccelShortcut::equals() const" << endl;
	if( m_rgSequences.size() != cut.m_rgSequences.size() )
		return false;

	for( uint i = 0; i < m_rgSequences.size(); i++ ) {
		KAccelSequence a = m_rgSequences[i];
		KAccelSequence b = cut.m_rgSequences[i];
		if( KAccelSequence::compare( a, b ) != 0 )
			return false;
	}
	return true;
	//KAccelSequences seqs( m_rgSequences );
	//return seqs == cut.m_rgSequences;
}

bool KAccelShortcut::operator ==( KAccelShortcut& cut )
{
	//kdDebug(125) << "== non-const" << endl;
	return this->equals( cut );
}

bool KAccelShortcut::operator ==( const KAccelShortcut& cut ) const
{
	//kdDebug(125) << "== const" << endl;
	return this->equals( cut );
}

//----------------------------------------------------

KAccelShortcuts::KAccelShortcuts()
	{ }
KAccelShortcuts::KAccelShortcuts( const QString& s )
	{ init( s ); }
KAccelShortcuts::KAccelShortcuts( KKeySequence key )
	{ init( key ); }
KAccelShortcuts::~KAccelShortcuts()
	{ }

bool KAccelShortcuts::init( const KAccelShortcuts& cuts )
{
	//kdDebug(125) << "KAccelShortcuts::init() -- cuts.size() = " << cuts.size() << endl;
	// *(int*)0 = 0;
	resize( cuts.size() );
	//kdDebug(125) << "KAccelShortcuts::init() B" << endl;
	for( uint i = 0; i < size(); i++ )
		at(i).init( cuts[i] );
	return true;
}

bool KAccelShortcuts::init( const QString& s )
{
	QStringList rgs = QStringList::split( ';', s );
	resize( rgs.size() );
	for( uint i = 0; i < rgs.size(); i++ ) {
		QString& sCut = rgs[i];
		if( sCut.startsWith( "default(" ) )
			sCut = sCut.mid( 8, sCut.length() - 9 );
		at(i) = KAccelShortcut( sCut );
	}
	return true;
}

bool KAccelShortcuts::init( KKeySequence key )
{
	resize( 1 );
	at(0).init( key );
	return true;
}

QString KAccelShortcuts::toString( KKeySequence::I18N bi18n, const KAccelShortcuts* prgCutDefaults ) const
{
	QString s;

	for( uint i = 0; i < size(); i++ ) {
		const KAccelShortcut& cut = at(i);
		if( prgCutDefaults && i < prgCutDefaults->size() && cut == (*prgCutDefaults)[i] ) {
			s += "default(";
			s += cut.toString( bi18n );
			s += ")";
		} else
			s += cut.toString( bi18n );
		if( i < size() - 1 )
			s += ';';
	}

	return s;
}

KAccelShortcut KAccelShortcuts::getShortcut( uint i ) const
	{ return (i < size()) ? operator[]( i ) : KAccelShortcut(); }

bool KAccelShortcuts::equal( KAccelShortcuts& a, KAccelShortcuts& b )
{
	//kdDebug(125) << "KAccelShortcuts::equal()" << endl;

	if( a.size() != b.size() )
		return false;

	for( uint i = 0; i < a.size(); i++ ) {
		if( !a[i].equals( b[i] ) )
			return false;
	}
	return true;
}

bool KAccelShortcuts::operator ==( KAccelShortcuts& cuts )
{
	return equal( *this, cuts );
}

bool KAccelShortcuts::operator ==( const KAccelShortcuts& cuts )
{
	KAccelShortcuts b( cuts );
	return equal( *this, b );
}

bool KAccelShortcuts::operator ==( KAccelShortcuts& cuts ) const
{
	KAccelShortcuts a( *this );
	return equal( a, cuts );
}

bool KAccelShortcuts::operator ==( const KAccelShortcuts& cuts ) const
{
	KAccelShortcuts a( *this );
	KAccelShortcuts b( cuts );
	return equal( a, b );
}

//----------------------------------------------------

class KShortcutsPrivate : public KAccelShortcuts
{
};

KShortcuts::KShortcuts()
{
	d = new KShortcutsPrivate();
}

KShortcuts::KShortcuts( const KShortcuts& cuts )
{
	d = new KShortcutsPrivate();
	d->init( *cuts.d );
}

KShortcuts::KShortcuts( const QString& s )
{
	d = new KShortcutsPrivate();
	d->init( s );
}

KShortcuts::KShortcuts( int qkey )
{
	KKeySequence key( qkey );
	d = new KShortcutsPrivate();
	d->init( key );
}

KShortcuts::KShortcuts( QKeySequence qkey )
{
	KKeySequence key( qkey );
	d = new KShortcutsPrivate();
	d->init( key );
}

KShortcuts::KShortcuts( KKeySequence key )
{
	d = new KShortcutsPrivate();
	d->init( key );
}

KShortcuts::~KShortcuts()
	{ delete d; }
KShortcuts& KShortcuts::operator =( const KShortcuts& cuts )
	{ d->init( *cuts.d ); return *this; }
KAccelShortcuts& KShortcuts::base()
	{ return *d; }
KShortcuts::operator const KAccelShortcuts&() const
	{ return *d; }

//----------------------------------------------------

KAccelAction::KAccelAction()
{
	m_pObjSlot = 0;
	m_psMethodSlot = 0;
	m_bConfigurable = true;
	m_bEnabled = true;
	m_nIDAccel = 0;
	m_nIDMenu = 0;
	m_pMenu = 0;
}

KAccelAction::KAccelAction( const QString& sName, const QString& sDesc,
			const char* rgCutDefaults3, const char* rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled )
{
	init( sName, sDesc,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu,
		bConfigurable, bEnabled );
}

KAccelAction::KAccelAction( const QString& sName, const QString& sDesc,
			const KAccelShortcuts& rgCutDefaults3, const KAccelShortcuts& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled )
{
	init( sName, sDesc,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu,
		bConfigurable, bEnabled );
}

KAccelAction::~KAccelAction()
{
}

bool KAccelAction::init( const QString& sName, const QString& sDesc,
			const char* rgCutDefaults3, const char* rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled )
{
	m_sName = sName;
	m_sDesc = sDesc;
	//kdDebug(125) << "KAccelAction::init() B" << endl;
	m_rgCutDefaults3.init( rgCutDefaults3 );
	//kdDebug(125) << "KAccelAction::init() C" << endl;
	m_rgCutDefaults4.init( rgCutDefaults4 );
	m_pObjSlot = pObjSlot;
	m_psMethodSlot = psMethodSlot;
	m_bConfigurable = bConfigurable;
	m_bEnabled = bEnabled;
	m_nIDMenu = nIDMenu;
	m_pMenu = pMenu;
	m_rgShortcuts = shortcutDefaults();
	//kdDebug(125) << "KAccelAction::init() D" << endl;
	return true;
}

bool KAccelAction::init( const QString& sName, const QString& sDesc,
			const KAccelShortcuts& rgCutDefaults3, const KAccelShortcuts& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled )
{
	m_sName = sName;
	m_sDesc = sDesc;
	//kdDebug(125) << "KAccelAction::init() B" << endl;
	m_rgCutDefaults3 = rgCutDefaults3;
	//kdDebug(125) << "KAccelAction::init() C" << endl;
	m_rgCutDefaults4 = rgCutDefaults4;
	m_pObjSlot = pObjSlot;
	m_psMethodSlot = psMethodSlot;
	m_bConfigurable = bConfigurable;
	m_bEnabled = bEnabled;
	m_nIDMenu = nIDMenu;
	m_pMenu = pMenu;
	m_rgShortcuts = shortcutDefaults();
	//kdDebug(125) << "KAccelAction::init() C" << endl;
	return true;
}

void KAccelAction::clear()
	{ m_rgShortcuts.clear(); }

uint KAccelAction::shortcutCount() const
{
	return m_rgShortcuts.size();
}

KAccelShortcuts::iterator KAccelAction::begin()  { return m_rgShortcuts.begin(); }
KAccelShortcuts::iterator KAccelAction::end()    { return m_rgShortcuts.end(); }
//KAccelShortcuts::reference KAccelAction::front() { return m_rgShortcuts.front(); }

/*KKeySequence& KAccelAction::key( uint iShortcut, uint iSequence, uint iKey )
{
	if( iShortcut < shortcutCount() ) {
		KAccelShortcut& cut = m_rgShortcuts[i];
		if( iSequence < cut.count() ) {
			KAccelSequence& seq = cut.m_rgSequences[iSequence];
			if( iKey < seq.count() ) {
				return Keys
			}
	}
}*/

const KAccelShortcuts& KAccelAction::shortcutDefaults() const
	{ return (KKeySequence::useFourModifierKeys()) ? m_rgCutDefaults4 : m_rgCutDefaults3; }

KAccelShortcut KAccelAction::getShortcut( uint i ) const
	{ return (i < shortcutCount()) ? m_rgShortcuts[i] : KAccelShortcut(); }

QString KAccelAction::toString( KKeySequence::I18N bi18n ) const
	{ return m_rgShortcuts.toString( bi18n, &shortcutDefaults() ); }

/*KAccelShortcut* KAccelAction::insertShortcut( const KAccelShortcut& cut )
{
	m_rgShortcuts.push_back( cut );
	return &m_rgShortcuts.back();
}*/

bool KAccelAction::setShortcuts( const KAccelShortcuts& rgCuts )
{
	m_rgShortcuts = rgCuts;
	return true;
}

bool KAccelAction::setShortcut( uint i, const KAccelShortcut& cut )
{
	if( i < shortcutCount() )
		m_rgShortcuts[i] = cut;
	else if( i == shortcutCount() )
		m_rgShortcuts.push_back( cut );
	else
		return false;
	return true;
}

void KAccelAction::clearShortcuts()
{
	m_rgShortcuts.clear();
}

bool KAccelAction::contains( KAccelShortcut& cut )
{
	for( uint i = 0; i < m_rgShortcuts.size(); i++ ) {
		if( m_rgShortcuts[i] == cut )
			return true;
	}
	return false;
}

//----------------------------------------------------

bool KAccelActions::init( KAccelActions& actions )
{
	*static_cast<QValueVector<KAccelAction>*>(this)
		= static_cast<QValueVector<KAccelAction>&>(actions);
	return true;
}

bool KAccelActions::init( KConfigBase& config, QString sGroup )
{
	QMap<QString, QString> mapEntry = config.entryMap( sGroup );

	QMap<QString, QString>::Iterator it( mapEntry.begin() );
	for( ; it != mapEntry.end(); ++it ) {
		QString sShortcuts = *it;
		if( !sShortcuts.isEmpty() ) {
			kdDebug(125) << it.key() << " = " << sShortcuts << endl;
			KAccelAction action;
			action.m_sName = it.key();
			action.setShortcuts( KAccelShortcuts( sShortcuts ) );
			push_back( action );
		}
	}
}

void KAccelActions::updateShortcuts( KAccelActions& actions2 )
{
	kdDebug(125) << "KAccelActions::updateShortcuts()" << endl;
	for( KAccelActions::iterator it = begin(); it != end(); ++it ) {
		KAccelAction* pAction2 = actions2.actionPtr( (*it).m_sName );
		if( pAction2 ) {
			QString sOld( (*it).m_rgShortcuts.toString() );
			(*it).m_rgShortcuts = pAction2->m_rgShortcuts;
			kdDebug(125) << "\t" << (*it).m_sName
				<< " found: " << sOld
				<< " => " << pAction2->m_rgShortcuts.toString()
				<< " = " << (*it).m_rgShortcuts.toString() << endl;
		}
	}
}

KAccelActions::iterator KAccelActions::actionIterator( const QString& sAction )
{
	KAccelActions::iterator it = begin();
	for( ; it != end(); ++it ) {
		if( (*it).m_sName == sAction )
			break;
	}
	return it;
}

KAccelAction* KAccelActions::actionPtr( const QString& sAction )
{
	KAccelActions::iterator it = actionIterator( sAction );
	if( it != end() )
		return &(*it);
	else
		return 0;
}

KAccelAction* KAccelActions::actionPtr( KAccelShortcut cut )
{
	for( KAccelActions::iterator it = begin(); it != end(); ++it ) {
		if( (*it).contains( cut ) )
			return &(*it);
	}
	return 0;
}

bool KAccelActions::insertLabel( const QString& sName, const QString& sDesc )
{
	if( actionPtr( sName ) )
		return false;

	kdDebug(125) << "KAccelActions::insertLabel() A" << endl;
	resize( size() + 1 );
	kdDebug(125) << "KAccelActions::insertLabel() B" << endl;
	KAccelAction& action = back();
	action.m_sName = sName;
	action.m_sDesc = sDesc;
	action.m_bConfigurable = false;
	action.m_bEnabled = false;

	return true;
}

KAccelAction* KAccelActions::insertAction( const QString& sAction, const QString& sDesc,
			const char* rgCutDefaults3, const char* rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "KAccelActions::insertAction() begin" << endl;
	//kdDebug(125) << QString( "insertItem("+sAction+", 0x%1, 0x%2)\n" ).arg(keyDef3.key(),0,16).arg(keyDef4.key(),0,16);
	if( actionPtr( sAction ) )
		return false;

	//kdDebug(125) << "KAccelActions::insertAction() resize" << endl;
	resize( size() + 1 );
	//kdDebug(125) << "KAccelActions::insertAction() back().init" << endl;
	back().init( sAction, sDesc,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu,
		bConfigurable, bEnabled );
	//kdDebug(125) << "KAccelActions::insertAction() end" << endl;

	return &back();
}

KAccelAction* KAccelActions::insertAction( const QString& sAction, const QString& sDesc,
			const KAccelShortcuts& rgCutDefaults3, const KAccelShortcuts& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "KAccelActions::insertAction() begin" << endl;
	//kdDebug(125) << QString( "insertItem("+sAction+", 0x%1, 0x%2)\n" ).arg(keyDef3.key(),0,16).arg(keyDef4.key(),0,16);
	if( actionPtr( sAction ) )
		return false;

	resize( size() + 1 );
	back().init( sAction, sDesc,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu,
		bConfigurable, bEnabled );
	//kdDebug(125) << "KAccelActions::insertAction() end" << endl;

	return &back();
}

bool KAccelActions::removeAction( const QString& sAction )
{
	iterator it = actionIterator( sAction );
	if( it != end() ) {
		erase( it );
		return true;
	} else
		return false;
}

void KAccelActions::readActions( const QString& sConfigGroup, KConfigBase* pConfig )
{
	kdDebug(125) << "readActions start" << endl;
	if( !pConfig )
		pConfig = KGlobal::config();
	KConfigGroupSaver cgs( pConfig, sConfigGroup );

	for( iterator it = begin(); it != end(); ++it ) {
		KAccelAction& action = *it;

		QString sEntry = pConfig->readEntry( action.m_sName );
		if( !sEntry.isNull() ) {
			if( sEntry == "none" )
				action.clearShortcuts();
			else
				action.m_rgShortcuts.init( sEntry );
		}

		kdDebug(125) << "\t" << action.m_sName << " = '" << sEntry << "'" << endl;
	}

	kdDebug(125) << "readActions done" << endl;
}

/*
	1) KAccelAction = "Something"
		1) KAccelShortcut = "Meta+X,Asterisk"
			1) KAccelSequence = "Meta+X"
				1) KKeySequence = Meta+X
			2) KAccelSequence = "Asterisk"
				1) KKeySequence = Shift+8 (English layout)
				2) KKeySequence = Keypad_Asterisk
		2) KAccelShortcut = "Alt+F2"
			1) KAccelSequence = "Alt+F2"
				1) KKeySequence = Alt+F2
	-> "Something=Meta+X,Asterisk;Alt+F2"
*/
void KAccelActions::writeActions( const QString &sGroup, KConfig *config,
			bool bWriteAll, bool bGlobal ) const
{
	KConfig *pConfig = config ? config : KGlobal::config();
	KConfigGroupSaver cs( pConfig, sGroup );

	for( const_iterator it = begin(); it != end(); ++it ) {
		const KAccelAction& action = *it;

		QString s;
		bool bConfigHasAction = !pConfig->readEntry( action.m_sName ).isEmpty();
		bool bSameAsDefault = true;
		bool bWriteAction = false;

		if( action.m_bConfigurable ) {
			s = action.toString( KKeySequence::I18N_No );
			bSameAsDefault = (action.m_rgShortcuts == action.shortcutDefaults());

			if( bWriteAll && s.isEmpty() )
				s = "none";

			// If we're using a global config or this setting
			//  differs from the default, then we want to write.
			if( bWriteAll || !bSameAsDefault )
				bWriteAction = true;
			// Otherwise, this key is the same as default
			//  but exists in config file.  Remove it.
			else if( bConfigHasAction ) {
				s = "";
				bWriteAction = true;
			}

			if( bWriteAction ) {
				kdDebug(125) << "writing " << action.m_sName << " = " << s << endl;
				pConfig->writeEntry( action.m_sName, s, true, bGlobal );
			}
		}
	}

	pConfig->sync();
}

//----------------------------------------------------

KAccelBase::KAccelBase()
{
    m_bEnabled = true;
    m_sConfigGroup = "Keys";
    m_bConfigIsGlobal = false;
    m_bAutoUpdate = false;
    d = 0;
}

KAccelBase::~KAccelBase()
{
}

uint KAccelBase::actionCount() const { return m_rgActions.size(); }
KAccelActions& KAccelBase::actions() { return m_rgActions; }
bool KAccelBase::isEnabled() const { return m_bEnabled; }

KAccelActions::iterator KAccelBase::actionIterator( const QString& sAction )
	{ return m_rgActions.actionIterator( sAction ); }

KAccelAction* KAccelBase::actionPtr( const QString& sAction )
	{ return m_rgActions.actionPtr( sAction ); }

const KAccelAction* KAccelBase::actionPtr( const QString& sAction ) const
{
	for( KAccelActions::const_iterator it = m_rgActions.begin(); it != m_rgActions.end(); ++it ) {
		if( (*it).m_sName == sAction )
			return &(*it);
	}
	return 0;
}

KAccelAction* KAccelBase::actionPtr( KKeySequence key )
{
	if( !m_mapKeyToAction.contains( key ) )
		return 0;
	return m_mapKeyToAction[key];
}

void KAccelBase::setConfigGroup( const QString& sConfigGroup )
	{ m_sConfigGroup = sConfigGroup; }

void KAccelBase::setConfigGlobal( bool global )
	{ m_bConfigIsGlobal = global; }

void KAccelBase::removeDeletedMenu( QPopupMenu* pMenu )
{
	for( KAccelActions::iterator it = m_rgActions.begin(); it != m_rgActions.end(); ++it ) {
		if( (*it).m_pMenu == pMenu )
			(*it).m_pMenu = 0;
	}
}

void KAccelBase::setEnabled( bool bActivate )
{
	kdDebug(125) << "setEnabled( " << bActivate << " )" << endl;
	m_bEnabled = bActivate;
}

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

bool KAccelBase::insertLabel( const QString& sName, const QString& sDesc )
	{ return m_rgActions.insertLabel( sName, sDesc ); }

KAccelAction* KAccelBase::insertAction( const QString& sAction, const QString& sDesc,
			const KAccelShortcuts& rgCutDefaults3, const KAccelShortcuts& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled )
{
	KAccelAction* pAction = m_rgActions.insertAction(
		sAction, sDesc,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu,
		bConfigurable, bEnabled );

	if( pAction && m_bAutoUpdate )
		insertConnection( *pAction );

	return pAction;
}

bool KAccelBase::removeAction( const QString& sAction )
{
	KAccelAction* pAction = m_rgActions.actionPtr( sAction );
	if( pAction && m_bAutoUpdate )
		removeConnection( *pAction );
	return m_rgActions.removeAction( sAction );
}

// BCI: make virtual ASAP, and then make changes to KAccel::connectItem()
bool KAccelBase::setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
{
	KAccelAction* pAction = m_rgActions.actionPtr( sAction );
	if( pAction ) {
		pAction->m_pObjSlot = pObjSlot;
		pAction->m_psMethodSlot = psMethodSlot;
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
// 32 bits:
//  00-15	Action #
//  16-23	Series #
//  24-31	Key #
inline uint CREATE_KEY_ID( uint iAction, uint iShortcut, uint iKey )
	{ return iAction | (iShortcut << 16) | (iKey << 24); }

inline void PARSE_KEY_ID( uint nID, uint& iAction, uint& iShortcut, uint& iKey )
{
	iAction = nID & 0xffff; nID >>= 16;
	iShortcut = nID & 0xff; nID >>= 8;
	iKey = nID;
}

struct X
{
	uint iAction, iShortcut, iKey;
	KKeySequence key;

	X() {}
	X( uint _iAction, uint _iShortcut, uint _iKey, KKeySequence _key )
		{ iAction = _iAction; iShortcut = _iShortcut; iKey = _iKey; key = _key; }

	bool operator <( const X& x ) {
		uint n1 = CREATE_KEY_ID( iAction, iShortcut, iKey );
		uint n2 = CREATE_KEY_ID( x.iAction, x.iShortcut, x.iKey );
		return n1 < n2;
	}
	bool operator >( const X& x ) {
		uint n1 = CREATE_KEY_ID( iAction, iShortcut, iKey );
		uint n2 = CREATE_KEY_ID( x.iAction, x.iShortcut, x.iKey );
		return n1 > n2;
	}
	bool operator <=( const X& x ) {
		uint n1 = CREATE_KEY_ID( iAction, iShortcut, iKey );
		uint n2 = CREATE_KEY_ID( x.iAction, x.iShortcut, x.iKey );
		return n1 <= n2;
	}
};

bool KAccelBase::updateConnections()
{
	kdDebug(125) << "updateConnections()" << endl;
	// Retrieve the list of keys to be connected, sorted by priority.
	QValueVector<X> rgKeys;
	createKeyList( rgKeys );

	KKeyToActionMap mapKeyToAction;
	for( uint i = 0; i < rgKeys.size(); i++ ) {
		X& x = rgKeys[i];
		KKeySequence key = x.key;
		KAccelAction& action = m_rgActions[x.iAction];

		// Search whether the same key has already been connected.
		uint j;
		for( j = 0; j < i && key != rgKeys[j].key; j++ )
			;

		// If key sequence is not present in preceeding entries,
		//	associate key with action in mapKeyToAction
		if( j == i )
			mapKeyToAction[key] = &action;
		// Else, key is superceeded by another:
		else {
			// If it is currently connected, disconnect it.
			if( m_mapKeyToAction.contains( key ) ) {
				m_mapKeyToAction.remove( key );
				disconnectKey( action, key );
			}
		}
	}

	// Disconnect key which no longer have bindings:
	for( KKeyToActionMap::iterator it = m_mapKeyToAction.begin(); it != m_mapKeyToAction.end(); ++it ) {
		const KKeySequence& key = it.key();
		KAccelAction* pAction = *it;
		if( !mapKeyToAction.contains( key ) && pAction )
			disconnectKey( *pAction, key );
	}

	// Connect any unconnected keys:
	// In other words, connect any keys which are present in the
	//  new action map, but which are _not_ present in the old one.
	for( KKeyToActionMap::iterator it = mapKeyToAction.begin(); it != mapKeyToAction.end(); ++it ) {
		if( !m_mapKeyToAction.contains( it.key() ) && (*it) ) {
			// TODO: Decide what to do if connect fails.
			//  Probably should remove this item from map.
			connectKey( *(*it), it.key() );
		}
	}

	// Store new map.
	m_mapKeyToAction = mapKeyToAction;

	for( KKeyToActionMap::iterator it = mapKeyToAction.begin(); it != mapKeyToAction.end(); ++it )
		kdDebug(125) << "Key: " << it.key().toString() << " => '" << (*it)->m_sName << "'" << endl;

	return true;
}

// Construct a list of keys to be connected, sorted highest priority first.
void KAccelBase::createKeyList( QValueVector<X>& rgKeys )
{
	if( !m_bEnabled )
		return;

	// create the list
	uint iAction = 0;
	for( KAccelActions::iterator itAction = m_rgActions.begin(); itAction != m_rgActions.end(); ++itAction ) {
		KAccelAction& action = *itAction;
		if( action.m_bEnabled ) {
			uint iShortcut = 0;
			for( KAccelShortcuts::iterator itShortcut = action.begin(); itShortcut != action.end(); ++itShortcut ) {
				KAccelShortcut& shortcut = *itShortcut;
				if( shortcut.count() > 0 ) {
					KAccelSequence& seq = shortcut.front();
					uint iKey = 0;
					for( KKeySequences::iterator itKey = seq.begin(); itKey != seq.end(); ++itKey ) {
						KKeySequence& key = *itKey;
						rgKeys.push_back( X( iAction, iShortcut, iKey, key ) );
						iKey++;
					}
				}
				iShortcut++;
			}
		}
		iAction++;
	}

	// sort by priority: iVariation[of first sequence], iShorcut, iAction
	qHeapSort( rgKeys.begin(), rgKeys.end() );
}

bool KAccelBase::insertConnection( KAccelAction& action )
{
	kdDebug(125) << "insertConnection( " << action.m_sName << " )" << endl;

	if( !action.m_bEnabled )
		return true;

	for( KAccelShortcuts::iterator itShortcut = action.begin(); itShortcut != action.end(); ++itShortcut ) {
		KAccelShortcut& shortcut = *itShortcut;
		if( shortcut.count() > 0 ) {
			KAccelSequence& seq = shortcut.front();
			for( KKeySequences::iterator itKey = seq.begin(); itKey != seq.end(); ++itKey ) {
				KKeySequence& key = *itKey;
				if( !m_mapKeyToAction.contains( key ) ) {
					m_mapKeyToAction[key] = &action;
					connectKey( action, key );
				}
				// There is a key conflict.  A full update
				//  check is necessary.
				else
					return updateConnections();
			}
		}
	}

	return true;
}

bool KAccelBase::removeConnection( KAccelAction& action )
{
	kdDebug(125) << "removeConnection( " << action.m_sName << " )" << endl;

	for( KAccelShortcuts::iterator itShortcut = action.begin(); itShortcut != action.end(); ++itShortcut ) {
		KAccelShortcut& shortcut = *itShortcut;
		if( shortcut.count() > 0 ) {
			KAccelSequence& seq = shortcut.front();
			for( KKeySequences::iterator itKey = seq.begin(); itKey != seq.end(); ++itKey ) {
				KKeySequence& key = *itKey;
				if( !m_mapKeyToAction.contains( key ) && m_mapKeyToAction[key] == &action ) {
					m_mapKeyToAction.remove( key );
					disconnectKey( action, key );
				}
			}
		}
	}

	return true;
}

bool KAccelBase::setShortcuts( const QString& sAction, const KAccelShortcuts& rgCuts )
{
	KAccelAction* pAction = actionPtr( sAction );
	if( pAction ) {
		if( m_bAutoUpdate )
			removeConnection( *pAction );

		pAction->setShortcuts( rgCuts );

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
