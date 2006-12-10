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

#define PHONON_CLASSNAME MediaQueue

namespace Phonon
{
PHONON_HEIR_IMPL( MediaObject )

PHONON_GETTER( KUrl, nextUrl, d->nextUrl )

void MediaQueue::setNextUrl( const KUrl& url )
{
	K_D( MediaQueue );
	if( iface() )
		BACKEND_CALL1( "setNextUrl", KUrl, url );
	else
		d->nextUrl = url;
}

PHONON_GETTER( bool, doCrossfade, d->doCrossfade )
PHONON_SETTER( setDoCrossfade, doCrossfade, bool )
PHONON_GETTER( qint32, timeBetweenMedia, d->timeBetweenMedia )
PHONON_SETTER( setTimeBetweenMedia, timeBetweenMedia, qint32 )

bool MediaQueuePrivate::aboutToDeleteIface()
{
	pBACKEND_GET( KUrl, nextUrl, "nextUrl" );
	pBACKEND_GET( bool, doCrossfade, "doCrossfade" );
	pBACKEND_GET( qint32, timeBetweenMedia, "timeBetweenMedia" );
	return true;
}

void MediaQueuePrivate::_k_needNextUrl()
{
	K_Q( MediaQueue );
	nextUrl.clear();
	emit q->needNextUrl();
}

void MediaQueue::setupIface()
{
	K_D( MediaQueue );
	Q_ASSERT( d->backendObject );
	MediaObject::setupIface();

	connect( d->backendObject, SIGNAL( needNextUrl() ), SLOT( _k_needNextUrl() ) );
	BACKEND_CALL1( "setNextUrl", KUrl, d->nextUrl );
	BACKEND_CALL1( "setDoCrossfade", bool, d->doCrossfade );
	BACKEND_CALL1( "setTimeBetweenMedia", qint32, d->timeBetweenMedia );
}
} // namespace Phonon

#include "mediaqueue.moc"
// vim: sw=4 ts=4
