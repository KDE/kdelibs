// -*- indent-tabs-mode:t; tab-width:4; c-basic-offset: 4 -*-
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kshortcut.h"
#include "kkeynative.h"
#include "kkeyserver.h"

#include <qevent.h>
#include <qkeysequence.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"
#include "ksimpleconfig.h"

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
	if( pEvent->modifiers() & Qt::ShiftModifier )   keyQt |= Qt::SHIFT;
	if( pEvent->modifiers() & Qt::ControlModifier ) keyQt |= Qt::CTRL;
	if( pEvent->modifiers() & Qt::AltModifier )     keyQt |= Qt::ALT;
	if( pEvent->modifiers() & Qt::MetaModifier )     keyQt |= Qt::META;
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

	QString sKey = sSpec.trimmed();
	if( sKey.startsWith( "default(" ) && sKey.endsWith( ")" ) )
		sKey = sKey.mid( 8, sKey.length() - 9 );
	// i.e., "Ctrl++" = "Ctrl+Plus"
	if( sKey.endsWith( "++" ) )
		sKey = sKey.left( sKey.length() - 1 ) + "plus";
	QStringList rgs = sKey.split( '+', QString::KeepEmptyParts);

	int i;
	// Check for modifier keys first.
	for( i = 0; i < rgs.size(); i++ ) {
		QString s = rgs[i].toLower();
		if( s == "shift" )     m_mod |= KKey::SHIFT;
		else if( s == "ctrl" ) m_mod |= KKey::CTRL;
		else if( s == "alt" )  m_mod |= KKey::ALT;
		else if( s == "win" )  m_mod |= KKey::WIN;
		else if( s == "meta" ) m_mod |= KKey::WIN;
		else {
			uint m = KKeyServer::stringUserToMod( s );
			if( m != 0 ) m_mod |= m;
			else break;
		}
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
	m_seq = QKeySequence();
	m_bTriggerOnRelease = false;
}

bool KKeySequence::init( const QKeySequence& seq )
{
	m_seq = seq;
	return true;
}

bool KKeySequence::init( const KKey& key )
{
	m_seq = QKeySequence(key.keyCodeQt());
	return true;
}

bool KKeySequence::init( const KKeySequence& seq )
{
	m_seq = seq.m_seq;
	return true;
}

bool KKeySequence::init( const QString& s )
{
	m_seq = QKeySequence(s);
	return true;
}

uint KKeySequence::count() const
{
	return m_seq.count();
}

const KKey KKeySequence::key( uint i ) const
{
	return KKey(m_seq[i]);
}

bool KKeySequence::isTriggerOnRelease() const
	{ return m_bTriggerOnRelease; }

bool KKeySequence::setKey( uint iKey, const KKey& key )
{
	if(iKey >= 4) return false;

	int keys[4];
	for(int i=0;i<4;++i) {
		keys[i] = m_seq[i];
	}

	keys[iKey] = key.keyCodeQt();

	m_seq = QKeySequence(keys[0], keys[1], keys[2], keys[3]);

	return true;
}

bool KKeySequence::isNull() const
{
	return m_seq.isEmpty();
}

bool KKeySequence::startsWith( const KKeySequence& seq ) const
{
	if( m_seq.count() != seq.m_seq.count()) return false;

	for( uint i = 0; i < m_seq.count(); ++i ) {
		if ( m_seq[i] != seq.m_seq[i] ) return false;
	}

	return true;
}

int KKeySequence::compare( const KKeySequence& seq ) const
{

	for( uint i = 0; i < m_seq.count(); ++i ) {
		if ( m_seq[i] != seq.m_seq[i] ) return m_seq[i] - seq.m_seq[i];
	}

	return 0;
}

QKeySequence KKeySequence::qt() const
{
	return m_seq;
}

int KKeySequence::keyCodeQt() const
{
	return m_seq[0];
}

QString KKeySequence::toString() const
{
	return m_seq;
}

QString KKeySequence::toStringInternal() const
{
	// FIXME: probably shouldn't be the same as toString()
	return m_seq;
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
	return init( QKeySequence( keyQt ) );
}

bool KShortcut::init( const QKeySequence& keySeq )
{
	m_nSeqs = 1;
	m_seq[0] = keySeq;
	return true;
}

bool KShortcut::init( const KKey& spec )
{
	return init( QKeySequence(spec.keyCodeQt()) );
}

bool KShortcut::init( const KKeySequence& seq )
{
	m_nSeqs = 1;
	m_seq[0] = seq;
	return true;
}

bool KShortcut::init( const KShortcut& cut )
{
	m_nSeqs = cut.m_nSeqs;
	for( uint i = 0; i < m_nSeqs; i++ )
		m_seq[i] = cut.m_seq[i];
	return true;
}

