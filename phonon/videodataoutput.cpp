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

VideoDataOutput::Format VideoDataOutput::format() const
{
	K_D( const VideoDataOutput );
	return d->iface() ? d->iface()->format() : d->format;
}

int VideoDataOutput::frameRate() const
{
	K_D( const VideoDataOutput );
	return d->iface() ? d->iface()->frameRate() : -1;
}

void VideoDataOutput::setFormat( Format newformat )
{
	K_D( VideoDataOutput );
	if( iface() )
		d->iface()->setFormat( newformat );
	else
		d->format = newformat;
}

bool VideoDataOutputPrivate::aboutToDeleteIface()
{
	Q_ASSERT( iface() );
	format = iface()->format();

	return AbstractVideoOutputPrivate::aboutToDeleteIface();
}

void VideoDataOutput::setupIface()
{
	K_D( VideoDataOutput );
	Q_ASSERT( d->iface() );
	AbstractVideoOutput::setupIface();

	// set up attributes
	d->iface()->setFormat( d->format );
	connect( d->iface()->qobject(), SIGNAL( frameReady( const Phonon::VideoFrame& ) ),
			SIGNAL( frameReady( const Phonon::VideoFrame& ) ) );
	connect( d->iface()->qobject(), SIGNAL( endOfMedia() ), SIGNAL( endOfMedia() ) );
}

} //namespace Phonon

#include "videodataoutput.moc"

// vim: sw=4 ts=4 tw=80 noet
