/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_FAKE_AUDIODATAOUTPUT_H
#define Phonon_FAKE_AUDIODATAOUTPUT_H

#include "abstractaudiooutput.h"
#include <phonon/audiodataoutput.h>
#include <QVector>

namespace Phonon
{
namespace Fake
{
	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class AudioDataOutput : public AbstractAudioOutput
	{
		Q_OBJECT
		public:
			AudioDataOutput( QObject* parent );
			~AudioDataOutput();

		public slots:
			Phonon::AudioDataOutput::Format format() const;
			int dataSize() const;
			int sampleRate() const;
			void setFormat( Phonon::AudioDataOutput::Format format );
			void setDataSize( int size );

		public:
			// Fake specific:
			virtual void processBuffer( const QVector<float>& buffer );

		signals:
			void dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& data );
			void dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<float> >& data );
			void endOfMedia( int remainingSamples );

		private:
			void convertAndEmit( const QVector<float>& buffer );

			Phonon::AudioDataOutput::Format m_format;
			int m_dataSize;
			QVector<float> m_pendingData;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_AUDIODATAOUTPUT_H
