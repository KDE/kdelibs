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
		d->iface()->setNext( next->iface() );
}

qint32 MediaQueue::timeBetweenMedia() const
{
	K_D( const MediaQueue );
	return d->iface() ? d->iface()->timeBetweenMedia() : d->timeBetweenMedia;
}

void MediaQueue::setTimeBetweenMedia( qint32 milliseconds )
{
	K_D( MediaQueue );
	if( iface() )
		d->iface()->setTimeBetweenMedia( milliseconds );
	else
		d->timeBetweenMedia = milliseconds;
}

bool MediaQueue::doCrossfade() const
{
	K_D( const MediaQueue );
	return d->iface() ? d->iface()->doCrossfade() : d->doCrossfade;
}

void MediaQueue::setDoCrossfade( bool doCrossfade )
{
	K_D( MediaQueue );
	if( iface() )
		d->iface()->setDoCrossfade( doCrossfade );
	else
		d->doCrossfade = doCrossfade;
}

bool MediaQueuePrivate::aboutToDeleteIface()
{
	doCrossfade = iface()->doCrossfade();
	timeBetweenMedia = iface()->timeBetweenMedia();
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
	Q_ASSERT( d->iface() );

	connect( d->iface()->qobject(), SIGNAL( needNextMediaObject() ), SLOT( _k_needNextMediaObject() ) );
}
} // namespace Phonon

#include "mediaqueue.moc"
// vim: sw=4 ts=4 noet
