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
#include "audiofftoutput.h"
#include "audiofftoutput_p.h"
#include "../ifaces/audiodataoutput.h"
#include "../factory.h"

#include <QVector>

namespace Phonon
{
PHONON_HEIR_IMPL( AudioFftOutput, AbstractAudioOutput )

QVector<float> AudioFftOutput::fourierTransformedData() const
{
	K_D( const AudioFftOutput );
	return d->iface() ? d->iface()->fourierTransformedData() : QVector<float>();
}

int AudioFftOutput::bandwidth() const
{
	K_D( const AudioFftOutput );
	return d->iface() ? d->iface()->bandwidth() : d->bandwidth;
}

int AudioFftOutput::setBandwidth( int newBandwidth )
{
	K_D( AudioFftOutput );
	if( d->iface() )
		d->bandwidth = d->iface()->setBandwidth( newBandwidth );
	else
		d->bandwidth = newBandwidth;
	return d->bandwidth;
}

int AudioFftOutput::rate() const
{
	K_D( const AudioFftOutput );
	return d->iface() ? d->iface()->rate() : d->rate;
}

void AudioFftOutput::setRate( int newRate )
{
	K_D( AudioFftOutput );
	if( d->iface() )
		d->rate = d->iface()->setRate( newRate );
	else
		d->rate = newRate;
}

bool AudioFftOutputPrivate::aboutToDeleteIface()
{
	if( iface() )
	{
		bandwidth = iface()->bandwidth();
		rate = iface()->rate();
	}
	return AbstractAudioOutputPrivate::aboutToDeleteIface();
}

void AudioFftOutput::setupIface()
{
	K_D( AudioFftOutput );
	Q_ASSERT( d->iface() );
	AbstractAudioOutput::setupIface();

	connect( d->iface()->qobject(), SIGNAL( fourierTransformedData( const QVector<float>& ) ),
			SIGNAL( fourierTransformedData( const QVector<float>& ) ) );
	// set up attributes
	d->bandwidth = d->iface()->setBandwidth( d->bandwidth );
	d->iface()->setRate( d->rate );
}

} //namespace Phonon

#include "audiofftoutput.moc"

// vim: sw=4 ts=4 tw=80 noet
