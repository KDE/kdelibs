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
#include <QCoreApplication>

#define PHONON_CLASSNAME MediaObject
#define PHONON_INTERFACENAME MediaObjectInterface

namespace Phonon
{
PHONON_HEIR_IMPL( AbstractMediaProducer )

MediaObject::MediaObject( Phonon::MediaObjectPrivate& dd, QObject* parent )
	: AbstractMediaProducer( dd, parent )
{
}

//PHONON_INTERFACE_GETTER( KUrl, url, d->url )
KUrl MediaObject::url() const 
{ 
	K_D( const MediaObject ); 
	if( d->backendObject ) 
	{
		MediaObjectInterface *iface = qobject_cast<MediaObjectInterface*>( d->backendObject );
		ByteStreamInterface *iface2 = qobject_cast<ByteStreamInterface*>( d->backendObject );

		if( iface )
			return iface->url();
		else if( iface2 && d->kiojob)
			return d->kiojob->url();
	}
	return KUrl(""); 
}

PHONON_GETTER( qint32, aboutToFinishTime, d->aboutToFinishTime )

qint64 MediaObject::totalTime() const
{
	K_D( const MediaObject );
	if( !d->backendObject )
		return -1;
	MediaObjectInterface *iface = qobject_cast<MediaObjectInterface*>( d->backendObject );
	if( iface )
		return iface->totalTime();
	else
	{
		ByteStreamInterface *iface2 = qobject_cast<ByteStreamInterface*>( d->backendObject );
		if( iface2 )
			return iface2->totalTime();
	}
	return -1;
}

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
		//the stateChanged signal will be filtered in _k_stateChanged.
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
	Phonon::State prevState = state();
	if( prevState == Phonon::StoppedState || prevState == Phonon::LoadingState )
		return;
	AbstractMediaProducer::stop();
	K_D( MediaObject );

	// if( do pre-buffering )
	// {
	ByteStreamInterface* bs = qobject_cast<ByteStreamInterface*>( d->backendObject );
	if (bs)
		d->_k_setupKioJob();
	// }
}

void MediaObject::play()
{
	AbstractMediaProducer::play();
}

PHONON_SETTER( setAboutToFinishTime, aboutToFinishTime, qint32 )

void MediaObjectPrivate::_k_setupKioJob()
{
	K_Q( MediaObject );
	Q_ASSERT( backendObject );

	if( kiojob )
		kiojob->kill();

	kiojob = KIO::open( url, 1 /*ReadOnly*/ );
	if( !kiojob )
	{
		kiojob = KIO::get( url, false, false );
		if( !kiojob )
			return;
		pBACKEND_CALL1( "setStreamSeekable", bool, false );
		QObject::connect( kiojob, SIGNAL(totalSize(KJob*, qulonglong)),
				q, SLOT(_k_bytestreamTotalSize(KJob*,qulonglong)) );
	}
	else
	{
		//pBACKEND_CALL1( "setStreamSeekable", bool, false );
		pBACKEND_CALL1( "setStreamSeekable", bool, true );
		QObject::connect( kiojob, SIGNAL(open(KIO::Job*)),
				q, SLOT(_k_bytestreamFileJobOpen(KIO::Job*)) );
		QObject::connect( kiojob, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
				q, SLOT(_k_bytestreamSeekDone(KIO::Job*, KIO::filesize_t)) );

		if( !readTimer )
		{
			readTimer = new QTimer( q );
			readTimer->setInterval( 0 );
			QObject::connect( readTimer, SIGNAL( timeout() ), q, SLOT( _k_readTimerTimeout() ) );
		}
	}

	kiojob->addMetaData( "UserAgent", QLatin1String( "KDE Phonon" ) );
	QObject::connect( kiojob, SIGNAL(data(KIO::Job*,const QByteArray&)),
			q, SLOT(_k_bytestreamData(KIO::Job*,const QByteArray&)) );
	QObject::connect( kiojob, SIGNAL(result(KJob*)),
			q, SLOT(_k_bytestreamResult(KJob*)) );
}

void MediaObjectPrivate::setupKioStreaming()
{
	K_Q( MediaObject );
	Q_ASSERT( backendObject == 0 );
	backendObject = Factory::self()->createByteStream( q );
	if( backendObject )
	{
		QObject::connect( backendObject, SIGNAL( destroyed( QObject* ) ), q, SLOT( _k_cleanupByteStream() ) );
		_k_setupKioJob();
		endOfDataSent = false;

		//setupIface for ByteStream
		QObject::connect( backendObject, SIGNAL(finished()), q, SIGNAL(finished()) );
		QObject::connect( backendObject, SIGNAL(finished()), q, SLOT(_k_setupKioJob()) );
		QObject::connect( backendObject, SIGNAL(aboutToFinish(qint32)), q, SIGNAL(aboutToFinish(qint32)) );
		QObject::connect( backendObject, SIGNAL(length(qint64)), q, SIGNAL(length(qint64)) );

		QObject::connect( backendObject, SIGNAL(needData()), q, SLOT(_k_bytestreamNeedData()) );
		QObject::connect( backendObject, SIGNAL(enoughData()), q, SLOT(_k_bytestreamEnoughData()) );

		QObject::connect( backendObject, SIGNAL(seekStream(qint64)), q, SLOT(_k_bytestreamSeekStream(qint64)) );

		//TODO handle redirection ...

		//setupIface for AbstractMediaProducer
		static_cast<AbstractMediaProducer*>( q )->setupIface();
	}
}

