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

#include <QTimer>

#include <kdebug.h>
#include <QStringList>

namespace Phonon
{
PHONON_ABSTRACTBASE_IMPL( AbstractMediaProducer )

AbstractMediaProducer::~AbstractMediaProducer()
{
	K_D( AbstractMediaProducer );
	foreach( VideoPath* vp, d->videoPaths )
		vp->removeDestructionHandler( this );
	foreach( AudioPath* ap, d->audioPaths )
		ap->removeDestructionHandler( this );
}

bool AbstractMediaProducer::addVideoPath( VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( d->videoPaths.contains( videoPath ) )
		return false;

	if( iface() )
	{
		bool success;
		BACKEND_GET1( bool, success, "addVideoPath", QObject*, videoPath->iface() );
		if( success )
		{
			videoPath->addDestructionHandler( this );
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
		bool success;
		BACKEND_GET1( bool, success, "addAudioPath", QObject*, audioPath->iface() );
		if( success )
		{
			audioPath->addDestructionHandler( this );
			d->audioPaths.append( audioPath );
			return true;
		}
	}
	return false;
}

PHONON_GETTER( AbstractMediaProducer, Phonon::State, state, d->state )
PHONON_GETTER( AbstractMediaProducer, bool, hasVideo, false )
PHONON_GETTER( AbstractMediaProducer, bool, seekable, false )
PHONON_GETTER( AbstractMediaProducer, qint64, currentTime, d->currentTime )
PHONON_GETTER( AbstractMediaProducer, qint32, tickInterval, d->tickInterval )
PHONON_GETTER1( AbstractMediaProducer, QString, selectedAudioStream,    d->selectedAudioStream[    audioPath ], AudioPath*, audioPath )
PHONON_GETTER1( AbstractMediaProducer, QString, selectedVideoStream,    d->selectedVideoStream[    videoPath ], VideoPath*, videoPath )
PHONON_GETTER1( AbstractMediaProducer, QString, selectedSubtitleStream, d->selectedSubtitleStream[ videoPath ], VideoPath*, videoPath )
PHONON_GETTER( AbstractMediaProducer, QStringList, availableAudioStreams, QStringList() )
PHONON_GETTER( AbstractMediaProducer, QStringList, availableVideoStreams, QStringList() )
PHONON_GETTER( AbstractMediaProducer, QStringList, availableSubtitleStreams, QStringList() )

void AbstractMediaProducer::selectAudioStream( const QString& streamName, AudioPath* audioPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
		BACKEND_CALL2( "selectAudioStream", QString, streamName, const QObject*, audioPath->iface() );
	else
		d->selectedAudioStream[ audioPath ] = streamName;
}

void AbstractMediaProducer::selectVideoStream( const QString& streamName, VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
		BACKEND_CALL2( "selectVideoStream", QString, streamName, const QObject*, videoPath->iface() );
	else
		d->selectedVideoStream[ videoPath ] = streamName;
}

void AbstractMediaProducer::selectSubtitleStream( const QString& streamName, VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
		BACKEND_CALL2( "selectSubtitleStream", QString, streamName, const QObject*, videoPath->iface() );
	else
		d->selectedSubtitleStream[ videoPath ] = streamName;
}

PHONON_SETTER( AbstractMediaProducer, setTickInterval, tickInterval, qint32 )

const QList<VideoPath*>& AbstractMediaProducer::videoPaths() const
{
	K_D( const AbstractMediaProducer );
	return d->videoPaths;
}

const QList<AudioPath*>& AbstractMediaProducer::audioPaths() const
{
	K_D( const AbstractMediaProducer );
	return d->audioPaths;
}

void AbstractMediaProducer::play()
{
	K_D( AbstractMediaProducer );
	if( iface() )
		BACKEND_CALL( "play" );
}

void AbstractMediaProducer::pause()
{
	K_D( AbstractMediaProducer );
	if( iface() )
		BACKEND_CALL( "pause" );
}

void AbstractMediaProducer::stop()
{
	K_D( AbstractMediaProducer );
	if( iface() )
		BACKEND_CALL( "stop" );
}

void AbstractMediaProducer::seek( qint64 time )
{
	K_D( AbstractMediaProducer );
	State s = state();
	if( iface() && ( s == Phonon::PlayingState || s == Phonon::BufferingState || s == Phonon::PausedState ) )
	{
		BACKEND_CALL1( "seek", qint64, time );
	}
}

bool AbstractMediaProducerPrivate::aboutToDeleteIface()
{
	//kDebug( 600 ) << k_funcinfo << endl;
	if( backendObject )
	{
		pBACKEND_GET( Phonon::State, state, "state" );
		pBACKEND_GET( qint64, currentTime, "currentTime" );
		pBACKEND_GET( qint32, tickInterval, "tickInterval" );
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

	// set up attributes
	BACKEND_CALL1( "setTickInterval", qint32, d->tickInterval );

	bool success;
	foreach( AudioPath* a, d->audioPaths )
	{
		BACKEND_GET1( bool, success, "addAudioPath", QObject*, a->iface() );
		if( !success )
			d->audioPaths.removeAll( a );
	}
	foreach( VideoPath* v, d->videoPaths )
	{
		BACKEND_GET1( bool, success, "addVideoPath", QObject*, v->iface() );
		if( !success )
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
			QTimer::singleShot( 0, this, SLOT( resumePlay() ) );
			break;
		case PausedState:
			QTimer::singleShot( 0, this, SLOT( resumePause() ) );
			break;
	}
	BACKEND_GET( Phonon::State, d->state, "state" );
}

void AbstractMediaProducer::resumePlay()
{
	K_D( AbstractMediaProducer );
	BACKEND_CALL( "play" );
	if( d->currentTime > 0 )
		BACKEND_CALL1( "seek", qint64, d->currentTime );
}

void AbstractMediaProducer::resumePause()
{
	K_D( AbstractMediaProducer );
	BACKEND_CALL( "play" );
	if( d->currentTime > 0 )
		BACKEND_CALL1( "seek", qint64, d->currentTime );
	BACKEND_CALL( "pause" );
}

void AbstractMediaProducer::phononObjectDestroyed( Base* o )
{
	// this method is called from Phonon::Base::~Base(), meaning the AudioPath
	// dtor has already been called, also virtual functions don't work anymore
	// (therefore qobject_cast can only downcast from Base)
	K_D( AbstractMediaProducer );
	Q_ASSERT( o );
	AudioPath* audioPath = static_cast<AudioPath*>( o );
	VideoPath* videoPath = static_cast<VideoPath*>( o );
	if( d->audioPaths.contains( audioPath ) )
	{
		if( d->backendObject )
			BACKEND_CALL1( "removeAudioPath", QObject*, audioPath->iface() );
		d->audioPaths.removeAll( audioPath );
	}
	else if( d->videoPaths.contains( videoPath ) )
	{
		if( d->backendObject )
			BACKEND_CALL1( "removeVideoPath", QObject*, videoPath->iface() );
		d->videoPaths.removeAll( videoPath );
	}
}

} //namespace Phonon

#include "abstractmediaproducer.moc"

// vim: sw=4 ts=4 tw=80 noet
