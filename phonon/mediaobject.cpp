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
#include "ifaces/bytestream.h"
#include "factory.h"

#include <kdebug.h>

namespace Phonon
{
PHONON_HEIR_IMPL( MediaObject, AbstractMediaProducer )

KUrl MediaObject::url() const
{
	K_D( const MediaObject );
	if( d->bytestream )
		return d->url;
	return d->iface() ? d->iface()->url() : d->url;
}

qint64 MediaObject::totalTime() const
{
	K_D( const MediaObject );
	if( d->bytestream )
		return d->bytestream->totalTime();
	return d->iface() ? d->iface()->totalTime() : -1;
}

qint64 MediaObject::remainingTime() const
{
	K_D( const MediaObject );
	if( d->bytestream )
		return d->bytestream->remainingTime();
	return d->iface() ? d->iface()->remainingTime() : -1;
}

qint32 MediaObject::aboutToFinishTime() const
{
	K_D( const MediaObject );
	if( d->bytestream )
		return d->bytestream->aboutToFinishTime();
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
			//at this point MediaObject uses an Ifaces::ByteStream
			//instead and sends the data it receives from the KIO Job via writeBuffer.
			//This essentially makes all media frameworks read data via KIO...
			d->setupKioStreaming();
		}
	}
}

void MediaObject::setAboutToFinishTime( qint32 newAboutToFinishTime )
{
	K_D( MediaObject );
	//kDebug( 600 ) << k_funcinfo << endl;
	if( d->bytestream )
		d->bytestream->setAboutToFinishTime( newAboutToFinishTime );
	else if( iface() )
		d->iface()->setAboutToFinishTime( newAboutToFinishTime );
	else
		d->aboutToFinishTime = newAboutToFinishTime;
}

void MediaObjectPrivate::setupKioStreaming()
{
	K_Q( MediaObject );
	Q_ASSERT( bytestream == 0 );
	bytestream = Factory::self()->createByteStream( q );
	if( bytestream )
	{
		AbstractMediaProducerPrivate::setIface( static_cast<Ifaces::AbstractMediaProducer*>( bytestream ) );
		//setupIface for ByteStream
		if( kiojob )
			kiojob->kill();
		kiojob = KIO::get( url, false, false );
		kiojob->addMetaData( "UserAgent", QLatin1String( "KDE Phonon" ) );
		QObject::connect( kiojob, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
				q, SLOT( _k_bytestreamData( KIO::Job*, const QByteArray& ) ) );
		QObject::connect( kiojob, SIGNAL( result( KIO::Job* ) ),
				q, SLOT( _k_bytestreamResult( KIO::Job* ) ) );
		QObject::connect( kiojob, SIGNAL( totalSize( KIO::Job*, KIO::filesize_t ) ),
				q, SLOT( _k_bytestreamTotalSize( KIO::Job*, KIO::filesize_t ) ) );

		QObject::connect( bytestream->qobject(), SIGNAL( finished() ), q, SIGNAL( finished() ) );
		QObject::connect( bytestream->qobject(), SIGNAL( aboutToFinish( qint32 ) ), q, SIGNAL( aboutToFinish( qint32 ) ) );
		QObject::connect( bytestream->qobject(), SIGNAL( length( qint64 ) ), q, SIGNAL( length( qint64 ) ) );

		QObject::connect( bytestream->qobject(), SIGNAL( needData() ), kiojob, SLOT( resume() ) );
		QObject::connect( bytestream->qobject(), SIGNAL( enoughData() ), kiojob, SLOT( suspend() ) );

		bytestream->setStreamSeekable( false ); //FIXME: KIO doesn't support seeking at this point
		//connect( bytestream->qobject(), SIGNAL( seekStream( qint64 ), kiojob, SLOT(

		static_cast<AbstractMediaProducer*>( q )->setupIface();
	}
}

void MediaObjectPrivate::_k_bytestreamData( KIO::Job*, const QByteArray& data )
{
	Q_ASSERT( bytestream );
	bytestream->writeData( data );
}

void MediaObjectPrivate::_k_bytestreamResult( KIO::Job* job )
{
	Q_ASSERT( bytestream );
	bytestream->endOfData();
	kiojob = 0;

	if( job->error() )
	{
		//TODO
	}
}

void MediaObjectPrivate::_k_bytestreamTotalSize( KIO::Job*, KIO::filesize_t size )
{
	Q_ASSERT( bytestream );
	bytestream->setStreamSize( size );
}

bool MediaObjectPrivate::aboutToDeleteIface()
{
	//kDebug( 600 ) << k_funcinfo << endl;
	if( bytestream )
	{
		aboutToFinishTime = bytestream->aboutToFinishTime();
	}
	else if( iface() )
	{
		aboutToFinishTime = iface()->aboutToFinishTime();
	}
	return AbstractMediaProducerPrivate::aboutToDeleteIface();
}

// setupIface is not called for ByteStream
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
		//at this point MediaObject uses an Ifaces::ByteStream
		//instead and sends the data it receives from the KIO Job via writeBuffer.
		//This essentially makes all media frameworks read data via KIO...
		d->setupKioStreaming();
		return;
	}
	d->iface()->setAboutToFinishTime( d->aboutToFinishTime );
}

} //namespace Phonon

#include "mediaobject.moc"

// vim: sw=4 ts=4 tw=80 noet
