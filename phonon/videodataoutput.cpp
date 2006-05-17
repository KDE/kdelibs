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
#include "videodataoutput.h"
#include "videodataoutput_p.h"
#include "ifaces/videodataoutput.h"
#include "factory.h"
#include "ifaces/backend.h"
#include <QSize>

namespace Phonon
{

VideoDataOutput::VideoDataOutput( QObject* parent )
	: QObject( parent )
	, AbstractVideoOutput( *new VideoDataOutputPrivate )
{
	K_D( VideoDataOutput );
	d->createIface();
}

VideoDataOutput::VideoDataOutput( VideoDataOutputPrivate& dd, QObject* parent )
	: QObject( parent )
	, AbstractVideoOutput( dd )
{
}

Ifaces::VideoDataOutput* VideoDataOutput::iface()
{
	K_D( VideoDataOutput );
	if( !d->iface() )
		d->createIface();
	return d->iface();
}

void VideoDataOutputPrivate::createIface()
{
	if( iface_ptr )
		return;
	K_Q( VideoDataOutput );
	Ifaces::VideoDataOutput* iface = Factory::self()->createVideoDataOutput( q );
	if( iface )
	{
		setIface( iface );
		q->setupIface();
	}
}

quint32 VideoDataOutput::format() const
{
	K_D( const VideoDataOutput );
	return d->iface() ? d->iface()->format() : d->format;
}

//X int VideoDataOutput::displayLatency() const
//X {
//X 	K_D( const VideoDataOutput );
//X 	return d->iface() ? d->iface()->displayLatency() : d->displayLatency;
//X }

bool VideoDataOutput::formatSupported( quint32 fourcc )
{
	const Ifaces::Backend* backend = Factory::self()->backend();
	if( backend )
		return backend->supportsFourcc( fourcc );
	return false;
}

int VideoDataOutput::frameRate() const
{
	K_D( const VideoDataOutput );
	return d->iface() ? d->iface()->frameRate() : d->frameRate;
}

void VideoDataOutput::setFrameRate( int newRate )
{
	K_D( VideoDataOutput );
	if( iface() )
		d->iface()->setFrameRate( newRate );
	else
		d->frameRate = newRate;
}

void VideoDataOutput::setFormat( quint32 newformat )
{
	K_D( VideoDataOutput );
	if( iface() )
		d->iface()->setFormat( newformat );
	else
		d->format = newformat;
}

QSize VideoDataOutput::frameSize() const
{
	K_D( const VideoDataOutput );
	return d->iface() ? d->iface()->frameSize() : d->frameSize;
}

void VideoDataOutput::setFrameSize( const QSize& size, Qt::AspectRatioMode aspectRatioMode )
{
	K_D( VideoDataOutput );
	d->frameSize = size;
	d->frameAspectRatioMode = aspectRatioMode;

	if( iface() )
	{
		QSize newsize = d->iface()->naturalFrameSize();
		newsize.scale( size, aspectRatioMode );
		d->iface()->setFrameSize( newsize );
	}
}

void VideoDataOutput::setFrameSize( int width, int height, Qt::AspectRatioMode aspectRatioMode )
{
	setFrameSize( QSize( width, height ), aspectRatioMode );
}

//X void VideoDataOutput::setDisplayLatency( int milliseconds )
//X {
//X 	K_D( VideoDataOutput );
//X 	if( iface() )
//X 		d->iface()->setDisplayLatency( milliseconds );
//X 	else
//X 		d->displayLatency = milliseconds;
//X }

bool VideoDataOutputPrivate::aboutToDeleteIface()
{
	Q_ASSERT( iface() );
	format = iface()->format();
	//displayLatency = iface()->displayLatency();

	return AbstractVideoOutputPrivate::aboutToDeleteIface();
}

void VideoDataOutput::setupIface()
{
	K_D( VideoDataOutput );
	Q_ASSERT( d->iface() );
	AbstractVideoOutput::setupIface();

	// set up attributes
	d->iface()->setFormat( d->format );
	//d->iface()->setDisplayLatency( d->displayLatency );
	connect( d->iface()->qobject(), SIGNAL( frameReady( const Phonon::VideoFrame& ) ),
			SIGNAL( frameReady( const Phonon::VideoFrame& ) ) );
	connect( d->iface()->qobject(), SIGNAL( endOfMedia() ), SIGNAL( endOfMedia() ) );
}

} //namespace Phonon

#include "videodataoutput.moc"

// vim: sw=4 ts=4 tw=80 noet
