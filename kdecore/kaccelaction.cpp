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

#include "kaccelaction.h"
#include "kaccelbase.h"   // for KAccelBase::slotRemoveAction() & emitSignal()

#include <qkeycode.h>

#include <kconfig.h>
#include <kckey.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

//----------------------------------------------------
/*
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
	//kdDebug(125) << "KAccelShortcut::init( const QString& s )" << endl;
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
	//cerr << "KAccelShortcuts::init( const KAccelShortcuts& cuts ) begin" << endl;
	//kdDebug(125) << "KAccelShortcuts::init() -- cuts.size() = " << cuts.size() << endl;
	// *(int*)0 = 0;
	resize( cuts.size() );
	//kdDebug(125) << "KAccelShortcuts::init() B" << endl;
	for( uint i = 0; i < size(); i++ )
		at(i).init( cuts[i] );
	//cerr << "KAccelShortcuts::init( const KAccelShortcuts& cuts ) end" << endl;
	return true;
}

bool KAccelShortcuts::init( const QString& s )
{
	//kdDebug(125) << "KAccelShortcuts::init( const QString& s ) begin" << endl;
	QStringList rgs = QStringList::split( ';', s );
	resize( rgs.size() );
	for( uint i = 0; i < rgs.size(); i++ ) {
		QString& sCut = rgs[i];
		if( sCut.startsWith( "default(" ) )
			sCut = sCut.mid( 8, sCut.length() - 9 );
		at(i) = KAccelShortcut( sCut );
	}
	//kdDebug(125) << "KAccelShortcuts::init( const QString& s ) end" << endl;
	return true;
}

bool KAccelShortcuts::init( KKeySequence key )
{
	//kdDebug(125) << "KAccelShortcuts::init( KKeySequence key ) begin" << endl;
	resize( 1 );
	at(0).init( key );
	//kdDebug(125) << "KAccelShortcuts::init( KKeySequence key ) end" << endl;
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
	kdDebug(125) << "KShortcuts::KShortcuts( const KShortcuts& cuts )\n";
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
*/

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
	//m_nIDMenu = 0;
	d->m_nConnections = 0;
}

KAccelAction::KAccelAction( const KAccelAction& action )
{
	//kdDebug(125) << "KAccelAction( copy from \"" << action.m_sName << "\" ): this = " << this << endl;
	d = new KAccelActionPrivate;
	*this = action;
}

