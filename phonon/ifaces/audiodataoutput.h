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
#ifndef Phonon_IFACES_AUDIODATAOUTPUT_H
#define Phonon_IFACES_AUDIODATAOUTPUT_H

#include "abstractaudiooutput.h"
#include "../audiodataoutput.h"

template<class T> class QVector;

namespace Phonon
{
	class IntDataConsumer;
	class FloatDataConsumer;
namespace Ifaces
{
	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class AudioDataOutput : virtual public AbstractAudioOutput
	{
		public:
			virtual Phonon::AudioDataOutput::Format format() const = 0;
			virtual int dataSize() const = 0;
			virtual int sampleRate() const = 0;
			virtual void setFormat( Phonon::AudioDataOutput::Format format ) = 0;
			virtual void setDataSize( int size ) = 0;

		protected: //signals
			virtual void dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& data ) = 0;
			virtual void dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<float> >& data ) = 0;
			virtual void endOfMedia( int remainingSamples ) = 0;
	};
}} //namespace Phonon::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_IFACES_AUDIODATAOUTPUT_H
