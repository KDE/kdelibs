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
#include "ifaces/abstractmediaproducer.h"
#include "ifaces/audiopath.h"
#include "ifaces/videopath.h"
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

	if( iface() && d->iface()->addVideoPath( videoPath->iface() ) )
	{
		videoPath->addDestructionHandler( this );
		d->videoPaths.append( videoPath );
		return true;
	}
	return false;
}

bool AbstractMediaProducer::addAudioPath( AudioPath* audioPath )
{
	K_D( AbstractMediaProducer );
	if( d->audioPaths.contains( audioPath ) )
		return false;

	if( iface() && d->iface()->addAudioPath( audioPath->iface() ) )
	{
		audioPath->addDestructionHandler( this );
		d->audioPaths.append( audioPath );
		return true;
	}
	return false;
}

State AbstractMediaProducer::state() const
{
	K_D( const AbstractMediaProducer );
	return d->iface() ? d->iface()->state() : d->state;
}

bool AbstractMediaProducer::hasVideo() const
{
	K_D( const AbstractMediaProducer );
	return d->iface() ? d->iface()->hasVideo() : false;
}

bool AbstractMediaProducer::seekable() const
{
	K_D( const AbstractMediaProducer );
	return d->iface() ? d->iface()->seekable() : false;
}

long AbstractMediaProducer::currentTime() const
{
	K_D( const AbstractMediaProducer );
	return d->iface() ? d->iface()->currentTime() : d->currentTime;
}

long AbstractMediaProducer::tickInterval() const
{
	K_D( const AbstractMediaProducer );
	return d->iface() ? d->iface()->tickInterval() : d->tickInterval;
}

QString AbstractMediaProducer::selectedAudioStream( AudioPath* audioPath ) const
{
	K_D( const AbstractMediaProducer );
	return d->iface() ? d->iface()->selectedAudioStream( audioPath->iface() ) : d->selectedAudioStream[ audioPath->iface() ];
}

QString AbstractMediaProducer::selectedVideoStream( VideoPath* videoPath ) const
{
	K_D( const AbstractMediaProducer );
	return d->iface() ? d->iface()->selectedVideoStream( videoPath->iface() ) : d->selectedVideoStream[ videoPath->iface() ];
}

QString AbstractMediaProducer::selectedSubtitleStream( VideoPath* videoPath ) const
{
	K_D( const AbstractMediaProducer );
	return d->iface() ? d->iface()->selectedSubtitleStream( videoPath->iface() ) : d->selectedSubtitleStream[ videoPath->iface() ];
}

QStringList AbstractMediaProducer::availableAudioStreams() const
{
	K_D( const AbstractMediaProducer );
	if( d->iface() )
		return d->iface()->availableAudioStreams();
	return QStringList();
}

QStringList AbstractMediaProducer::availableVideoStreams() const
{
	K_D( const AbstractMediaProducer );
	if( d->iface() )
		return d->iface()->availableVideoStreams();
	return QStringList();
}

QStringList AbstractMediaProducer::availableSubtitleStreams() const
{
	K_D( const AbstractMediaProducer );
	if( d->iface() )
		return d->iface()->availableSubtitleStreams();
	return QStringList();
}

void AbstractMediaProducer::selectAudioStream( const QString& streamName, AudioPath* audioPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
		d->iface()->selectAudioStream( streamName, audioPath->iface() );
	else
		d->selectedAudioStream[ audioPath->iface() ] = streamName;
}

void AbstractMediaProducer::selectVideoStream( const QString& streamName, VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
		d->iface()->selectVideoStream( streamName, videoPath->iface() );
	else
		d->selectedVideoStream[ videoPath->iface() ] = streamName;
}

void AbstractMediaProducer::selectSubtitleStream( const QString& streamName, VideoPath* videoPath )
{
	K_D( AbstractMediaProducer );
	if( iface() )
		d->iface()->selectSubtitleStream( streamName, videoPath->iface() );
	else
		d->selectedSubtitleStream[ videoPath->iface() ] = streamName;
}

void AbstractMediaProducer::setTickInterval( long newTickInterval )
{
	K_D( AbstractMediaProducer );
	if( d->iface() )
		d->tickInterval = d->iface()->setTickInterval( newTickInterval );
	else
		d->tickInterval = newTickInterval;
}

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
		d->iface()->play();
}

void AbstractMediaProducer::pause()
{
	K_D( AbstractMediaProducer );
	if( iface() )
		d->iface()->pause();
}

void AbstractMediaProducer::stop()
{
	K_D( AbstractMediaProducer );
	if( iface() )
		d->iface()->stop();
}

void AbstractMediaProducer::seek( long time )
{
	K_D( AbstractMediaProducer );
	State s = state();
	if( iface() && ( s == Phonon::PlayingState || s == Phonon::BufferingState || s == Phonon::PausedState ) )
	{
		d->iface()->seek( time );
	}
}

bool AbstractMediaProducerPrivate::aboutToDeleteIface()
{
	//kDebug( 600 ) << k_funcinfo << endl;
	if( iface() )
	{
		state = iface()->state();
		currentTime = iface()->currentTime();
		tickInterval = iface()->tickInterval();
	}
	return true;
}

void AbstractMediaProducer::setupIface()
{
	K_D( AbstractMediaProducer );
	Q_ASSERT( d->iface() );
	//kDebug( 600 ) << k_funcinfo << endl;

	connect( d->iface()->qobject(), SIGNAL( stateChanged( Phonon::State, Phonon::State ) ), SIGNAL( stateChanged( Phonon::State, Phonon::State ) ) );
	connect( d->iface()->qobject(), SIGNAL( tick( long ) ), SIGNAL( tick( long ) ) );

	// set up attributes
	d->iface()->setTickInterval( d->tickInterval );

	foreach( AudioPath* a, d->audioPaths )
		d->iface()->addAudioPath( a->iface() );
	foreach( VideoPath* v, d->videoPaths )
		d->iface()->addVideoPath( v->iface() );

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
	d->state = d->iface()->state();
}

void AbstractMediaProducer::resumePlay()
{
	K_D( AbstractMediaProducer );
	d->iface()->play();
	if( d->currentTime > 0 )
		d->iface()->seek( d->currentTime );
}

void AbstractMediaProducer::resumePause()
{
	K_D( AbstractMediaProducer );
	d->iface()->play();
	if( d->currentTime > 0 )
		d->iface()->seek( d->currentTime );
	d->iface()->pause();
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
		if( d->iface() )
			d->iface()->removeAudioPath( audioPath->iface() );
		d->audioPaths.removeAll( audioPath );
	}
	else if( d->videoPaths.contains( videoPath ) )
	{
		if( d->iface() )
			d->iface()->removeVideoPath( videoPath->iface() );
		d->videoPaths.removeAll( videoPath );
	}
}

} //namespace Phonon

#include "abstractmediaproducer.moc"

// vim: sw=4 ts=4 tw=80 noet
