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
#ifndef Kdem2m_SOUNDCARDCAPTURE_H
#define Kdem2m_SOUNDCARDCAPTURE_H

#include "mediaproducer.h"

class QString;
class QStringList;

namespace Kdem2m
{
	namespace Ifaces
	{
		class SoundcardCapture;
	}

	class SoundcardCapture : public MediaProducer
	{
		Q_OBJECT
		public:
			SoundcardCapture( const QString& type, QObject* parent = 0 );
			~SoundcardCapture();

			// Attributes Getters:
			const QString& type() const;

		protected:
			SoundcardCapture( Ifaces::MediaProducer* iface, const QString& type, QObject* parent );
			virtual bool aboutToDeleteIface();
			virtual Ifaces::MediaProducer* createIface( bool initialized = true );
			virtual void setupIface();

		private:
			Ifaces::SoundcardCapture* iface();
			Ifaces::SoundcardCapture* m_iface;
			class Private;
			Private* d;
	};
} //namespace Kdem2m

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_SOUNDCARDCAPTURE_H
