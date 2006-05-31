/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "mediaqueue.h"
#include "mediaqueue_p.h"

namespace Phonon
{
PHONON_OBJECT_IMPL( MediaQueue )

MediaObject* MediaQueue::current() const
{
	K_D( const MediaQueue );
	return d->current;
}

MediaObject* MediaQueue::next() const
{
	K_D( const MediaQueue );
	return d->next;
}

void MediaQueue::setNext( MediaObject* next )
{
	K_D( MediaQueue );
	if( d->next )
		kWarning( 600 ) << "MediaQueue::setNext called overriding another MediaObject that was waiting" << endl;
	d->next = next;
	if( iface() )
		BACKEND_CALL1( "setNext", QObject*, next->iface() );
}

PHONON_GETTER( MediaQueue, qint32, timeBetweenMedia, d->timeBetweenMedia )
PHONON_SETTER( MediaQueue, setTimeBetweenMedia, timeBetweenMedia, qint32 )
PHONON_GETTER( MediaQueue, bool, doCrossfade, d->doCrossfade )
PHONON_SETTER( MediaQueue, setDoCrossfade, doCrossfade, bool )

bool MediaQueuePrivate::aboutToDeleteIface()
{
	pBACKEND_GET( bool, doCrossfade, "doCrossfade" );
	pBACKEND_GET( qint32, timeBetweenMedia, "timeBetweenMedia" );
	return true;
}

void MediaQueuePrivate::_k_needNextMediaObject()
{
	K_Q( MediaQueue );
	current = next;
	next = 0;
	emit q->needNextMediaObject();
}

void MediaQueue::setupIface()
{
	K_D( MediaQueue );
	Q_ASSERT( d->backendObject );

	connect( d->backendObject, SIGNAL( needNextMediaObject() ), SLOT( _k_needNextMediaObject() ) );
}
} // namespace Phonon

#include "mediaqueue.moc"
// vim: sw=4 ts=4 noet
