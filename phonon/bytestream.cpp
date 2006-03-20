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

#include "bytestream.h"
#include "bytestream_p.h"
#include "ifaces/bytestream.h"
#include "factory.h"

namespace Phonon
{
PHONON_HEIR_IMPL( ByteStream, AbstractMediaProducer )

long ByteStream::totalTime() const
{
	K_D( const ByteStream );
	return d->iface() ? d->iface()->totalTime() : -1;
}

long ByteStream::remainingTime() const
{
	K_D( const ByteStream );
	return d->iface() ? d->iface()->remainingTime() : -1;
}

long ByteStream::aboutToFinishTime() const
{
	K_D( const ByteStream );
	return d->iface() ? d->iface()->aboutToFinishTime() : d->aboutToFinishTime;
}

long ByteStream::streamSize() const
{
	K_D( const ByteStream );
	return d->iface() ? d->iface()->streamSize() : d->streamSize;
}

bool ByteStream::streamSeekable() const
{
	K_D( const ByteStream );
	return d->iface() ? d->iface()->streamSeekable() : d->streamSeekable;
}

void ByteStream::setStreamSeekable( bool seekable )
{
	K_D( ByteStream );
	if( d->iface() )
		d->iface()->setStreamSeekable( seekable );
	else
		d->streamSeekable = seekable;
}

void ByteStream::writeData( const QByteArray& data )
{
	K_D( ByteStream );
	if( iface() )
		d->iface()->writeData( data );
}

void ByteStream::setStreamSize( long streamSize )
{
	K_D( ByteStream );
	if( d->iface() )
		d->iface()->setStreamSize( streamSize );
	else
		d->streamSize = streamSize;
}

void ByteStream::endOfData()
{
	K_D( ByteStream );
	if( iface() )
		d->iface()->endOfData();
}

void ByteStream::setAboutToFinishTime( long newAboutToFinishTime )
{
	K_D( ByteStream );
	if( d->iface() )
		d->iface()->setAboutToFinishTime( newAboutToFinishTime );
	else
		d->aboutToFinishTime = newAboutToFinishTime;
}

bool ByteStreamPrivate::aboutToDeleteIface()
{
	if( iface() )
	{
		aboutToFinishTime = iface()->aboutToFinishTime();
		streamSize = iface()->streamSize();
		streamSeekable = iface()->streamSeekable();
	}
	return AbstractMediaProducerPrivate::aboutToDeleteIface();
}

void ByteStream::setupIface()
{
	K_D( ByteStream );
	Q_ASSERT( d->iface() );
	AbstractMediaProducer::setupIface();

	connect( d->iface()->qobject(), SIGNAL( finished() ), SIGNAL( finished() ) );
	connect( d->iface()->qobject(), SIGNAL( aboutToFinish( long ) ), SIGNAL( aboutToFinish( long ) ) );
	connect( d->iface()->qobject(), SIGNAL( length( long ) ), SIGNAL( length( long ) ) );
	connect( d->iface()->qobject(), SIGNAL( needData() ), SIGNAL( needData() ) );
	connect( d->iface()->qobject(), SIGNAL( enoughData() ), SIGNAL( enoughData() ) );
	connect( d->iface()->qobject(), SIGNAL( seekStream( long ) ), SIGNAL( seekStream( long ) ) );

	d->iface()->setAboutToFinishTime( d->aboutToFinishTime );
}

} //namespace Phonon

#include "bytestream.moc"

// vim: sw=4 ts=4 tw=80 noet
