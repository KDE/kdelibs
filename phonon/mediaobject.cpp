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
#include "factory.h"
#include "bytestreaminterface.h"
#include "mediaobjectinterface.h"

#include <kdebug.h>

#define PHONON_CLASSNAME MediaObject
#define PHONON_INTERFACENAME MediaObjectInterface

namespace Phonon
{
PHONON_HEIR_IMPL( AbstractMediaProducer )

MediaObject::MediaObject( Phonon::MediaObjectPrivate& dd, QObject* parent )
	: AbstractMediaProducer( dd, parent )
{
}

PHONON_INTERFACE_GETTER( KUrl, url, d->url )
PHONON_INTERFACE_GETTER( qint64, totalTime, -1 )
PHONON_GETTER( qint32, aboutToFinishTime, d->aboutToFinishTime )

qint64 MediaObject::remainingTime() const
{
	K_D( const MediaObject );
	if( !d->backendObject )
		return -1;
	qint64 ret;
	if( !BACKEND_GET( qint64, ret, "remainingTime" ) )
		ret = totalTime() - currentTime();
	if( ret < 0 )
		return -1;
	return ret;
}

void MediaObject::setUrl( const KUrl& url )
{
	K_D( MediaObject );
	d->url = url;
	if( iface() )
	{
		stop(); // first call stop as that often is the expected state
		                    // for setting a new URL
		INTERFACE_CALL1( setUrl, url );
		//FIXME: the stateChanged signal will be emitted. Perhaps it should be
		//disabled for the setUrl call and then replayed when it didn't go into
		//ErrorState
		if( state() == Phonon::ErrorState )
		{
			d->deleteIface();
			//at this point MediaObject uses a ByteStream
			//instead and sends the data it receives from the KIO Job via writeBuffer.
			//This essentially makes all media frameworks read data via KIO...
			d->setupKioStreaming();
		}
	}
}

void MediaObject::stop()
{
	AbstractMediaProducer::stop();
	K_D( MediaObject );
	if( d->kiojob )
	{
		d->kiojob->kill();
		// if( do pre-buffering )
		d->setupKioJob();
	}
}

PHONON_SETTER( setAboutToFinishTime, aboutToFinishTime, qint32 )

void MediaObjectPrivate::setupKioJob()
{
	K_Q( MediaObject );
	Q_ASSERT( backendObject );

	kiojob = KIO::get( url, false, false );
	kiojob->addMetaData( "UserAgent", QLatin1String( "KDE Phonon" ) );
	QObject::connect( kiojob, SIGNAL(data(KIO::Job*,const QByteArray&)),
			q, SLOT(_k_bytestreamData(KIO::Job*,const QByteArray&)) );
	QObject::connect( kiojob, SIGNAL(result(KJob*)),
			q, SLOT(_k_bytestreamResult(KJob*)) );
	QObject::connect( kiojob, SIGNAL(totalSize(KJob*, qulonglong)),
			q, SLOT(_k_bytestreamTotalSize(KJob*,qulonglong)) );

	QObject::connect( backendObject, SIGNAL(finished()), q, SIGNAL(finished()) );
	QObject::connect( backendObject, SIGNAL(aboutToFinish(qint32)), q, SIGNAL(aboutToFinish(qint32)) );
	QObject::connect( backendObject, SIGNAL(length(qint64)), q, SIGNAL(length(qint64)) );

	QObject::connect( backendObject, SIGNAL(needData()), q, SLOT(_k_bytestreamNeedData()) );
	QObject::connect( backendObject, SIGNAL(enoughData()), q, SLOT(_k_bytestreamEnoughData()) );

	pBACKEND_CALL1( "setStreamSeekable", bool, false ); //FIXME: KIO doesn't support seeking at this point
	//connect( backendObject, SIGNAL(seekStream(qint64)), kiojob, SLOT(
}

void MediaObjectPrivate::setupKioStreaming()
{
	K_Q( MediaObject );
	Q_ASSERT( backendObject == 0 );
	backendObject = Factory::self()->createByteStream( q );
	if( backendObject )
	{
		QObject::connect( backendObject, SIGNAL( destroyed( QObject* ) ), q, SLOT( _k_cleanupByteStream() ) );
		//setupIface for ByteStream
		if( kiojob )
			kiojob->kill();
		setupKioJob();
		static_cast<AbstractMediaProducer*>( q )->setupIface();
	}
}

void MediaObjectPrivate::_k_bytestreamNeedData()
{
	if( kiojob->isSuspended() )
		kiojob->resume();
}

void MediaObjectPrivate::_k_bytestreamEnoughData()
{
	if( !kiojob->isSuspended() )
		kiojob->suspend();
}

void MediaObjectPrivate::_k_bytestreamData( KIO::Job*, const QByteArray& data )
{
	ByteStreamInterface* bs = qobject_cast<ByteStreamInterface*>( backendObject );
	bs->writeData( data );
}

void MediaObjectPrivate::_k_bytestreamResult( KJob* job )
{
	ByteStreamInterface* bs = qobject_cast<ByteStreamInterface*>( backendObject );
	bs->endOfData();
	kiojob = 0;

	if( job->error() )
	{
		//TODO
	}
}

void MediaObjectPrivate::_k_bytestreamTotalSize( KJob*, qulonglong size )
{
	pBACKEND_CALL1( "setStreamSize", quint64, size );
}

void MediaObjectPrivate::_k_cleanupByteStream()
{
	if( kiojob )
	{
		kiojob->kill();
		kiojob = 0;
	}
}

bool MediaObjectPrivate::aboutToDeleteIface()
{
	//kDebug( 600 ) << k_funcinfo << endl;
	pBACKEND_GET( qint32, aboutToFinishTime, "aboutToFinishTime" );
	return AbstractMediaProducerPrivate::aboutToDeleteIface();
}

// setupIface is not called for ByteStream
void MediaObject::setupIface()
{
	K_D( MediaObject );
	Q_ASSERT( d->backendObject );
	//kDebug( 600 ) << k_funcinfo << endl;
	AbstractMediaProducer::setupIface();

	connect( d->backendObject, SIGNAL( finished() ), SIGNAL( finished() ) );
	connect( d->backendObject, SIGNAL( aboutToFinish( qint32 ) ), SIGNAL( aboutToFinish( qint32 ) ) );
	connect( d->backendObject, SIGNAL( length( qint64 ) ), SIGNAL( length( qint64 ) ) );

	// set up attributes
	if( !d->url.isEmpty() )
		INTERFACE_CALL1( setUrl, d->url );
	if( state() == Phonon::ErrorState )
	{
		d->deleteIface();
		//at this point MediaObject uses a ByteStream
		//instead and sends the data it receives from the KIO Job via writeBuffer.
		//This essentially makes all media frameworks read data via KIO...
		d->setupKioStreaming();
		return;
	}
	BACKEND_CALL1( "setAboutToFinishTime", qint32, d->aboutToFinishTime );
}

} //namespace Phonon

#include "mediaobject.moc"

// vim: sw=4 ts=4 tw=80 noet
