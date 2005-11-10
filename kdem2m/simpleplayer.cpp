/*  This file is part of the KDE project
    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "simpleplayer.h"
#include "mediaobject.h"
#include "audiopath.h"
#include "audiooutput.h"
#include <kurl.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>

namespace KDEM2M
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
		KURL url;
		State state;
		float volume;
		long time;
		QString title;
		QString type;
};

SimplePlayer::SimplePlayer( QObject * parent )
	: QObject( parent )
	, d( new Private )
{
	d->output = new AudioOutput( this );
	d->path = new AudioPath( this );
	d->path->addOutput( d->output );
}

SimplePlayer::~SimplePlayer()
{
	delete d->player;
	delete d->path;
	delete d->output;
}

void SimplePlayer::play( const KURL & url )
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
	d->player = new MediaObject( url, this );
	d->player->addAudioPath( d->path );
		
	if( ErrorState == d->player->state() )
	{
		delete d->player;
		d->player = 0;
		return;
	}

	d->url = url;

	connect( d->player, SIGNAL( stateChanged( KDEM2M::State, KDEM2M::State ) ),
			SLOT( stateChanged( KDEM2M::State, KDEM2M::State ) ) );
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
