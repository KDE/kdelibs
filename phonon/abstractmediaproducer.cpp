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
#include "addoninterface.h"
#include "trackinterface.h"
#include "chapterinterface.h"

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
    K_D(AbstractMediaProducer);
    if (d->backendObject) {
        switch (state()) {
            case PlayingState:
            case BufferingState:
            case PausedState:
                stop();
                break;
            case ErrorState:
            case StoppedState:
            case LoadingState:
                break;
        }
    }
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

Phonon::State AbstractMediaProducer::state() const
{
    K_D(const AbstractMediaProducer);
    if (!d->backendObject || d->errorOverride) {
        return d->state;
    }
    return INTERFACE_CALL(state());
}

PHONON_INTERFACE_SETTER( setTickInterval, tickInterval, qint32 )
PHONON_INTERFACE_GETTER( qint32, tickInterval, d->tickInterval )
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
        INTERFACE_CALL(selectAudioStream(streamName, audioPath->iface()));
	else
		d->selectedAudioStream[ audioPath ] = streamName;
}

void AbstractMediaProducer::selectVideoStream( const QString& streamName, VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
        INTERFACE_CALL(selectVideoStream(streamName, videoPath->iface()));
	else
		d->selectedVideoStream[ videoPath ] = streamName;
}

void AbstractMediaProducer::selectSubtitleStream( const QString& streamName, VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
        INTERFACE_CALL(selectSubtitleStream(streamName, videoPath->iface()));
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
        INTERFACE_CALL(play());
}

void AbstractMediaProducer::pause()
{
	K_D( AbstractMediaProducer );
	if( iface() )
        INTERFACE_CALL(pause());
}

void AbstractMediaProducer::stop()
{
	K_D( AbstractMediaProducer );
	if( iface() )
	{
        INTERFACE_CALL(stop());
	}
}

void AbstractMediaProducer::seek( qint64 time )
{
	K_D( AbstractMediaProducer );
	State s = state();
	if( iface() && ( s == Phonon::PlayingState || s == Phonon::BufferingState || s == Phonon::PausedState ) )
        INTERFACE_CALL(seek(time));
}

QString AbstractMediaProducer::errorString() const
{
    if (state() == Phonon::ErrorState) {
        K_D(const AbstractMediaProducer);
        if (d->errorOverride) {
            return d->errorString;
        }
        return INTERFACE_CALL(errorString());
    }
    return QString();
}

ErrorType AbstractMediaProducer::errorType() const
{
    if (state() == Phonon::ErrorState) {
        K_D(const AbstractMediaProducer);
        if (d->errorOverride) {
            return d->errorType;
        }
        return INTERFACE_CALL(errorType());
    }
    return Phonon::NoError;
}

template<>
bool AbstractMediaProducer::hasInterface<TrackInterface>() const
{
    K_D(const AbstractMediaProducer);
    if (d->backendObject) {
        return qobject_cast<AddonInterface *>(d->backendObject)
            ->hasInterface(AddonInterface::TrackInterface);
    }
    return false;
}

template<>
bool AbstractMediaProducer::hasInterface<ChapterInterface>() const
{
    K_D(const AbstractMediaProducer);
    if (d->backendObject) {
        return qobject_cast<AddonInterface *>(d->backendObject)
            ->hasInterface(AddonInterface::ChapterInterface);
    }
    return false;
}

bool AbstractMediaProducerPrivate::aboutToDeleteIface()
{
	//kDebug( 600 ) << k_funcinfo << endl;
	if( backendObject )
	{
        state = pINTERFACE_CALL(state());
        currentTime = pINTERFACE_CALL(currentTime());
        tickInterval = pINTERFACE_CALL(tickInterval());
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
    connect(d->backendObject, SIGNAL(seekableChanged(bool)), SIGNAL(seekableChanged(bool)));
    connect(d->backendObject, SIGNAL(hasVideoChanged(bool)), SIGNAL(hasVideoChanged(bool)));
    connect(d->backendObject, SIGNAL(bufferStatus(int)), SIGNAL(bufferStatus(int)));

	// set up attributes
    INTERFACE_CALL(setTickInterval(d->tickInterval));

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
    const State backendState = INTERFACE_CALL(state());
    if (d->state != backendState && d->state != ErrorState) {
        // careful: if d->state is ErrorState we might be switching from a
        // MediaObject to a ByteStream for KIO fallback. In that case the state
        // change to ErrorState was already supressed.
        kDebug(600) << "emitting a state change because the backend object has been replaced" << endl;
        emit stateChanged(backendState, d->state);
        d->state = backendState;
    }
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
#undef PHONON_CLASSNAME
#undef PHONON_INTERFACENAME
// vim: sw=4 ts=4 tw=80
