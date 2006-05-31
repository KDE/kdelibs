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

#include "simpleplayer.h"
#include "mediaobject.h"
#include "audiopath.h"
#include "audiooutput.h"
#include <kurl.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>

namespace Phonon
{

class SimplePlayer::Private
{
	public:
		Private()
			: player( 0 )
		{
		}

		MediaObject * player;
		AudioPath * path;
		AudioOutput * output;
		KUrl url;
		State state;
		float volume;
		long time;
		QString title;
		QString type;
};

SimplePlayer::SimplePlayer( Phonon::Category category, QObject* parent )
	: QObject( parent )
	, d( new Private )
{
	d->output = new AudioOutput( this );
	d->output->setCategory( category );
	d->path = new AudioPath( this );
	d->path->addOutput( d->output );
}

SimplePlayer::~SimplePlayer()
{
	delete d->player;
	delete d->path;
	delete d->output;
}

void SimplePlayer::play( const KUrl & url )
{
	if( d->player )
	{
		if( url == d->url )
		{
			if( !isPlaying() )
				d->player->play();
			return;
		}
		else
			delete d->player;
	}
	d->player = new MediaObject( this );
	d->player->setUrl( url );
	d->player->addAudioPath( d->path );
		
	if( ErrorState == d->player->state() )
	{
		delete d->player;
		d->player = 0;
		return;
	}

	d->url = url;

	connect( d->player, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
			SLOT( stateChanged( Phonon::State, Phonon::State ) ) );
	connect( d->player, SIGNAL( finished() ), SIGNAL( finished() ) );

	if( StoppedState == d->player->state() )
		d->player->play();
}

void SimplePlayer::pause()
{
	d->player->pause();
}

void SimplePlayer::stop()
{
	d->player->stop();
}

long SimplePlayer::totalTime() const
{
	return d->player->totalTime();
}

long SimplePlayer::currentTime() const
{
	return d->player->currentTime();
}

void SimplePlayer::seek( long ms )
{
	d->player->seek( ms );
}

float SimplePlayer::volume() const
{
	return d->output->volume();
}

void SimplePlayer::setVolume( float v )
{
	d->output->setVolume( v );
}

bool SimplePlayer::isPlaying() const
{
	return ( d->player->state() == PlayingState );
}

bool SimplePlayer::isPaused() const
{
	return ( d->player->state() == PausedState );
}

void SimplePlayer::stateChanged( State ns, State os )
{
	if( os == LoadingState && ns == StoppedState )
		d->player->play();
}

} // namespaces

#include "simpleplayer.moc"

// vim: sw=4 ts=4 noet
