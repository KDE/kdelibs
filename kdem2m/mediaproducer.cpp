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
#include "mediaproducer.h"
#include "ifaces/mediaproducer.h"
#include "factory.h"

#include "videopath.h"
#include "audiopath.h"
#include "state.h"

#include <QTimer>

#include <kdebug.h>

namespace KDEM2M
{
class MediaProducer::Private
{
	public:
		Private()
			: state( KDEM2M::LoadingState )
			, currentTime( 0 )
			, tickInterval( 0 )
		{ }

		State state;
		long currentTime;
		long tickInterval;
		QList<VideoPath*> videoPaths;
		QList<AudioPath*> audioPaths;
};

MediaProducer::MediaProducer( QObject* parent )
	: QObject( parent )
	, m_iface( 0 )
	, d( new Private() )
{
	kdDebug( 600 ) << k_funcinfo << endl;
	slotCreateIface();
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

MediaProducer::MediaProducer( Ifaces::MediaProducer* iface, QObject* parent )
	: QObject( parent )
	, m_iface( iface )
	, d( new Private() )
{
	kdDebug( 600 ) << k_funcinfo << endl;
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

MediaProducer::~MediaProducer()
{
	kdDebug( 600 ) << k_funcinfo << endl;
	slotDeleteIface();
	delete d;
	d = 0;
}

bool MediaProducer::addVideoPath( VideoPath* videoPath )
{
	d->videoPaths.append( videoPath );
	if( iface() )
		return m_iface->addVideoPath( videoPath->iface() );
	return false;
}

bool MediaProducer::addAudioPath( AudioPath* audioPath )
{
	d->audioPaths.append( audioPath );
	if( iface() )
		return m_iface->addAudioPath( audioPath->iface() );
	return false;
}

State MediaProducer::state() const
{
	return m_iface ? m_iface->state() : d->state;
}

bool MediaProducer::hasVideo() const
{
	return m_iface ? m_iface->hasVideo() : false;
}

bool MediaProducer::seekable() const
{
	return m_iface ? m_iface->seekable() : false;
}

long MediaProducer::currentTime() const
{
	return m_iface ? m_iface->currentTime() : d->currentTime;
}

long MediaProducer::tickInterval() const
{
	return m_iface ? m_iface->tickInterval() : d->tickInterval;
}

void MediaProducer::setTickInterval( long newTickInterval )
{
	if( m_iface )
		d->tickInterval = m_iface->setTickInterval( newTickInterval );
	else
		d->tickInterval = newTickInterval;
}

const QList<VideoPath*>& MediaProducer::videoPaths() const
{
	return d->videoPaths;
}

const QList<AudioPath*>& MediaProducer::audioPaths() const
{
	return d->audioPaths;
}

void MediaProducer::play()
{
	if( iface() )
		m_iface->play();
}

void MediaProducer::pause()
{
	if( iface() )
		m_iface->pause();
}

void MediaProducer::stop()
{
	if( iface() )
		m_iface->stop();
}

void MediaProducer::seek( long time )
{
	if( iface() )
		m_iface->seek( time );
}

bool MediaProducer::aboutToDeleteIface()
{
	kdDebug( 600 ) << k_funcinfo << endl;
	if( m_iface )
	{
		d->state = m_iface->state();
		d->currentTime = m_iface->currentTime();
		d->tickInterval = m_iface->tickInterval();
	}
	return true;
}

void MediaProducer::slotDeleteIface()
{
	kdDebug( 600 ) << k_funcinfo << endl;
	if( aboutToDeleteIface() )
	{
		delete m_iface;
		ifaceDeleted();
	}
}

void MediaProducer::ifaceDeleted()
{
	kdDebug( 600 ) << k_funcinfo << endl;
	m_iface = 0;
}

void MediaProducer::slotCreateIface()
{
	kdDebug( 600 ) << k_funcinfo << endl;
	if( d->state == KDEM2M::ErrorState )
		return;
	if( !m_iface )
		m_iface = createIface();
	setupIface();
}

void MediaProducer::setupIface()
{
	kdDebug( 600 ) << k_funcinfo << endl;
	if( !m_iface )
		return;

	connect( m_iface->qobject(), SIGNAL( stateChanged( KDEM2M::State, KDEM2M::State ) ), SIGNAL( stateChanged( KDEM2M::State, KDEM2M::State ) ) );
	connect( m_iface->qobject(), SIGNAL( tick( long ) ), SIGNAL( tick( long ) ) );

	// set up attributes
	m_iface->setTickInterval( d->tickInterval );

	// FIXME: seeking probably only works in PlayingState
	if( d->currentTime > 0 )
		m_iface->seek( d->currentTime );

	foreach( AudioPath* a, d->audioPaths )
		m_iface->addAudioPath( a->iface() );
	foreach( VideoPath* v, d->videoPaths )
		m_iface->addVideoPath( v->iface() );

	switch( d->state )
	{
		case LoadingState:
		case StoppedState:
		case ErrorState:
			break;
		case PlayingState:
		case BufferingState:
			QTimer::singleShot( 0, this, SLOT( play() ) );
			break;
		case PausedState:
			// FIXME: Going from StoppedState or even LoadingState into
			// PausedState is not going to work.
			QTimer::singleShot( 0, this, SLOT( pause() ) );
			break;
	}
	d->state = m_iface->state();
}

Ifaces::MediaProducer* MediaProducer::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace KDEM2M

#include "mediaproducer.moc"

// vim: sw=4 ts=4 tw=80 noet
