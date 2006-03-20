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
#include "audiodataoutput.h"
#include "audiodataoutput_p.h"
#include "ifaces/audiodataoutput.h"
#include "factory.h"

namespace Phonon
{

PHONON_HEIR_IMPL( AudioDataOutput, AbstractAudioOutput )

AudioDataOutput::Format AudioDataOutput::format() const
{
	K_D( const AudioDataOutput );
	return d->iface() ? d->iface()->format() : d->format;
}

int AudioDataOutput::dataSize() const
{
	K_D( const AudioDataOutput );
	return d->iface() ? d->iface()->dataSize() : d->dataSize;
}

int AudioDataOutput::sampleRate() const
{
	K_D( const AudioDataOutput );
	return d->iface() ? d->iface()->sampleRate() : -1;
}

void AudioDataOutput::setFormat( AudioDataOutput::Format newformat )
{
	K_D( AudioDataOutput );
	if( iface() )
		d->iface()->setFormat( newformat );
	else
		d->format = newformat;
}

void AudioDataOutput::setDataSize( int size )
{
	K_D( AudioDataOutput );
	if( iface() )
		d->iface()->setDataSize( size );
	else
		d->dataSize = size;
}

bool AudioDataOutputPrivate::aboutToDeleteIface()
{
	Q_ASSERT( iface() );
	format = iface()->format();
	dataSize = iface()->dataSize();

	return AbstractAudioOutputPrivate::aboutToDeleteIface();
}

void AudioDataOutput::setupIface()
{
	K_D( AudioDataOutput );
	Q_ASSERT( d->iface() );
	AbstractAudioOutput::setupIface();

	// set up attributes
	d->iface()->setFormat( d->format );
	d->iface()->setDataSize( d->dataSize );
	connect( d->iface()->qobject(),
			SIGNAL( dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& ) ),
			SIGNAL( dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& ) ) );
	connect( d->iface()->qobject(),
			SIGNAL( dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<float> >& ) ),
			SIGNAL( dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<float> >& ) ) );
	connect( d->iface()->qobject(), SIGNAL( endOfMedia( int ) ), SIGNAL( endOfMedia( int ) ) );
}

} //namespace Phonon

#include "audiodataoutput.moc"

// vim: sw=4 ts=4 tw=80 noet
