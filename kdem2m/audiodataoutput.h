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
#ifndef KDEM2M_AUDIODATAOUTPUT_H
#define KDEM2M_AUDIODATAOUTPUT_H

#include "audiooutputbase.h"

template<class T> class QVector;

namespace KDEM2M
{
	namespace Ifaces
	{
		class AudioDataOutput;
	}

	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class AudioDataOutput : public AudioOutputBase
	{
		Q_OBJECT
		public:
			AudioDataOutput( QObject* parent = 0 );
			~AudioDataOutput();

			// Operations:
			void readBuffer( QVector<float>& buffer );
			void readBuffer( QVector<int>& buffer );

			// Attributes Getters:
			int availableSamples() const;

		public slots:
			// Attributes Setters:

		protected:
			AudioDataOutput( Ifaces::AudioOutputBase* iface, QObject* parent );
			virtual bool aboutToDeleteIface();
			virtual Ifaces::AudioOutputBase* createIface( bool initialized = true );
			virtual void setupIface();

		private:
			Ifaces::AudioDataOutput* iface();
			Ifaces::AudioDataOutput* m_iface;
			class Private;
			Private* d;
	};
} //namespace KDEM2M

// vim: sw=4 ts=4 tw=80 noet
#endif // KDEM2M_AUDIODATAOUTPUT_H
