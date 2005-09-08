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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "k3shortcut.h"
#include "kkeyserver.h"
#include "kdebug.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qkeysequence.h>

static K3Shortcut* g_pcut = 0;

//---------------------------------------------------------------------
// K3Shortcut
//---------------------------------------------------------------------

K3Shortcut::K3Shortcut()                            { clear(); }
K3Shortcut::K3Shortcut( int keyQt )                 { init( keyQt ); }
K3Shortcut::K3Shortcut( const QKeySequence& key )   { init( key ); }
K3Shortcut::K3Shortcut( const KKey& key )           { init( key ); }
K3Shortcut::K3Shortcut( const K3KeySequence& seq )   { init( seq ); }
K3Shortcut::K3Shortcut( const K3Shortcut& cut )      { init( cut ); }
K3Shortcut::K3Shortcut( const char* ps )            { init( QString(ps) ); }
K3Shortcut::K3Shortcut( const QString& s )          { init( s ); }

K3Shortcut::~K3Shortcut()
{
}

void K3Shortcut::clear()
{
	m_nSeqs = 0;
}

bool K3Shortcut::init( int keyQt )
{
	if( keyQt ) {
		m_nSeqs = 1;
		m_rgseq[0].init( QKeySequence(keyQt) );
	} else
		clear();
	return true;
}

bool K3Shortcut::init( const QKeySequence& key )
{
	m_nSeqs = 1;
	m_rgseq[0].init( key );
	return true;
}

bool K3Shortcut::init( const KKey& spec )
{
	m_nSeqs = 1;
	m_rgseq[0].init( spec );
	return true;
}

bool K3Shortcut::init( const K3KeySequence& seq )
{
	m_nSeqs = 1;
	m_rgseq[0] = seq;
	return true;
}

bool K3Shortcut::init( const K3Shortcut& cut )
{
	m_nSeqs = cut.m_nSeqs;
	for( uint i = 0; i < m_nSeqs; i++ )
		m_rgseq[i] = cut.m_rgseq[i];
	return true;
}

bool K3Shortcut::init( const QString& s )
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
		QTextStream os( &sDebug, QIODevice::WriteOnly );
		os << "K3Shortcut::init( \"" << s << "\" ): ";
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

uint K3Shortcut::count() const
{
	return m_nSeqs;
}

const K3KeySequence& K3Shortcut::seq( uint i ) const
{
	return (i < m_nSeqs) ? m_rgseq[i] : K3KeySequence::null();
}

int K3Shortcut::keyCodeQt() const
{
	if( m_nSeqs >= 1 )
		return m_rgseq[0].keyCodeQt();
	return QKeySequence();
}

bool K3Shortcut::isNull() const
{
	return m_nSeqs == 0;
}

int K3Shortcut::compare( const K3Shortcut& cut ) const
{
	for( uint i = 0; i < m_nSeqs && i < cut.m_nSeqs; i++ ) {
		int ret = m_rgseq[i].compare( cut.m_rgseq[i] );
		if( ret != 0 )
			return ret;
	}
	return m_nSeqs - cut.m_nSeqs;
}

bool K3Shortcut::contains( const KKey& key ) const
{
	return contains( K3KeySequence(key) );
}

bool K3Shortcut::contains( const KKeyNative& keyNative ) const
{
	KKey key = keyNative.key();
	key.simplify();

	for( uint i = 0; i < count(); i++ ) {
		if( !m_rgseq[i].isNull()
		    && m_rgseq[i].count() == 1
		    && m_rgseq[i].key(0) == key )
			return true;
	}
	return false;
}

bool K3Shortcut::contains( const K3KeySequence& seq ) const
{
	for( uint i = 0; i < count(); i++ ) {
		if( !m_rgseq[i].isNull() && m_rgseq[i] == seq )
			return true;
	}
	return false;
}

bool K3Shortcut::setSeq( uint iSeq, const K3KeySequence& seq )
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

void K3Shortcut::remove( const K3KeySequence& seq )
{
	if (seq.isNull()) return;
	
	for( uint iSeq = 0; iSeq < m_nSeqs; iSeq++ )
	{
		if (m_rgseq[iSeq] == seq)
		{
			for( uint jSeq = iSeq + 1; jSeq < m_nSeqs; jSeq++)
				m_rgseq[jSeq-1] = m_rgseq[jSeq];
			m_nSeqs--;
		}
	}
}

bool K3Shortcut::append( const K3KeySequence& seq )
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

bool K3Shortcut::append( const KKey& spec )
{
	if( m_nSeqs < MAX_SEQUENCES ) {
		m_rgseq[m_nSeqs].init( spec );
		m_nSeqs++;
		return true;
	} else
		return false;
}

bool K3Shortcut::append( const K3Shortcut& cut )
{
	uint seqs = m_nSeqs, co = cut.count();
	for( uint i=0; i<co; i++ ) {
	    if (!contains(cut.seq(i))) seqs++;
	}
	if( seqs > MAX_SEQUENCES ) return false;

	for( uint i=0; i<co; i++ ) {
		const K3KeySequence& seq = cut.seq(i);
		if(!contains(seq)) {
			m_rgseq[m_nSeqs] = seq;
			m_nSeqs++;
		}
	}
	return true;
}

K3Shortcut::operator QKeySequence () const
{
	if( count() >= 1 )
		return m_rgseq[0].qt();
	else
		return QKeySequence();
}

QString K3Shortcut::toString() const
{
	QString s;

	for( uint i = 0; i < count(); i++ ) {
		s += m_rgseq[i].toString();
		if( i < count() - 1 )
			s += ';';
	}

	return s;
}

QString K3Shortcut::toStringInternal( const K3Shortcut* pcutDefault ) const
{
	QString s;

	for( uint i = 0; i < count(); i++ ) {
		const K3KeySequence& seq = m_rgseq[i];
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

K3Shortcut& K3Shortcut::null()
{
	if( !g_pcut )
		g_pcut = new K3Shortcut;
	if( !g_pcut->isNull() )
		g_pcut->clear();
	return *g_pcut;
}
