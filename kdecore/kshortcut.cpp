/*  This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

#include "kshortcut.h"
#include "kkeynative.h"
#ifdef Q_WS_X11
#include "kkeyserver_x11.h"
#endif

#include <qevent.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksimpleconfig.h>

//----------------------------------------------------

static KKey* g_pspec = 0;
static KKeySequence* g_pseq = 0;
static KShortcut* g_pcut = 0;

//----------------------------------------------------
// KKey
//----------------------------------------------------

KKey::KKey()                          { clear(); }
KKey::KKey( uint key, uint modFlags ) { init( key, modFlags ); }
KKey::KKey( int keyQt )               { init( keyQt ); }
KKey::KKey( const QKeySequence& seq ) { init( seq ); }
KKey::KKey( const QKeyEvent* pEvent ) { init( pEvent ); }
KKey::KKey( const KKey& key )         { init( key ); }
KKey::KKey( const QString& sKey )     { init( sKey ); }

KKey::~KKey()
{
}

void KKey::clear()
{
	m_sym = 0;
	m_mod = 0;
}

bool KKey::init( uint key, uint modFlags )
{
	m_sym = key;
	m_mod = modFlags;
	return true;
}

bool KKey::init( int keyQt )
{
	//KKeyServer::Sym sym;

	//if( sym.initQt( keyQt )
	if( KKeyServer::keyQtToSym( keyQt, m_sym )
	    && KKeyServer::keyQtToMod( keyQt, m_mod ) )
		return true;
	else {
		m_sym = 0;
		m_mod = 0;
		return false;
	}
}

bool KKey::init( const QKeySequence& key )
{
	// TODO: if key.count() > 1, should we return failure?
	return init( (int) key );
}

bool KKey::init( const QKeyEvent* pEvent )
{
	int keyQt = pEvent->key();
	if( pEvent->state() & Qt::ShiftButton )   keyQt |= Qt::SHIFT;
	if( pEvent->state() & Qt::ControlButton ) keyQt |= Qt::CTRL;
	if( pEvent->state() & Qt::AltButton )     keyQt |= Qt::ALT;
	return init( keyQt );
}

bool KKey::init( const KKey& key )
{
	m_sym = key.m_sym;
	m_mod = key.m_mod;
	return true;
}

bool KKey::init( const QString& sSpec )
{
	clear();

	QString sKey = sSpec.stripWhiteSpace();
	if( sKey.startsWith( "default(" ) && sKey.endsWith( ")" ) )
		sKey = sKey.mid( 8, sKey.length() - 9 );
	// i.e., "Ctrl++" = "Ctrl+Plus"
	if( sKey.endsWith( "++" ) )
		sKey = sKey.left( sKey.length() - 1 ) + "plus";
	QStringList rgs = QStringList::split( '+', sKey, true );

	uint i;
	// Check for modifier keys first.
	for( i = 0; i < rgs.size(); i++ ) {
		QString s = rgs[i].lower();
		if( s == "shift" )     m_mod |= KKey::SHIFT;
		else if( s == "ctrl" ) m_mod |= KKey::CTRL;
		else if( s == "alt" )  m_mod |= KKey::ALT;
		else if( s == "win" )  m_mod |= KKey::WIN;
		else if( s == "meta" ) m_mod |= KKey::WIN;
		else break;
	}
	// If there is one non-blank key left:
	if( (i == rgs.size() - 1 && !rgs[i].isEmpty()) ) {
		KKeyServer::Sym sym( rgs[i] );
		m_sym = sym.m_sym;
	}

	if( m_sym == 0 )
		m_mod = 0;

	kdDebug(125) << "KKey::init( \"" << sSpec << "\" ):"
		<< " m_sym = " << QString::number(m_sym, 16)
		<< ", m_mod = " << QString::number(m_mod, 16) << endl;

	return m_sym != 0;
}

bool KKey::isNull() const          { return m_sym == 0; }
uint KKey::sym() const             { return m_sym; }
uint KKey::modFlags() const        { return m_mod; }

int KKey::compare( const KKey& spec ) const
{
	if( m_sym != spec.m_sym )
		return m_sym - spec.m_sym;
	if( m_mod != spec.m_mod )
		return m_mod - spec.m_mod;
	return 0;
}

int KKey::keyCodeQt() const
{
	return KKeyNative( *this ).keyCodeQt();
}

QString KKey::toString() const
{
	QString s;

	s = KKeyServer::modToStringUser( m_mod );
	if( !s.isEmpty() )
		s += '+';
	s += KKeyServer::Sym(m_sym).toString();

	return s;
}

QString KKey::toStringInternal() const
{
	//kdDebug(125) << "KKey::toStringInternal(): this = " << this
	//	<< " mod = " << QString::number(m_mod, 16)
	//	<< " key = " << QString::number(m_sym, 16) << endl;
	QString s;

	s = KKeyServer::modToStringInternal( m_mod );
	if( !s.isEmpty() )
		s += '+';
	s += KKeyServer::Sym(m_sym).toStringInternal();
	return s;
}

KKey& KKey::null()
{
	if( !g_pspec )
		g_pspec = new KKey;
	if( !g_pspec->isNull() )
		g_pspec->clear();
	return *g_pspec;
}

QString KKey::modFlagLabel( ModFlag modFlag )
{
	return KKeyServer::modToStringUser( modFlag );
}

//---------------------------------------------------------------------
// KKeySequence
//---------------------------------------------------------------------

KKeySequence::KKeySequence()                          { clear(); }
KKeySequence::KKeySequence( const QKeySequence& seq ) { init( seq ); }
KKeySequence::KKeySequence( const KKey& key )         { init( key ); }
KKeySequence::KKeySequence( const KKeySequence& seq ) { init( seq ); }
KKeySequence::KKeySequence( const QString& s )        { init( s ); }

KKeySequence::~KKeySequence()
{
}

void KKeySequence::clear()
{
	m_nKeys = 0;
	m_bTriggerOnRelease = false;
}

bool KKeySequence::init( const QKeySequence& seq )
{
	clear();
	if( !seq.isEmpty() ) {
		for( uint i = 0; i < seq.count(); i++ ) {	
			m_rgvar[i].init( seq[i] );
			if( m_rgvar[i].isNull() )
				return false;
		}
		m_nKeys = seq.count();
		m_bTriggerOnRelease = false;
	}
	return true;
}

bool KKeySequence::init( const KKey& key )
{
	if( !key.isNull() ) {
		m_nKeys = 1;
		m_rgvar[0].init( key );
		m_bTriggerOnRelease = false;
	} else
		clear();
	return true;
}

bool KKeySequence::init( const KKeySequence& seq )
{
	m_bTriggerOnRelease = false;
	m_nKeys = seq.m_nKeys;
	for( uint i = 0; i < m_nKeys; i++ ) {
		if( seq.m_rgvar[i].isNull() ) {
			kdWarning(125) << "KKeySequence::init( seq ): key[" << i << "] is null." << endl;
			m_nKeys = 0;
			return false;
		}
		m_rgvar[i] = seq.m_rgvar[i];
	}
	return true;
}

bool KKeySequence::init( const QString& s )
{
	m_bTriggerOnRelease = false;
	//kdDebug(125) << "KKeySequence::init( " << s << " )" << endl;
	QStringList rgs = QStringList::split( ',', s );
	if( s == "none" || rgs.size() == 0 ) {
		clear();
		return true;
	} else if( rgs.size() <= MAX_KEYS ) {
		m_nKeys = rgs.size();
		for( uint i = 0; i < m_nKeys; i++ ) {
			m_rgvar[i].init( KKey(rgs[i]) );
			//kdDebug(125) << "\t'" << rgs[i] << "' => " << m_rgvar[i].toStringInternal() << endl;
		}
		return true;
	} else {
		clear();
		return false;
	}
}

uint KKeySequence::count() const
{
	return m_nKeys;
}

const KKey& KKeySequence::key( uint i ) const
{
	if( i < m_nKeys )
		return m_rgvar[i];
	else
		return KKey::null();
}

bool KKeySequence::isTriggerOnRelease() const
	{ return m_bTriggerOnRelease; }

bool KKeySequence::setKey( uint iKey, const KKey& key )
{
	if( iKey <= m_nKeys && iKey < MAX_KEYS ) {
		m_rgvar[iKey].init( key );
		if( iKey == m_nKeys )
			m_nKeys++;
		return true;
	} else
		return false;
}

bool KKeySequence::isNull() const
{
	return m_nKeys == 0;
}

bool KKeySequence::startsWith( const KKeySequence& seq ) const
{
	if( m_nKeys < seq.m_nKeys )
		return false;

	for( uint i = 0; i < seq.m_nKeys; i++ ) {
		if( m_rgvar[i] != seq.m_rgvar[i] )
			return false;
	}

	return true;
}

int KKeySequence::compare( const KKeySequence& seq ) const
{
	for( uint i = 0; i < m_nKeys && i < seq.m_nKeys; i++ ) {
		int ret = m_rgvar[i].compare( seq.m_rgvar[i] );
		if( ret != 0 )
			return ret;
	}
	if( m_nKeys != seq.m_nKeys )
		return m_nKeys - seq.m_nKeys;
	else
		return 0;
}

QKeySequence KKeySequence::qt() const
{
	int k[4] = { 0, 0, 0, 0 };
	
	for( uint i = 0; i < count(); i++ )
		k[i] = KKeyNative(key(i)).keyCodeQt();

	QKeySequence seq( k[0], k[1], k[2], k[3] );
	return seq;
}

int KKeySequence::keyCodeQt() const
{
	return (count() == 1) ? KKeyNative(key(0)).keyCodeQt() : 0;
}

QString KKeySequence::toString() const
{
	if( m_nKeys < 1 ) return QString::null;

	QString s;
	s = m_rgvar[0].toString();
	for( uint i = 1; i < m_nKeys; i++ ) {
		s += ",";
		s += m_rgvar[i].toString();
	}

	return s;
}

QString KKeySequence::toStringInternal() const
{
	if( m_nKeys < 1 ) return QString::null;

	QString s;
	s = m_rgvar[0].toStringInternal();
	for( uint i = 1; i < m_nKeys; i++ ) {
		s += ",";
		s += m_rgvar[i].toStringInternal();
	}

	return s;
}

KKeySequence& KKeySequence::null()
{
	if( !g_pseq )
		g_pseq = new KKeySequence;
	if( !g_pseq->isNull() )
		g_pseq->clear();
	return *g_pseq;
}

//---------------------------------------------------------------------
// KShortcut
//---------------------------------------------------------------------

KShortcut::KShortcut()                            { clear(); }
KShortcut::KShortcut( int keyQt )                 { init( keyQt ); }
KShortcut::KShortcut( const QKeySequence& key )   { init( key ); }
KShortcut::KShortcut( const KKey& key )           { init( key ); }
KShortcut::KShortcut( const KKeySequence& seq )   { init( seq ); }
KShortcut::KShortcut( const KShortcut& cut )      { init( cut ); }
KShortcut::KShortcut( const char* ps )            { init( QString(ps) ); }
KShortcut::KShortcut( const QString& s )          { init( s ); }

KShortcut::~KShortcut()
{
}

void KShortcut::clear()
{
	m_nSeqs = 0;
}

bool KShortcut::init( int keyQt )
{
	if( keyQt ) {
		m_nSeqs = 1;
		m_rgseq[0].init( QKeySequence(keyQt) );
	} else
		clear();
	return true;
}

bool KShortcut::init( const QKeySequence& key )
{
	m_nSeqs = 1;
	m_rgseq[0].init( key );
	return true;
}

bool KShortcut::init( const KKey& spec )
{
	m_nSeqs = 1;
	m_rgseq[0].init( spec );
	return true;
}

bool KShortcut::init( const KKeySequence& seq )
{
	m_nSeqs = 1;
	m_rgseq[0] = seq;
	return true;
}

bool KShortcut::init( const KShortcut& cut )
{
	m_nSeqs = cut.m_nSeqs;
	for( uint i = 0; i < m_nSeqs; i++ )
		m_rgseq[i] = cut.m_rgseq[i];
	return true;
}

bool KShortcut::init( const QString& s )
{
	bool bRet = true;
	QStringList rgs = QStringList::split( ';', s );

	if( s == "none" || rgs.size() == 0 ) 
		clear();
	else if( rgs.size() <= MAX_SEQUENCES ) {
		m_nSeqs = rgs.size();
		for( uint i = 0; i < m_nSeqs; i++ ) {
			QString& sSeq = rgs[i];
			if( sSeq.startsWith( "default(" ) )
				sSeq = sSeq.mid( 8, sSeq.length() - 9 );
			m_rgseq[i].init( sSeq );
			//kdDebug(125) << "*\t'" << sSeq << "' => " << m_rgseq[i].toStringInternal() << endl;
		}
	} else {
		clear();
		bRet = false;
	}

	if( !s.isEmpty() ) {
		QString sDebug;
		QTextStream os( &sDebug, IO_WriteOnly );
		os << "KShortcut::init( \"" << s << "\" ): ";
		for( uint i = 0; i < m_nSeqs; i++ ) {
			os << " m_rgseq[" << i << "]: ";
			KKeyServer::Variations vars;
			vars.init( m_rgseq[i].key(0), true );
			for( uint j = 0; j < vars.count(); j++ )
				os << QString::number(vars.m_rgkey[j].keyCodeQt(),16) << ',';
		}
		kdDebug(125) << sDebug << endl;
	}

	return bRet;
}

uint KShortcut::count() const
{
	return m_nSeqs;
}

const KKeySequence& KShortcut::seq( uint i ) const
{
	return (i < m_nSeqs) ? m_rgseq[i] : KKeySequence::null();
}

int KShortcut::keyCodeQt() const
{
	if( m_nSeqs >= 1 )
		return m_rgseq[0].keyCodeQt();
	return QKeySequence();
}

bool KShortcut::isNull() const
{
	return m_nSeqs == 0;
}

int KShortcut::compare( const KShortcut& cut ) const
{
	for( uint i = 0; i < m_nSeqs && i < cut.m_nSeqs; i++ ) {
		int ret = m_rgseq[i].compare( cut.m_rgseq[i] );
		if( ret != 0 )
			return ret;
	}
	return m_nSeqs - cut.m_nSeqs;
}

bool KShortcut::contains( const KKey& key ) const
{
	return contains( KKeySequence(key) );
}

bool KShortcut::contains( const KKeyNative& key ) const
{
	for( uint i = 0; i < count(); i++ ) {
		if( !m_rgseq[i].isNull()
		    && m_rgseq[i].count() == 1
		    && KKeyNative(m_rgseq[i].key(0)) == key )
			return true;
	}
	return false;
}

bool KShortcut::contains( const KKeySequence& seq ) const
{
	for( uint i = 0; i < count(); i++ ) {
		if( !m_rgseq[i].isNull() && m_rgseq[i] == seq )
			return true;
	}
	return false;
}

bool KShortcut::setSeq( uint iSeq, const KKeySequence& seq )
{
	// TODO: check if seq is null, and act accordingly.
	if( iSeq <= m_nSeqs && iSeq < MAX_SEQUENCES ) {
		m_rgseq[iSeq] = seq;
		if( iSeq == m_nSeqs )
			m_nSeqs++;
		return true;
	} else
		return false;
}

bool KShortcut::append( const KKeySequence& seq )
{
	if( m_nSeqs < MAX_SEQUENCES ) {
		if( !seq.isNull() ) {
			m_rgseq[m_nSeqs] = seq;
			m_nSeqs++;
		}
		return true;
	} else
		return false;
}

KShortcut::operator QKeySequence () const
{
	if( count() >= 1 )
		return m_rgseq[0].qt();
	else
		return QKeySequence();
}

QString KShortcut::toString() const
{
	QString s;

	for( uint i = 0; i < count(); i++ ) {
		s += m_rgseq[i].toString();
		if( i < count() - 1 )
			s += ';';
	}

	return s;
}

QString KShortcut::toStringInternal( const KShortcut* pcutDefault ) const
{
	QString s;

	for( uint i = 0; i < count(); i++ ) {
		const KKeySequence& seq = m_rgseq[i];
		if( pcutDefault && i < pcutDefault->count() && seq == (*pcutDefault).seq(i) ) {
			s += "default(";
			s += seq.toStringInternal();
			s += ")";
		} else
			s += seq.toStringInternal();
		if( i < count() - 1 )
			s += ';';
	}

	return s;
}

KShortcut& KShortcut::null()
{
	if( !g_pcut )
		g_pcut = new KShortcut;
	if( !g_pcut->isNull() )
		g_pcut->clear();
	return *g_pcut;
}
