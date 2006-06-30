// -*- indent-tabs-mode:t; tab-width:4; c-basic-offset: 4 -*-
/*  This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

class KShortcutPrivate : public QSharedData
{
public:
	QList<QKeySequence> seq;
};

//---------------------------------------------------------------------
// KShortcut
//---------------------------------------------------------------------

KShortcut::KShortcut()
	: d(new KShortcutPrivate)
{
}

KShortcut::KShortcut( int keyQt )
	: d(new KShortcutPrivate)
{
	init( keyQt );
}

KShortcut::KShortcut( const QKeySequence& key )
	: d(new KShortcutPrivate)
{
	init( key );
}

KShortcut::KShortcut( const QKeySequence& key1, const QKeySequence& key2 )
	: d(new KShortcutPrivate)
{
	init( key1 );
	d->seq.append(key2);
}

KShortcut::KShortcut( const KShortcut& cut )
	: d(cut.d)
{
}

KShortcut::KShortcut( const QString& s )
	: d(new KShortcutPrivate)
{
	init( s );
}

KShortcut::~KShortcut()
{
}

void KShortcut::clear()
{
	d->seq.clear();
}

bool KShortcut::init( int keyQt )
{
	if ( keyQt )
		return init( QKeySequence( keyQt ) );
	else {
		clear();
		return true;
	}
}

bool KShortcut::init( const QKeySequence& keySeq )
{
	clear();
	d->seq.append(keySeq);
	return true;
}

bool KShortcut::init( const KShortcut& cut )
{
	clear();
	d->seq = cut.d->seq;
	return true;
}

bool KShortcut::init( const QString& s )
{
	clear();

	if( s != "none" ) {
		foreach (const QString& sequence, s.split( ';')) {
			if( sequence.startsWith( "default(" ) )
				d->seq.append(QKeySequence::fromString(sequence.mid( 8, sequence.length() - 9 )));
			else
				d->seq.append(QKeySequence::fromString(sequence));
		}
	}

	return true;
}

int KShortcut::count() const
{
	return d->seq.count();
}

const QKeySequence KShortcut::seq( int i ) const
{
	if (i >= 0 && i < d->seq.count())
		return d->seq[i];

	return QKeySequence();
}

int KShortcut::keyQt() const
{
	foreach (const QKeySequence& seq, d->seq)
		if (!seq.isEmpty())
			return seq[0];

	return 0;
}

bool KShortcut::isNull() const
{
	return d->seq.count() == 0;
}

int KShortcut::compare( const KShortcut& cut ) const
{
	if (d == cut.d)
		return 0;

	if (d->seq.count() != cut.d->seq.count())
		return d->seq.count() > cut.d->seq.count() ? 1 : -1;

	for (int i = 0; i < d->seq.count(); ++i)
		if (d->seq[i] != cut.d->seq[i])
			return d->seq[i] > cut.d->seq[i] ? 1 : -1;

	return 0;
}

bool KShortcut::operator == ( const KShortcut& cut ) const
{
	return d == cut.d || d->seq == cut.d->seq;
}

bool KShortcut::contains( int keyQt ) const
{
	foreach (const QKeySequence& seq, d->seq)
		if (seq.count() && keyQt == seq[0])
			return true;

	return false;
}

bool KShortcut::contains( const QKeySequence& otherSeq ) const
{
	foreach (const QKeySequence& seq, d->seq)
		if (seq.matches(otherSeq) == QKeySequence::ExactMatch)
			return true;

	return false;
}

void KShortcut::setSeq( int iSeq, const QKeySequence& seq )
{
	if (iSeq < d->seq.count()) {
		d->seq[iSeq] = seq;
	} else {
		d->seq.append(seq);
	}
}

void KShortcut::remove( const QKeySequence& seq )
{
	if (seq.isEmpty())
		return;

	QMutableListIterator<QKeySequence> it = d->seq;
	while (it.hasNext()) {
		it.next();
		if (it.value() == seq) {
			it.remove();
		}
	}
}

void KShortcut::append( const QKeySequence& seq )
{
	if (!seq.isEmpty()) {
		d->seq.append(seq);
	}
}

KShortcut::operator QKeySequence () const
{
	if ( count() >= 1 )
		return d->seq[0];
	return QKeySequence();
}

QString KShortcut::toString() const
{
	QString s;
	for( int i = 0; i < count(); i++ ) {
		s += d->seq[i].toString();
		if( i < count() - 1 )
			s += ';';
	}
	return s;
}

QString KShortcut::toStringInternal( const KShortcut* pcutDefault ) const
{
	QString s;

	for( int i = 0; i < count(); i++ ) {
		const QKeySequence& seq = d->seq[i];
		if( pcutDefault && i < pcutDefault->count() && seq == pcutDefault->seq(i) ) {
			s += "default(";
			s += seq.toString();
			s += ')';
		} else
			s += seq.toString();
		if( i < count() - 1 )
			s += ';';
	}

	return s;
}

const KShortcut& KShortcut::null()
{
	static KShortcut n;
	return n;
}

const QList<QKeySequence>& KShortcut::sequences() const
{
	return d->seq;
}
