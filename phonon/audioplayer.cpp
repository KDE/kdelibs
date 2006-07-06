/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#include "audioplayer.h"
#include "mediaobject.h"
#include "audiopath.h"
#include "audiooutput.h"
#include <kurl.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>

namespace Phonon
{

class AudioPlayer::Private
{
	public:
		Private()
			: player( 0 )
		{
		}

		MediaObject* player;
		AudioPath* path;
		AudioOutput* output;
		KUrl url;

		void _k_stateChanged( Phonon::State, Phonon::State );
};

AudioPlayer::AudioPlayer( Phonon::Category category, QObject* parent )
	: QObject( parent )
	, d( new Private )
{
	d->output = new AudioOutput( category, this );
	d->path = new AudioPath( this );
	d->path->addOutput( d->output );
	d->player = new MediaObject( this );
	d->player->addAudioPath( d->path );

	connect( d->player, SIGNAL( _k_stateChanged( Phonon::State, Phonon::State ) ),
			SLOT( _k_stateChanged( Phonon::State, Phonon::State ) ) );
	connect( d->player, SIGNAL( finished() ), SIGNAL( finished() ) );
}

AudioPlayer::~AudioPlayer()
{
	delete d->player;
	delete d->path;
	delete d->output;
}

void AudioPlayer::load( const KUrl& url )
{
	// new URL
	d->player->setUrl( url );
	d->url = url;
}

void AudioPlayer::play( const KUrl& url )
{
	if( url == d->url )
	{
		if( !isPlaying() )
			d->player->play();
		return;
	}
	// new URL
	d->player->setUrl( url );
		
	if( ErrorState == d->player->state() )
		return;

	d->url = url;

	if( StoppedState == d->player->state() )
		d->player->play();
}

void AudioPlayer::play()
{
	play( d->url );
}

void AudioPlayer::pause()
{
	d->player->pause();
}

void AudioPlayer::stop()
{
	d->player->stop();
}

qint64 AudioPlayer::totalTime() const
{
	return d->player->totalTime();
}

qint64 AudioPlayer::currentTime() const
{
	return d->player->currentTime();
}

void AudioPlayer::seek( qint64 ms )
{
	d->player->seek( ms );
}

float AudioPlayer::volume() const
{
	return d->output->volume();
}

void AudioPlayer::setVolume( float v )
{
	d->output->setVolume( v );
}

bool AudioPlayer::isPlaying() const
{
	return ( d->player->state() == PlayingState );
}

bool AudioPlayer::isPaused() const
{
	return ( d->player->state() == PausedState );
}

void AudioPlayer::Private::_k_stateChanged( State ns, State os )
{
	if( os == LoadingState && ns == StoppedState )
		player->play();
}

} // namespaces

#include "audioplayer.moc"

// vim: sw=4 ts=4 noet
