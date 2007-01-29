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
#include <kprotocolmanager.h>

#define PHONON_CLASSNAME MediaObject
#define PHONON_INTERFACENAME MediaObjectInterface

namespace Phonon
{
PHONON_HEIR_IMPL( AbstractMediaProducer )

MediaObject::MediaObject( Phonon::MediaObjectPrivate& dd, QObject* parent )
	: AbstractMediaProducer( dd, parent )
{
}

KUrl MediaObject::url() const 
{
	K_D( const MediaObject );
	if( d->backendObject )
	{
		MediaObjectInterface *iface = qobject_cast<MediaObjectInterface*>( d->backendObject );
		ByteStreamInterface *iface2 = qobject_cast<ByteStreamInterface*>( d->backendObject );

		if( iface )
			return iface->url();
		else if( iface2 && d->kiojob )
			return d->kiojob->url();
	}
	return KUrl();
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
		MediaObjectInterface *iface = qobject_cast<MediaObjectInterface*>( d->backendObject );
		if( iface )
		{
			iface->setUrl( url );
            // if the state changes to ErrorState it will be handled in
            // _k_stateChanged and a ByteStream will be used.
			return;
		}

		// we're using a ByteStream
		// first try to do with a proper MediaObject
		d->deleteIface();
		d->createIface();
		// createIface will set up a ByteStream (in setupIface) if needed
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

	if( KProtocolManager::supportsOpening( url ) )
	{
		kiojob = KIO::open( url, 1 /*ReadOnly*/ );
		if( !kiojob )
			return;

		pBACKEND_CALL1( "setStreamSeekable", bool, true );
		QObject::connect( kiojob, SIGNAL(open(KIO::Job*)),
				q, SLOT(_k_bytestreamFileJobOpen(KIO::Job*)) );
		QObject::connect( kiojob, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
				q, SLOT(_k_bytestreamSeekDone(KIO::Job*, KIO::filesize_t)) );
	}
	else
	{
		kiojob = KIO::get( url, false, false );
		if( !kiojob )
			return;

		pBACKEND_CALL1( "setStreamSeekable", bool, false );
		QObject::connect( kiojob, SIGNAL(totalSize(KJob*, qulonglong)),
				q, SLOT(_k_bytestreamTotalSize(KJob*,qulonglong)) );
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
    backendObject = Factory::createByteStream(q);
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
		pBACKEND_CALL1( "setAboutToFinishTime", qint32, aboutToFinishTime );
	}
}

void MediaObjectPrivate::_k_bytestreamSeekStream( qint64 position )
{
	kDebug( 600 ) << k_funcinfo << position << " = " << qulonglong( position ) << endl;
	KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>( kiojob );
	seeking = true;
	reading = false;

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
	reading = true;
	KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>( kiojob );
	if( filejob )
		filejob->read( 32768 );
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
		filejob->read( 32768 );
}

void MediaObjectPrivate::_k_bytestreamEnoughData()
{
	kDebug( 600 ) << k_funcinfo << endl;
    // Don't suspend when using a FileJob. The FileJob is controlled by calls to
    // FileJob::read()
    if (kiojob && !qobject_cast<KIO::FileJob*>(kiojob) && !kiojob->isSuspended()) {
        kiojob->suspend();
    }
	reading = false;
}

void MediaObjectPrivate::_k_bytestreamData( KIO::Job*, const QByteArray& data )
{
	if( seeking )
	{
		kDebug( 600 ) << k_funcinfo << "seeking: do nothing" << endl;
		reading = false;
		return;
	}
	if( data.isEmpty() )
	{
		reading = false;
		if( !endOfDataSent )
		{
			kDebug( 600 ) << k_funcinfo << "empty data: stopping the stream" << endl;
			endOfDataSent = true;
			ByteStreamInterface* bs = qobject_cast<ByteStreamInterface*>( backendObject );
			bs->endOfData();
		}
		return;
	}

    //kDebug(600) << k_funcinfo << "calling writeData on the Backend ByteStream " << data.size() << endl;
	ByteStreamInterface* bs = qobject_cast<ByteStreamInterface*>( backendObject );
	bs->writeData( data );
	if( reading )
	{
		KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>( kiojob );
		if( filejob )
			filejob->read( 32768 );
	}
}

void MediaObjectPrivate::_k_bytestreamResult( KJob* job )
{
    K_Q(MediaObject);
    if (job->error()) {
        kDebug(600) << "KIO Job error: " << job->errorString() << endl;
        QObject::disconnect(kiojob, SIGNAL(data(KIO::Job*,const QByteArray&)),
                q, SLOT(_k_bytestreamData(KIO::Job*,const QByteArray&)));
        QObject::disconnect(kiojob, SIGNAL(result(KJob*)),
                q, SLOT(_k_bytestreamResult(KJob*)));
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>(kiojob);
        if (filejob) {
            QObject::disconnect(kiojob, SIGNAL(open(KIO::Job*)),
                    q, SLOT(_k_bytestreamFileJobOpen(KIO::Job*)));
            QObject::disconnect(kiojob, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
                    q, SLOT(_k_bytestreamSeekDone(KIO::Job*, KIO::filesize_t)));
        } else {
            QObject::disconnect(kiojob, SIGNAL(totalSize(KJob*, qulonglong)),
                    q, SLOT(_k_bytestreamTotalSize(KJob*,qulonglong)));
        }
    }
    kiojob = 0;

	endOfDataSent = true;
	ByteStreamInterface* bs = qobject_cast<ByteStreamInterface*>( backendObject );
	bs->endOfData();
	reading = false;
}

void MediaObjectPrivate::_k_bytestreamFileJobOpen( KIO::Job* )
{
	endOfDataSent = false;
	KIO::FileJob *filejob = static_cast<KIO::FileJob*>( kiojob );
	kDebug( 600 ) << k_funcinfo << filejob->size() << endl;
	pBACKEND_CALL1( "setStreamSize", qint64, filejob->size() );

	// start streaming data to the Backend until it signals enoughData
	reading = true;
	filejob->read( 32768 );
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
    K_Q(MediaObject);

    // backend MediaObject reached ErrorState, try a ByteStream
    if (newstate == Phonon::ErrorState && !kiojob) {
        kDebug(600) << "backend MediaObject reached ErrorState, trying ByteStream now" << endl;
        deleteIface();
        if (oldstate != Phonon::LoadingState) {
            emit q->stateChanged(Phonon::LoadingState, oldstate);
        }
        setupKioStreaming();
		return;
	}
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
    BACKEND_CALL1("setAboutToFinishTime", qint32, d->aboutToFinishTime);
    if(!d->url.isEmpty()) {
        INTERFACE_CALL1(setUrl, d->url);
        // if the state changes to ErrorState it will be handled in
        // _k_stateChanged and a ByteStream will be used.
    }
}

} //namespace Phonon

#include "mediaobject.moc"

// vim: sw=4 ts=4 tw=80
