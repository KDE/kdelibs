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

#include "k3keysequence.h"

#include <qkeysequence.h>
#include <qstring.h>
#include "kdebug.h"
#include <qstringlist.h>
#include "kkeynative.h"

static K3KeySequence* g_pseq = 0;

//---------------------------------------------------------------------
// K3KeySequence
//---------------------------------------------------------------------

K3KeySequence::K3KeySequence()                          { clear(); }
K3KeySequence::K3KeySequence( const QKeySequence& seq ) { init( seq ); }
K3KeySequence::K3KeySequence( const KKey& key )         { init( key ); }
K3KeySequence::K3KeySequence( const K3KeySequence& seq ) { init( seq ); }
K3KeySequence::K3KeySequence( const QString& s )        { init( s ); }

K3KeySequence::~K3KeySequence()
{
}

void K3KeySequence::clear()
{
	m_nKeys = 0;
	m_bTriggerOnRelease = false;
}

bool K3KeySequence::init( const QKeySequence& seq )
{
	clear();
	if( !seq.isEmpty() ) {
		for( uint i = 0; i < seq.count(); i++ ) {
			m_rgkey[i].init( seq[i] );
			if( m_rgkey[i].isNull() )
				return false;
		}
		m_nKeys = seq.count();
		m_bTriggerOnRelease = false;
	}
	return true;
}

bool K3KeySequence::init( const KKey& key )
{
	if( !key.isNull() ) {
		m_nKeys = 1;
		m_rgkey[0].init( key );
		m_bTriggerOnRelease = false;
	} else
		clear();
	return true;
}

bool K3KeySequence::init( const K3KeySequence& seq )
{
	m_bTriggerOnRelease = false;
	m_nKeys = seq.m_nKeys;
	for( uint i = 0; i < m_nKeys; i++ ) {
		if( seq.m_rgkey[i].isNull() ) {
			kdDebug(125) << "K3KeySequence::init( seq ): key[" << i << "] is null." << endl;
			m_nKeys = 0;
			return false;
		}
		m_rgkey[i] = seq.m_rgkey[i];
	}
	return true;
}

bool K3KeySequence::init( const QString& s )
{
	m_bTriggerOnRelease = false;
	//kdDebug(125) << "K3KeySequence::init( " << s << " )" << endl;
	QStringList rgs = QStringList::split( ',', s );
	if( s == "none" || rgs.size() == 0 ) {
		clear();
		return true;
	} else if( rgs.size() <= MAX_KEYS ) {
		m_nKeys = rgs.size();
		for( uint i = 0; i < m_nKeys; i++ ) {
			m_rgkey[i].init( KKey(rgs[i]) );
			//kdDebug(125) << "\t'" << rgs[i] << "' => " << m_rgkey[i].toStringInternal() << endl;
		}
		return true;
	} else {
		clear();
		return false;
	}
}

uint K3KeySequence::count() const
{
	return m_nKeys;
}

const KKey& K3KeySequence::key( uint i ) const
{
	if( i < m_nKeys )
		return m_rgkey[i];
	else
		return KKey::null();
}

bool K3KeySequence::isTriggerOnRelease() const
	{ return m_bTriggerOnRelease; }

bool K3KeySequence::setKey( uint iKey, const KKey& key )
{
	if( iKey <= m_nKeys && iKey < MAX_KEYS ) {
		m_rgkey[iKey].init( key );
		if( iKey == m_nKeys )
			m_nKeys++;
		return true;
	} else
		return false;
}

bool K3KeySequence::isNull() const
{
	return m_nKeys == 0;
}

bool K3KeySequence::startsWith( const K3KeySequence& seq ) const
{
	if( m_nKeys < seq.m_nKeys )
		return false;

	for( uint i = 0; i < seq.m_nKeys; i++ ) {
		if( m_rgkey[i] != seq.m_rgkey[i] )
			return false;
	}

	return true;
}

int K3KeySequence::compare( const K3KeySequence& seq ) const
{
	for( uint i = 0; i < m_nKeys && i < seq.m_nKeys; i++ ) {
		int ret = m_rgkey[i].compare( seq.m_rgkey[i] );
		if( ret != 0 )
			return ret;
	}
	if( m_nKeys != seq.m_nKeys )
		return m_nKeys - seq.m_nKeys;
	else
		return 0;
}

QKeySequence K3KeySequence::qt() const
{
	int k[4] = { 0, 0, 0, 0 };
	
	for( uint i = 0; i < count(); i++ )
		k[i] = KKeyNative(key(i)).keyCodeQt();
	QKeySequence seq( k[0], k[1], k[2], k[3] );
	return seq;
}

int K3KeySequence::keyCodeQt() const
{
	return (count() == 1) ? KKeyNative(key(0)).keyCodeQt() : 0;
}

QString K3KeySequence::toString() const
{
	if( m_nKeys < 1 ) return QString();

	QString s;
	s = m_rgkey[0].toString();
	for( uint i = 1; i < m_nKeys; i++ ) {
		s += ",";
		s += m_rgkey[i].toString();
	}

	return s;
}

QString K3KeySequence::toStringInternal() const
{
	if( m_nKeys < 1 ) return QString();

	QString s;
	s = m_rgkey[0].toStringInternal();
	for( uint i = 1; i < m_nKeys; i++ ) {
		s += ",";
		s += m_rgkey[i].toStringInternal();
	}

	return s;
}

K3KeySequence& K3KeySequence::null()
{
	if( !g_pseq )
		g_pseq = new K3KeySequence;
	if( !g_pseq->isNull() )
		g_pseq->clear();
	return *g_pseq;
}