void MediaObjectPrivate::_k_bytestreamSeekStream( qint64 position )
{
	kDebug( 600 ) << k_funcinfo << position << " = " << qulonglong( position ) << endl;
	KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>( kiojob );
	seeking = true;
	readTimer->stop();

	// don't suspend when seeking as that will make the application hang,
	// waiting for the FileJob::position signal
	if( kiojob->isSuspended() )
		kiojob->resume();

	filejob->seek( position );
	// seek doesn't block, so don't send data to the backend until it signals us
	// that the seek is done
}

void MediaObjectPrivate::_k_bytestreamSeekDone( KIO::Job*, KIO::filesize_t offset )
{
	kDebug( 600 ) << k_funcinfo << offset << endl;
	seeking = false;
	endOfDataSent = false;
	readTimer->start();
}

void MediaObjectPrivate::_k_bytestreamNeedData()
{
	// while seeking the backend won't get any data
	if( seeking )
		return;

	if( kiojob && kiojob->isSuspended() )
		kiojob->resume();
	KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>( kiojob );
	if( filejob )
	{
		filejob->read( 4096 );
		readTimer->start();
	}
}

void MediaObjectPrivate::_k_readTimerTimeout()
{
	kDebug( 600 ) << k_funcinfo << endl;
	KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>( kiojob );
	if( filejob )
		filejob->read( 4096 );
}

void MediaObjectPrivate::_k_bytestreamEnoughData()
{
	kDebug( 600 ) << k_funcinfo << endl;
	// don't suspend when seeking as that will make the application hang,
	// waiting for the FileJob::position signal
	if( !seeking && kiojob && !kiojob->isSuspended() )
		kiojob->suspend();
	readTimer->stop();
}

void MediaObjectPrivate::_k_bytestreamData( KIO::Job*, const QByteArray& data )
{
	if( seeking )
	{
		kDebug( 600 ) << k_funcinfo << "seeking: do nothing" << endl;
		return;
	}
	if( data.isEmpty() )
	{
		if( readTimer )
			readTimer->stop();
		if( !endOfDataSent )
		{
			kDebug( 600 ) << k_funcinfo << "empty data: stopping the stream" << endl;
			endOfDataSent = true;
			ByteStreamInterface* bs = qobject_cast<ByteStreamInterface*>( backendObject );
			bs->endOfData();
		}
		return;
	}

	kDebug( 600 ) << k_funcinfo << "calling writeData on the Backend ByteStream " << data.size() << endl;
	ByteStreamInterface* bs = qobject_cast<ByteStreamInterface*>( backendObject );
	bs->writeData( data );
}

void MediaObjectPrivate::_k_bytestreamResult( KJob* job )
{
	K_Q( MediaObject );
	kiojob = 0;
	if( job->error() )
	{
		kDebug(600)<<"Error :kiojob in "<<k_funcinfo<<endl;
		QObject::disconnect( kiojob, SIGNAL(data(KIO::Job*,const QByteArray&)),
			q, SLOT(_k_bytestreamData(KIO::Job*,const QByteArray&)) );
		QObject::disconnect( kiojob, SIGNAL(result(KJob*)),
			q, SLOT(_k_bytestreamResult(KJob*)) );
		QObject::disconnect( kiojob, SIGNAL(totalSize(KJob*, qulonglong)),
			q, SLOT(_k_bytestreamTotalSize(KJob*,qulonglong)) );
		QObject::disconnect( kiojob, SIGNAL(open(KIO::Job*)),
				q, SLOT(_k_bytestreamFileJobOpen(KIO::Job*)) );
	}

	endOfDataSent = true;
	ByteStreamInterface* bs = qobject_cast<ByteStreamInterface*>( backendObject );
	bs->endOfData();
	if( readTimer )
		readTimer->stop();
}

void MediaObjectPrivate::_k_bytestreamFileJobOpen( KIO::Job* )
{
	endOfDataSent = false;
	KIO::FileJob *filejob = static_cast<KIO::FileJob*>( kiojob );
	kDebug( 600 ) << k_funcinfo << filejob->size() << endl;
	pBACKEND_CALL1( "setStreamSize", qint64, filejob->size() );

	// start streaming data to the Backend until it signals enoughData
	readTimer->start();
}

void MediaObjectPrivate::_k_bytestreamTotalSize( KJob*, qulonglong size )
{
	kDebug( 600 ) << k_funcinfo << size << endl;
	pBACKEND_CALL1( "setStreamSize", qint64, size );
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

void MediaObjectPrivate::_k_stateChanged( Phonon::State newstate, Phonon::State oldstate )
{
	if( newstate == Phonon::ErrorState && oldstate == Phonon::LoadingState )
	{
		// setup ByteStream -> see setUrl
		return;
	}
	K_Q( MediaObject );
	emit q->stateChanged( newstate, oldstate );
}

// setupIface is not called for ByteStream
void MediaObject::setupIface()
{
	K_D( MediaObject );
	Q_ASSERT( d->backendObject );
	//kDebug( 600 ) << k_funcinfo << endl;
	AbstractMediaProducer::setupIface();

	// disconnect what AbstractMediaProducer::setupIface connected to filter out
	// the LoadingState -> ErrorState change on setUrl
	disconnect( d->backendObject, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ), this, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ) );
	connect( d->backendObject, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ), SLOT( _k_stateChanged( Phonon::State, Phonon::State ) ) );

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