bool KShortcut::init( const QString& s )
{
	bool bRet = true;
	QStringList rgs = s.split( ';');

	if( s == "none" || rgs.size() == 0 )
		clear();
	else if( rgs.size() <= MAX_SEQUENCES ) {
		m_nSeqs = rgs.size();
		for( uint i = 0; i < m_nSeqs; i++ ) {
			QString& sSeq = rgs[i];
			if( sSeq.startsWith( "default(" ) )
				sSeq = sSeq.mid( 8, sSeq.length() - 9 );
			m_seq[i].init( sSeq );
			//kdDebug(125) << "*\t'" << sSeq << "' => " << m_seq[i].toStringInternal() << endl;
		}
	} else {
		clear();
		bRet = false;
	}

	if( !s.isEmpty() ) {
#ifndef NDEBUG
		QString sDebug;
		QTextStream os( &sDebug, QIODevice::WriteOnly );
		os << "KShortcut::init( \"" << s << "\" ): ";
#endif
		for( uint i = 0; i < m_nSeqs; i++ ) {
#ifndef NDEBUG
			os << " m_seq[" << i << "]: ";
#endif
			KKeyServer::Variations vars;
			vars.init( m_seq[i].key(0), true );
#ifndef NDEBUG
			for( uint j = 0; j < vars.count(); j++ )
				os << QString::number(vars.m_rgkey[j].keyCodeQt(),16) << ',';
#endif
		}
#ifndef NDEBUG
		kdDebug(125) << sDebug << endl;
#endif
	}

	return bRet;
}

uint KShortcut::count() const
{
	return m_nSeqs;
}

const KKeySequence KShortcut::seq( uint i ) const
{
	return KKeySequence( m_seq[i] );
}

int KShortcut::keyCodeQt() const
{
	if( m_nSeqs >= 1 )
		return m_seq[0].keyCodeQt();
	return 0;
}

bool KShortcut::isNull() const
{
	return m_nSeqs == 0;
}

int KShortcut::compare( const KShortcut& cut ) const
{
	/*
	  for( uint i = 0; i < m_seq.count(); ++i ) {
	  if ( m_seq[i] != cut.m_seq[i] ) return (int)(m_seq[i] - cut.m_seq[i]);
	  }
	  return 0;
	*/
	for( uint i = 0; i < m_nSeqs && i < cut.m_nSeqs; i++ ) {
		int ret = m_seq[i].compare( cut.m_seq[i] );
		if( ret != 0 )
			return ret;
	}
	return m_nSeqs - cut.m_nSeqs;
}

bool KShortcut::contains( const KKey& key ) const
{
	for( uint i = 0; i < m_nSeqs; i++ ) {
		if( !m_seq[i].isNull()
			&& m_seq[i].key(0) == key )
			return true;
	}
	return false;
}

bool KShortcut::contains( const KKeyNative& keyNative ) const
{
	KKey key = keyNative.key();
	key.simplify();
	return contains( KKeySequence(key) );
}

bool KShortcut::contains( const KKeySequence& seq ) const
{
	for( uint i = 0; i < count(); i++ ) {
		if ( m_seq[i].m_seq.matches(seq.m_seq) == QKeySequence::ExactMatch )
			return true;
	}
	return false;
}

bool KShortcut::setSeq( uint iSeq, const KKeySequence& seq )
{
	// TODO: check if seq is null, and act accordingly.
	if(iSeq >= MAX_SEQUENCES) return false;

	if( iSeq <= m_nSeqs && iSeq < MAX_SEQUENCES ) {
		m_seq[iSeq] = seq;
		if( iSeq == m_nSeqs )
			m_nSeqs++;
		return true;
	} else
		return false;
}

void KShortcut::remove( const KKeySequence& seq )
{
	if (seq.isNull()) return;

	for( uint iSeq = 0; iSeq < m_nSeqs; iSeq++ )
	{
		if (m_seq[iSeq] == seq)
		{
			for( uint jSeq = iSeq + 1; jSeq < m_nSeqs; jSeq++)
				m_seq[jSeq-1] = m_seq[jSeq];
			m_nSeqs--;
		}
	}
}

bool KShortcut::append( const KKeySequence& seq )
{
	if( m_nSeqs < MAX_SEQUENCES ) {
		if( !seq.isNull() ) {
			m_seq[m_nSeqs] = seq;
			m_nSeqs++;
		}
		return true;
	} else
		return false;
}

KShortcut::operator QKeySequence () const
{
	if ( count() >= 1 )
		return m_seq[0].qt();
	return QKeySequence();
}

QString KShortcut::toString() const
{
	QString s;
	for( uint i = 0; i < count(); i++ ) {
		s += m_seq[i].toString();
		if( i < count() - 1 )
			s += ';';
	}
	return s;
}

QString KShortcut::toStringInternal( const KShortcut* pcutDefault ) const
{
	QString s;

	for( uint i = 0; i < count(); i++ ) {
		const KKeySequence& seq = m_seq[i];
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