KAccelAction::KAccelAction( const QString& sName, const QString& sDesc, const QString& sHelp,
			const char* rgCutDefaults3, const char* rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "KAccelAction( \"" << sName << "\" ): this = " << this << endl;
	d = new KAccelActionPrivate;
	init( sName, sDesc, sHelp,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

KAccelAction::KAccelAction( const QString& sName, const QString& sDesc, const QString& sHelp,
			const KShortcut& rgCutDefaults3, const KShortcut& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "KAccelAction( \"" << sName << "\" ): this = " << this << endl;
	d = new KAccelActionPrivate;
	init( sName, sDesc, sHelp,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
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
	//m_nIDMenu = 0;
	d->m_nConnections = 0;
}

bool KAccelAction::init( const QString& sName, const QString& sDesc, const QString& sHelp,
			const char* rgCutDefaults3, const char* rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	kdDebug(125) << "KAccelAction::init()" << endl;
	m_sName = sName;
	m_sDesc = sDesc;
	m_sHelp = sHelp;
	m_cutDefault3.init( rgCutDefaults3 );
	m_cutDefault4.init( rgCutDefaults4 );
	m_pObjSlot = pObjSlot;
	m_psMethodSlot = psMethodSlot;
	m_bConfigurable = bConfigurable;
	m_bEnabled = bEnabled;
	m_nIDAccel = 0;
	//m_nIDMenu = 0;
	//m_pMenu = pMenu;
	m_cut = shortcutDefault();
	d->m_nConnections = 0;
	//kdDebug(125) << "KAccelAction::init() D" << endl;
	return true;
}

bool KAccelAction::init( const QString& sName, const QString& sDesc, const QString& sHelp,
			const KShortcut& rgCutDefaults3, const KShortcut& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	m_sName = sName;
	m_sDesc = sDesc;
	m_sHelp = sHelp;
	m_cutDefault3 = rgCutDefaults3;
	m_cutDefault4 = rgCutDefaults4;
	m_pObjSlot = pObjSlot;
	m_psMethodSlot = psMethodSlot;
	m_bConfigurable = bConfigurable;
	m_bEnabled = bEnabled;
	m_nIDAccel = 0;
	//m_nIDMenu = 0;
	m_cut = shortcutDefault();
	d->m_nConnections = 0;
	return true;
}

KAccelAction& KAccelAction::operator =( const KAccelAction& action )
{
	m_sName          = action.m_sName;
	m_sDesc          = action.m_sDesc;
	m_sHelp          = action.m_sHelp;
	m_cutDefault3    = action.m_cutDefault3;
	m_cutDefault4    = action.m_cutDefault4;
	m_pObjSlot       = action.m_pObjSlot;
	m_psMethodSlot   = action.m_psMethodSlot;
	m_bConfigurable  = action.m_bConfigurable;
	m_bEnabled       = action.m_bEnabled;
	m_nIDAccel       = action.m_nIDAccel;
	//m_nIDMenu        = action.m_nIDMenu;
	m_cut            = action.m_cut;
	d->m_nConnections = action.d->m_nConnections;

	return *this;
}

uint KAccelAction::sequenceCount() const
{
	return m_cut.count();
}

void KAccelAction::setName( const QString& s )
	{ m_sName = s; }
void KAccelAction::setDesc( const QString& s )
	{ m_sDesc = s; }
void KAccelAction::setHelpext( const QString& s )
	{ m_sHelp = s; }

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

/*KKeySequence& KAccelAction::key( uint iShortcut, uint iSequence, uint iKey )
{
	if( iShortcut < count() ) {
		KAccelShortcut& cut = m_cut[i];
		if( iSequence < cut.count() ) {
			KAccelSequence& seq = cut.m_rgSequences[iSequence];
			if( iKey < seq.count() ) {
				return Keys
			}
	}
}*/

const KKeySequence& KAccelAction::seq( uint i ) const
	{ return m_cut.seq( i ); }
	//{ return (i < count()) ? m_cut[i] : KKeySequence(); }

/*KKeySequences KAccelAction::keyList()
{
	KKeySequences keys;

	for( KShortcut::iterator itShortcut = m_cut.begin(); itShortcut != m_cut.end(); ++itShortcut ) {
		KKeySequence& shortcut = *itShortcut;
		if( shortcut.count() > 0 ) {
			// First key of any sequence (Ctrl+X in Ctrl+X,I)
			KAccelSequence& seq = shortcut.front();
			// All variations on that key
			for( KKeySequences::iterator itKey = seq.begin(); itKey != seq.end(); ++itKey )
				keys.push_back( *itKey );
		}
	}

	return keys;
}*/

QString KAccelAction::toString() const
	{ return m_cut.toString(); }

QString KAccelAction::toStringInternal() const
	{ return m_cut.toStringInternal( &shortcutDefault() ); }

/*KKeySequence* KAccelAction::insertShortcut( const KKeySequence& cut )
{
	m_cut.push_back( cut );
	return &m_cut.back();
}*/

bool KAccelAction::setKeySequence( uint i, const KKeySequence& seq )
{
	if( i < sequenceCount() ) {
		m_cut.seq(i) = seq;
		return true;
	} else if( i == sequenceCount() )
		return m_cut.insert( seq );
	return false;
}

void KAccelAction::clearShortcuts()
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

const KShortcut& KAccelAction::shortcutDefault() const
	{ return (useFourModifierKeys()) ? m_cutDefault4 : m_cutDefault3; }
bool KAccelAction::isConnected() const
	{ return d->m_nConnections; }
void KAccelAction::incConnections()
	{ d->m_nConnections++; }
void KAccelAction::decConnections()
	{ if( d->m_nConnections > 0 ) d->m_nConnections--; }

// Indicate whether to default to the 3- or 4- modifier keyboard schemes
static int KAccelAction::g_bUseFourModifierKeys = -1;

bool KAccelAction::useFourModifierKeys()
{
	if( KAccelAction::g_bUseFourModifierKeys == -1 ) {
		// Read in whether to use 4 modifier keys
		KConfigGroupSaver cgs( KGlobal::config(), "Keyboard Layout" );
		bool b = KGlobal::config()->readBoolEntry( "Use Four Modifier Keys",  false );
		KAccelAction::g_bUseFourModifierKeys = b && KKeyNative::keyboardHasMetaKey();
	}
	return KAccelAction::g_bUseFourModifierKeys == 1;
}

void KAccelAction::useFourModifierKeys( bool b )
{
	if( KAccelAction::g_bUseFourModifierKeys != (int)b ) {
		KAccelAction::g_bUseFourModifierKeys = b && KKeyNative::keyboardHasMetaKey();
		// If we're 'turning off' the meta key or, if we're turning it on,
		//  the keyboard must actually have a meta key.
		if( b && !KKeyNative::keyboardHasMetaKey() )
			kdDebug(125) << "Tried to use four modifier keys on a keyboard layout without a Meta key.\n";
	}
	KConfigGroupSaver cgs( KGlobal::config(), "Keyboard Layout" );
	KGlobal::config()->writeEntry( "Use Four Modifier Keys", KAccelAction::g_bUseFourModifierKeys, true, true);

	kdDebug(125) << "bUseFourModifierKeys = " << KAccelAction::g_bUseFourModifierKeys << endl;
}

//---------------------------------------------------------------------
// KAccelActions
//---------------------------------------------------------------------

class KAccelActionsPrivate
{
 public:
	bool m_bChanged;
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
	delete d;
}

void KAccelActions::initPrivate( KAccelBase* pKAccelBase )
{
	m_pKAccelBase = pKAccelBase;
	m_nSizeAllocated = m_nSize = 0;
	m_prgActions = 0;
	d = new KAccelActionsPrivate;
	d->m_bChanged = false;
}

void KAccelActions::clear()
{
	kdDebug(125) << "\tKAccelActions::clear()" << endl;
	for( uint i = 0; i < m_nSize; i++ )
		delete m_prgActions[i];
	delete[] m_prgActions;

	m_nSizeAllocated = m_nSize = 0;
	m_prgActions = 0;
}

bool KAccelActions::init( const KAccelActions& actions )
{
	clear();
	resize( actions.size() );
	for( uint i = 0; i < m_nSize; i++ ) {
		KAccelAction* pAction = actions.m_prgActions[i];
		if( pAction )
			m_prgActions[i] = new KAccelAction( *pAction );
		else
			m_prgActions[i] = 0;
	}

	return true;
}

bool KAccelActions::init( KConfigBase& config, const QString& sGroup )
{
	kdDebug(125) << "KAccelActions::init( " << sGroup << " )" << endl;
	QMap<QString, QString> mapEntry = config.entryMap( sGroup );
	resize( mapEntry.size() );

	QMap<QString, QString>::Iterator it( mapEntry.begin() );
	for( uint i = 0; it != mapEntry.end(); ++it, i++ ) {
		QString sShortcuts = *it;
		KShortcut cuts;

		kdDebug(125) << it.key() << " = " << sShortcuts << endl;
		if( !sShortcuts.isEmpty() && sShortcuts != "none" )
			cuts.init( sShortcuts );

		m_prgActions[i] = new KAccelAction( it.key(), it.key(), it.key(),
			cuts, cuts,
			0, 0,          // pObjSlot, psMethodSlot,
			true, false ); // bConfigurable, bEnabled
	}

	return true;
}

void KAccelActions::resize( uint nSize )
{
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
}

void KAccelActions::insertActionPtr( KAccelAction* pAction )
{
	resize( m_nSize + 1 );
	m_prgActions[m_nSize-1] = pAction;
}

void KAccelActions::updateShortcuts( KAccelActions& actions2 )
{
	kdDebug(125) << "KAccelActions::updateShortcuts()" << endl;
	bool bChanged = false;

	for( uint i = 0; i < m_nSize; i++ ) {
		KAccelAction* pAction = m_prgActions[i];
		if( pAction && pAction->m_bConfigurable ) {
			KAccelAction* pAction2 = actions2.actionPtr( pAction->m_sName );
			if( pAction2 ) {
				QString sOld( pAction->m_cut.toString() );
				pAction->m_cut = pAction2->m_cut;
				kdDebug(125) << "\t" << pAction->m_sName
					<< " found: " << sOld
					<< " => " << pAction2->m_cut.toString()
					<< " = " << pAction->m_cut.toString() << endl;
				bChanged = true;
			}
		}
	}

	if( bChanged ) {
		setChanged( true );
		emitKeycodeChanged();
	}
}

int KAccelActions::actionIndex( const QString& sAction ) const
{
	for( uint i = 0; i < m_nSize; i++ ) {
		if( m_prgActions[i] == 0 )
			kdWarning(125) << "KAccelActions::actionPtr( " << sAction << " ): encountered null pointer at m_prgActions[" << i << "]" << endl;
		else if( m_prgActions[i]->m_sName == sAction )
			return (int) i;
	}
	return -1;
}

KAccelAction* KAccelActions::actionPtr( uint i )
{
	return m_prgActions[i];
}

const KAccelAction* KAccelActions::actionPtr( uint i ) const
{
	return m_prgActions[i];
}

KAccelAction* KAccelActions::actionPtr( const QString& sAction )
{
	int i = actionIndex( sAction );
	return (i >= 0) ? m_prgActions[i] : 0;
}

const KAccelAction* KAccelActions::actionPtr( const QString& sAction ) const
{
	int i = actionIndex( sAction );
	return (i >= 0) ? m_prgActions[i] : 0;
}

KAccelAction* KAccelActions::actionPtr( KKeySequence cut )
{
	for( uint i = 0; i < m_nSize; i++ ) {
		if( m_prgActions[i] == 0 )
			kdWarning(125) << "KAccelActions::actionPtr( " << cut.toString() << " ): encountered null pointer at m_prgActions[" << i << "]" << endl;
		else if( m_prgActions[i]->contains( cut ) )
			return m_prgActions[i];
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

bool KAccelActions::insertLabel( const QString& sName, const QString& sDesc )
{
	if( actionPtr( sName ) ) {
		kdWarning(125) << "KAccelActions::insertLabel( " << sName << ", " << sDesc << " ): action with same name already present." << endl;
		return false;
	}

	KAccelAction* pAction = new KAccelAction;
	pAction->m_sName = sName;
	pAction->m_sDesc = sDesc;
	pAction->m_bConfigurable = false;
	pAction->m_bEnabled = false;

	insertActionPtr( pAction );
	return true;
}

KAccelAction* KAccelActions::insertAction( const QString& sAction, const QString& sDesc, const QString& sHelp,
			const char* rgCutDefaults3, const char* rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "KAccelActions::insertAction()1 begin" << endl;
	if( actionPtr( sAction ) ) {
		kdWarning(125) << "KAccelActions::insertAction( " << sAction << " ): action with same name already present." << endl;
		return 0;
	}

	KAccelAction* pAction = new KAccelAction(
		sAction, sDesc, sHelp,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
	insertActionPtr( pAction );

	//kdDebug(125) << "KAccelActions::insertAction()1 end" << endl;
	return pAction;
}

KAccelAction* KAccelActions::insertAction( const QString& sAction, const QString& sDesc, const QString& sHelp,
			const KShortcut& rgCutDefaults3, const KShortcut& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "KAccelActions::insertAction()2 begin" << endl;
	if( actionPtr( sAction ) ) {
		kdWarning(125) << "KAccelActions::insertAction( " << sAction << " ): action with same name already present." << endl;
		return 0;
	}

	KAccelAction* pAction = new KAccelAction(
		sAction, sDesc, sHelp,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
	insertActionPtr( pAction );

	//kdDebug(125) << "KAccelActions::insertAction()2 end" << endl;
	return pAction;
}

bool KAccelActions::removeAction( const QString& sAction )
{
	kdDebug(125) << "KAccelActions::removeAction( \"" << sAction << "\" ): this = " << this << " m_pKAccelBase = " << m_pKAccelBase << endl;

	int iAction = actionIndex( sAction );
	if( iAction < 0 )
		return false;

	if( m_pKAccelBase )
		m_pKAccelBase->slotRemoveAction( m_prgActions[iAction] );
	delete m_prgActions[iAction];

	for( uint i = iAction; i < m_nSize - 1; i++ )
		m_prgActions[i] = m_prgActions[i+1];
	m_nSize--;

	return true;
}

void KAccelActions::readActions( const QString& sConfigGroup, KConfigBase* pConfig )
{
	kdDebug(125) << "readActions( \"" << sConfigGroup << "\", " << pConfig << " ) start" << endl;
	if( !pConfig )
		pConfig = KGlobal::config();
	KConfigGroupSaver cgs( pConfig, sConfigGroup );

	/*QMap<QString, QString> mapEntry = pConfig->entryMap( sConfigGroup );
	QMap<QString, QString>::Iterator it( mapEntry.begin() );
	for( uint i = 0; it != mapEntry.end(); ++it, i++ ) {
		QString sShortcuts = *it;
		kdDebug(125) << "\t" << it.key() << " = " << sShortcuts << endl;
	}

	kdDebug(125) << "--------------" << endl;
	for( uint i = 0; i < m_nSize; i++ ) {
		KAccelAction* pAction = m_prgActions[i];
		kdDebug(125) << pAction->m_sName << " bConfigurable = " << pAction->m_bConfigurable << endl;
	}*/

	for( uint i = 0; i < m_nSize; i++ ) {
		if( m_prgActions[i] == 0 ) {
			kdWarning(125) << "KAccelActions::readActions(): encountered null pointer at m_prgActions[" << i << "]" << endl;
			continue;
		}
		KAccelAction& action = *m_prgActions[i];

		if( action.m_bConfigurable ) {
			QString sEntry = pConfig->readEntry( action.m_sName );
			if( !sEntry.isNull() ) {
				if( sEntry == "none" )
					action.clearShortcuts();
				else
					action.m_cut.init( sEntry );
			}
			kdDebug(125) << "\t" << action.m_sName << " = '" << sEntry << "'" << endl;
		}
	}

	setChanged( true );
	emitKeycodeChanged();
	kdDebug(125) << "readActions done" << endl;
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
void KAccelActions::writeActions( const QString &sGroup, KConfig *config,
			bool bWriteAll, bool bGlobal ) const
{
	kdDebug(125) << "KAccelActions::writeActions( " << sGroup << ", " << config << ", " << bWriteAll << ", " << bGlobal << " )" << endl;
	KConfig *pConfig = config ? config : KGlobal::config();
	KConfigGroupSaver cs( pConfig, sGroup );

	for( uint i = 0; i < m_nSize; i++ ) {
		if( m_prgActions[i] == 0 ) {
			kdWarning(125) << "KAccelActions::writeActions(): encountered null pointer at m_prgActions[" << i << "]" << endl;
			continue;
		}
		const KAccelAction& action = *m_prgActions[i];

		QString s;
		bool bConfigHasAction = !pConfig->readEntry( action.m_sName ).isEmpty();
		bool bSameAsDefault = true;
		bool bWriteAction = false;

		if( action.m_bConfigurable ) {
			s = action.toStringInternal();
			bSameAsDefault = (action.m_cut == action.shortcutDefault());

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
				kdDebug(125) << "\twriting " << action.m_sName << " = " << s << endl;
				pConfig->writeEntry( action.m_sName, s, true, bGlobal );
			}
		}
	}

	pConfig->sync();
}

void KAccelActions::emitKeycodeChanged()
{
	if( m_pKAccelBase )
		m_pKAccelBase->emitSignal( KAccelBase::KEYCODE_CHANGED );
}

bool KAccelActions::hasChanged() const
	{ return d->m_bChanged; }
void KAccelActions::setChanged( bool bChanged )
	{ d->m_bChanged = bChanged; }
