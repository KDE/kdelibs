/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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

#include "abstractmediaproducer.h"
#include "abstractmediaproducer_p.h"
#include "factory.h"

#include "videopath.h"
#include "audiopath.h"

#include "mediaproducerinterface.h"


#include <QTimer>

#include <kdebug.h>
#include <QStringList>

#define PHONON_CLASSNAME AbstractMediaProducer
#define PHONON_INTERFACENAME MediaProducerInterface

namespace Phonon
{
PHONON_ABSTRACTBASE_IMPL

AbstractMediaProducer::~AbstractMediaProducer()
{
	K_D( AbstractMediaProducer );
	Phonon::State s = state();
	if( s != ErrorState || s != StoppedState || s != LoadingState )
		stop();
    foreach(VideoPath* vp, d->videoPaths)
        d->removeDestructionHandler(vp, d);
    foreach(AudioPath* ap, d->audioPaths)
        d->removeDestructionHandler(ap, d);
}

bool AbstractMediaProducer::addVideoPath( VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( d->videoPaths.contains( videoPath ) )
		return false;

	if( iface() )
	{
		if( qobject_cast<MediaProducerInterface*>( d->backendObject )->addVideoPath( videoPath->iface() ) )
		{
            d->addDestructionHandler(videoPath, d);
			d->videoPaths.append( videoPath );
			return true;
		}
	}
	return false;
}

bool AbstractMediaProducer::addAudioPath( AudioPath* audioPath )
{
	K_D( AbstractMediaProducer );
	if( d->audioPaths.contains( audioPath ) )
		return false;

	if( iface() )
	{
		if( qobject_cast<MediaProducerInterface*>( d->backendObject )->addAudioPath( audioPath->iface() ) )
		{
            d->addDestructionHandler(audioPath, d);
			d->audioPaths.append( audioPath );
			return true;
		}
	}
	return false;
}

PHONON_INTERFACE_SETTER( setTickInterval, tickInterval, qint32 )
PHONON_INTERFACE_GETTER( qint32, tickInterval, d->tickInterval )
PHONON_INTERFACE_GETTER( Phonon::State, state, d->state )
PHONON_INTERFACE_GETTER( bool, hasVideo, false )
PHONON_INTERFACE_GETTER( bool, isSeekable, false )
PHONON_INTERFACE_GETTER( qint64, currentTime, d->currentTime )
PHONON_INTERFACE_GETTER1( QString, selectedAudioStream,    d->selectedAudioStream[    audioPath ], AudioPath*, audioPath )
PHONON_INTERFACE_GETTER1( QString, selectedVideoStream,    d->selectedVideoStream[    videoPath ], VideoPath*, videoPath )
PHONON_INTERFACE_GETTER1( QString, selectedSubtitleStream, d->selectedSubtitleStream[ videoPath ], VideoPath*, videoPath )
PHONON_INTERFACE_GETTER( QStringList, availableAudioStreams, QStringList() )
PHONON_INTERFACE_GETTER( QStringList, availableVideoStreams, QStringList() )
PHONON_INTERFACE_GETTER( QStringList, availableSubtitleStreams, QStringList() )

void AbstractMediaProducer::selectAudioStream( const QString& streamName, AudioPath* audioPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
		INTERFACE_CALL2( selectAudioStream, streamName, audioPath->iface() );
	else
		d->selectedAudioStream[ audioPath ] = streamName;
}

void AbstractMediaProducer::selectVideoStream( const QString& streamName, VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
		INTERFACE_CALL2( selectVideoStream, streamName, videoPath->iface() );
	else
		d->selectedVideoStream[ videoPath ] = streamName;
}

void AbstractMediaProducer::selectSubtitleStream( const QString& streamName, VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
		INTERFACE_CALL2( selectSubtitleStream, streamName, videoPath->iface() );
	else
		d->selectedSubtitleStream[ videoPath ] = streamName;
}

QList<VideoPath*> AbstractMediaProducer::videoPaths() const
{
	K_D( const AbstractMediaProducer );
	return d->videoPaths;
}

QList<AudioPath*> AbstractMediaProducer::audioPaths() const
{
	K_D( const AbstractMediaProducer );
	return d->audioPaths;
}

void AbstractMediaProducer::play()
{
	K_D( AbstractMediaProducer );
	if( iface() )
		INTERFACE_CALL( play );
}

void AbstractMediaProducer::pause()
{
	K_D( AbstractMediaProducer );
	if( iface() )
		INTERFACE_CALL( pause );
}

void AbstractMediaProducer::stop()
{
	K_D( AbstractMediaProducer );
	if( iface() )
	{
		INTERFACE_CALL( stop );
		if( tickInterval() > 0 )
			emit tick( 0 );
	}
}

void AbstractMediaProducer::seek( qint64 time )
{
	K_D( AbstractMediaProducer );
	State s = state();
	if( iface() && ( s == Phonon::PlayingState || s == Phonon::BufferingState || s == Phonon::PausedState ) )
		INTERFACE_CALL1( seek, time );
}

bool AbstractMediaProducerPrivate::aboutToDeleteIface()
{
	//kDebug( 600 ) << k_funcinfo << endl;
	if( backendObject )
	{
		state = qobject_cast<MediaProducerInterface*>( backendObject )->state();
		currentTime = qobject_cast<MediaProducerInterface*>( backendObject )->currentTime();
		tickInterval = qobject_cast<MediaProducerInterface*>( backendObject )->tickInterval();
	}
	return true;
}

void AbstractMediaProducer::setupIface()
{
	K_D( AbstractMediaProducer );
	Q_ASSERT( d->backendObject );
	//kDebug( 600 ) << k_funcinfo << endl;

	connect( d->backendObject, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ), SIGNAL( stateChanged( Phonon::State, Phonon::State ) ) );
	connect( d->backendObject, SIGNAL( tick( qint64 ) ), SIGNAL( tick( qint64 ) ) );
	connect( d->backendObject, SIGNAL( metaDataChanged( const QMultiMap<QString, QString>& ) ), SLOT( _k_metaDataChanged( const QMultiMap<QString, QString>& ) ) );

