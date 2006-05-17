/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

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
#include "mediaobject.h"
#include "mediaobject_p.h"
#include "ifaces/mediaobject.h"
#include "factory.h"

#include <kdebug.h>

namespace Phonon
{
PHONON_HEIR_IMPL( MediaObject, AbstractMediaProducer )

KUrl MediaObject::url() const
{
	K_D( const MediaObject );
	return d->iface() ? d->iface()->url() : d->url;
}

qint64 MediaObject::totalTime() const
{
	K_D( const MediaObject );
	return d->iface() ? d->iface()->totalTime() : -1;
}

qint64 MediaObject::remainingTime() const
{
	K_D( const MediaObject );
	return d->iface() ? d->iface()->remainingTime() : -1;
}

qint32 MediaObject::aboutToFinishTime() const
{
	K_D( const MediaObject );
	return d->iface() ? d->iface()->aboutToFinishTime() : d->aboutToFinishTime;
}

void MediaObject::setUrl( const KUrl& url )
{
	K_D( MediaObject );
	d->url = url;
	if( iface() )
	{
		d->iface()->stop(); // first call stop as that often is the expected state
		                    // for setting a new URL
		d->iface()->setUrl( url );
		if( state() == Phonon::ErrorState )
		{
			d->deleteIface();
			//TODO: at this point MediaObject should try to use an Ifaces::ByteStream
			//instead and send the data it receives from a KIO Job via writeBuffer.
			//This essentially makes all media frameworks read data via KIO...
		}
	}
}

void MediaObject::setAboutToFinishTime( qint32 newAboutToFinishTime )
{
	K_D( MediaObject );
	//kDebug( 600 ) << k_funcinfo << endl;
	if( d->iface() )
		d->iface()->setAboutToFinishTime( newAboutToFinishTime );
	else
		d->aboutToFinishTime = newAboutToFinishTime;
}

bool MediaObjectPrivate::aboutToDeleteIface()
{
	//kDebug( 600 ) << k_funcinfo << endl;
	if( iface() )
		aboutToFinishTime = iface()->aboutToFinishTime();
	return AbstractMediaProducerPrivate::aboutToDeleteIface();
}

void MediaObject::setupIface()
{
	K_D( MediaObject );
	Q_ASSERT( d->iface() );
	//kDebug( 600 ) << k_funcinfo << endl;
	AbstractMediaProducer::setupIface();

	connect( d->iface()->qobject(), SIGNAL( finished() ), SIGNAL( finished() ) );
	connect( d->iface()->qobject(), SIGNAL( aboutToFinish( qint32 ) ), SIGNAL( aboutToFinish( qint32 ) ) );
	connect( d->iface()->qobject(), SIGNAL( length( qint64 ) ), SIGNAL( length( qint64 ) ) );

	// set up attributes
	if( !d->url.isEmpty() )
		d->iface()->setUrl( d->url );
	if( state() == Phonon::ErrorState )
	{
		d->deleteIface();
		//TODO: at this point MediaObject should try to use an Ifaces::ByteStream
		//instead and send the data it receives from a KIO Job via writeBuffer.
		//This essentially makes all media frameworks read data via KIO...
		return;
	}
	d->iface()->setAboutToFinishTime( d->aboutToFinishTime );
}

} //namespace Phonon

#include "mediaobject.moc"

// vim: sw=4 ts=4 tw=80 noet