	// set up attributes
	INTERFACE_CALL1( setTickInterval, d->tickInterval );

	foreach( AudioPath* a, d->audioPaths )
	{
		if( !qobject_cast<MediaProducerInterface*>( d->backendObject )->addAudioPath( a->iface() ) )
			d->audioPaths.removeAll( a );
	}
	foreach( VideoPath* v, d->videoPaths )
	{
		if( !qobject_cast<MediaProducerInterface*>( d->backendObject )->addVideoPath( v->iface() ) )
			d->videoPaths.removeAll( v );
	}

	switch( d->state )
	{
		case LoadingState:
		case StoppedState:
		case ErrorState:
			break;
		case PlayingState:
		case BufferingState:
			QTimer::singleShot( 0, this, SLOT( _k_resumePlay() ) );
			break;
		case PausedState:
			QTimer::singleShot( 0, this, SLOT( _k_resumePause() ) );
			break;
	}
	d->state = qobject_cast<MediaProducerInterface*>( d->backendObject )->state();
}

void AbstractMediaProducerPrivate::_k_resumePlay()
{
	qobject_cast<MediaProducerInterface*>( backendObject )->play();
	if( currentTime > 0 )
		qobject_cast<MediaProducerInterface*>( backendObject )->seek( currentTime );
}

void AbstractMediaProducerPrivate::_k_resumePause()
{
	qobject_cast<MediaProducerInterface*>( backendObject )->play();
	if( currentTime > 0 )
		qobject_cast<MediaProducerInterface*>( backendObject )->seek( currentTime );
	qobject_cast<MediaProducerInterface*>( backendObject )->pause();
}

void AbstractMediaProducerPrivate::_k_metaDataChanged( const QMultiMap<QString, QString>& newMetaData )
{
	metaData = newMetaData;
	emit q_func()->metaDataChanged();
}

QStringList AbstractMediaProducer::metaDataKeys() const
{
	K_D( const AbstractMediaProducer );
	return d->metaData.keys();
}

QString AbstractMediaProducer::metaDataItem( const QString& key ) const
{
	K_D( const AbstractMediaProducer );
	return d->metaData.value( key );
}

QStringList AbstractMediaProducer::metaDataItems( const QString& key ) const
{
	K_D( const AbstractMediaProducer );
	return d->metaData.values( key );
}

void AbstractMediaProducerPrivate::phononObjectDestroyed( Base* o )
{
	// this method is called from Phonon::Base::~Base(), meaning the AudioPath
	// dtor has already been called, also virtual functions don't work anymore
	// (therefore qobject_cast can only downcast from Base)
	Q_ASSERT( o );
	AudioPath* audioPath = static_cast<AudioPath*>( o );
	VideoPath* videoPath = static_cast<VideoPath*>( o );
	if( audioPaths.contains( audioPath ) )
	{
		if( backendObject )
			qobject_cast<MediaProducerInterface*>( backendObject )->removeAudioPath( audioPath->iface() );
		audioPaths.removeAll( audioPath );
	}
	else if( videoPaths.contains( videoPath ) )
	{
		if( backendObject )
			qobject_cast<MediaProducerInterface*>( backendObject )->removeVideoPath( videoPath->iface() );
		videoPaths.removeAll( videoPath );
	}
}

} //namespace Phonon

#include "abstractmediaproducer.moc"
// vim: sw=4 ts=4 tw=80
